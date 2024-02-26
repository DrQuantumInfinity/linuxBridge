
#pragma once

#include <functional>
#include <stdbool.h>
#include <stdint.h>

#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>
using namespace ::chip;
/**************************************************************************
 *                                  Constants
 **************************************************************************/
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
class Device;
class TransportLayer
{
public:
    virtual void Send(const Device* pDevice, ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer) = 0;

protected:

private:

};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/