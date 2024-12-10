#include "LevelControlCluster.h"
#include "EndpointApi.h"

#include <app-common/zap-generated/callback.h>
#include <app/util/attribute-storage.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;

/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define ZCL_LEVEL_CLUSTER_REVISION (5u)

const EmberAfGenericClusterFunction chipFuncArrayLevelControlServer[] = {
    (EmberAfGenericClusterFunction) emberAfLevelControlClusterServerInitCallback,
    (EmberAfGenericClusterFunction) MatterLevelControlClusterServerShutdownCallback,
};

static constexpr EmberAfAttributeMetadata attributes[] = {
    {   .defaultValue  = ZAP_EMPTY_DEFAULT(),
        .attributeId   = LevelControl::Attributes::CurrentLevel::Id,
        .size          = 1,
        .attributeType = ZAP_TYPE(INT8U),
        .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) 
    },
    {   .defaultValue  = ZAP_EMPTY_DEFAULT(),
        .attributeId   = LevelControl::Attributes::MinLevel::Id,
        .size          = 1,
        .attributeType = ZAP_TYPE(INT8U),
        .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) 
    },
    {   .defaultValue  = ZAP_EMPTY_DEFAULT(),
        .attributeId   = LevelControl::Attributes::MaxLevel::Id,
        .size          = 1,
        .attributeType = ZAP_TYPE(INT8U),
        .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) 
    },
    {   .defaultValue  = ZAP_EMPTY_DEFAULT(),
        .attributeId   = LevelControl::Attributes::FeatureMap::Id,
        .size          = 4,
        .attributeType = ZAP_TYPE(BITMAP32),
        .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) 
    },
    {   .defaultValue  = ZAP_EMPTY_DEFAULT(),
        .attributeId   = 0xFFFD,
        .size          = 2,
        .attributeType = ZAP_TYPE(INT16U),
        .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) 
    },
};

static constexpr CommandId acceptedCommandList[] = {
    app::Clusters::LevelControl::Commands::MoveToLevel::Id,
    app::Clusters::LevelControl::Commands::Move::Id,
    app::Clusters::LevelControl::Commands::Step::Id,
    app::Clusters::LevelControl::Commands::Stop::Id,
    app::Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id,
    app::Clusters::LevelControl::Commands::MoveWithOnOff::Id,
    app::Clusters::LevelControl::Commands::StepWithOnOff::Id,
    app::Clusters::LevelControl::Commands::StopWithOnOff::Id,
    app::Clusters::LevelControl::Commands::MoveToClosestFrequency::Id,
    kInvalidCommandId,
};

static EmberAfCluster cluster = { 
    .clusterId            = LevelControl::Id,
    .attributes           = attributes,
    .attributeCount       = ArraySize(attributes),
    .clusterSize          = 0, //Assigned dynamically upon GetObject()
    .mask                 = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION) | ZAP_CLUSTER_MASK(SHUTDOWN_FUNCTION), 
    .functions            = chipFuncArrayLevelControlServer,
    .acceptedCommandList  = acceptedCommandList,
    .generatedCommandList = nullptr,
    .eventList            = nullptr,
    .eventCount           = 0 
};
/**************************************************************************
 *                                  Class Functions
 **************************************************************************/
void LevelControlCluster::SetLevel(uint8_t level, uint16_t index)
{
    if (_level != level)
    {
        _level = level;
        EndpointReportChange(index, LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id);
    }
}

EmberAfStatus LevelControlCluster::Write(chip::AttributeId attributeId, uint8_t * buffer)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (attributeId)
    {
    case LevelControl::Attributes::CurrentLevel::Id:
        _level = (uint8_t) buffer[0];
        break;
    default:
        status = EMBER_ZCL_STATUS_FAILURE;
        break;
    }
    return status;
}

EmberAfStatus LevelControlCluster::Read(chip::AttributeId attributeId, uint8_t* buffer, uint16_t maxReadLength){
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    if ((attributeId == LevelControl::Attributes::CurrentLevel::Id) && (maxReadLength == 1))
    {
        *buffer = _level;
    }
    else if ((attributeId == LevelControl::Attributes::MinLevel::Id) && (maxReadLength == 1))
    {
        *buffer = _minLevel;
    }
    else if ((attributeId == LevelControl::Attributes::MaxLevel::Id) && (maxReadLength == 1))
    {
        *buffer = _maxLevel;
    }
    else if ((attributeId == LevelControl::Attributes::FeatureMap::Id) && (maxReadLength == 4))
    {
        *buffer = (1<<0) | (1<<1);
    }
    else if ((attributeId == LevelControl::Attributes::ClusterRevision::Id) && (maxReadLength == 2))
    {
        uint16_t rev = ZCL_LEVEL_CLUSTER_REVISION;
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
 EmberAfCluster ClusterLevelControlGetObject(void)
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