#pragma once
#include <app/util/attribute-metadata.h>

#include "Cluster.h"
#include <app/util/attribute-storage.h>

#include <app/util/af-types.h>

class LevelControlCluster : public Cluster
{
public:
    LevelControlCluster(void) { _id = chip::app::Clusters::LevelControl::Id; }
    uint8_t _level;
    uint8_t _minLevel;
    uint8_t _maxLevel;
    void SetLevel(uint8_t level, uint16_t index);
    EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer) override;
    EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;

    static constexpr EmberAfAttributeMetadata levelAttrs[] = {
        { .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = chip::app::Clusters::LevelControl::Attributes::CurrentLevel::Id,
          .size          = 1,
          .attributeType = ZAP_TYPE(INT8U),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },/*
        { .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = LevelControl::Attributes::MinLevel::Id,
          .size          = 1,
          .attributeType = ZAP_TYPE(INT8U),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = LevelControl::Attributes::MaxLevel::Id,
          .size          = 1,
          .attributeType = ZAP_TYPE(INT8U),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },*/
        { .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = chip::app::Clusters::LevelControl::Attributes::FeatureMap::Id,
          .size          = 4,
          .attributeType = ZAP_TYPE(BITMAP32),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = 0xFFFD,
          .size          = 2,
          .attributeType = ZAP_TYPE(INT16U),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    };

    static constexpr chip::CommandId incomingCommands[] = {
        chip::app::Clusters::LevelControl::Commands::MoveToLevel::Id,
        chip::app::Clusters::LevelControl::Commands::Move::Id,
        chip::app::Clusters::LevelControl::Commands::Step::Id,
        chip::app::Clusters::LevelControl::Commands::Stop::Id,
        chip::app::Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id,
        chip::app::Clusters::LevelControl::Commands::MoveWithOnOff::Id,
        chip::app::Clusters::LevelControl::Commands::StepWithOnOff::Id,
        chip::app::Clusters::LevelControl::Commands::StopWithOnOff::Id,
        chip::app::Clusters::LevelControl::Commands::MoveToClosestFrequency::Id,
        chip::kInvalidCommandId,
    };

    static constexpr EmberAfCluster cluster = { .clusterId            = chip::app::Clusters::LevelControl::Id,
                                                .attributes           = levelAttrs,
                                                .attributeCount       = ArraySize(levelAttrs),
                                                .clusterSize          = 3,
                                                .mask                 = ZAP_CLUSTER_MASK(SERVER),
                                                .functions            = NULL,
                                                .acceptedCommandList  = incomingCommands,
                                                .generatedCommandList = nullptr,
                                                .eventList            = nullptr,
                                                .eventCount           = 0 };
};