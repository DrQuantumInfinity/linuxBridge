#pragma once
#include <stdint.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef void DYNAMIC_LIST;

typedef bool (*DYNAMIC_LIST_ITEM_MATCHER)(const void* pItemNew, const void* pItemStored);
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
DYNAMIC_LIST* DynamicListCreate(DYNAMIC_LIST_ITEM_MATCHER pfnItemMatcher, uint32_t itemSize);
void DynamicListDestroy(DYNAMIC_LIST* pList);

void* DynamicListAddFetchItem(DYNAMIC_LIST* pList, void* pItem);
void DynamicListRemoveItem(DYNAMIC_LIST* pList, void *pItem);

void *DynamicListGetOldestItem(const DYNAMIC_LIST* pList);