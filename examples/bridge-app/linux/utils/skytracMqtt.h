/*******************************************************************************************
File: skytrac/mqtt.h

MQTT header for the skytrac utilities project

(c) Copyright 2021, SKYTRAC Systems Inc. All rights reserved.
This source code contains confidential, trade secret material. Any attempt or participation
in deciphering, decoding, reverse engineering or in any way altering the source code is
strictly prohibited, unless the prior written consent of SKYTRAC Systems is obtained.
 ********************************************************************************************/

#ifndef __SKYTRAC_MQTT_H__
#define __SKYTRAC_MQTT_H__

#include "message.h"

/**************************************************************************
 *                                  Prototypes
 **************************************************************************/

int mqtt_subscribe(char *const*const topics, size_t count) _nonnull_;
int mqtt_unsubscribe(char *const*const topics, size_t count) _nonnull_;
int mqtt_publish_formatted(const char* topic, const char* format, ...) _nonnull_ _format_(2, 3) _nonnull_;
int mqtt_publish_retained(const char *topic, unsigned int pubDataSize, void const *pubData) _nonnull_;
int mqtt_publish(const char *topic, unsigned int pubDataSize, void const *pubData) _nonnull_;

// Note: This is a temporary function to get the underlying mosquitto handle so it can be
// cast'ed to a skymqtt_handle_t for use with API functions. Remove once the skymqtt library
// is deprecated
struct mosquitto * mqtt_get_mosquitto_handle(void);
#endif
