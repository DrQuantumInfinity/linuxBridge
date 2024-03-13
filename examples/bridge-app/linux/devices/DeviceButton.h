
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
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/

class DeviceButton : public Device
{
public:
    DeviceButton(const char * pName, const char * pLocation, TransportLayer* pTransportLayer);
    ~DeviceButton(void);
    void SetOn(bool on) { onOffCluster.SetOn(on, GetIndex()); }
    void Toggle(void) { onOffCluster.SetOn(!onOffCluster._isOn, GetIndex()); }

private:
    OnOffCluster onOffCluster;
    DescriptorCluster descriptorCluster;
    ENDPOINT_DATA _endpointData;
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/