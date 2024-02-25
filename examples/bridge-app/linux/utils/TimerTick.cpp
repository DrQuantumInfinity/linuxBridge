#include "TimerTick.h"

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
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
static bool TimerTickMsbIsSet(TickType_t tick);
/**************************************************************************
 *                                  Variables
 **************************************************************************/
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
bool TimerTick::HasElapsed(void)
{
    bool hasElapsed = false;
    if (_tick)
    {
        TickType_t ticksRemaining = _tick - xTaskGetTickCount();
        //Is the MSB set? We would have underflow if the timer has elapsed.
        hasElapsed = TimerTickMsbIsSet(ticksRemaining) || ticksRemaining == 0;
    }
    return hasElapsed;
}
TickType_t TimerTick::GetRemaining(void)
{
    TickType_t ticksRemaining = 0xFFFFFFFF;
    if (_tick)
    {
        ticksRemaining = _tick - xTaskGetTickCount();
        //Is the MSB set? We would have underflow if the timer has elapsed.
        if (TimerTickMsbIsSet(ticksRemaining))
        {
            ticksRemaining = 0;
        }
    }
    return ticksRemaining;
}
void TimerTick::SetFromNow(uint32_t msFromNow)
{
    _tick = xTaskGetTickCount() + pdMS_TO_TICKS(msFromNow);
    if (_tick == 0)
    {
        _tick++;
    }
}
void TimerTick::Increment(uint32_t numMs)
{
    _tick += pdMS_TO_TICKS(numMs);
    if (_tick == 0)
    {
        _tick++;
    }
    if (HasElapsed())
    {
        SetFromNow(numMs);
    }
}
void TimerTick::Disable(void)
{
    _tick = 0;
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static bool TimerTickMsbIsSet(TickType_t tick)
{
    return tick & (1 << (sizeof(TickType_t)*8  - 1));
}
