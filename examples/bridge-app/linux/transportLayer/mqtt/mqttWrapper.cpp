#include <string.h>
#include <unistd.h>
#include "mqttWrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include "Log.h"
/**************************************************************************
 *                                  Variables
 **************************************************************************/
bool g_initted = false;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
static void on_connect(struct mosquitto *mosq, void *obj, int reason_code);
static void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);
static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
static void subscribe(mosquitto * mosq, const char * topic);
static void unsubscribe(mosquitto * mosq, const char * topic);
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
mqtt_inst* mqtt_wrap_init(const char* broker, mqtt_msgCallback messageHandler)
{
    mqtt_inst* inst = new mqtt_inst();
    inst->messageHandler = messageHandler;
    inst->connected = false;

	int rc;

    if(!g_initted)
    {
        mosquitto_lib_init();
        g_initted = true;
    }

	inst->mosq = mosquitto_new(NULL, true, inst);
	mosquitto *mosq = inst->mosq;
	if(mosq == NULL){
        log_error("MQTT OOM error");
		return NULL;
	}

	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_subscribe_callback_set(mosq, on_subscribe);
	mosquitto_message_callback_set(mosq, on_message);

	rc = mosquitto_connect(mosq, broker, 1883, 60);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
        log_error("MQTT connect error: %s", mosquitto_strerror(rc));
		return NULL;
	}
    log_info("MQTT init successful");
    return inst;
}

void mqtt_wrap_loopstart(mqtt_inst * inst){
    mosquitto *mosq = inst->mosq;
    int rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
        log_error("MQTT loop start error: %s", mosquitto_strerror(rc));
	}
}

void mqtt_wrap_add_sub(mqtt_inst * inst, const char * topic){
    log_info("-----starting to sub to %s", topic);
    log_info("subs size b4 %i", inst->subs.size());
    for(auto x: inst->subs)
    {
        log_info("%s", x.c_str());
    }
    std::string cpptopic = topic;
    inst->subs.insert(cpptopic);
    log_info("subs size l8r %i", inst->subs.size());
    
    for(auto x: inst->subs)
    {
        log_info("%s", x.c_str());
    }
    log_info("end sub\n\n");
    // if(inst->connected)
        // subscribe(inst->mosq, topic);
}

void mqtt_wrap_unsub(mqtt_inst * inst, const char * topic){
    inst->subs.erase(topic);
    if(inst->connected)
        unsubscribe(inst->mosq, topic);
}

void mqtt_wrap_publish(mqtt_inst * inst, const char * topic, const char * message){
    int rc = mosquitto_publish(inst->mosq, NULL, topic, (int)strlen(message), message, 2, false);
	if(rc != MOSQ_ERR_SUCCESS){
		log_error("Error publishing: %s", mosquitto_strerror(rc));
	}
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static void subscribe(mosquitto * mosq, const char * topic){
    log_info("subscribing to: %s", topic);
    int rc = mosquitto_subscribe(mosq, NULL, topic, 1);
    if(rc != MOSQ_ERR_SUCCESS){
        log_error("Error subscribing: %s", mosquitto_strerror(rc));
        mosquitto_disconnect(mosq);
    }
}

static void unsubscribe(mosquitto * mosq, const char* topic){
    int rc = mosquitto_unsubscribe(mosq, NULL, topic);
	if(rc != MOSQ_ERR_SUCCESS){
		log_error("Error unsubscribing: %s", mosquitto_strerror(rc));
		mosquitto_disconnect(mosq);
	}
}


static void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
    mqtt_inst* inst = (mqtt_inst*)obj;
    log_info("on_connect: %s", mosquitto_connack_string(reason_code));
    if(reason_code != 0){
        mosquitto_disconnect(mosq);
    }
    inst->connected = true;
    for(auto x: inst->subs)
    {
        subscribe(inst->mosq, x.c_str());
    }
}

static void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    int i;
    bool have_subscription = false;

    for(i=0; i<qos_count; i++){
        log_info("on_subscribe: %d:granted qos = %d", i, granted_qos[i]);
        if(granted_qos[i] <= 2){
            have_subscription = true;
        }
    }
    if(have_subscription == false){
        log_error("Error: All subscriptions rejected.\n");
        mosquitto_disconnect(mosq);
    }
}

static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    mqtt_inst* inst = (mqtt_inst*)obj;
    inst->messageHandler(msg->topic, (char*)msg->payload);
}