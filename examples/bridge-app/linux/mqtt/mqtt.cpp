#include <string.h>
#include <unistd.h>
#include "mqtt.h"

#include <stdio.h>
#include <stdlib.h>

/**************************************************************************
 *                                  Variables
 **************************************************************************/
bool g_initted = false;
bool g_connected = false;
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
int mqtt_init(mqtt_inst * inst, char* broker, mqtt_msgCallback messageHandler)
{
    inst = (mqtt_inst*)malloc(sizeof(mqtt_inst));
    inst->messageHandler = messageHandler;
	mosquitto *mosq = inst->mosq;
	int rc;

    if(!g_initted)
    {
        mosquitto_lib_init();
        g_initted = true;
    }

	mosq = mosquitto_new(NULL, true, inst);
	if(mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_subscribe_callback_set(mosq, on_subscribe);
	mosquitto_message_callback_set(mosq, on_message);

	rc = mosquitto_connect(mosq, broker, 1883, 60);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

    rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}
    return 0;
}

void mqtt_add_sub(mqtt_inst * inst, const char * topic){
    inst->subs.insert(topic);
    if(g_connected)
        subscribe(inst->mosq, topic);
}

void mqtt_unsub(mqtt_inst * inst, const char * topic){
    inst->subs.erase(topic);
    if(g_connected)
        unsubscribe(inst->mosq, topic);
}

void mqtt_publish(mqtt_inst * inst, const char * topic, const char * message){
    int rc = mosquitto_publish(inst->mosq, NULL, topic, strlen(message), message, 2, false);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
	}
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static void subscribe(mosquitto * mosq, const char * topic){
    int rc = mosquitto_subscribe(mosq, NULL, topic, 1);
    if(rc != MOSQ_ERR_SUCCESS){
        fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
        mosquitto_disconnect(mosq);
    }
}

static void unsubscribe(mosquitto * mosq, const char* topic){
    int rc = mosquitto_unsubscribe(mosq, NULL, topic);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error unsubscribing: %s\n", mosquitto_strerror(rc));
		mosquitto_disconnect(mosq);
	}
}


static void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
    mqtt_inst* inst = (mqtt_inst*)obj;
    int rc;
    printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
    if(reason_code != 0){
        mosquitto_disconnect(mosq);
    }
    g_connected = true;
    std::unordered_set<const char*>::iterator itr;
    for (itr = inst->subs.begin(); itr != inst->subs.end(); itr++)
        subscribe(inst->mosq, *itr);
}

static void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    int i;
    bool have_subscription = false;

    for(i=0; i<qos_count; i++){
        printf("on_subscribe: %d:granted qos = %d\n", i, granted_qos[i]);
        if(granted_qos[i] <= 2){
            have_subscription = true;
        }
    }
    if(have_subscription == false){
        fprintf(stderr, "Error: All subscriptions rejected.\n");
        mosquitto_disconnect(mosq);
    }
}

static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    mqtt_inst* inst = (mqtt_inst*)obj;
    inst->messageHandler(msg->topic, (char*)msg->payload);
}