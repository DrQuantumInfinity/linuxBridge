
#pragma once

#include <functional>
#include <stdbool.h>
#include <stdint.h>

#include "transportLayer.h"
#include "DeviceList.h"

#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>

#include "mqttWrapper.h"
#include "PersistDevList.h"
/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define PING_MAX_DEVICE_NAME_LENGTH (32)
#define PING_IP_ADDRESS_LENGTH      (strlen("xxx.xxx.xxx.xxx") + 1)
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
class TransportPing : public TransportLayer
{
public:
    TransportPing(const char* pIpAddress);
    virtual ~TransportPing(void);
    static void Init(void);
    static void HandleTopicRx(const char* pTopic, const char* pPayload);
    
protected:
    void Send(const Device* pDevice, chip::ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer);

private:
    char _ipAddress[PING_IP_ADDRESS_LENGTH];
    uint8_t _failedPingCount;
    static DeviceList _deviceList;
    static mqtt_inst* _mqttInst;
    struct Private;
    static PersistDevList _persistList;
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
