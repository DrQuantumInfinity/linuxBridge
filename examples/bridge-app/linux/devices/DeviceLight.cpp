
#include "DeviceLight.h"

#include "BasicCluster.h"
#include "DescriptorCluster.h"
#include "OnOffCluster.h"
#include "SerialTask.h"
#include <app/util/attribute-storage.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;
/**************************************************************************
 *                                  Constants
 **************************************************************************/

const EmberAfCluster bridgedClusters[] = {
    OnOffCluster::cluster,
    DescriptorCluster::cluster,
    BasicCluster::cluster,
};

// Declare Bridged Light endpoint
const EmberAfEndpointType bridgedEndpoint = { .cluster      = bridgedClusters,
                                              .clusterCount = ArraySize(bridgedClusters),
                                              .endpointSize = 0 };

// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGED_NODE 0x0013
// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1
const EmberAfDeviceType bridgedDeviceTypes[] = {
    { .deviceId = 0x0100, .deviceVersion = DEVICE_VERSION_DEFAULT },
    { .deviceId = DEVICE_TYPE_BRIDGED_NODE, .deviceVersion = DEVICE_VERSION_DEFAULT }
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
DeviceLight::DeviceLight(const char * pName, const char * pLocation, DEVICE_WRITE_CALLBACK pfnWriteCallback)
{
    _pfnWriteCallback           = pfnWriteCallback;
    DataVersion * pDataVersions = (DataVersion *) malloc(sizeof(DataVersion) * ArraySize(bridgedClusters));
    ENDPOINT_DATA endpointData  = {
         .index                    = GetIndex(),
         .pObject                  = this,
         .pfnReadCallback          = GoogleReadCallback,
         .pfnWriteCallback         = GoogleWriteCallback,
         .pfnInstantActionCallback = NULL, // worry about this later
         .name                     = { 0 },
         .location                 = { 0 },
         .ep                       = &bridgedEndpoint,
         .pDeviceTypeList          = bridgedDeviceTypes,
         .deviceTypeListLength     = ArraySize(bridgedDeviceTypes),
         .pDataVersionStorage      = pDataVersions,
         .dataVersionStorageLength = ArraySize(bridgedClusters),
         .parentEndpointId         = 1,
    };
    AddCluster(&descriptorCluster);
    AddCluster(&onOffCluster);
    strcpy(endpointData.name, pName);
    strcpy(endpointData.location, pLocation);

    memcpy(&_endpointData, &endpointData, sizeof(_endpointData));
    EndpointAdd(&_endpointData);
}

DeviceLight::~DeviceLight(void)
{
    free(_endpointData.pDataVersionStorage);
    EndpointRemove(GetIndex());
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
void DeviceLight::sendEspNowMessage()
{
    _espNowData.data.lightOnOff.onOff = onOffCluster._isOn;
    SerialTransmit(&_espNowData, sizeof(_espNowData));
}