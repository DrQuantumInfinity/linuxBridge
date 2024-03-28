
#include "EndpointApi.h"
#include <app/util/attribute-storage.h>
#include <app/reporting/reporting.h> //for MatterReportingAttributeChangeCallback()
#include <span>
#include "Log.h"

using namespace ::chip;
using namespace ::chip::app::Clusters;
/**************************************************************************
 *                                  Constants
 **************************************************************************/
static const char * TAG = "endpoint-api";
// (taken from chip-devices.xml)
#define DEVICE_TYPE_ROOT_NODE 0x0016
// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGE 0x000e
// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1
const EmberAfDeviceType rootDeviceTypes[]          = { { DEVICE_TYPE_ROOT_NODE, DEVICE_VERSION_DEFAULT } };
const EmberAfDeviceType aggregateNodeDeviceTypes[] = { { DEVICE_TYPE_BRIDGE, DEVICE_VERSION_DEFAULT } };

/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef struct
{
    uint16_t index;
    void *pObject;
    GOOGLE_READ_CALLBACK pfnReadCallback;
    GOOGLE_WRITE_CALLBACK pfnWriteCallback;
    GOOGLE_INSTANT_ACTION_CALLBACK pfnInstantActionCallback;
}ENDPOINT;

typedef struct
{
    EndpointId currentEndpointId;
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
}
void EndpointAdd(ENDPOINT_DATA *pData)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(EndpointAddWorker, reinterpret_cast<intptr_t>(pData));
}
void EndpointRemove(uint16_t index)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(EndpointRemoveWorker, reinterpret_cast<intptr_t>((void*)(uint32_t)index));
}
void EndpointReportChange(uint16_t index, ClusterId cluster, AttributeId attribute)
{
    auto * path = Platform::New<app::ConcreteAttributePath>(index + FIXED_ENDPOINT_COUNT, cluster, attribute);
    DeviceLayer::PlatformMgr().ScheduleWork(EndpointReportUpdateWorker, reinterpret_cast<intptr_t>(path));
}
/**************************************************************************
 *                                  Magic Callback Functions
 **************************************************************************/
EmberAfStatus emberAfExternalAttributeReadCallback(
    EndpointId endpoint, ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer, uint16_t maxReadLength)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;

    uint16_t index = emberAfGetDynamicIndexFromEndpoint(endpoint);

    if (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (endpointApi.endpoint[index].pfnReadCallback)
        {
            status = endpointApi.endpoint[index].pfnReadCallback(endpointApi.endpoint[index].pObject, clusterId, attributeMetadata, buffer, maxReadLength);
        }
    }
    else
    {
        log_error("Read invalid endpoint %u, %u", endpoint, endpoint);
    }

    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        uint32_t temp = 0;
        uint32_t copyLength = maxReadLength;
        if (sizeof(temp) < copyLength)
        {
            copyLength = sizeof(temp);
        }
        memcpy(&temp, buffer, copyLength);
        log_info("Read callback for endpoint %u, cluster %04lX, attr %04lX, val %08lX", endpoint, clusterId, attributeMetadata->attributeId, temp);
    }
    else
    {
        log_error("Read failed for endpoint %u, cluster %04lX, attr %04lX", endpoint, clusterId, attributeMetadata->attributeId);
    }
    return status;
}
EmberAfStatus emberAfExternalAttributeWriteCallback(
    EndpointId endpoint, ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    
    uint16_t index = emberAfGetDynamicIndexFromEndpoint(endpoint);

    if (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (endpointApi.endpoint[index].pfnWriteCallback)
        {
            status = endpointApi.endpoint[index].pfnWriteCallback(endpointApi.endpoint[index].pObject, clusterId, attributeMetadata, buffer);
        }
    }
    else
    {
        log_error("Write invalid endpoint %u, %u", endpoint, endpoint);
    }
    
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        uint32_t temp;
        memcpy(&temp, buffer, sizeof(temp));
        log_info("Write callback for endpoint %u, cluster %04lX, attr %04lX, val %08lX", endpoint, clusterId, attributeMetadata->attributeId, temp);
    }
    else
    {
        log_error("Write failed for endpoint %u, cluster %04lX, attr %04lX", endpoint, clusterId, attributeMetadata->attributeId);
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

    // Set starting endpoint id where dynamic endpoints will be assigned, which
    // will be the next consecutive endpoint id after the last fixed endpoint.
    endpointApi.firstDynamicEndpointId = static_cast<chip::EndpointId>(static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    endpointApi.currentEndpointId = endpointApi.firstDynamicEndpointId;

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
    log_info("Adding device %u: %s", pData->index, pData->name);
    
    if (pData->index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (endpointApi.endpoint[pData->index].index == 0)
        {
            endpointApi.endpoint[pData->index].index = pData->index;
            endpointApi.endpoint[pData->index].pObject = pData->pObject;
            endpointApi.endpoint[pData->index].pfnReadCallback = pData->pfnReadCallback;
            endpointApi.endpoint[pData->index].pfnWriteCallback = pData->pfnWriteCallback;
            endpointApi.endpoint[pData->index].pfnInstantActionCallback = pData->pfnInstantActionCallback;

            while (true)
            {
                Span<DataVersion> dataVersion = Span<DataVersion>(pData->pDataVersionStorage, pData->dataVersionStorageLength);
                Span<const EmberAfDeviceType> deviceTypeList= Span<const EmberAfDeviceType>(pData->pDeviceTypeList, pData->deviceTypeListLength);
                CHIP_ERROR ret = emberAfSetDynamicEndpoint(
                    pData->index, endpointApi.currentEndpointId, pData->ep, 
                    dataVersion, deviceTypeList, 
                    pData->parentEndpointId);
                if (ret == CHIP_NO_ERROR)
                {
                    log_info("Added device %u: %s at dynamic endpoint %u", pData->index, pData->name, endpointApi.currentEndpointId);
                    endpointApi.currentEndpointId++;
                    return;
                }
                else //if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
                {
                    log_error("add failed for device %u: retVal %u", pData->index, ret);
                    // return;
                }
                // Handle wrap condition
                if (++endpointApi.currentEndpointId < endpointApi.firstDynamicEndpointId)
                {
                    endpointApi.currentEndpointId = endpointApi.firstDynamicEndpointId;
                }
            }
        }
        else
        {
            log_error("Index already in use: %u: %s", pData->index, pData->name);
        }
    }
    else
    {
        log_error("Index out of range: %u: %s", pData->index, pData->name);
    }
    log_error("Failed to add dynamic endpoint: No endpoints available!");
}
static void EndpointRemoveWorker(intptr_t context)
{
    uint16_t index = (uint16_t)(uint64_t)reinterpret_cast<void*>(context);
    log_info("Removing device: %u", index);

    if (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (endpointApi.endpoint[index].index == index)
        {
            memset(&endpointApi.endpoint[index], 0x00, sizeof(endpointApi.endpoint[index]));
            EndpointId ep = emberAfClearDynamicEndpoint(index);
            ChipLogProgress(DeviceLayer, "Removed device %u from dynamic endpoint %d", index, ep);

            // Silence complaints about unused ep when progress logging is disabled.
            UNUSED_VAR(ep);
        }
        else
        {
            log_error("Index not in use: %u", index);
        }
    }
    else
    {
        log_error("Index out of range: %u", index);
    }
}
static void EndpointReportUpdateWorker(intptr_t closure)
{
    auto path = reinterpret_cast<app::ConcreteAttributePath *>(closure);
    path->mEndpointId = emberAfEndpointFromIndex(path->mEndpointId);
    if(path->mEndpointId == kInvalidEndpointId){
        log_error("Invalid endpoint %u", index);
        Platform::Delete(path);
        return;
    }
    log_info("Update endpoint/cluster/attr %u/%04lX/%04lX", path->mEndpointId, path->mClusterId, path->mAttributeId);
    MatterReportingAttributeChangeCallback(*path);
    Platform::Delete(path);
}