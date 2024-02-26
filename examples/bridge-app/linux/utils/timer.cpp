/*******************************************************************************************
File: timer.c

********************************************************************************************/

#include "timer.h"

#include <signal.h>
#include <string.h>
#include <pthread.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
/**************************************************************************
 *                                  Macros
 **************************************************************************/
#ifdef TIMER_DEBUG_TYPE
#define TIMER_PRINTF(level, ...)        _DPRINTF(TIMER_DEBUG_TYPE, level, __VA_ARGS__)
#define TIMER_PRINTS(level, s)          _DPRINTS(TIMER_DEBUG_TYPE, level, s)
#define TIMER_PRINT_BUF(level, b, s)    _DPRINTBUF(TIMER_DEBUG_TYPE, level, b, s)
#else
#define TIMER_PRINTF(level, ...)
#define TIMER_PRINTS(level, s)
#define TIMER_PRINT_BUF(level, b, s)
#endif
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef struct
{
   uint32_t startTime;
}TIMER;
/**************************************************************************
 *                                  Variables
 **************************************************************************/
static TIMER timer;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
void TimerInit(void)
{
   memset(&timer, 0x00, sizeof(timer));
   timer.startTime = TimerGetMs();
}
uint32_t TimerGetMs(void)
{
   struct timespec spec;
   clock_gettime(CLOCK_REALTIME, &spec);
   return (uint32_t)spec.tv_sec*1000 + (uint32_t)spec.tv_nsec/1000000 - timer.startTime;
}
uint64_t TimerGetMsUtc(void)
{
   struct timespec spec;
   clock_gettime(CLOCK_REALTIME, &spec);
   return (uint64_t)spec.tv_sec*1000 + (uint64_t)spec.tv_nsec/1000000;
}
void TimerSleepMs(uint32_t numMs)
{
   struct timespec spec;
   spec.tv_sec = (long)numMs/1000;
   numMs -= (uint32_t)spec.tv_sec*1000;
   spec.tv_nsec = (long)numMs*1000000;
   nanosleep(&spec, NULL);
}
bool TimerHasElapsedMs(uint32_t timerEndMs)
{
   return (TimerGetMs() > timerEndMs);
}
uint32_t TimerRemainingMs(uint32_t timerEndMs)
{
   int32_t msRemaining = (int32_t)(timerEndMs - TimerGetMs());
   if (msRemaining <= 0)
   {
      msRemaining = 0;
   }
   return (uint32_t)msRemaining;
}
bool TimerStartOneShot(timer_t *pTimerId, int32_t expireTimeMs, TIMER_PFN pfnCallback)
{
   return TimerStartRepeating(pTimerId, expireTimeMs, 0, pfnCallback);
}
bool TimerStartRepeating(timer_t *pTimerId, int32_t expireTimeMs, uint32_t intervalTimeMs, TIMER_PFN pfnCallback)
{
   /* Set up signal handler. */
   struct sigaction signalAction;
   signalAction.sa_flags = SA_SIGINFO;
   signalAction.sa_sigaction = (void (*)(int, siginfo_t *, void *))pfnCallback;
   sigemptyset(&signalAction.sa_mask);
   if (sigaction(SIGRTMIN, &signalAction, NULL) == -1)
   {
      TIMER_PRINTF(ERROR, "Failed to setup signal handling for %08X.", pTimerId);
      return false;
   }

   /* Set and enable alarm */
   struct sigevent signalEvent;
   signalEvent.sigev_notify = SIGEV_SIGNAL;
   signalEvent.sigev_signo = SIGRTMIN;
   signalEvent.sigev_value.sival_ptr = pTimerId;
   timer_create(CLOCK_REALTIME, &signalEvent, pTimerId);

   struct itimerspec iTimerSpec;
   iTimerSpec.it_interval.tv_sec = (int32_t)intervalTimeMs / 1000;
   intervalTimeMs -= (uint32_t)iTimerSpec.it_interval.tv_sec * 1000;
   iTimerSpec.it_interval.tv_nsec = (int32_t)intervalTimeMs * 1000000;
   iTimerSpec.it_value.tv_sec = expireTimeMs / 1000;
   expireTimeMs -= iTimerSpec.it_value.tv_sec * 1000;
   iTimerSpec.it_value.tv_nsec = expireTimeMs * 1000000;

   if (timer_settime(*pTimerId, 0, &iTimerSpec, NULL) == -1)
   {
      TIMER_PRINTF(ERROR, "Failed to start signal handling for %08X.", pTimerId);
      return false;
   }

   return true;
}
void TimerStop(timer_t const *pTimerId)
{
   struct itimerspec value;

   value.it_value.tv_sec = 0;
   value.it_value.tv_nsec = 0;

   value.it_interval.tv_sec = 0;
   value.it_interval.tv_nsec = 0;

   TIMER_PRINTF(DEBUG, "Stopped %08X.", pTimerId);
   timer_settime(*pTimerId, 0, &value, NULL);
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
