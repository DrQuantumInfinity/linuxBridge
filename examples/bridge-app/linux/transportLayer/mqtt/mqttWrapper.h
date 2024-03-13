#pragma once

#include <unordered_set>
#include <mosquitto.h>

typedef void (*mqtt_msgCallback)(const char* topic, const char* payload);

typedef struct
{
    mosquitto * mosq;
    mqtt_msgCallback messageHandler;
    std::unordered_set<std::string> subs;
    bool connected;
} mqtt_inst;

mqtt_inst* mqtt_wrap_init(const char* broker, mqtt_msgCallback messageHandler);
void mqtt_wrap_loopstart(mqtt_inst * inst);
void mqtt_wrap_add_sub(mqtt_inst * inst, const char * topic);
void mqtt_wrap_unsub(mqtt_inst * inst, const char * topic);
void mqtt_wrap_publish(mqtt_inst * inst, const char * topic, const char * message);