#pragma once

#include <mosquitto.h>

typedef void (*mqtt_msgCallback)(const struct mosquitto_message *);

typedef struct mqtt_inst
{
    mosquitto * mosq;
    mqtt_msgCallback messageHandler;
    std::unordered_set<char*> subs;
} mqtt_inst_t;


void add_sub(mqtt_inst * inst, char * topic);
void unsub(mqtt_inst * inst, char * topic);
int mqtt_init(mqtt_inst * inst, char* broker);