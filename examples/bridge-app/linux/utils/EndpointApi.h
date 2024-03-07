
#pragma once
//TODO: can these be deleted?
#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>
#include <stdint.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define ENDPOINT_NAME_LENGTH        (32)
#define ENDPOINT_LOCATION_LENGTH    (32)
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef EmberAfStatus (*GOOGLE_READ_CALLBACK)(void *pObject, chip::ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer, uint16_t maxReadLength);
typedef EmberAfStatus (*GOOGLE_WRITE_CALLBACK)(void *pObject, chip::ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer);
typedef bool (*GOOGLE_INSTANT_ACTION_CALLBACK)(chip::app::CommandHandler* commandObj, const chip::app::ConcreteCommandPath & commandPath, const chip::app::Clusters::Actions::Commands::InstantAction::DecodableType & commandData);
typedef struct
{
    uint16_t index;
    void *pObject;
    GOOGLE_READ_CALLBACK pfnReadCallback;
    GOOGLE_WRITE_CALLBACK pfnWriteCallback;
    GOOGLE_INSTANT_ACTION_CALLBACK pfnInstantActionCallback;

    char name[ENDPOINT_NAME_LENGTH];
    char location[ENDPOINT_LOCATION_LENGTH];
    const EmberAfEndpointType* ep;
    const EmberAfDeviceType *pDeviceTypeList;
    uint32_t deviceTypeListLength;
    chip::DataVersion *pDataVersionStorage;
    uint32_t dataVersionStorageLength;
    chip::EndpointId parentEndpointId;
}ENDPOINT_DATA;

    
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
void EndpointApiInit(void);
void EndpointAdd(ENDPOINT_DATA *pData);
void EndpointRemove(uint16_t index);
void EndpointReportChange(uint16_t index, chip::ClusterId cluster, chip::AttributeId attribute);