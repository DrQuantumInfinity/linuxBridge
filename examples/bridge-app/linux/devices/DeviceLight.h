
#pragma once
#include "Device.h"
#include "EndpointApi.h"
#include "DescriptorCluster.h"
#include "OnOffCluster.h"
#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>
/**************************************************************************
 *                                  Constants
 **************************************************************************/
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
class DeviceLight : public Device
{
public:
    DeviceLight(const char* pName, const char* pLocation, TransportLayer* pTransportLayer);
    ~DeviceLight(void);

    void SetOn(bool isOn) { onOffCluster.SetOn(isOn, GetIndex()); }
    
    OnOffCluster onOffCluster;
    DescriptorCluster descriptorCluster;

private:    
    void sendEspNowMessage(void);
    ENDPOINT_DATA _endpointData;
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/