#pragma once

#include <unordered_set>
#include <mosquitto.h>

typedef void (*mqtt_msgCallback)(const char* topic, const char* payload);

typedef struct
{
    mosquitto * mosq;
    mqtt_msgCallback messageHandler;
    std::unordered_set<const char*> subs;
    bool connected;
} mqtt_inst;


void mqtt_wrap_add_sub(mqtt_inst * inst, const char * topic);
void mqtt_wrap_unsub(mqtt_inst * inst, const char * topic);
void mqtt_wrap_publish(mqtt_inst * inst, const char * topic, const char * message);
int mqtt_wrap_init(mqtt_inst * inst, const char* broker, mqtt_msgCallback messageHandler);