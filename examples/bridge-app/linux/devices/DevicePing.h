
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
class DevicePing; //Forward declare.

typedef struct
{
    DevicePing* pSelf;
    char ipAddress[PING_IP_ADDRESS_LENGTH];
    uint8_t failedPingCount;
    volatile bool threadIsActive;
}thread_data_t;

class DevicePing : public Device
{
public:
    DevicePing(const char* pName, const char* pLocation, TransportLayer* pTransportLayer, uint16_t deviceIndex, const char* pIpAddress);
    DevicePing(const char * pName, const char * pLocation, TransportLayer* pTransportLayer, const char* pIpAddress);
    ~DevicePing(void);
    void SetOn(bool on) { onOffCluster.SetOn(on, GetIndex()); }
    void Toggle(void) { onOffCluster.SetOn(!onOffCluster._isOn, GetIndex()); }

private:
    thread_data_t _threadData;
    pthread_t _thread;
    OnOffCluster onOffCluster;
    DescriptorCluster descriptorCluster;
    ENDPOINT_DATA _endpointData;

    void DevicePingLocal(const char* pName, const char* pLocation, TransportLayer *pTransportLayer,const char* pIpAddress);
    static void* DevicePingRun(void* pArgs);
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/