#pragma once
#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage.h>
#include "esp_log.h"
using namespace ::chip;
using namespace ::chip::app::Clusters;

class Cluster
{
public:
    virtual ~Cluster() = default;
    ClusterId _id = 0;
    ClusterId GetId() { return _id; }
    virtual EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer) =0;
    virtual EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) = 0;
};