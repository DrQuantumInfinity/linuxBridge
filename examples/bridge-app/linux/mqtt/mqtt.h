#pragma once

#include <unordered_set>
#include <mosquitto.h>

typedef void (*mqtt_msgCallback)(const char* topic, const char* payload);

typedef struct mqtt_inst
{
    mosquitto * mosq;
    mqtt_msgCallback messageHandler;
    std::unordered_set<const char*> subs;
    bool connected = false;
} mqtt_inst_t;


void mqtt_add_sub(mqtt_inst * inst, const char * topic);
void mqtt_unsub(mqtt_inst * inst, const char * topic);
void mqtt_publish(mqtt_inst * inst, const char * topic, const char * message);
int mqtt_init(mqtt_inst * inst, const char* broker, mqtt_msgCallback messageHandler);