
#pragma once
#include "DescriptorCluster.h"
#include "HumidityCluster.h"
#include "TempCluster.h"
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

class DeviceTemperature : public Device
{
public:
    DeviceTemperature(const char* pName, const char* pLocation, TransportLayer* pTransportLayer, uint16_t deviceIndex);
    DeviceTemperature(const char* pName, const char* pLocation, TransportLayer* pTransportLayer);
    ~DeviceTemperature(void);
    void UpdateTemp(float temp) { tempCluster.UpdateTemp(temp, GetIndex()); }
    void UpdateHumidity(float humidity) { humidityCluster.UpdateHumidity(humidity, GetIndex()); }

private:
    TempCluster tempCluster;
    HumidityCluster humidityCluster;
    DescriptorCluster descriptorCluster;
    ENDPOINT_DATA _endpointData;

    void DeviceTemperatureLocal(const char* pName, const char* pLocation, TransportLayer* pTransportLayer);
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/