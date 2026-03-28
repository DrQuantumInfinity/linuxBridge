#pragma once
#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>
#include "Log.h"

using Status = chip::Protocols::InteractionModel::Status;
#ifndef ArraySize
#define ArraySize MATTER_ARRAY_SIZE
#endif

class Cluster
{
public:
    virtual ~Cluster() = default;
    chip::ClusterId _id = 0;
    chip::ClusterId GetId() { return _id; }
    virtual Status Write(chip::AttributeId attributeId, uint8_t * buffer) =0;
    virtual Status Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) = 0;
};
