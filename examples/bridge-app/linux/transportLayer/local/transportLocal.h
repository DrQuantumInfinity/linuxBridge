
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
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
class TransportLocal : public TransportLayer
{
public:
    TransportLocal(const char* pIpAddress);
    virtual ~TransportLocal(void);
    static void Init(void);
    static void HandleTopicRx(const char* pTopic, const char* pPayload);

protected:
    void Send(const Device* pDevice, chip::ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer);

private:
    static DeviceList _deviceList;
    static mqtt_inst* _mqttInst;
    struct Private;
    static PersistDevList _persistList;
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
