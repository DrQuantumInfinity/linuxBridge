
#pragma once
#include "DescriptorCluster.h"
#include "OnOffCluster.h"
#include "Device.h"
#include "EndpointApi.h"
#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>
/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define PING_IP_ADDRESS_LENGTH      (sizeof("xxx.xxx.xxx.xxx") + 1)
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/

class DevicePing : public Device
{
public:
    DevicePing(const char* pName, const char* pLocation, TransportLayer* pTransportLayer, uint16_t deviceIndex, const char* pIpAddress);
    DevicePing(const char * pName, const char * pLocation, TransportLayer* pTransportLayer, const char* pIpAddress);
    ~DevicePing(void);
    void SetOn(bool on) { onOffCluster.SetOn(on, GetIndex()); }
    void Toggle(void) { onOffCluster.SetOn(!onOffCluster._isOn, GetIndex()); }
    bool SendPing();

private:
    char _ipAddress[PING_IP_ADDRESS_LENGTH];
    uint8_t _failedPingCount;
    OnOffCluster onOffCluster;
    DescriptorCluster descriptorCluster;
    ENDPOINT_DATA _endpointData;

    void DevicePingLocal(const char* pName, const char* pLocation, TransportLayer *pTransportLayer,const char* pIpAddress);
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/