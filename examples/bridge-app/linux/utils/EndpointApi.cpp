
#include "EndpointApi.h"
#include <app/util/attribute-storage.h>
#include <app/reporting/reporting.h> //for MatterReportingAttributeChangeCallback()
#include <span>
#include "Log.h"
#include "BasicCluster.h" //for BasicAttrs

using namespace ::chip;
using namespace ::chip::app::Clusters;
/**************************************************************************
 *                                  Constants
 **************************************************************************/
// (taken from chip-devices.xml)
#define DEVICE_TYPE_ROOT_NODE 0x0016
// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGE 0x000e
// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1
const EmberAfDeviceType rootDeviceTypes[]          = { { DEVICE_TYPE_ROOT_NODE, DEVICE_VERSION_DEFAULT } };
const EmberAfDeviceType aggregateNodeDeviceTypes[] = { { DEVICE_TYPE_BRIDGE, DEVICE_VERSION_DEFAULT } };
#define DEVICE_INDEX_UNUSED (65535)

/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef struct
{
    uint16_t deviceIndex;
    void *pObject;
    GOOGLE_READ_CALLBACK pfnReadCallback;
    GOOGLE_WRITE_CALLBACK pfnWriteCallback;
    GOOGLE_INSTANT_ACTION_CALLBACK pfnInstantActionCallback;
    char name[32];
}ENDPOINT;

typedef struct
{
    EndpointId firstDynamicEndpointId;
    ENDPOINT endpoint[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];
}ENDPOINT_API;

/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
//workers
static void EndpointApiInitWorker(intptr_t context);
static void EndpointAddWorker(intptr_t context);
static void EndpointRemoveWorker(intptr_t context);
static void EndpointReportUpdateWorker(intptr_t closure);
static uint16_t EndpointGetIndexFromDeviceIndex(uint16_t deviceIndex);
static uint16_t EndpointGetFreeIndexFromDeviceIndex(void);
static const char* EndpointReadNameFromBasicCluster(ENDPOINT* pEndpoint);
/**************************************************************************
 *                                  Variables
 **************************************************************************/
static ENDPOINT_API endpointApi;
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
void EndpointApiInit(void)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(EndpointApiInitWorker);
    memset(&endpointApi, 0x00, sizeof(endpointApi));
}
void EndpointAdd(ENDPOINT_DATA *pData)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(EndpointAddWorker, reinterpret_cast<intptr_t>(pData));
}
void EndpointRemove(uint16_t deviceIndex)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(EndpointRemoveWorker, reinterpret_cast<intptr_t>((void*)(uint64_t)deviceIndex));
}
void EndpointReportChange(uint16_t deviceIndex, ClusterId cluster, AttributeId attribute)
{
    auto * path = Platform::New<app::ConcreteAttributePath>(deviceIndex, cluster, attribute);
    DeviceLayer::PlatformMgr().ScheduleWork(EndpointReportUpdateWorker, reinterpret_cast<intptr_t>(path));
}
/**************************************************************************
 *                                  Magic Callback Functions
 **************************************************************************/
EmberAfStatus emberAfExternalAttributeReadCallback(
    EndpointId endpoint, ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer, uint16_t maxReadLength)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;

    uint16_t deviceIndex = endpoint - endpointApi.firstDynamicEndpointId;
    uint16_t endpointIndex = EndpointGetIndexFromDeviceIndex(deviceIndex);

    if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        ENDPOINT* pEndpoint = &endpointApi.endpoint[endpointIndex];
        if (pEndpoint->pfnReadCallback)
        {
            status = pEndpoint->pfnReadCallback(pEndpoint->pObject, clusterId, attributeMetadata, buffer, maxReadLength);

            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                uint32_t temp = 0;
                uint32_t copyLength = maxReadLength;
                if (sizeof(temp) < copyLength)
                {
                    copyLength = sizeof(temp);
                }
                memcpy(&temp, buffer, copyLength);
                log_info("Read callback for device %s %u, cluster %04lX, attr %04lX, val %08lX", pEndpoint->name, deviceIndex, clusterId, attributeMetadata->attributeId, temp);
            }
            else
            {
                log_error("Read failed for device %s %u, cluster %04lX, attr %04lX", pEndpoint->name, deviceIndex, clusterId, attributeMetadata->attributeId);
            }
        }
    }
    else
    {
        log_error("Read invalid endpoint for device %u", deviceIndex);
    }
    return status;
}
EmberAfStatus emberAfExternalAttributeWriteCallback(
    EndpointId endpoint, ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    
    uint16_t deviceIndex = endpoint - endpointApi.firstDynamicEndpointId;
    uint16_t endpointIndex = EndpointGetIndexFromDeviceIndex(deviceIndex);

    if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (endpointApi.endpoint[endpointIndex].pfnWriteCallback)
        {
            status = endpointApi.endpoint[endpointIndex].pfnWriteCallback(endpointApi.endpoint[endpointIndex].pObject, clusterId, attributeMetadata, buffer);
            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                uint32_t temp;
                memcpy(&temp, buffer, sizeof(temp));
                log_info("Write callback for device %s %u, cluster %04lX, attr %04lX, val %08lX", endpointApi.endpoint[endpointIndex].name, deviceIndex, clusterId, attributeMetadata->attributeId, temp);
            }
            else
            {
                log_error("Write failed for device %s %u, cluster %04lX, attr %04lX", endpointApi.endpoint[endpointIndex].name, deviceIndex, clusterId, attributeMetadata->attributeId);
            }
        }
    }
    else
    {
        log_error("Write invalid endpoint for device %u", deviceIndex);
    }
    
    return status;
}
//Another magic callback. I think we want to use this. Review the path below for examples.
//"/root/esp-matter/connectedhomeip/connectedhomeip/examples/bridge-app/linux/main.cpp"
bool emberAfActionsClusterInstantActionCallback(
    app::CommandHandler* commandObj, const app::ConcreteCommandPath & commandPath, 
    const Actions::Commands::InstantAction::DecodableType & commandData)
{
    bool retVal = true;
    
    //TODO: how do I know what index received an instant action?
/*  if (endpointApi.endpoint[index].pfnInstantActionCallback)
    {
        retVal = endpointApi.endpoint[index].pfnInstantActionCallback(commandObj, commandPath, commandData);
    }
    else*/
    {
        // No actions are implemented, just return status NotFound.
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::NotFound);
    }
    return retVal;
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static void EndpointApiInitWorker(intptr_t context)
{
    // Silence complaints about unused variable.
    UNUSED_VAR(context);

    log_info("Init");
    memset(&endpointApi, 0x00, sizeof(endpointApi));

    for (int endpointIndex = 0; endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; endpointIndex++)
    {
        endpointApi.endpoint[endpointIndex].deviceIndex = DEVICE_INDEX_UNUSED;
    }

    // Set starting endpoint id where dynamic endpoints will be assigned, which
    // will be the next consecutive endpoint id after the last fixed endpoint.
    endpointApi.firstDynamicEndpointId = static_cast<chip::EndpointId>(static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);
   
    // A bridge has root node device type on EP0 and aggregate node device type (bridge) at EP1
    emberAfSetDeviceTypeList(0, Span<const EmberAfDeviceType>(rootDeviceTypes));
    emberAfSetDeviceTypeList(1, Span<const EmberAfDeviceType>(aggregateNodeDeviceTypes));
}
static void EndpointAddWorker(intptr_t context)
{
    ENDPOINT_DATA *pData = reinterpret_cast<ENDPOINT_DATA *>(context);
    log_info("Adding device %u: %s", pData->deviceIndex, pData->name);
    uint16_t endpointIndex = EndpointGetFreeIndexFromDeviceIndex();

    if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        endpointApi.endpoint[endpointIndex].deviceIndex = pData->deviceIndex;
        endpointApi.endpoint[endpointIndex].pObject = pData->pObject;
        endpointApi.endpoint[endpointIndex].pfnReadCallback = pData->pfnReadCallback;
        endpointApi.endpoint[endpointIndex].pfnWriteCallback = pData->pfnWriteCallback;
        endpointApi.endpoint[endpointIndex].pfnInstantActionCallback = pData->pfnInstantActionCallback;
        strcpy(endpointApi.endpoint[endpointIndex].name, pData->name);

        Span<DataVersion> dataVersion = Span<DataVersion>(pData->pDataVersionStorage, pData->dataVersionStorageLength);
        Span<const EmberAfDeviceType> deviceTypeList= Span<const EmberAfDeviceType>(pData->pDeviceTypeList, pData->deviceTypeListLength);
        CHIP_ERROR ret = emberAfSetDynamicEndpoint(
            endpointIndex, endpointApi.firstDynamicEndpointId + pData->deviceIndex, pData->ep, 
            dataVersion, deviceTypeList, 
            pData->parentEndpointId);
        if (ret == CHIP_NO_ERROR)
        {
            log_info("Added device %u: %s at dynamic endpoint %u", pData->deviceIndex, pData->name, endpointIndex);
        }
        else //if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
        {
            log_error("add failed for device %u: retVal %u", pData->deviceIndex, ret);
        }
    }
    else
    {
        log_error("Failed to add dynamic endpoint: No endpoints available! %u: %s", pData->deviceIndex, pData->name);
    }
}
static void EndpointRemoveWorker(intptr_t context)
{
    uint16_t deviceIndex = (uint16_t)(uint64_t)reinterpret_cast<void*>(context);
    uint16_t endpointIndex = EndpointGetIndexFromDeviceIndex(deviceIndex);

    if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        ENDPOINT* pEndpoint = &endpointApi.endpoint[endpointIndex];
        log_info("Removing device: %s %u from %u", EndpointReadNameFromBasicCluster(pEndpoint), deviceIndex, endpointIndex);
        memset(pEndpoint, 0x00, sizeof(*pEndpoint));
        pEndpoint->deviceIndex = DEVICE_INDEX_UNUSED;
        EndpointId ep = emberAfClearDynamicEndpoint(endpointIndex);
        UNUSED_VAR(ep);
    }
    else
    {
        log_error("device not in use: %u", deviceIndex);
    }
}
static void EndpointReportUpdateWorker(intptr_t closure)
{
    auto path = reinterpret_cast<app::ConcreteAttributePath *>(closure);
    uint16_t deviceIndex = (uint16_t)path->mEndpointId;
    uint16_t endpointIndex = EndpointGetIndexFromDeviceIndex(deviceIndex);
    path->mEndpointId = deviceIndex + endpointApi.firstDynamicEndpointId;

    if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        ENDPOINT* pEndpoint = &endpointApi.endpoint[endpointIndex];
        log_info("Update name/device/cluster/attr %s/%i/%04lX/%04lX", EndpointReadNameFromBasicCluster(pEndpoint), deviceIndex, path->mClusterId, path->mAttributeId);
        MatterReportingAttributeChangeCallback(*path);
    }
    else
    {
        log_warn("Update device %u doesn't exist!", deviceIndex);
    }
    Platform::Delete(path);
}
static uint16_t EndpointGetIndexFromDeviceIndex(uint16_t deviceIndex)
{
    uint16_t endpointIndex;
    for (endpointIndex = 0; endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; endpointIndex++)
    {
        if (endpointApi.endpoint[endpointIndex].deviceIndex == deviceIndex)
        {
            break;
        }
    }
    return endpointIndex;
}
static uint16_t EndpointGetFreeIndexFromDeviceIndex(void)
{
    uint16_t endpointIndex;
    for (endpointIndex = 0; endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; endpointIndex++)
    {
        if (endpointApi.endpoint[endpointIndex].deviceIndex == DEVICE_INDEX_UNUSED)
        {
            break;
        }
    }
    return endpointIndex;
}
static const char* EndpointReadNameFromBasicCluster(ENDPOINT* pEndpoint)
{
    const char noName[] = "no name";
    const char* pRetVal = noName;
    
    if (pEndpoint->pfnReadCallback)
    {
        pEndpoint->pfnReadCallback(
            pEndpoint->pObject, 
            chip::app::Clusters::BridgedDeviceBasicInformation::Id, 
            &BasicCluster::BasicAttrs[0], 
            (uint8_t*)pEndpoint->name,
            sizeof(pEndpoint->name)
        );
        pRetVal = pEndpoint->name;
    }
    
    /*
    if (pEndpoint->pObject)
    {
        Device* pDevice = (Device*)pEndpoint->pObject;
        pRetVal = pDevice->basicCluster._name;
    }*/
    return pRetVal;
}