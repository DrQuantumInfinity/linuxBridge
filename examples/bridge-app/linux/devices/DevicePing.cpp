//NOTE: This is a fake Device. Devices in this project normally exist as
//a proxy for a physical device somewhere in the home. This Device instead
//acts as the physical device and performs the work that's normally done
//by the physical device. 
#include "DevicePing.h"

#include "BasicCluster.h"
#include "DescriptorCluster.h"
#include "OnOffCluster.h"
#include "pingUtil.h"
#include <app/util/attribute-storage.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;
/**************************************************************************
 *                                  Constants
 **************************************************************************/

const EmberAfCluster bridgedClusters[] = {
    ClusterOnOffGetObject(),
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
// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1
const EmberAfDeviceType bridgedDeviceTypes[] = {
    {.deviceId = 0x0100, .deviceVersion = DEVICE_VERSION_DEFAULT},
    {.deviceId = DEVICE_TYPE_BRIDGED_NODE,    .deviceVersion = DEVICE_VERSION_DEFAULT}
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
DevicePing::DevicePing(const char* pName, const char* pLocation, TransportLayer* pTransportLayer, uint16_t deviceIndex, const char* pIpAddress) : Device(deviceIndex)
{
    DevicePingLocal(pName, pLocation, pTransportLayer, pIpAddress);
}
DevicePing::DevicePing(const char* pName, const char* pLocation, TransportLayer* pTransportLayer, const char* pIpAddress)
{
    DevicePingLocal(pName, pLocation, pTransportLayer, pIpAddress);
}
DevicePing::~DevicePing(void)
{
    free(_endpointData.pDataVersionStorage);
    EndpointRemove(GetIndex());
}
void DevicePing::SendPing(void)
{
    if (send_ping(this->_ipAddress))
    {
        _failedPingCount = 0;
        _successPingCount++;
        if (this->_successPingCount >= 3)
        {
            onOffCluster.SetOn(true, GetIndex());
        }
    }
    else
    {
        _successPingCount = 0;
        this->_failedPingCount++;
        if (this->_failedPingCount >= 3)
        {
            onOffCluster.SetOn(false, GetIndex());
        }
    }
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
void DevicePing::DevicePingLocal(const char* pName, const char* pLocation, TransportLayer* pTransportLayer, const char* pIpAddress)
{
    strcpy(_ipAddress, pIpAddress);
    _failedPingCount = 0;

    _pTransportLayer = pTransportLayer;
    DataVersion* pDataVersions = (DataVersion*)malloc(sizeof(DataVersion) * ArraySize(bridgedClusters));
    ENDPOINT_DATA endpointData = {
        .deviceIndex = GetIndex(),
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
    AddCluster(&onOffCluster);
    basicCluster.SetName(pName, GetIndex());
    strcpy(endpointData.name, pName);
    strcpy(endpointData.location, pLocation);

    memcpy(&_endpointData, &endpointData, sizeof(_endpointData));
    EndpointAdd(&_endpointData);
    log_info("Created device %u %s", _endpointData.deviceIndex, _endpointData.name);
}