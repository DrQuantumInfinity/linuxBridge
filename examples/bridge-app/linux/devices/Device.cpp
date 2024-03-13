/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "Device.h"
#include "EndpointApi.h"
using namespace ::chip;

//static const char * TAG = "Device";
Device::Device(void)
{
    for (_index = 0; _index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; _index++)
    {
        if (!_indexList[_index])
        {
            _indexList[_index] = true;
            break;
        }
    }
    AddCluster(&basicCluster);
    basicCluster._reachable = true;
}
Device::~Device(void)
{
    _indexList[_index] = false;
}

uint16_t Device::GetIndex(void)
{
    return _index;
}

void Device::AddCluster(Cluster * newCluster)
{
    _clusters.push_back(newCluster);
}

EmberAfStatus Device::GoogleReadCallback(void * pObject, ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata,
                                         uint8_t * buffer, uint16_t maxReadLength)
{
    Device * pDevice = (Device *) pObject;
    return pDevice->ReadCluster(clusterId, attributeMetadata, buffer, maxReadLength);
}

EmberAfStatus Device::GoogleWriteCallback(void * pObject, ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata,
                                          uint8_t * buffer)
{
    Device * pDevice     = (Device *) pObject;
    EmberAfStatus status = pDevice->WriteCluster(clusterId, attributeMetadata, buffer);
    pDevice->_pTransportLayer->Send(pDevice, clusterId, attributeMetadata, buffer);
    return status;
}

EmberAfStatus Device::ReadCluster(ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                  uint16_t maxReadLength)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    if (basicCluster._reachable)
    {
        for (auto & cluster : _clusters)
        {
            if (cluster->_id == clusterId)
            {
                status = cluster->Read(attributeMetadata->attributeId, buffer, maxReadLength);
                break;
            }
        }
    }
    return status;
}
EmberAfStatus Device::WriteCluster(ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    if (basicCluster._reachable)
    {
        for (auto & cluster : _clusters)
        {
            if (cluster->_id == clusterId)
            {
                status = cluster->Write(attributeMetadata->attributeId, buffer);
                break;
            }
        }
    }
    return status;
}