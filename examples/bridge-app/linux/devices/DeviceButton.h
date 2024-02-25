
#pragma once
#include "DescriptorCluster.h"
#include "OnOffCluster.h"
#include "Device.h"
#include "EndpointApi.h"
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

class DeviceButton : public Device
{
public:
    DeviceButton(const char * pName, const char * pLocation, DEVICE_WRITE_CALLBACK pfnWriteCallback);
    ~DeviceButton(void);
    void Toggle(void) { onOffCluster.SetOn(!onOffCluster._isOn, GetIndex()); }

private:
    OnOffCluster onOffCluster;
    DescriptorCluster descriptorCluster;
    ENDPOINT_DATA _endpointData;
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/