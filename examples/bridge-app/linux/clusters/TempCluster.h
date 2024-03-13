#pragma once
#include <app/util/attribute-metadata.h>

#include "Cluster.h"
#include <app/util/attribute-storage.h>

#include <app/util/af-types.h>

class TempCluster : public Cluster
{
public:
    TempCluster() { _id = chip::app::Clusters::TemperatureMeasurement::Id; }
    int16_t _temp = 0;
    void UpdateTemp(float temp, uint16_t index);
    EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer) override;
    EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;

    static constexpr EmberAfAttributeMetadata levelAttrs[] = {
        { .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Id,
          .size          = 2,
          .attributeType = ZAP_TYPE(INT16S),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = 0xFFFD,
          .size          = 2,
          .attributeType = ZAP_TYPE(INT16U),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    };

    static constexpr EmberAfCluster cluster = { .clusterId            = chip::app::Clusters::TemperatureMeasurement::Id,
                                                .attributes           = levelAttrs,
                                                .attributeCount       = ArraySize(levelAttrs),
                                                .clusterSize          = 0,
                                                .mask                 = ZAP_CLUSTER_MASK(SERVER),
                                                .functions            = NULL,
                                                .acceptedCommandList  = nullptr,
                                                .generatedCommandList = nullptr,
                                                .eventList            = nullptr,
                                                .eventCount           = 0 };
};