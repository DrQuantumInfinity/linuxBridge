#include "LevelControlCluster.h"
#include "EndpointApi.h"

#include <app/util/attribute-storage.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;

#define ZCL_LEVEL_CLUSTER_REVISION (5u)
void LevelControlCluster::SetLevel(uint8_t level, uint16_t index)
{
    _level = level;
    EndpointReportChange(index, LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id);
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