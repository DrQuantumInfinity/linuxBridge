#pragma once
#include <app/util/attribute-metadata.h>

#include "Cluster.h"
#include <app/util/attribute-storage.h>

#include <app/util/af-types.h>

class ColourCluster : public Cluster
{
public:
    ColourCluster(void) { _id = chip::app::Clusters::ColorControl::Id; }
    uint8_t _hue;
    uint8_t _sat;
    uint16_t _temp;

    void SetColourHS(uint8_t hue, uint8_t sat, uint16_t index);
    void SetColourTemp(uint16_t temp, uint16_t index);

    EmberAfStatus Write(chip::AttributeId attributeId, uint8_t * buffer) override;
    EmberAfStatus Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;
};

EmberAfCluster ClusterColorControlGetObjectHS(void);
EmberAfCluster ClusterColorControlGetObjectTemp(void);
EmberAfCluster ClusterColorControlGetObjectBoth(void);