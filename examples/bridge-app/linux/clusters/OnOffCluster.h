#pragma once
#include "Cluster.h"
#include <app/util/attribute-metadata.h>

#include <app/util/attribute-storage.h>

#include <app/util/af-types.h>

class OnOffCluster : public Cluster
{
public:
    OnOffCluster(void) { _id = chip::app::Clusters::OnOff::Id; }
    bool _isOn = false;
    void SetOn(bool on, uint16_t index);
    EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer) override;
    EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;

    static constexpr EmberAfAttributeMetadata onOffAttrs[] = {
        { // onOff attribute
          .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = chip::app::Clusters::OnOff::Attributes::OnOff::Id,
          .size          = 1,
          .attributeType = ZAP_TYPE(BOOLEAN),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { // end?
          .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = 0xFFFD,
          .size          = 2,
          .attributeType = ZAP_TYPE(INT16U),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    };
    
    static constexpr chip::CommandId incomingCommands[] = {
        chip::app::Clusters::OnOff::Commands::Off::Id,
        chip::app::Clusters::OnOff::Commands::On::Id,
        chip::app::Clusters::OnOff::Commands::Toggle::Id,
        chip::app::Clusters::OnOff::Commands::OffWithEffect::Id,
        chip::app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id,
        chip::app::Clusters::OnOff::Commands::OnWithTimedOff::Id,
        chip::kInvalidCommandId,
    };

    static constexpr EmberAfCluster cluster = { .clusterId            = chip::app::Clusters::OnOff::Id,
                                                .attributes           = onOffAttrs,
                                                .attributeCount       = ArraySize(onOffAttrs),
                                                .clusterSize          = 3,
                                                .mask                 = ZAP_CLUSTER_MASK(SERVER),
                                                .functions            = NULL,
                                                .acceptedCommandList  = incomingCommands,
                                                .generatedCommandList = nullptr,
                                                .eventList            = nullptr,
                                                .eventCount           = 0 };
};