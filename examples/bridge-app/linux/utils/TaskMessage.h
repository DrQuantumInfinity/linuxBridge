#pragma once
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include <platform/CHIPDeviceLayer.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef enum
{
    MID_DUMMY,
    MID_SERIAL_TX,
    MID_SERIAL_RX,
    MID_MATTER_PROCESS_ESP_NOW,
    MID_COUNT
}MID;

typedef struct
{
    MID mid;
    TickType_t timeTick;
    uint32_t length;
    uint8_t data[];
}MSG_HEADER;

/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
void MsgPost(QueueHandle_t queue, MID mid, const void* pData, uint32_t dataLength);