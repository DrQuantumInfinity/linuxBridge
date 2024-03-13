#pragma once
#include "Cluster.h"
#include <app/util/attribute-metadata.h>

#include <app/util/attribute-storage.h>

#include <app/util/af-types.h>

class ModeCluster : public Cluster
{
public:
    ModeCluster(void) { _id = chip::app::Clusters::OnOff::Id; }
    bool _isOn;
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

    static constexpr EmberAfCluster cluster = { .clusterId            = chip::app::Clusters::OnOff::Id,
                                                .attributes           = onOffAttrs,
                                                .attributeCount       = ArraySize(onOffAttrs),
                                                .clusterSize          = 0,
                                                .mask                 = ZAP_CLUSTER_MASK(SERVER),
                                                .functions            = NULL,
                                                .acceptedCommandList  = onOffIncomingCommands,
                                                .generatedCommandList = nullptr,
                                                .eventList            = nullptr,
                                                .eventCount           = 0 };
};