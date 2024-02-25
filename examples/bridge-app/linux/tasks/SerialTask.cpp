
#include "SerialTask.h"
#include "MatterTask.h"
#include "TaskMessage.h"
#include "TimerTick.h"
#include "Crc16.h"
#include "EspNowData.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"

using namespace chip;

/**************************************************************************
 *                                  Constants
 **************************************************************************/
static const char * TAG =               "serial-task";
#define SERIAL_TASK_MSG_QUEUE_DEPTH     (10)
#define SERIAL_TASK_NAME                "SERIAL"
#define SERIAL_TASK_STACK_SIZE          (2000)

#if SOC_UART_NUM > 2
#define SERIAL_UART                     (UART_NUM_2)
#else
#define SERIAL_UART                     (UART_NUM_1)
#endif
#define SERIAL_TX_MAX_SIZE              (300)
#define SERIAL_TX_QUEUE_DEPTH           (10)
#define SERIAL_RX_MAX_SIZE              (300)
const uint32_t startKey =               0x1f5a3db9;

// Application-defined error codes in the CHIP_ERROR space.
#define ERROR_EVENT_QUEUE_FAILED        CHIP_APPLICATION_ERROR(0x01)
#define ERROR_CREATE_TASK_FAILED        CHIP_APPLICATION_ERROR(0x02)
#define ERROR_UNHANDLED_EVENT           CHIP_APPLICATION_ERROR(0x03)
#define ERROR_CREATE_TIMER_FAILED       CHIP_APPLICATION_ERROR(0x04)
#define ERROR_START_TIMER_FAILED        CHIP_APPLICATION_ERROR(0x05)
#define ERROR_STOP_TIMER_FAILED         CHIP_APPLICATION_ERROR(0x06)
/**************************************************************************
 *                                  Macros
 **************************************************************************/
#ifndef MAX
#define MAX(x, y)   ( ((x) > (y))? (x) : (y) )
#endif
#ifndef MIN
#define MIN(x, y)   ( ((x) < (y))? (x) : (y) )
#endif
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
    QueueHandle_t publicQueue;
    TaskHandle_t task;
    TimerTick timerTick;
    QueueHandle_t uartQueue;
    RX_FRAMING rxFraming;
}SERIAL_TASK;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
static void SerialMain(void* pvParameter);
//Setup
static void SerialSetup(void);
static void SerialUartInit(void);
//Timer Handling
static TickType_t SerialGetTimeoutTick(void);
static void SerialHandleTimeout(void);
static void SerialFetchRxData(void);
static void SerialParseEspNowFrame(void);
//Message Processing
static void SerialProcessMyMsg(const MSG_HEADER* pMsg);
static void SerialTxMsg(const void* pData, uint32_t dataLength);
static void SerialPrintDebug(bool tx, const uint8_t* pData, uint32_t dataLength);
/**************************************************************************
 *                                  Variables
 **************************************************************************/
SERIAL_TASK serialTask;
/**************************************************************************
 *                                  OS Functions
 **************************************************************************/
CHIP_ERROR SerialTaskStart(void)
{
    memset(&serialTask, 0x00, sizeof(serialTask));
    serialTask.publicQueue = xQueueCreate(SERIAL_TASK_MSG_QUEUE_DEPTH, sizeof(void*));
    if (serialTask.publicQueue == NULL)
    {
        log_error("Failed to allocate app event queue");
        return ERROR_EVENT_QUEUE_FAILED;
    }

    // Start App task.
    BaseType_t xReturned;
    xReturned = xTaskCreate(SerialMain, SERIAL_TASK_NAME, SERIAL_TASK_STACK_SIZE, NULL, 1, &serialTask.task);
    return (xReturned == pdPASS) ? CHIP_NO_ERROR : ERROR_CREATE_TASK_FAILED;
}
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
void SerialTransmit(const void* pData, uint32_t dataLength)
{
    MsgPost(serialTask.publicQueue, MID_SERIAL_TX, pData, dataLength);
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static void SerialMain(void* pvParameter)
{
    SerialSetup();
    log_info("Task started");

    MSG_HEADER *pMsg;
    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(serialTask.publicQueue, &pMsg, SerialGetTimeoutTick());
        if (eventReceived == pdTRUE)
        {
            SerialProcessMyMsg(pMsg);
            free(pMsg);
        }
        else
        {
            SerialHandleTimeout();
        }
    }
}
//Setup
static void SerialSetup(void)
{
    #if SOC_UART_NUM > 2
    SerialUartInit();
    serialTask.timerTick.SetFromNow(1000);
    #endif
}
static void SerialUartInit(void)
{
    uart_config_t uartConfig = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122, //dummy number
        .source_clk = (uart_sclk_t)0, //dummy number
    };
    ESP_ERROR_CHECK(uart_param_config(SERIAL_UART, &uartConfig));
    ESP_ERROR_CHECK(uart_set_pin(SERIAL_UART, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(SERIAL_UART, SERIAL_TX_MAX_SIZE, SERIAL_RX_MAX_SIZE, SERIAL_TX_QUEUE_DEPTH, &serialTask.uartQueue, 0));
}
//Timer Handling
static TickType_t SerialGetTimeoutTick(void)
{
    return serialTask.timerTick.GetRemaining();
}
static void SerialHandleTimeout(void)
{
    if (serialTask.timerTick.HasElapsed())
    {
        serialTask.timerTick.Increment(100);
        SerialFetchRxData();
    }
}
static void SerialFetchRxData(void)
{
    uint8_t* pData = &serialTask.rxFraming.data[serialTask.rxFraming.offset];
    uint32_t spaceRemaining = sizeof(serialTask.rxFraming.data) - serialTask.rxFraming.offset;
    int readLength = uart_read_bytes(SERIAL_UART, pData, spaceRemaining, 0);
    if (readLength > 0)
    {
        SerialPrintDebug(false, pData, readLength);

        serialTask.rxFraming.offset += readLength;
        SerialParseEspNowFrame();
    }
}
static void SerialParseEspNowFrame(void)
{
    uint8_t* pRxBuf = serialTask.rxFraming.data;
    uint32_t rxBufOffset = serialTask.rxFraming.offset;

    while (rxBufOffset) 
    {
        int consumeSize = 1;
        int parseOffset = 0;
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
                            MatterProcessEspNowMessage(pEspData, len);
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
        serialTask.rxFraming.offset = rxBufOffset;
        memmove(&pRxBuf[0], &pRxBuf[consumeSize], rxBufOffset);
    }
}
//Message Processing
static void SerialProcessMyMsg(const MSG_HEADER* pMsg)
{
    switch (pMsg->mid)
    {
    case MID_SERIAL_TX: SerialTxMsg(pMsg->data, pMsg->length);  break;
    default:            /*Do Nothing*/                          break;
    }
}
static void SerialTxMsg(const void* pData, uint32_t dataLength)
{
    SerialPrintDebug(true, (uint8_t*)pData, dataLength);
    if (dataLength < 256)
    {
        uint8_t byteLength = dataLength;
        #if SOC_UART_NUM > 2
        uint32_t startKey = 0x1f5a3db9;
        uart_write_bytes(SERIAL_UART, (uint8_t*)&startKey, sizeof(startKey));
        uart_write_bytes(SERIAL_UART, (uint8_t*)&byteLength, sizeof(byteLength));
        uart_write_bytes(SERIAL_UART, (uint8_t*)pData, byteLength);
        uint16_t crc = Crc16Block(0, pData, byteLength);
        uart_write_bytes(SERIAL_UART, (uint8_t*)&crc, sizeof(crc));
        #endif
    }
    else
    {
        log_error("Tx too long");
    }
}
static void SerialPrintDebug(bool tx, const uint8_t* pData, uint32_t dataLength)
{
    static char debugBuf[300];
    debugBuf[0] = '\0';
    for (int i = 0; i < dataLength; i++)
    {
        sprintf(&debugBuf[strlen(debugBuf)], " %02X", pData[i]);
    }
    const char* pTxStr = tx ? "Tx" : "Rx";
    log_info("%s %lu:%s", pTxStr, dataLength, debugBuf);
}