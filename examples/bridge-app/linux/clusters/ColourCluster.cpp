#include "ColourCluster.h"
#include "EndpointApi.h"

#include <app-common/zap-generated/callback.h>
#include <app/util/attribute-storage.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;

/**************************************************************************
 *                                  Constants
 **************************************************************************/

#define ZCL_COLOR_CLUSTER_REVISION (6u)

const EmberAfGenericClusterFunction chipFuncArrayColorServer[] = {
    (EmberAfGenericClusterFunction) emberAfColorControlClusterServerInitCallback,
    (EmberAfGenericClusterFunction) MatterColorControlClusterServerShutdownCallback,
};
static constexpr EmberAfAttributeMetadata hsAttrs[] = {
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = ColorControl::Attributes::CurrentHue::Id,
      .size          = 1,
      .attributeType = ZAP_TYPE(INT8U),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = ColorControl::Attributes::CurrentSaturation::Id,
      .size          = 1,
      .attributeType = ZAP_TYPE(INT8U),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = ColorControl::Attributes::FeatureMap::Id,
      .size          = 4,
      .attributeType = ZAP_TYPE(BITMAP32),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = 0xFFFD,
      .size          = 2,
      .attributeType = ZAP_TYPE(INT16U),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
};

static constexpr EmberAfAttributeMetadata tempAttrs[] = {
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = ColorControl::Attributes::ColorTemperatureMireds::Id,
      .size          = 2,
      .attributeType = ZAP_TYPE(INT16U),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = ColorControl::Attributes::FeatureMap::Id,
      .size          = 4,
      .attributeType = ZAP_TYPE(BITMAP32),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = 0xFFFD,
      .size          = 2,
      .attributeType = ZAP_TYPE(INT16U),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
};

static constexpr EmberAfAttributeMetadata bothAttrs[] = {
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = ColorControl::Attributes::CurrentHue::Id,
      .size          = 1,
      .attributeType = ZAP_TYPE(INT8U),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = ColorControl::Attributes::CurrentSaturation::Id,
      .size          = 1,
      .attributeType = ZAP_TYPE(INT8U),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = ColorControl::Attributes::ColorTemperatureMireds::Id,
      .size          = 2,
      .attributeType = ZAP_TYPE(INT16U),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = ColorControl::Attributes::FeatureMap::Id,
      .size          = 4,
      .attributeType = ZAP_TYPE(BITMAP32),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    { .defaultValue  = ZAP_EMPTY_DEFAULT(),
      .attributeId   = 0xFFFD,
      .size          = 2,
      .attributeType = ZAP_TYPE(INT16U),
      .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
};

static constexpr CommandId incomingCommands[] = {
    app::Clusters::ColorControl::Commands::MoveToHue::Id,
    app::Clusters::ColorControl::Commands::MoveHue::Id,
    app::Clusters::ColorControl::Commands::StepHue::Id,
    app::Clusters::ColorControl::Commands::MoveToSaturation::Id,
    app::Clusters::ColorControl::Commands::MoveSaturation::Id,
    app::Clusters::ColorControl::Commands::StepSaturation::Id,
    app::Clusters::ColorControl::Commands::MoveToHueAndSaturation::Id,
    app::Clusters::ColorControl::Commands::MoveToColor::Id,
    app::Clusters::ColorControl::Commands::MoveColor::Id,
    app::Clusters::ColorControl::Commands::StepColor::Id,
    app::Clusters::ColorControl::Commands::MoveToColorTemperature::Id,
    app::Clusters::ColorControl::Commands::EnhancedMoveToHue::Id,
    app::Clusters::ColorControl::Commands::EnhancedMoveHue::Id,
    app::Clusters::ColorControl::Commands::EnhancedStepHue::Id,
    app::Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Id,
    app::Clusters::ColorControl::Commands::ColorLoopSet::Id,
    app::Clusters::ColorControl::Commands::StopMoveStep::Id,
    app::Clusters::ColorControl::Commands::MoveColorTemperature::Id,
    app::Clusters::ColorControl::Commands::StepColorTemperature::Id,
    kInvalidCommandId,
};

static constexpr CommandId incomingCommandsHS[] = {
    app::Clusters::ColorControl::Commands::MoveToHue::Id,
    app::Clusters::ColorControl::Commands::MoveHue::Id,
    app::Clusters::ColorControl::Commands::StepHue::Id,
    app::Clusters::ColorControl::Commands::MoveToSaturation::Id,
    app::Clusters::ColorControl::Commands::MoveSaturation::Id,
    app::Clusters::ColorControl::Commands::StepSaturation::Id,
    app::Clusters::ColorControl::Commands::MoveToHueAndSaturation::Id,
    app::Clusters::ColorControl::Commands::MoveToColor::Id,
    app::Clusters::ColorControl::Commands::MoveColor::Id,
    app::Clusters::ColorControl::Commands::StepColor::Id,
    app::Clusters::ColorControl::Commands::EnhancedMoveToHue::Id,
    app::Clusters::ColorControl::Commands::EnhancedMoveHue::Id,
    app::Clusters::ColorControl::Commands::EnhancedStepHue::Id,
    app::Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Id,
    app::Clusters::ColorControl::Commands::ColorLoopSet::Id,
    app::Clusters::ColorControl::Commands::StopMoveStep::Id,
    kInvalidCommandId,
};

static constexpr CommandId incomingCommandsTemp[] = {
    app::Clusters::ColorControl::Commands::MoveToColorTemperature::Id,
    app::Clusters::ColorControl::Commands::MoveColorTemperature::Id,
    app::Clusters::ColorControl::Commands::StepColorTemperature::Id,
    kInvalidCommandId,
};

static EmberAfCluster hsCluster = { .clusterId      = ColorControl::Id,
                                              .attributes     = hsAttrs,
                                              .attributeCount = ArraySize(hsAttrs),
                                              .clusterSize    = 0, // Assigned dynamically upon GetObject()
                                              .mask           = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION) |
                                                  ZAP_CLUSTER_MASK(SHUTDOWN_FUNCTION),
                                              .functions            = chipFuncArrayColorServer,
                                              .acceptedCommandList  = incomingCommandsHS,
                                              .generatedCommandList = nullptr,
                                              .eventList            = nullptr,
                                              .eventCount           = 0 };

static  EmberAfCluster tempCluster = { .clusterId      = ColorControl::Id,
                                                .attributes     = tempAttrs,
                                                .attributeCount = ArraySize(tempAttrs),
                                                .clusterSize    = 0, // Assigned dynamically upon GetObject()
                                                .mask           = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION) |
                                                    ZAP_CLUSTER_MASK(SHUTDOWN_FUNCTION),
                                                .functions            = chipFuncArrayColorServer,
                                                .acceptedCommandList  = incomingCommandsTemp,
                                                .generatedCommandList = nullptr,
                                                .eventList            = nullptr,
                                                .eventCount           = 0 };

static  EmberAfCluster bothCluster = { .clusterId      = ColorControl::Id,
                                                .attributes     = bothAttrs,
                                                .attributeCount = ArraySize(bothAttrs),
                                                .clusterSize    = 0, // Assigned dynamically upon GetObject()
                                                .mask           = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION) |
                                                    ZAP_CLUSTER_MASK(SHUTDOWN_FUNCTION),
                                                .functions            = chipFuncArrayColorServer,
                                                .acceptedCommandList  = incomingCommands,
                                                .generatedCommandList = nullptr,
                                                .eventList            = nullptr,
                                                .eventCount           = 0 };
/**************************************************************************
 *                                  Class Functions
 **************************************************************************/
void ColourCluster::SetColourHS(uint8_t hue, uint8_t sat, uint16_t index)
{
    _hue = hue;
    _sat = sat;
    EndpointReportChange(index, ColorControl::Id, ColorControl::Attributes::CurrentHue::Id);
    EndpointReportChange(index, ColorControl::Id, ColorControl::Attributes::CurrentSaturation::Id);
}

void ColourCluster::SetColourTemp(uint16_t temp, uint16_t index)
{
    _temp = temp;
    EndpointReportChange(index, ColorControl::Id, ColorControl::Attributes::ColorTemperatureMireds::Id);
}

EmberAfStatus ColourCluster::Write(chip::AttributeId attributeId, uint8_t * buffer)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (attributeId)
    {
    case ColorControl::Attributes::CurrentHue::Id:
        _hue = (uint8_t) buffer[0];
        break;
    case ColorControl::Attributes::CurrentSaturation::Id:
        _sat = (uint8_t) buffer[0];
        break;
    case ColorControl::Attributes::ColorTemperatureMireds::Id:
        memcpy(&_temp, buffer, sizeof(_temp));
        break;
    default:
        status = EMBER_ZCL_STATUS_FAILURE;
        break;
    }
    return status;
}

EmberAfStatus ColourCluster::Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    uint16_t rev         = ZCL_COLOR_CLUSTER_REVISION;
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (attributeId)
    {
    case ColorControl::Attributes::ClusterRevision::Id:
        memcpy(buffer, &rev, sizeof(rev));
        break;
    case ColorControl::Attributes::CurrentHue::Id:
        buffer[0] = _hue;
        break;
    case ColorControl::Attributes::CurrentSaturation::Id:
        buffer[0] = _sat;
        break;
        // TODO set correctly
    case ColorControl::Attributes::FeatureMap::Id:
        buffer[0] = 0x1F;
        break;
    case ColorControl::Attributes::ColorTemperatureMireds::Id:
        if (maxReadLength == 2)
        {
            memcpy(buffer, &_temp, maxReadLength);
        }
        else
        {
            status = EMBER_ZCL_STATUS_FAILURE;
        }
        break;
    default:
        status = EMBER_ZCL_STATUS_FAILURE;
        break;
    }
    return status;
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
EmberAfCluster InnerClusterColorControlGetObject(EmberAfCluster * cluster)
{
    if (cluster->clusterSize == 0) // only perform this the first time
    {
        for (int i = 0; i < cluster->attributeCount; i++)
        {
            cluster->clusterSize += cluster->attributes[i].size;
        }
    }
    return *cluster;
}
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
EmberAfCluster ClusterColorControlGetObjectHS(void)
{
    return InnerClusterColorControlGetObject(&hsCluster);
}
EmberAfCluster ClusterColorControlGetObjectTemp(void)
{
    return InnerClusterColorControlGetObject(&tempCluster);
}
EmberAfCluster ClusterColorControlGetObjectBoth(void)
{
    return InnerClusterColorControlGetObject(&bothCluster);
}
