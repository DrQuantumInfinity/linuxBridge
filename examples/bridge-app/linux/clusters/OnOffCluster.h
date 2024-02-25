#pragma once
#include "Cluster.h"
#include <app/util/attribute-metadata.h>

#include <app/util/attribute-storage.h>

#include <app/util/af-types.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;

class OnOffCluster : public Cluster
{
public:
    OnOffCluster(void) { _id = OnOff::Id; }
    bool _isOn = false;
    void SetOn(bool on, uint16_t index);
    EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer) override;
    EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;

    static constexpr EmberAfAttributeMetadata onOffAttrs[] = {
        { // onOff attribute
          .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = OnOff::Attributes::OnOff::Id,
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
    
    static constexpr CommandId incomingCommands[] = {
        app::Clusters::OnOff::Commands::Off::Id,
        app::Clusters::OnOff::Commands::On::Id,
        app::Clusters::OnOff::Commands::Toggle::Id,
        app::Clusters::OnOff::Commands::OffWithEffect::Id,
        app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id,
        app::Clusters::OnOff::Commands::OnWithTimedOff::Id,
        kInvalidCommandId,
    };

    static constexpr EmberAfCluster cluster = { .clusterId            = OnOff::Id,
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