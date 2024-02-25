
#pragma once
#include "BasicCluster.h"
#include "DescriptorCluster.h"
#include "LevelControlCluster.h"
#include "ColourCluster.h"
#include "OnOffCluster.h"
#include "Device.h"
#include "EndpointApi.h"
#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>
using namespace ::chip;
/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define DEVICE_LIGHT_TEMP_NUM_CLUSTERS (5) // ArraySize(bridgedLightClusters)
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/

class DeviceLightTemp : public Device
{
public:
    DeviceLightTemp(const char * pName, const char * pLocation, DEVICE_WRITE_CALLBACK pfnWriteCallback);
    ~DeviceLightTemp(void);

    void SetOn(bool isOn) { onOffCluster.SetOn(isOn, GetIndex()); }
    void SetLevel(uint8_t level) { levelControlCluster.SetLevel(level, GetIndex()); }
    void SetColourTemp(uint16_t temp) { colourCluster.SetColourTemp(temp, GetIndex()); }
    OnOffCluster GetOnOffCluster(void) { return onOffCluster; }
    LevelControlCluster GetLevelControlCluster(void) { return levelControlCluster; }
    ColourCluster GetColourCluster(void) { return colourCluster; }
    DescriptorCluster GetDescriptorCluster(void) { return descriptorCluster; }

private:
    OnOffCluster onOffCluster;
    LevelControlCluster levelControlCluster;
    ColourCluster colourCluster;
    DescriptorCluster descriptorCluster;
    ENDPOINT_DATA _endpointData;
    void sendEspNowMessage(void);
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/