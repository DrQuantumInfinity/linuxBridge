
#pragma once

#include <functional>
#include <stdbool.h>
#include <stdint.h>

#include "transportLayer.h"
#include "DeviceList.h"

#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>

#include "mqttWrapper.h"
/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define MQTT_MAX_DEVICE_NAME_LENGTH (32)
#define MQTT_MAC_ADDRESS_LENGTH     (12 + 1)
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef enum
{
    MQTT_DIMMER_SWITCH_FEIT,
    MQTT_OUTLET_GORDON,
    MQTT_LAMP_RGB,
    MQTT_TYPE_COUNT
}MQTT_TYPE;

class TransportMqtt : public TransportLayer
{
public:
    TransportMqtt(MQTT_TYPE type, const char* pMacAddr);
    virtual ~TransportMqtt(void);
    static void Init(void);
    static void HandleTopicRx(const char* pTopic, const char* pPayload);
    
protected:
    void Send(const Device* pDevice, chip::ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer);

private:
    MQTT_TYPE _type;
    char _macAddr[MQTT_MAC_ADDRESS_LENGTH];
    static DeviceList _deviceList;
    static mqtt_inst* _mqttInst;
    struct Private;
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
