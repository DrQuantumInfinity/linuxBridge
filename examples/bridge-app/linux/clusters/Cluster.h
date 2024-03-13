#pragma once
#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage.h>
#include "Log.h"

class Cluster
{
public:
    virtual ~Cluster() = default;
    chip::ClusterId _id = 0;
    chip::ClusterId GetId() { return _id; }
    virtual EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer) =0;
    virtual EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) = 0;
};