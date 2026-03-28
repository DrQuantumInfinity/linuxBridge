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

// The new SDK requires READABLE flag on attributes for reads to work.
// Our bridge attributes are all external+readable, so override the mask
// used in all cluster headers to include both flags.
#undef MATTER_ATTRIBUTE_FLAG_EXTERNAL_STORAGE
#define MATTER_ATTRIBUTE_FLAG_EXTERNAL_STORAGE (0x10 | 0x20) /* EXTERNAL_STORAGE | READABLE */

class Cluster
{
public:
    virtual ~Cluster() = default;
    chip::ClusterId _id = 0;
    chip::ClusterId GetId() { return _id; }
    virtual Status Write(chip::AttributeId attributeId, uint8_t * buffer) =0;
    virtual Status Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) = 0;
};
