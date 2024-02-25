/*******************************************************************************************
File: mqtt.c

MQTT handling for Skytrac services

(c) Copyright 2021, SKYTRAC Systems Inc. All rights reserved.
This source code contains confidential, trade secret material. Any attempt or participation
in deciphering, decoding, reverse engineering or in any way altering the source code is
strictly prohibited, unless the prior written consent of SKYTRAC Systems is obtained.
 ********************************************************************************************/

#include <errno.h>

#include <systemd/sd-event.h>

#include <mosquitto.h>
#include <mqtt_protocol.h>

#define LOG_COMPONENT LOG_SKYMQTT
#include "log.h"
#include "queue.h"
#include "skytrac/mqtt.h"
#include "skytrac/service.h"
#include "skytrac/telemetry.h"
#include "skytrac/utils.h"

/**************************************************************************
 *                              Constants
 **************************************************************************/

#define MQTT_DEFAULT_QOS (2)
#define MQTT_CONNECT_KEEPALIVE (60)

#define MQTT_TRIGGER_PERIOD_US (1000000)
#define MQTT_LOG_LEVEL (MOSQ_LOG_NOTICE)

/**************************************************************************
 *                              Global Variables
 **************************************************************************/

static sd_event_source * g_mqtt_timer_source  = NULL;
static sd_event_source * g_mqtt_socket_source = NULL;
static struct mosquitto * g_mqtt_mosq         = NULL;
static bool g_mqtt_connected                  = false;

static telemetry_random_variable_t g_mqtt_timer_handling_time_usec  = {};
static telemetry_random_variable_t g_mqtt_socket_handling_time_usec = {};

/**************************************************************************
 *                              Private Functions
 **************************************************************************/

static void mqtt_failure()
{
    if (g_mqtt_connected)
    {
        g_mqtt_connected = false;
        mosquitto_disconnect(g_mqtt_mosq);
    }

    // Exit normally so that systemd can restart us
    service_exit(EXIT_SUCCESS);
}

static void mqtt_error(const char * msg, int errcode)
{
    // TODO: Does mosquitto_strerror handle MOSQ_ERR_ERRNO correctly by internally
    // calling strerror?
    // TODO: If MOSQ_ERR_ERRNO, properly log the errno using sky_log_perror()
    const char * errormsg = mosquitto_strerror(errcode);
    sky_log_start_msg(LOG_ERR, 2, "%s: %s", msg, errormsg);
    sky_log_add_field("MQTT_ERROR_CODE", "%d", errcode);
    sky_log_add_field("MQTT_ERROR_STRING", "%s", errormsg);
    sky_log_send_msg();
}

static void mqtt_check_result(int r)
{
    switch (r)
    {
    case MOSQ_ERR_SUCCESS:
        // Yay!
        break;
    case MOSQ_ERR_NO_CONN:
    case MOSQ_ERR_CONN_LOST: {
        sky_log(LOG_WARNING, "Connection with broker lost");
        mqtt_failure();
    }
    break;
    case MOSQ_ERR_PROTOCOL: {
        // TODO: Is this recoverable or fatal?
        sky_logf(LOG_ERR, "MQTT protocol error");
        mqtt_failure();
    }
    break;
    case MOSQ_ERR_ERRNO:
        sky_log_perror(LOG_ERR, "System error during mosquitto call");
        _fallthrough_;
    case MOSQ_ERR_INVAL:
    case MOSQ_ERR_NOMEM:
    default: {
        mqtt_error("Fatal error ", r);
        service_terminate(EXIT_FAILURE);
    }
    break;
    };
}

static int mqtt_socket_handler(_unused_ sd_event_source * es, _unused_ int fd, _unused_ uint32_t revents, void * userdata)
{
    uint64_t start          = telemetry_timer_start();
    struct mosquitto * mosq = (struct mosquitto *) userdata;
    int r                   = mosquitto_loop_read(mosq, 1);
    mqtt_check_result(r);

    if (mosquitto_want_write(mosq))
    {
        r = mosquitto_loop_write(mosq, 1);
        mqtt_check_result(r);
    }

    uint64_t elapsed = telemetry_timer_stop(start);
    telemetry_rv_sample(elapsed, &g_mqtt_socket_handling_time_usec);
    return 0;
}

static int mqtt_timer_handler(sd_event_source * es, uint64_t usec, void * userdata)
{
    struct mosquitto * mosq = (struct mosquitto *) userdata;

    if (g_mqtt_connected)
    {
        uint64_t start = telemetry_timer_start();

        int r = mosquitto_loop_misc(mosq);
        mqtt_check_result(r);

        uint64_t elapsed = telemetry_timer_stop(start);
        telemetry_rv_sample(elapsed, &g_mqtt_timer_handling_time_usec);
    }

    int r = sd_event_source_set_time(es, usec + MQTT_TRIGGER_PERIOD_US);
    check_return(r, "update timer event for MQTT");

    return 0;
};

static void mqtt_on_connect(_unused_ struct mosquitto * mosq, _unused_ void * obj, int reason_code)
{
    switch (reason_code)
    {
    case CONNACK_ACCEPTED: {
        sky_log(LOG_NOTICE, "MQTT broker responded.");
        g_mqtt_connected = true;
    }
    break;
    case CONNACK_REFUSED_SERVER_UNAVAILABLE:
        sky_log(LOG_WARNING, "MQTT broker failed to respond.");
        mqtt_failure();
        break;
    default:
        sky_logf(LOG_ERR, "Failed to connect: %s [%d]", mosquitto_connack_string(reason_code), reason_code);
        service_terminate(EXIT_FAILURE);
        break;
    }
}

static void mqtt_on_disconnect(_unused_ struct mosquitto * mosq, _unused_ void * obj, int rc)
{
    if (rc < 0)
    {
        sky_logf(LOG_ERR, "MQTT disconnected: %s (%d)", mosquitto_connack_string(rc), rc);
        mqtt_failure();
    }
}

static void mqtt_on_subscribe(_unused_ struct mosquitto * mosq, _unused_ void * obj, int mid, int qos_count,
                              const int * granted_qos)
{
    sky_logf(LOG_DEBUG, "On subscribed: mid=%d, qos_count=%d", mid, qos_count);

    for (int i = 0; i < qos_count; ++i)
    {
        if (granted_qos[i] < 2)
        {
            sky_logf(LOG_ERR, "Failed to subscribe with QOS %d", granted_qos[i]);
            mqtt_failure();
            break;
        }
    }
}

static void mqtt_on_unsubscribe(_unused_ struct mosquitto * mosq, _unused_ void * obj, int mid)
{
    sky_logf(LOG_DEBUG, "Unsubscribed: mid=%d", mid);
}

static void mqtt_on_log(_unused_ struct mosquitto * mosq, _unused_ void * obj, int level, const char * msg)
{
    if (level <= MQTT_LOG_LEVEL)
    {
        log_level_t logLevel;
        switch (level)
        {
        case MOSQ_LOG_INFO:
            logLevel = LOG_INFO;
            break;
        case MOSQ_LOG_NOTICE:
            logLevel = LOG_NOTICE;
            break;
        case MOSQ_LOG_WARNING:
            logLevel = LOG_WARNING;
            break;
        case MOSQ_LOG_ERR:
            logLevel = LOG_ERR;
            break;
        case MOSQ_LOG_DEBUG:
        case MOSQ_LOG_SUBSCRIBE:
        case MOSQ_LOG_UNSUBSCRIBE:
        case MOSQ_LOG_WEBSOCKETS:
            logLevel = LOG_DEBUG;
            break;
        default:
            logLevel = LOG_EMERG;
            break;
        }

        sky_log(logLevel, msg);
    }
}

static void mqtt_on_message(_unused_ struct mosquitto * mosq, _unused_ void * obj, const struct mosquitto_message * msg)
{
    sky_log_start_msg(LOG_DEBUG, 6, "Message of %d bytes received for topic '%s'", msg->payloadlen, msg->topic);
    sky_log_add_field("MQTT_TOPIC", "%s", msg->topic);
    sky_log_add_field("MQTT_PAYLOAD_LEN", "%d", msg->payloadlen);
    sky_log_add_field("MQTT_PAYLOAD", "%.*s", msg->payloadlen, (char *) msg->payload);
    sky_log_add_field("MQTT_QOS", "%d", msg->qos);
    sky_log_add_field("MQTT_RETAIN", "%d", msg->retain);
    sky_log_add_field("MQTT_MID", "%d", msg->mid);
    sky_log_send_msg();

    int r = skytrac_post_external_message(msg->topic, msg->payloadlen, msg->payload);
    if (r != 0)
    {
        // TODO: what do with the error result?
        sky_logf(LOG_ERR, "Could not queue MQTT message!");
    }
}

static int publish(const char * topic, unsigned int pubDataSize, void const * pubData, bool retained)
{
    // For MID argument we pass NULL, but could use it in future to provide tracking of messages
    int rc = mosquitto_publish(g_mqtt_mosq, NULL, topic, pubDataSize, pubData, MQTT_DEFAULT_QOS, retained);
    if (rc != MOSQ_ERR_SUCCESS)
    { // All possible non-success return codes indicate a non-recoverable error
        mqtt_error("Failed to publish", rc);
        return rc;
    }
    return 0;
}

/**************************************************************************
 *                              Internal Functions
 **************************************************************************/

int mqtt_init(sd_event * ev)
{
    mosquitto_lib_init();

    telemetry_register("MQTT timer handling", "usec", &g_mqtt_timer_handling_time_usec);
    telemetry_register("MQTT socket handling", "usec", &g_mqtt_socket_handling_time_usec);

    g_mqtt_mosq = mosquitto_new(NULL, true, NULL);
    if (g_mqtt_mosq == NULL)
    {
        sky_log(LOG_ERR, "Failed to create mosquitto object");
        return -1;
    }

    mosquitto_connect_callback_set(g_mqtt_mosq, mqtt_on_connect);
    mosquitto_disconnect_callback_set(g_mqtt_mosq, mqtt_on_disconnect);
    mosquitto_message_callback_set(g_mqtt_mosq, mqtt_on_message);
    mosquitto_subscribe_callback_set(g_mqtt_mosq, mqtt_on_subscribe);
    mosquitto_unsubscribe_callback_set(g_mqtt_mosq, mqtt_on_unsubscribe);
    mosquitto_log_callback_set(g_mqtt_mosq, mqtt_on_log);

    sky_logf(LOG_NOTICE, "Connecting to mosquitto address: %s port: %d", MQTT_BROKER_IP, MQTT_BROKER_PORT);

    int r = mosquitto_connect(g_mqtt_mosq, MQTT_BROKER_IP, MQTT_BROKER_PORT, MQTT_CONNECT_KEEPALIVE);
    if (r != MOSQ_ERR_SUCCESS)
    {
        mqtt_error("Connecting", r);
        return -1;
    }

    int fd = mosquitto_socket(g_mqtt_mosq);
    if (fd < 0)
    {
        sky_log_perror(LOG_ERR, "Retrieving MQTT socket");
        return -1;
    }

    r = sd_event_add_io(ev, &g_mqtt_socket_source, fd, EPOLLIN, &mqtt_socket_handler, g_mqtt_mosq);
    check_return(r, "Create IO event for MQTT socket");

    uint64_t usec;
    r = sd_event_now(ev, CLOCK_MONOTONIC, &usec);
    check_return(r, "Get current monotonic time for MQTT");

    r = sd_event_add_time(ev, &g_mqtt_timer_source, CLOCK_MONOTONIC, usec + MQTT_TRIGGER_PERIOD_US, 0, &mqtt_timer_handler,
                          g_mqtt_mosq);
    check_return(r, "Create timer event for MQTT");

    r = sd_event_source_set_enabled(g_mqtt_timer_source, SD_EVENT_ON);
    check_return(r, "Set MQTT timer to repeat");

    return 0;
};

void mqtt_cleanup()
{
    g_mqtt_timer_source  = sd_event_source_disable_unref(g_mqtt_timer_source);
    g_mqtt_socket_source = sd_event_source_disable_unref(g_mqtt_socket_source);

    if (g_mqtt_mosq)
    {
        mosquitto_destroy(g_mqtt_mosq);
    }

    mosquitto_lib_cleanup();
};

/**************************************************************************
 *                              Public Functions
 **************************************************************************/
int mqtt_subscribe(char * const * const topics, size_t count)
{
    assert(g_mqtt_mosq != NULL);
    assert(count > 0);

    int mid;
    for (size_t i = 0; i < count; ++i)
    {
        sky_logf(LOG_INFO, "Subscribing to '%s'", topics[i]);
    }

    int r = mosquitto_subscribe_multiple(g_mqtt_mosq, &mid, count, topics, MQTT_DEFAULT_QOS, 0, NULL);

    if (r != MOSQ_ERR_SUCCESS)
    {
        mqtt_error("subscribing", r);
        mqtt_failure();
    }

    sky_logf(LOG_NOTICE, "Subscribed %zu subs with mid %d", count, mid);

    return mid;
}

int mqtt_unsubscribe(char * const * const topics, size_t count)
{
    assert(g_mqtt_mosq != NULL);
    assert(count > 0);

    sky_logf(LOG_NOTICE, "Unsubscribing %zu subs", count);
    int mid;
    for (size_t i = 0; i < count; ++i)
    {
        sky_logf(LOG_INFO, "Unsubscribing to '%s'", topics[i]);
    }

    int r = mosquitto_unsubscribe_multiple(g_mqtt_mosq, &mid, count, topics, NULL);

    if (r != MOSQ_ERR_SUCCESS)
    {
        mqtt_error("unsubscribing", r);
        mqtt_failure();
    }

    return mid;
}

/*****************************************************************************
 ** Publish a payload to the given topic
 **
 ** Input parameters:
 **  handle           - opaque handle representing the client
 **  topic            - the topic on which this message will be published
 **  pubDataSize      - the length of our payload in bytes
 **  pubData          - the actual payload
 **
 ** Return value:
 **  int - errorcode, with 0 if successfull
 **
 ******************************************************************************/
int mqtt_publish(const char * topic, unsigned int pubDataSize, void const * pubData)
{
    return publish(topic, pubDataSize, pubData, false);
}

/*****************************************************************************
 ** Publish a payload to the given topic, but have the broker retain a copy
 **
 ** Input parameters:
 **  handle           - opaque handle representing the client
 **  topic            - the topic on which this message will be published
 **  pubDataSize      - the length of our payload in bytes
 **  pubData          - the actual payload
 **
 ** Return value:
 **  int - errorcode, with 0 if successfull
 **
 ******************************************************************************/
int mqtt_publish_retained(const char * topic, unsigned int pubDataSize, void const * pubData)
{
    return publish(topic, pubDataSize, pubData, true);
}

/*****************************************************************************
 ** Publish a formatted string to the given topic
 **
 ** Input parameters:
 **  handle           - opaque handle representing the client
 **  topic            - the topic on which this message will be published
 **  fmt              - printf-style formatted string
 **
 ** Return value:
 **  int - errorcode, with 0 if successfull
 **
 ******************************************************************************/
int mqtt_publish_formatted(const char * topic, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    int size = vsnprintf(NULL, 0, format, ap);
    va_end(ap);

    if (size < 0)
        return size;

    // Note that size does not include the terminating null
    char * buf = malloc(size + 1);

    if (buf == NULL)
        return -ENOMEM;

    vsnprintf(buf, size + 1, format, ap);
    va_start(ap, format);
    int rc = publish(topic, size, buf, false);
    va_end(ap);
    free(buf);

    return rc;
}

// Note: Temporary function, see header for description
struct mosquitto * mqtt_get_mosquitto_handle(void)
{
    return g_mqtt_mosq;
}
