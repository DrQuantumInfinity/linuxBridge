#pragma once

#include <stdint.h>

#define _ASSERT_ACTIVE 1

#if _ASSERT_ACTIVE
#define ASSERT_PARAM(expr) ((expr) ? (void)0 : AssertFailed((char *)__FILE__, __LINE__, #expr))
void AssertFailed(char const *pFileName, uint32_t lineNumber, char const *pMsg);
#else
#define ASSERT_PARAM(expr)
#endif
