/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <functional>
#include <stdbool.h>
#include <stdint.h>
#include "Cluster.h"
#include "BasicCluster.h"
#include "transportLayer.h"

#include "EndpointApi.h"
#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>
/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define DEVICE_INVALID              ((uint16_t)65535)
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/

class Device
{
public:
    Device(uint16_t index);
    Device(void);
    uint16_t GetIndex(void);
    EmberAfStatus ReadCluster(chip::ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer, uint16_t maxReadLength);
    EmberAfStatus WriteCluster(chip::ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer);
    BasicCluster basicCluster;
    static EmberAfStatus GoogleWriteCallback(void * pObject, chip::ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata,
                                         uint8_t * buffer);
    static EmberAfStatus GoogleReadCallback(void * pObject, chip::ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata,
                                        uint8_t * buffer, uint16_t maxReadLength);
    
    protected:
    void AddCluster(Cluster* newCluster);
    TransportLayer* _pTransportLayer;

private:
    uint16_t _index;
    static inline uint16_t _lastIndex = 0;
    std::vector<Cluster*> _clusters;
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/