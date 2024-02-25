/*******************************************************************************************
File: skymqtt.h

(c) Copyright 2021, SKYTRAC Systems Inc. All rights reserved.
This source code contains confidential, trade secret material. Any attempt or participation
in deciphering, decoding, reverse engineering or in any way altering the source code is
strictly prohibited, unless the prior written consent of SKYTRAC Systems is obtained.
********************************************************************************************/

#ifndef skymqtt_h
#define skymqtt_h

#include <mosquitto.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
// The MQTT standard limits topic strings to 65536 bytes.
// so maximum topic length should be at least MOSQ_MQTT_ID_MAX_LENGTH + 1
// for the ID plus separator. Or we can just use the standard limit

#define MAX_TOPIC_LEN (MOSQ_MQTT_ID_MAX_LENGTH + 2048 + 1)
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef void (*skymqtt_msgCallback)(void */*pUserContext*/, const struct mosquitto_message *);

typedef struct skymqtt_subscription
{
  const char *topic;
} skymqtt_subscription_t;

typedef struct mosquitto* skymqtt_handle_t;

/**************************************************************************
 *                                  Variables
 **************************************************************************/

/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
skymqtt_handle_t skymqtt_init(skymqtt_subscription_t const *pSubs, int numSubs, skymqtt_msgCallback callback, void const *pUserContext);

bool skymqtt_publish(skymqtt_handle_t handle, const char *topic, unsigned int pubDataSize, void const *pubData);
bool skymqtt_publish_retained(skymqtt_handle_t handle, const char *topic, unsigned int pubDataSize, void const *pubData);

void skymqtt_disconnect(skymqtt_handle_t handle);

bool MidIsValid(MID mid);
MID MidGetMidFromTopic(char const *pTopic);
char const *MidGetTopicFromMid(MID mid);

void MidPostEmpty(skymqtt_handle_t skyMqttHandle, MID mid);
void MidPost(skymqtt_handle_t skyMqttHandle, MID mid, void const *pSrc, uint32_t length);
void MidPostDouble(skymqtt_handle_t skyMqttHandle, MID mid, void const *pSrc1, uint32_t length1, void const *pSrc2, uint32_t length2);

void MidPostEmptyRetained(skymqtt_handle_t skyMqttHandle, MID mid);
void MidPostRetained(skymqtt_handle_t skyMqttHandle, MID mid, void const *pSrc, uint32_t length);
void MidPostDoubleRetained(skymqtt_handle_t skyMqttHandle, MID mid, void const *pSrc1, uint32_t length1, void const *pSrc2, uint32_t length2);


#endif
