
#include "DynamicList.h"
#include "freertos/FreeRTOS.h"
#include <platform/CHIPDeviceLayer.h>

#include <string.h>
#include <stdlib.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
//static const char * TAG = "Dynamic-List";
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef struct _DYNAMIC_NODE 
{
    struct _DYNAMIC_NODE *pNextNode;
    void *pItem;
    TickType_t addUpdateTimeTick;
} DYNAMIC_NODE;

typedef struct
{
    DYNAMIC_NODE* pHeadNode;
    DYNAMIC_NODE* pTailNode;
    uint32_t numDevices;
    DYNAMIC_LIST_ITEM_MATCHER pfnItemMatcher;
    uint32_t itemSize;
} DYNAMIC_LIST_REAL;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
static DYNAMIC_NODE* DynamicListGetMatchingNode(const DYNAMIC_LIST* pList, void* pItem);
/**************************************************************************
 *                                  Variables
 **************************************************************************/
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
DYNAMIC_LIST* DynamicListCreate(DYNAMIC_LIST_ITEM_MATCHER pfnItemMatcher, uint32_t itemSize)
{
    DYNAMIC_LIST_REAL *pList = (DYNAMIC_LIST_REAL*)malloc(sizeof(DYNAMIC_LIST_REAL));
    memset(pList, 0x00, sizeof(*pList));
    pList->pfnItemMatcher = pfnItemMatcher;
    pList->itemSize = itemSize;
    return (DYNAMIC_LIST*)pList;
}
void DynamicListDestroy(DYNAMIC_LIST* pList)
{
    DYNAMIC_LIST_REAL* pListReal = (DYNAMIC_LIST_REAL*)pList;
    
    DYNAMIC_NODE* pNode = pListReal->pHeadNode;
    while (pNode != NULL)
    {
        DYNAMIC_NODE* pFree = pNode;
        pNode = pNode->pNextNode;
        free(pFree->pItem);
        free(pFree);
    }
    free(pList);
}
void* DynamicListAddFetchItem(DYNAMIC_LIST* pList, void* pItem)
{
    DYNAMIC_LIST_REAL* pListReal = (DYNAMIC_LIST_REAL*)pList;

    DYNAMIC_NODE* pNode = DynamicListGetMatchingNode(pList, pItem);
    if (pNode == NULL)
    {
        //This is a new device
        pNode = (DYNAMIC_NODE*)malloc(sizeof(DYNAMIC_NODE));
        pNode->pItem = malloc(pListReal->itemSize);
        memcpy(pNode->pItem, pItem, pListReal->itemSize);
        pNode->pNextNode = NULL;

        if (pListReal->numDevices == 0)
        {
            pListReal->pHeadNode = pNode;
            pListReal->pTailNode = pNode;
        }
        else
        {
            pListReal->pTailNode->pNextNode = pNode;
            pListReal->pTailNode = pNode;
        }
        pListReal->numDevices++;
    }
    else
    {
        //Node already exists. Stored data will be updated.
    }
    pNode->addUpdateTimeTick = xTaskGetTickCount();

    return pNode->pItem;
}
void DynamicListRemoveItem(DYNAMIC_LIST* pList, void *pItem)
{
    DYNAMIC_LIST_REAL* pListReal = (DYNAMIC_LIST_REAL*)pList;
    
    DYNAMIC_NODE* pPreviousNode = pListReal->pHeadNode;
    DYNAMIC_NODE* pNode = pPreviousNode;
    if (pPreviousNode != NULL)
    {
        if (pListReal->pfnItemMatcher(pItem, pPreviousNode->pItem))  
        {
            pListReal->pHeadNode = pPreviousNode->pNextNode;
            free(pPreviousNode->pItem);
            free(pPreviousNode);
            pListReal->numDevices--;
        }
        else
        {
            pNode = pPreviousNode->pNextNode;
            while (pNode != NULL)
            {
                if (pListReal->pfnItemMatcher(pItem, pNode->pItem))
                {
                    pPreviousNode->pNextNode = pNode->pNextNode;
                    free(pNode);
                    pListReal->numDevices--;
                    break;
                }
                else
                {
                    pNode = pNode->pNextNode;
                }
            }
        }
    }

    if (pNode == NULL)
    {
        //Device never existed in the list...
        //Nothing to remove...
    }
}
void *DynamicListGetOldestItem(const DYNAMIC_LIST* pList)
{
    DYNAMIC_LIST_REAL* pListReal = (DYNAMIC_LIST_REAL*)pList;

    DYNAMIC_NODE* pNode = pListReal->pHeadNode;
    DYNAMIC_NODE* pOldestNode = pNode;
    while (pNode != NULL)
    {
        if (pNode->addUpdateTimeTick < pOldestNode->addUpdateTimeTick)
        {
            pOldestNode = pNode;
        }
        pNode = pNode->pNextNode;
    }

    void *pOldestItem = NULL;
    if (pOldestNode)
    {
        pOldestItem = pOldestNode->pItem;
    }
    return pOldestItem;
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static DYNAMIC_NODE* DynamicListGetMatchingNode(const DYNAMIC_LIST* pList, void* pItem)
{
    DYNAMIC_LIST_REAL* pListReal = (DYNAMIC_LIST_REAL*)pList;
    
    DYNAMIC_NODE* pNode = pListReal->pHeadNode;
    while (pNode != NULL)
    {
        if (pListReal->pfnItemMatcher(pItem, pNode->pItem))
        {
            break;
        }
        else
        {
            pNode = pNode->pNextNode;
        }
    }
    return pNode;
}