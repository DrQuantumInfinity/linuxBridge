#pragma once
#include <app/util/attribute-metadata.h>

#include "Cluster.h"
#include <app/util/attribute-storage.h>

#include <app/util/af-types.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;

#define DESCRIPTION_ATTR_ARRAY_LEN (254) // max

class DescriptorCluster : public Cluster
{
public:
    DescriptorCluster(void) { _id = Descriptor::Id; }
    EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer)
    {
        ESP_LOGE("cluster", "Base cluster Write called. This shouldn't happen");
        return EMBER_ZCL_STATUS_FAILURE;
    }

    EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
    {
        ESP_LOGE("cluster", "Base cluster Read called. This shouldn't happen");
        return EMBER_ZCL_STATUS_FAILURE;
    }
    static constexpr EmberAfAttributeMetadata descriptorAttrs[] = {
        { // device list
          .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = Descriptor::Attributes::DeviceTypeList::Id,
          .size          = DESCRIPTION_ATTR_ARRAY_LEN,
          .attributeType = ZAP_TYPE(ARRAY),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { // server list
          .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = Descriptor::Attributes::ServerList::Id,
          .size          = DESCRIPTION_ATTR_ARRAY_LEN,
          .attributeType = ZAP_TYPE(ARRAY),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { // client list
          .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = Descriptor::Attributes::ClientList::Id,
          .size          = DESCRIPTION_ATTR_ARRAY_LEN,
          .attributeType = ZAP_TYPE(ARRAY),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { // parts list
          .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = Descriptor::Attributes::PartsList::Id,
          .size          = DESCRIPTION_ATTR_ARRAY_LEN,
          .attributeType = ZAP_TYPE(ARRAY),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { // end?
          .defaultValue  = ZAP_EMPTY_DEFAULT(),
          .attributeId   = 0xFFFD,
          .size          = 2,
          .attributeType = ZAP_TYPE(INT16U),
          .mask          = ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
    };

    static constexpr EmberAfCluster cluster = { .clusterId            = Descriptor::Id,
                                                .attributes           = descriptorAttrs,
                                                .attributeCount       = ArraySize(descriptorAttrs),
                                                .clusterSize          = 1018,
                                                .mask                 = ZAP_CLUSTER_MASK(SERVER),
                                                .functions            = NULL,
                                                .acceptedCommandList  = nullptr,
                                                .generatedCommandList = nullptr,
                                                .eventList            = nullptr,
                                                .eventCount           = 0 };
};