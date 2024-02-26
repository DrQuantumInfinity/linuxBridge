
#include "DeviceTemperature.h"

#include "BasicCluster.h"
#include "DescriptorCluster.h"
#include "TempCluster.h"
#include <app/util/attribute-storage.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;
/**************************************************************************
 *                                  Constants
 **************************************************************************/

const EmberAfCluster bridgedClusters[] = {
    TempCluster::cluster,
    HumidityCluster::cluster,
    DescriptorCluster::cluster,
    BasicCluster::cluster,
};

// Declare Bridged Light endpoint
const EmberAfEndpointType bridgedEndpoint = { 
    .cluster = bridgedClusters, 
    .clusterCount = ArraySize(bridgedClusters), 
    .endpointSize = 0 
};


// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGED_NODE 0x0013
// (taken from lo-devices.xml)
#define DEVICE_TYPE_TEMP_MEASURE 0x0302
#define DEVICE_TYPE_HUMID_MEASURE 0x0307
#define DEV_THEMOSTAT 0x0301
// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1
const EmberAfDeviceType bridgedDeviceTypes[] = { 
     {.deviceId = DEVICE_TYPE_TEMP_MEASURE, .deviceVersion = DEVICE_VERSION_DEFAULT},
    //  {.deviceId = DEVICE_TYPE_HUMID_MEASURE, .deviceVersion = DEVICE_VERSION_DEFAULT},
    //  {.deviceId = DEV_THEMOSTAT, .deviceVersion = DEVICE_VERSION_DEFAULT},
     {.deviceId = DEVICE_TYPE_BRIDGED_NODE,  .deviceVersion = DEVICE_VERSION_DEFAULT} 
};
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
/**************************************************************************
 *                                  Variables
 **************************************************************************/
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
DeviceTemperature::DeviceTemperature(const char* pName, const char* pLocation, TransportLayer* pTransportLayer, float temp, float humid)
{
    _pTransportLayer = pTransportLayer;
    DataVersion* pDataVersions = (DataVersion*)malloc(sizeof(DataVersion)*ArraySize(bridgedClusters));
    ENDPOINT_DATA endpointData = {
        .index = GetIndex(),
        .pObject = this,
        .pfnReadCallback = GoogleReadCallback,
        .pfnWriteCallback = GoogleWriteCallback,
        .pfnInstantActionCallback = NULL, //worry about this later
        .name = {0},
        .location = {0},
        .ep = &bridgedEndpoint,
        .pDeviceTypeList = bridgedDeviceTypes,
        .deviceTypeListLength = ArraySize(bridgedDeviceTypes),
        .pDataVersionStorage = pDataVersions,
        .dataVersionStorageLength = ArraySize(bridgedClusters),
        .parentEndpointId = 1,
    };
    AddCluster(&descriptorCluster);
    AddCluster(&tempCluster);
    AddCluster(&humidityCluster);

    tempCluster._temp = temp;
    humidityCluster._humidity = humid;
    strncpy(basicCluster._name, pName, sizeof(basicCluster._name));
    
    strcpy(endpointData.name, pName);
    strcpy(endpointData.location, pLocation);

    memcpy(&_endpointData, &endpointData, sizeof(_endpointData));
    EndpointAdd(&_endpointData);
}

DeviceTemperature::~DeviceTemperature(void)
{
    free(_endpointData.pDataVersionStorage);
    EndpointRemove(GetIndex());
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
