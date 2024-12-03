
#include "SerialFramerEspNow.h"

#include "assert.h"
#include "TimerTick.h"
#include "Crc16.h"
#include "EspNowData.h"
#include "uart.h"
#include "Log.h"
#include "utils.h"
#include "timer.h"
#include "transportEspNow.h"

#include <string.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define SERIAL_TX_MAX_SIZE              (255)
#define SERIAL_RX_MAX_SIZE              (300)
const uint32_t startKey =               0x1f5a3db9;

/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef struct
{
    uint32_t offset;
    uint8_t data[SERIAL_RX_MAX_SIZE];
}RX_FRAMING;

typedef struct
{
    UART_HANDLE uartHandle;
    RX_FRAMING rxFraming;
}SERIAL_TASK;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
static void SerialHandleRxCallback(UART_HANDLE uartHandle, void* pData, uint32_t dataLen);
static void SerialParseEspNowFrame(void);
static void SerialPrintDebug(bool tx, const uint8_t* pData, uint32_t dataLength);
/**************************************************************************
 *                                  Variables
 **************************************************************************/
SERIAL_TASK serial;
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
void SerialInit(void)
{
    UartInit();
    UART_PARAMS uartParams = {
        .baud = UART_BAUD_115200,
        .mode = UART_MODE_RX_TX,
        .stopBits = UART_STOP_BITS_1,
        .parity = UART_PARITY_NONE,
        .hardwareFlowCtrl = UART_HFC_DISABLED,
        .rxCallback = SerialHandleRxCallback,
    };
    serial.uartHandle = UartRegister("/dev/ttyEspNow", &uartParams);

    //Test code to inject a fake EspNow message
    /*
    TimerSleepMs(100);
    ESP_NOW_DATA espMsg;
    memset(&espMsg, 0x00, sizeof(espMsg));
    memset(espMsg.macAddr, 0xFF, sizeof(espMsg.macAddr));
    espMsg.type = ESP_NOW_DEVICE_TYPE_LIGHT_RGB;
    espMsg.data.lightRgb.onOff = true;
    espMsg.data.lightRgb.mode = ESP_NOW_DATA_LIGHT_RGB_MODE_STATIC;
    espMsg.data.lightRgb.hue = 122;
    espMsg.data.lightRgb.saturation = 100;
    espMsg.data.lightRgb.brightness = 75;

    const void* pData = &espMsg;
    uint8_t byteLength = OFFSET_OF(typeof(espMsg), data) + sizeof(espMsg.data.lightRgb);    
   
    SerialHandleRxCallback(serial.uartHandle, (uint8_t*)&startKey, sizeof(startKey));
    SerialHandleRxCallback(serial.uartHandle, (uint8_t*)&byteLength, sizeof(byteLength));
    SerialHandleRxCallback(serial.uartHandle, (uint8_t*)pData, byteLength);
    uint16_t crc = Crc16Block(0, pData, byteLength);
    SerialHandleRxCallback(serial.uartHandle, (uint8_t*)&crc, sizeof(crc));
    */
}
void SerialTransmit(const void* pData, uint32_t dataLength)
{
    uint8_t byteLength = (uint8_t)dataLength;
    ASSERT_PARAM(dataLength < BIT(sizeof(byteLength)*8));
    SerialPrintDebug(true, (uint8_t*)pData, dataLength);
    if (dataLength < SERIAL_TX_MAX_SIZE)
    {
        byteLength = (uint8_t)dataLength;
        UartWriteBlocking(serial.uartHandle, (uint8_t*)&startKey, sizeof(startKey));
        UartWriteBlocking(serial.uartHandle, (uint8_t*)&byteLength, sizeof(byteLength));
        UartWriteBlocking(serial.uartHandle, (uint8_t*)pData, byteLength);
        uint16_t crc = Crc16Block(0, pData, byteLength);
        UartWriteBlocking(serial.uartHandle, (uint8_t*)&crc, sizeof(crc));
    }
    else
    {
        log_error("Tx too long");
    }
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static void SerialHandleRxCallback(UART_HANDLE uartHandle, void* pData, uint32_t dataLen)
{
    char* pRxData = (char*)pData;
    while (dataLen)
    {
        uint8_t* pBuf = &serial.rxFraming.data[serial.rxFraming.offset];
        uint32_t spaceRemaining = sizeof(serial.rxFraming.data) - serial.rxFraming.offset;
        uint32_t copyLength = MIN(spaceRemaining, dataLen);
        memcpy(pBuf, pRxData, copyLength);
        
        SerialPrintDebug(false, pBuf, copyLength);

        serial.rxFraming.offset += copyLength;
        SerialParseEspNowFrame();

        dataLen -= copyLength;
        pRxData += copyLength;
    }
}
static void SerialParseEspNowFrame(void)
{
    uint8_t* pRxBuf = serial.rxFraming.data;
    uint32_t rxBufOffset = serial.rxFraming.offset;

    while (rxBufOffset) 
    {
        uint32_t consumeSize = 1;
        uint32_t parseOffset = 0;
        if (rxBufOffset >= sizeof(startKey)) 
        {
            if (memcmp(&startKey, &pRxBuf[parseOffset], sizeof(startKey)) == 0) 
            {
                parseOffset += sizeof(startKey);
                if (rxBufOffset >= parseOffset + sizeof(uint8_t)) 
                {
                    uint8_t len = pRxBuf[sizeof(startKey)];
                    parseOffset += sizeof(len);
                    if (rxBufOffset >= parseOffset + len + sizeof(uint16_t)) 
                    {
                        ESP_NOW_DATA *pEspData = (ESP_NOW_DATA *)&pRxBuf[parseOffset];
                        uint16_t crc = Crc16Block(0, &pRxBuf[parseOffset], len);
                        parseOffset += len;
                        if (memcmp(&crc, &pRxBuf[parseOffset], sizeof(crc)) == 0) 
                        {
                            parseOffset += sizeof(crc);
                            consumeSize = parseOffset;

                            log_info("Parsed EspNow message");
                            TransportEspNow::HandleSerialRx(pEspData, len);
                        }
                    } 
                    else 
                    {
                        break;
                    }
                }   
                else 
                {
                    break;
                }
            }
        } 
        else 
        {
            break;
        }

        rxBufOffset -= consumeSize;
        serial.rxFraming.offset = rxBufOffset;
        memmove(&pRxBuf[0], &pRxBuf[consumeSize], rxBufOffset);
    }
}
static void SerialPrintDebug(bool tx, const uint8_t* pData, uint32_t dataLength)
{
    static char debugBuf[300];
    debugBuf[0] = '\0';
    for (uint32_t i = 0; i < dataLength; i++)
    {
        sprintf(&debugBuf[strlen(debugBuf)], " %02X", pData[i]);
    }
    const char* pTxStr = tx ? "Tx" : "Rx";
    log_info("%s %lu:%s", pTxStr, dataLength, debugBuf);
}