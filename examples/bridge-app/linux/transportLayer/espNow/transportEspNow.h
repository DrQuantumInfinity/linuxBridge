
#pragma once

#include <functional>
#include <stdbool.h>
#include <stdint.h>

#include "transportLayer.h"
#include "EspNowData.h"
#include "DeviceList.h"

#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>
#include "PersistDevList.h"
/**************************************************************************
 *                                  Constants
 **************************************************************************/
 /**************************************************************************
  *                                  Macros
  **************************************************************************/
  /**************************************************************************
   *                                  Types
   **************************************************************************/

class TransportEspNow : public TransportLayer
{
public:
    TransportEspNow(ESP_NOW_DEVICE_TYPE type, const uint8_t* pMacAddr);
    virtual ~TransportEspNow(void);
    static void Init(void);
    static void HandleSerialRx(const ESP_NOW_DATA* pData, uint32_t dataLength);

protected:
    void Send(const Device* pDevice, chip::ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer);

private:
    ESP_NOW_DATA _data;
    static DeviceList _deviceList;
    struct Private;
    static PersistDevList _persistList;
};
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
