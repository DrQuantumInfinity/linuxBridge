
#pragma once
#include "Device.h"
#include "EndpointApi.h"
#include "DescriptorCluster.h"
#include "OnOffCluster.h"
#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>
using namespace ::chip;
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
    DeviceLight(const char* pName, const char* pLocation, DEVICE_WRITE_CALLBACK pfnWriteCallback);
    ~DeviceLight(void);

    void SetOn(bool isOn) { onOffCluster.SetOn(isOn, GetIndex()); }

private:    
    OnOffCluster onOffCluster;
    DescriptorCluster descriptorCluster;
    ENDPOINT_DATA _endpointData;
    void sendEspNowMessage(void);
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/