
#pragma once
#include "BasicCluster.h"
#include "DescriptorCluster.h"
#include "LevelControlCluster.h"
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

class DeviceLightLevel : public Device
{
public:
    DeviceLightLevel(const char* pName, const char* pLocation, TransportLayer* pTransportLayer);
    ~DeviceLightLevel(void);

    void SetOn(bool isOn) { onOffCluster.SetOn(isOn, GetIndex()); }
    void SetLevel(uint8_t level) { levelControlCluster.SetLevel(level, GetIndex()); }
    OnOffCluster GetOnOffCluster(void) { return onOffCluster; }
    LevelControlCluster GetLevelControlCluster(void) { return levelControlCluster; }
    DescriptorCluster GetDescriptorCluster(void) { return descriptorCluster; }
    
    OnOffCluster onOffCluster;
    LevelControlCluster levelControlCluster;

private:
    DescriptorCluster descriptorCluster;
    ENDPOINT_DATA _endpointData;
    void sendEspNowMessage(void);
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/