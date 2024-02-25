
#include "MatterTask.h"
#include "TaskMessage.h"
#include "TimerTick.h"
#include "EspNowData.h"
#include "Device.h"
#include "DeviceLight.h"
#include "DeviceLightTemp.h"
#include "DynamicList.h"
#include "freertos/FreeRTOS.h"
#include "DeviceButton.h"
#include "DeviceTemperature.h"
#include "DeviceLightRGB.h"

using namespace chip;

/**************************************************************************
 *                                  Constants
 **************************************************************************/
static const char * TAG =               "matter-task";
#define MATTER_TASK_MSG_QUEUE_DEPTH     (10)
#define MATTER_TASK_NAME                "MATTER"
#define MATTER_TASK_STACK_SIZE          (2000)

#if SOC_UART_NUM > 2
#define MATTER_UART                     (UART_NUM_2)
#else
#define MATTER_UART                     (UART_NUM_1)
#endif
#define MATTER_TX_MAX_SIZE              (300)
#define MATTER_TX_QUEUE_DEPTH           (10)
#define MATTER_RX_MAX_SIZE              (300)
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
    Device *pDevice;
    uint8_t macAddr[6];
}LIST_ITEM;


typedef struct
{
    uint32_t offset;
    uint8_t data[MATTER_RX_MAX_SIZE];
}RX_FRAMING;

typedef struct
{
    QueueHandle_t publicQueue;
    TaskHandle_t task;
    TimerTick timerTick;
    DYNAMIC_LIST* pList;
}MATTER_TASK;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
static void MatterMain(void* pvParameter);
//Setup
static void MatterSetup(void);
static bool MatterListItemMatcher(const void* pItem1, const void* pItem2);
//Timer Handling
static TickType_t MatterGetTimeoutTick(void);
static void MatterHandleTimeout(void);
//Message Processing
static void MatterProcessMyMsg(const MSG_HEADER* pMsg);
static void MatterEspNowRxMsg(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength);
static void MatterEspNowDht(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength, LIST_ITEM* pItem, const char* pName);
static void MatterEspNowMotion(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength, LIST_ITEM* pItem, const char* pName);
static void MatterEspNowBool(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength, LIST_ITEM* pItem, const char* pName);
static void MatterEspNowLightRGB(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength, LIST_ITEM* pItem, const char* pName);
static void MatterEspNowToggle(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength, LIST_ITEM* pItem, const char* pName);
/**************************************************************************
 *                                  Variables
 **************************************************************************/
static MATTER_TASK matterTask;
/**************************************************************************
 *                                  OS Functions
 **************************************************************************/
CHIP_ERROR MatterTaskStart(void)
{
    memset(&matterTask, 0x00, sizeof(matterTask));
    matterTask.publicQueue = xQueueCreate(MATTER_TASK_MSG_QUEUE_DEPTH, sizeof(void*));
    if (matterTask.publicQueue == NULL)
    {
        log_error("Failed to allocate app event queue");
        return ERROR_EVENT_QUEUE_FAILED;
    }

    // Start App task.
    BaseType_t xReturned;
    xReturned = xTaskCreate(MatterMain, MATTER_TASK_NAME, MATTER_TASK_STACK_SIZE, NULL, 1, &matterTask.task);
    return (xReturned == pdPASS) ? CHIP_NO_ERROR : ERROR_CREATE_TASK_FAILED;
}
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
void MatterProcessEspNowMessage(const ESP_NOW_DATA *pEspMsg, uint32_t dataLength)
{
    MsgPost(matterTask.publicQueue, MID_MATTER_PROCESS_ESP_NOW, pEspMsg, dataLength);
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static void MatterMain(void* pvParameter)
{
    MatterSetup();
    log_info("Task started");

    MSG_HEADER *pMsg;
    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(matterTask.publicQueue, &pMsg, MatterGetTimeoutTick());
        if (eventReceived == pdTRUE)
        {
            MatterProcessMyMsg(pMsg);
            free(pMsg);
        }
        else
        {
            MatterHandleTimeout();
        }
    }
}
//Setup
static void MatterSetup(void)
{
    matterTask.pList = DynamicListCreate(MatterListItemMatcher, sizeof(LIST_ITEM));
    matterTask.timerTick.Disable();
}
static bool MatterListItemMatcher(const void* pItemNew, const void* pItemStored)
{
    const LIST_ITEM* pItem11 = (const LIST_ITEM*)pItemNew;
    const LIST_ITEM* pItem22 = (const LIST_ITEM*)pItemStored;
    return (memcmp(pItem11->macAddr, pItem22->macAddr, sizeof(pItem11->macAddr)) == 0); //0 == MEM_CMP_MATCH
}
//Timer Handling
static TickType_t MatterGetTimeoutTick(void)
{
    return matterTask.timerTick.GetRemaining();
}
static void MatterHandleTimeout(void)
{
    if (matterTask.timerTick.HasElapsed())
    {
        matterTask.timerTick.Disable();
    }
}
//Message Processing
static void MatterProcessMyMsg(const MSG_HEADER* pMsg)
{
    const void* pData = pMsg->data;
    switch (pMsg->mid)
    {
    case MID_MATTER_PROCESS_ESP_NOW:    MatterEspNowRxMsg((const ESP_NOW_DATA*)pData, pMsg->length);    break;
    default:                            /*Do Nothing*/                                                  break;
    }
}
static void MatterEspNowRxMsg(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength)
{
    char nameBuf[32];
    sprintf(nameBuf, "%s %02X:%02X:%02X:%02X:%02X:%02X", EspNowGetName(pEspMsg),
        pEspMsg->macAddr[0], pEspMsg->macAddr[1], pEspMsg->macAddr[2], 
        pEspMsg->macAddr[3], pEspMsg->macAddr[4], pEspMsg->macAddr[5]);

    log_info("From %s", nameBuf);
    
    LIST_ITEM item = {
        .pDevice = NULL,
        .macAddr = {0},
    };
    memcpy(item.macAddr, pEspMsg->macAddr, sizeof(item.macAddr));
    LIST_ITEM* pItem = (LIST_ITEM*)DynamicListAddFetchItem(matterTask.pList, &item);

    switch (pEspMsg->type)
    {
    case ESP_NOW_DEVICE_TYPE_DHT:       MatterEspNowDht(pEspMsg, dataLength, pItem, nameBuf);       break;
    case ESP_NOW_DEVICE_TYPE_MOTION:    MatterEspNowMotion(pEspMsg, dataLength, pItem, nameBuf);    break;
    case ESP_NOW_DEVICE_TYPE_BOOL:      MatterEspNowBool(pEspMsg, dataLength, pItem, nameBuf);      break;
    // case ESP_NOW_DEVICE_TYPE_LIGHT_ON_OFF:
    // case ESP_NOW_DEVICE_TYPE_LIGHT_DIMMER:
    case ESP_NOW_DEVICE_TYPE_LIGHT_RGB:   MatterEspNowLightRGB(pEspMsg, dataLength, pItem, nameBuf);      break;
    // case ESP_NOW_DEVICE_TYPE_LIGHT_TEMP:
    // case ESP_NOW_DEVICE_TYPE_LIGHT_TEMP_RGB:  MatterEspNowLightTempRGB(pEspMsg, dataLength, pItem, nameBuf);      break;
    case ESP_NOW_DEVICE_TYPE_TOGGLE:    MatterEspNowToggle(pEspMsg, dataLength, pItem, nameBuf);    break;
    default:                            log_info("invalid EspNow type %u", pEspMsg->type);     break;
    }
}
static void MatterEspNowDht(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength, LIST_ITEM* pItem, const char* pName)
{  
    log_warn("temp: %f", pEspMsg->data.dht.temperature);
    log_warn("humid: %f", pEspMsg->data.dht.humidity);
    DeviceTemperature *pDevice;
    if (pItem->pDevice == NULL)
    {
        pDevice = new DeviceTemperature(pName, "Z", NULL, pEspMsg->data.dht.temperature, pEspMsg->data.dht.humidity);
        pItem->pDevice = pDevice;
    }
    else
    {
        pDevice = (DeviceTemperature*)pItem->pDevice;
        pDevice->UpdateHumidity(pEspMsg->data.dht.humidity);
        pDevice->UpdateTemp(pEspMsg->data.dht.temperature);
    }
}
static void MatterEspNowMotion(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength, LIST_ITEM* pItem, const char* pName)
{
}
static void MatterEspNowBool(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength, LIST_ITEM* pItem, const char* pName)
{
}
static void MatterEspNowLightRGB(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength, LIST_ITEM* pItem, const char* pName)
{
    DeviceLightRGB* pDevice;
    if (pItem->pDevice == NULL)
    {
        pDevice = new DeviceLightRGB(pName, "Z", NULL);
        pItem->pDevice = pDevice;
    }
    else
    {
        //TODO: Handle KeepAlives here.
        //pDevice = (DeviceLightRGB*)pItem->pDevice;
        //pDevice->SetLevel(74);
        //pDevice->SetColourHS(121, 100);
    }
}
static void MatterEspNowToggle(const ESP_NOW_DATA* pEspMsg, uint32_t dataLength, LIST_ITEM* pItem, const char* pName)
{
    DeviceButton *pButton;
    if (pItem->pDevice == NULL)
    {
        pButton = new DeviceButton(pName, "Z", NULL);
        pItem->pDevice = pButton;
    }
    else
    {
        pButton = (DeviceButton*)pItem->pDevice;
        pButton->Toggle();
    }
}