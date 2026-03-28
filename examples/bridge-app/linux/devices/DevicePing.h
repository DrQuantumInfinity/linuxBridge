
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
    void SendPing(void);

private:
    char _ipAddress[PING_IP_ADDRESS_LENGTH];
    uint32_t _successPingCount;
    uint32_t _failedPingCount;
    OnOffCluster onOffCluster;
    DescriptorCluster descriptorCluster;
    ENDPOINT_DATA _endpointData;

    void DevicePingLocal(const char* pName, const char* pLocation, TransportLayer *pTransportLayer,const char* pIpAddress);
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/