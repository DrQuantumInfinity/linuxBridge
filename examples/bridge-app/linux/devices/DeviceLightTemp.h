
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
/**************************************************************************
 *                                  Constants
 **************************************************************************/
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/

class DeviceLightTemp : public Device
{
public:
    DeviceLightTemp(const char* pName, const char* pLocation, TransportLayer* pTransportLayer, uint16_t deviceIndex);
    DeviceLightTemp(const char* pName, const char* pLocation, TransportLayer* pTransportLayer);
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
    
    void DeviceLightTempLocal(const char* pName, const char* pLocation, TransportLayer* pTransportLayer);
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/