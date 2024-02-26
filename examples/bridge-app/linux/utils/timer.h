#pragma once
/*******************************************************************************************
timer.h

********************************************************************************************/

#include <stdint.h>
#include <sys/types.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef void (*TIMER_PFN)(void);
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
uint32_t TimerGetMs(void);
uint64_t TimerGetMsUtc(void);
void TimerSleepMs(uint32_t numMs);
bool TimerHasElapsedMs(uint32_t timerEndMs);
uint32_t TimerRemainingMs(uint32_t timerEndMs);

bool TimerStartOneShot(timer_t* pTimerId, int32_t expireTimeMs, TIMER_PFN pfnCallback);
bool TimerStartRepeating(timer_t* pTimerId, int32_t expireTimeMs, uint32_t intervalTimeMs, TIMER_PFN pfnCallback);
void TimerStop(const timer_t* pTimerId);
