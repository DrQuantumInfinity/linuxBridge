#pragma once
#include <app/util/attribute-metadata.h>

#include "Cluster.h"
#include <app/util/attribute-storage.h>

#include <app/util/af-types.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;

class ColourCluster : public Cluster
{
public:
    ColourCluster(void) { _id = ColorControl::Id; }
    uint8_t _hue;
    uint8_t _sat;
    uint16_t _temp;

    void SetColourHS(uint8_t hue, uint8_t sat, uint16_t index);
    void SetColourTemp(uint16_t temp, uint16_t index);

    EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer) override;
    EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;

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

    static constexpr EmberAfCluster hsCluster = { .clusterId            = ColorControl::Id,
                                                  .attributes           = hsAttrs,
                                                  .attributeCount       = ArraySize(hsAttrs),
                                                  .clusterSize          = 4,
                                                  .mask                 = ZAP_CLUSTER_MASK(SERVER),
                                                  .functions            = NULL,
                                                  .acceptedCommandList  = incomingCommandsHS,
                                                  .generatedCommandList = nullptr,
                                                  .eventList            = nullptr,
                                                  .eventCount           = 0 };

    static constexpr EmberAfCluster tempCluster = { .clusterId            = ColorControl::Id,
                                                    .attributes           = tempAttrs,
                                                    .attributeCount       = ArraySize(tempAttrs),
                                                    .clusterSize          = 4,
                                                    .mask                 = ZAP_CLUSTER_MASK(SERVER),
                                                    .functions            = NULL,
                                                    .acceptedCommandList  = incomingCommandsTemp,
                                                    .generatedCommandList = nullptr,
                                                    .eventList            = nullptr,
                                                    .eventCount           = 0 };

    static constexpr EmberAfCluster bothcluster = { .clusterId            = ColorControl::Id,
                                                    .attributes           = bothAttrs,
                                                    .attributeCount       = ArraySize(bothAttrs),
                                                    .clusterSize          = 6,
                                                    .mask                 = ZAP_CLUSTER_MASK(SERVER),
                                                    .functions            = NULL,
                                                    .acceptedCommandList  = incomingCommands,
                                                    .generatedCommandList = nullptr,
                                                    .eventList            = nullptr,
                                                    .eventCount           = 0 };
};