#pragma once
#include <app/util/attribute-metadata.h>

#include "Cluster.h"
#include <app/util/attribute-storage.h>

#include <app/util/af-types.h>

class LevelControlCluster : public Cluster
{
public:
    LevelControlCluster(void) { _id = chip::app::Clusters::LevelControl::Id; }
    uint8_t _level=0;
    uint8_t _minLevel=1;
    uint8_t _maxLevel=255;
    void SetLevel(uint8_t level, uint16_t index);
    EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer) override;
    EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;
};

 EmberAfCluster ClusterLevelControlGetObject(void);