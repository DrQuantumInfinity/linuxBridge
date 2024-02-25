/*******************************************************************************************
File: skymqtt.c

Mosquitto connection library for the SDL-350

(c) Copyright 2021, SkyTrac Systems Inc. All rights reserved.
This source code contains confidential, trade secret material. Any attempt or participation
in deciphering, decoding, reverse engineering or in any way altering the source code is
strictly prohibited, unless the prior written consent of SkyTrac Systems is obtained.
********************************************************************************************/

#include "skymqtt.h"
#define LOG_COMPONENT (LOG_SKYMQTT)
#include "log.h"

#include <mqtt_protocol.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define DEFAULT_QOS (2)
#define CONNECT_KEEPALIVE (60)

#define LARGE_BUFFER_SIZE (65535U)
#define HOSTNAME_MAX (255)
#define STR_CMP_MATCH (0)
/**************************************************************************
 *                                  Macros
 **************************************************************************/

/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef struct client_subs
{
    skymqtt_subscription_t * subscriptions;
    size_t subscriptionCount;
} client_subs_t;

typedef struct client_state
{
    struct mosquitto * mosq;
} client_state_t;

typedef struct client_context
{
    client_subs_t subs;
    client_state_t state;
    void const * pUserContext;
    skymqtt_msgCallback callback;
} client_context_t;

/**************************************************************************
 *                                  Variables
 **************************************************************************/
static char hostname[HOSTNAME_MAX];

// max two contexts
static client_context_t g_ctx[2];
static int context_count = 0;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
static bool publish(skymqtt_handle_t handle, const char * topic, unsigned int pubDataSize, void const * pubData, bool retained);
static bool subscribe(struct mosquitto * mosq, const char * topic);
// Callback functions
static void skymqtt_OnConnect(struct mosquitto * mosq, void * obj, int reason_code);
static void skymqtt_OnDisconnect(struct mosquitto * mosq, void * obj, int rc);
static void skymqtt_OnMessage(struct mosquitto * mosq, void * obj, const struct mosquitto_message * msg);
static void skymqtt_OnSubscribe(struct mosquitto * mosq, void * obj, int mid, int qos_count, const int * granted_qos);

/**************************************************************************
 *                                  Constructors
 **************************************************************************/
void __attribute__((constructor)) load(void)
{
    gethostname(hostname, sizeof(hostname));
    mosquitto_lib_init();
    sky_logf(LOG_NOTICE, "Skymqtt loaded : %s", hostname);
}

void __attribute__((destructor)) unload(void)
{
    for (int context_index = 0; context_index < context_count; context_index++)
    {
        client_context_t * ctx = &g_ctx[context_index];
        if (ctx->subs.subscriptions)
        {
            free(ctx->subs.subscriptions);
        }
    }

    mosquitto_lib_cleanup();

    sky_logf(LOG_NOTICE, "Skymqtt unloaded");
}
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
skymqtt_handle_t skymqtt_init(skymqtt_subscription_t const * pSubs, int numSubs, skymqtt_msgCallback callback,
                              void const * pUserContext)
{
    int rc;

    assert(context_count < 2);

    client_context_t * ctx = &g_ctx[context_count++];

    ctx->callback     = callback;
    ctx->pUserContext = pUserContext;

    ctx->subs.subscriptionCount = numSubs;
    ctx->subs.subscriptions     = calloc(ctx->subs.subscriptionCount, sizeof(skymqtt_subscription_t));
    memcpy(ctx->subs.subscriptions, pSubs, sizeof(skymqtt_subscription_t) * ctx->subs.subscriptionCount);

    if (!(ctx->state.mosq = mosquitto_new(NULL, true, ctx)))
    {
        sky_logf(LOG_ERR, "mosquitto_new failed");
        return NULL;
    }
    // Configure callbacks. This should be done before connecting ideally.
    mosquitto_connect_callback_set(ctx->state.mosq, skymqtt_OnConnect);
    mosquitto_disconnect_callback_set(ctx->state.mosq, skymqtt_OnDisconnect);
    mosquitto_subscribe_callback_set(ctx->state.mosq, skymqtt_OnSubscribe);
    mosquitto_message_callback_set(ctx->state.mosq, skymqtt_OnMessage);

    sky_logf(LOG_NOTICE, "Connecting to mosquitto address: %s port: %d", MQTT_BROKER_IP, MQTT_BROKER_PORT);

    rc = mosquitto_connect_async(ctx->state.mosq, MQTT_BROKER_IP, MQTT_BROKER_PORT, CONNECT_KEEPALIVE);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        const char * errormsg = mosquitto_strerror(rc);
        sky_logf(LOG_ERR, "mosquitto_connect_async failed: %s", errormsg);
        return NULL;
    }

    rc = mosquitto_loop_start(ctx->state.mosq);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        const char * errormsg = mosquitto_strerror(rc);
        sky_logf(LOG_ERR, "mosquitto_loop_start failed: %s", errormsg);
        return NULL;
    }

    return ctx->state.mosq;
}

void skymqtt_disconnect(skymqtt_handle_t handle)
{
    for (int context_index = 0; context_index < context_count; context_index++)
    {
        client_context_t * ctx = &g_ctx[context_index];

        if (ctx->state.mosq == handle)
        {
            mosquitto_disconnect(ctx->state.mosq);
            mosquitto_destroy(ctx->state.mosq);
        }
    }
}

bool skymqtt_publish(skymqtt_handle_t handle, const char * topic, unsigned int pubDataSize, void const * pubData)
{
    return publish(handle, topic, pubDataSize, pubData, false);
}

bool skymqtt_publish_retained(skymqtt_handle_t handle, const char * topic, unsigned int pubDataSize, void const * pubData)
{
    return publish(handle, topic, pubDataSize, pubData, true);
}

/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static bool publish(skymqtt_handle_t handle, const char * topic, unsigned int pubDataSize, void const * pubData, bool retained)
{
    assert(handle != NULL && topic != NULL && pubDataSize < MQTT_MAX_PAYLOAD);
    int rc;

    // We pass NULL, but could provide tracking
    rc = mosquitto_publish(handle, NULL, topic, pubDataSize, pubData, DEFAULT_QOS, retained);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        const char * errormsg = mosquitto_strerror(rc);
        sky_logf(LOG_ERR, "mosquitto_publish failed: %s", errormsg);
        return false;
    }
    return true;
}

static bool subscribe(struct mosquitto * mosq, const char * pTopic)
{
    int rc = mosquitto_subscribe(mosq, NULL, pTopic, DEFAULT_QOS);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        const char * errormsg = mosquitto_strerror(rc);
        sky_logf(LOG_ERR, "mosquitto_subscribe failed: %s", errormsg);
    }
    return rc == MOSQ_ERR_SUCCESS;
}

/**************************************************************************
 *                                  Callback Functions
 **************************************************************************/
/*****************************************************************************
** Callback called when the client receives a CONNACK message from the broker.
**
** Input parameters:
**  mosq    - the mosquitto instance making the callback.
**  obj     - the library context object provided in <mosquitto_new>
**  rc      - reason code for connection
**
** Return value:
**    Void - acts as pass-through
**
******************************************************************************/
static void skymqtt_OnConnect(struct mosquitto * mosq, void * obj, int reason_code)
{
    client_context_t * ctx = (client_context_t *) obj;
    assert(ctx != NULL);

    sky_logf(LOG_NOTICE, "connection result: %s [%d]", mosquitto_connack_string(reason_code), reason_code);

    switch (reason_code)
    {
    case CONNACK_ACCEPTED: { // mosquitto_connack_string() for MQTT v3.x, mosquitto_reason_string() for MQTT v5.0
        for (size_t i = 0; i < ctx->subs.subscriptionCount; i++)
        {
            if (!subscribe(mosq, ctx->subs.subscriptions[i].topic))
            {
                break;
            }
        }
    }
    break;
    default: // For any non-recoverable error, disconnect so there is no retrying
        break;
    }
}

/*****************************************************************************
** Callback called when the client is disconnected. A reason code of 0
** means it was due to a call to mosquitto_disconnect.
**
** Input parameters:
**  mosq    - the mosquitto instance making the callback.
**  obj     - the library context object provided in <mosquitto_new>
**  rc      - reason code for disconnection
**
** Return value:
**    Void - acts as pass-through
**
******************************************************************************/
static void skymqtt_OnDisconnect(_unused_ struct mosquitto * mosq, _unused_ void * obj, int rc)
{
    sky_logf(LOG_INFO, "disconnected: %s [%d]", mosquitto_connack_string(rc), rc);
}

/*****************************************************************************
** Callback called when the client receives a subscribed message. Searches
** the list of subscriptions for a matching topic, then calls the associated
** callback.
**
** Input parameters:
**  mosq    - the mosquitto instance making the callback.
**  obj     - the library context object provided in <mosquitto_new>
**  message - the message data. This variable and associated memory will be
**            freed by the library after the callback completes. The client
**            should make copies of any of the data it requires.
**
** Return value:
**    Void - acts as pass-through
**
******************************************************************************/
static void skymqtt_OnMessage(_unused_ struct mosquitto * mosq, void * obj, const struct mosquitto_message * msg)
{
    client_context_t * ctx = (client_context_t *) obj;
    assert(ctx != NULL);

    (*ctx->callback)((void *) ctx->pUserContext, msg);
}

/*****************************************************************************
** Callback called when the broker sends a SUBACK in response to a SUBSCRIBE.
**
** Input parameters:
**  mosq        - the mosquitto instance making the callback.
**  obj         - the library context object provided in <mosquitto_new>
**  mid         - internal message id
**  qos_count   - number of subscriptions registered
**  granted_qos - array of QOS levels for each subscription registered
**
** Return value:
**    Void - acts as pass-through
**
******************************************************************************/
static void skymqtt_OnSubscribe(struct mosquitto * mosq, _unused_ void * obj, int mid, int qos_count, const int * granted_qos)
{
    for (int i = 0; i < qos_count; i++) // qos_count is always 1 but what the heck
    {
        sky_logf(LOG_NOTICE, "subscribed, MID: %d, QOS: %d", mid, granted_qos[i]);
        if (granted_qos[i] != DEFAULT_QOS)
        {
            mosquitto_disconnect(mosq);
            sky_logf(LOG_ERR, "Subscription QOS was rejected");
        }
    }
}

bool MidIsValid(MID mid)
{
    return (mid > MID_DUMMY) && (mid < MID_COUNT);
}
MID MidGetMidFromTopic(char const * pTopic)
{
    MID mid;
    for (mid = (MID) 1; mid < MID_COUNT; mid++)
    {
        if (strcmp(eStrMqttMid_MID[mid], pTopic) == STR_CMP_MATCH)
        {
            break;
        }
    }
    return mid;
}
char const * MidGetTopicFromMid(MID mid)
{
    char const * pTopic = NULL;
    if (MidIsValid(mid))
    {
        pTopic = eStrMqttMid_MID[mid];
    }
    return pTopic;
}

static void MidPostEmptyPrivate(skymqtt_handle_t skyMqttHandle, MID mid, bool retained)
{
    assert(mid < MID_COUNT);
    if (MidIsValid(mid))
    {
        if (retained)
        {
            skymqtt_publish_retained(skyMqttHandle, eStrMqttMid_MID[mid], 0, NULL);
        }
        else
        {
            skymqtt_publish(skyMqttHandle, eStrMqttMid_MID[mid], 0, NULL);
        }
    }
}
static void MidPostPrivate(skymqtt_handle_t skyMqttHandle, MID mid, void const * pSrc, uint32_t length, bool retained)
{
    assert(mid < MID_COUNT);
    if (MidIsValid(mid))
    {
        if (retained)
        {
            skymqtt_publish_retained(skyMqttHandle, eStrMqttMid_MID[mid], length,
                                     (void *) pSrc); // TODO: this should be const in the skymqtt library!
        }
        else
        {
            skymqtt_publish(skyMqttHandle, eStrMqttMid_MID[mid], length,
                            (void *) pSrc); // TODO: this should be const in the skymqtt library!
        }
    }
}
static void MidPostDoublePrivate(skymqtt_handle_t skyMqttHandle, MID mid, void const * pSrc1, uint32_t length1, void const * pSrc2,
                                 uint32_t length2, bool retained)
{
    assert(mid < MID_COUNT);
    if (MidIsValid(mid))
    {
        void * pData = MemoryConcatData(pSrc1, length1, pSrc2, length2);
        if (retained)
        {
            skymqtt_publish_retained(skyMqttHandle, eStrMqttMid_MID[mid], length1 + length2, pData);
        }
        else
        {
            skymqtt_publish(skyMqttHandle, eStrMqttMid_MID[mid], length1 + length2, pData);
        }
        free(pData);
    }
}

void MidPostEmpty(skymqtt_handle_t skyMqttHandle, MID mid)
{
    MidPostEmptyPrivate(skyMqttHandle, mid, false);
}
void MidPost(skymqtt_handle_t skyMqttHandle, MID mid, void const * pSrc, uint32_t length)
{
    MidPostPrivate(skyMqttHandle, mid, pSrc, length, false);
}
void MidPostDouble(skymqtt_handle_t skyMqttHandle, MID mid, void const * pSrc1, uint32_t length1, void const * pSrc2,
                   uint32_t length2)
{
    MidPostDoublePrivate(skyMqttHandle, mid, pSrc1, length1, pSrc2, length2, false);
}
void MidPostEmptyRetained(skymqtt_handle_t skyMqttHandle, MID mid)
{
    MidPostEmptyPrivate(skyMqttHandle, mid, true);
}
void MidPostRetained(skymqtt_handle_t skyMqttHandle, MID mid, void const * pSrc, uint32_t length)
{
    MidPostPrivate(skyMqttHandle, mid, pSrc, length, true);
}
void MidPostDoubleRetained(skymqtt_handle_t skyMqttHandle, MID mid, void const * pSrc1, uint32_t length1, void const * pSrc2,
                           uint32_t length2)
{
    MidPostDoublePrivate(skyMqttHandle, mid, pSrc1, length1, pSrc2, length2, true);
}
