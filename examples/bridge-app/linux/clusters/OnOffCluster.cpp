#include "OnOffCluster.h"
#include "EndpointApi.h"

#include <app-common/zap-generated/callback.h>
#include <lib/support/ZclString.h>
/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define ZCL_ON_OFF_CLUSTER_REVISION (4u)
using namespace ::chip;
using namespace ::chip::app::Clusters;

const EmberAfGenericClusterFunction chipFuncArrayOnOffServer[] = {
    (EmberAfGenericClusterFunction) emberAfOnOffClusterServerInitCallback,
    (EmberAfGenericClusterFunction) MatterOnOffClusterServerShutdownCallback,
};

static constexpr EmberAfAttributeMetadata attributes[] = {
    { // onOff attribute
        .defaultValue  = ZAP_EMPTY_DEFAULT(),
        .attributeId   = OnOff::Attributes::OnOff::Id,
        .size          = 1,
        .attributeType = ZAP_TYPE(BOOLEAN),
        .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    { // ClusterRevision attribute
        .defaultValue  = ZAP_EMPTY_DEFAULT(),
        .attributeId   = 0xFFFD,
        .size          = 2,
        .attributeType = ZAP_TYPE(INT16U),
        .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
};
static constexpr CommandId acceptedCommandList[] = {
    app::Clusters::OnOff::Commands::Off::Id,
    app::Clusters::OnOff::Commands::On::Id,
    app::Clusters::OnOff::Commands::Toggle::Id,
    app::Clusters::OnOff::Commands::OffWithEffect::Id,
    app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id,
    app::Clusters::OnOff::Commands::OnWithTimedOff::Id,
    kInvalidCommandId,
};

static EmberAfCluster cluster = { 
    .clusterId            = OnOff::Id,
    .attributes           = attributes,
    .attributeCount       = ArraySize(attributes),
    .clusterSize          = 0, //Assigned dynamically upon GetObject()
    .mask                 = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION) | ZAP_CLUSTER_MASK(SHUTDOWN_FUNCTION),
    .functions            = chipFuncArrayOnOffServer,
    .acceptedCommandList  = acceptedCommandList,
    .generatedCommandList = nullptr,
    .eventList            = nullptr,
    .eventCount           = 0 
};
/**************************************************************************
 *                                  Class Functions
 **************************************************************************/
void OnOffCluster::SetOn(bool on, uint16_t index)
{
    if (_isOn != on)
{
    _isOn = on;
    EndpointReportChange(index, OnOff::Id, OnOff::Attributes::OnOff::Id);
}
}

EmberAfStatus OnOffCluster::Write(chip::AttributeId attributeId, uint8_t* buffer)
{
    log_info("OnOffCluster Write called");
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (attributeId)
    {
        case OnOff::Attributes::OnOff::Id:  _isOn = (bool)buffer[0];   break;
        default:                            status = EMBER_ZCL_STATUS_FAILURE;      break;
    }
    return status;
}

EmberAfStatus OnOffCluster::Read(chip::AttributeId attributeId, uint8_t* buffer, uint16_t maxReadLength){

    log_info("OnOffCluster Read called");
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    if ((attributeId == OnOff::Attributes::OnOff::Id) && (maxReadLength == 1))
    {
        *buffer = _isOn ? 1 : 0;
    }
    else if ((attributeId == OnOff::Attributes::ClusterRevision::Id) && (maxReadLength == 2))
    {
        uint16_t rev = ZCL_ON_OFF_CLUSTER_REVISION;
        memcpy(buffer, &rev, sizeof(rev));
    }
    else
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }
    return status;
}
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
 EmberAfCluster ClusterOnOffGetObject(void)
{
    if (cluster.clusterSize == 0) //only perform this the first time
    {
        for (int i = 0; i < cluster.attributeCount; i++)
        {
            cluster.clusterSize += cluster.attributes[i].size;
        }
    }
    return cluster;
}
