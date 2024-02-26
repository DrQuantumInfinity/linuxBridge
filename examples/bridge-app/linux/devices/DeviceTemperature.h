
#pragma once
#include "DescriptorCluster.h"
#include "HumidityCluster.h"
#include "TempCluster.h"
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

class DeviceTemperature : public Device
{
public:
    DeviceTemperature(const char* pName, const char* pLocation, TransportLayer* pTransportLayer, float temp, float humid);
    ~DeviceTemperature(void);
    void UpdateTemp(float temp) { tempCluster.UpdateTemp(temp, GetIndex()); }
    void UpdateHumidity(float humidity) { humidityCluster.UpdateHumidity(humidity, GetIndex()); }

private:
    TempCluster tempCluster;
    HumidityCluster humidityCluster;
    DescriptorCluster descriptorCluster;
    ENDPOINT_DATA _endpointData;
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/