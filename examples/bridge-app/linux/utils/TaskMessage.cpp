
#include "TaskMessage.h"
#include "freertos/FreeRTOS.h"

/**************************************************************************
 *                                  Constants
 **************************************************************************/
static const char * TAG = "TaskMessage";
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
/**************************************************************************
 *                                  Variables
 **************************************************************************/
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
void MsgPost(QueueHandle_t queue, MID mid, const void * pData, uint32_t dataLength)
{
    MSG_HEADER* pMsg = (MSG_HEADER*)malloc(sizeof(MSG_HEADER) + dataLength);
    pMsg->mid         = mid;
    pMsg->timeTick    = xTaskGetTickCount();
    pMsg->length      = dataLength;
    memcpy(pMsg->data, pData, dataLength);

    BaseType_t status;
    if (xPortInIsrContext())
    {
        BaseType_t higherPrioTaskWoken = pdFALSE;
        status                         = xQueueSendFromISR(queue, &pMsg, &higherPrioTaskWoken);
    }
    else
    {
        status = xQueueSend(queue, &pMsg, 1);
    }

    if (!status)
    {
        log_error("Failed to post to queue");
        free(pMsg);
    }
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/