#pragma once
#include <app/util/attribute-metadata.h>

#include "Cluster.h"
#include <app/util/attribute-storage.h>

#include <app/util/af-types.h>

#define NODE_LABEL_SIZE (32)

class BasicCluster : public Cluster
{
public:
    BasicCluster();
    virtual ~BasicCluster() = default;
    void SetReachable(bool reachable, uint16_t index);
    void SetName(const char * name, uint16_t index);
    bool _reachable = true;
    char _name[32] = "cat lol";
    EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer) override;
    EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;

    static constexpr EmberAfAttributeMetadata BasicAttrs[] = {
        { // node label
          .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = chip::app::Clusters::BridgedDeviceBasicInformation::Attributes::NodeLabel::Id,
          .size          = NODE_LABEL_SIZE,
          .attributeType = ZAP_TYPE(CHAR_STRING),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { // reachable
          .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = chip::app::Clusters::BridgedDeviceBasicInformation::Attributes::Reachable::Id,
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

    static constexpr EmberAfCluster cluster = { .clusterId            = chip::app::Clusters::BridgedDeviceBasicInformation::Id,
                                                .attributes           = BasicAttrs,
                                                .attributeCount       = ArraySize(BasicAttrs),
                                                .clusterSize          = 35,
                                                .mask                 = ZAP_CLUSTER_MASK(SERVER),
                                                .functions            = NULL,
                                                .acceptedCommandList  = nullptr,
                                                .generatedCommandList = nullptr,
                                                .eventList            = nullptr,
                                                .eventCount           = 0 };

private:
};