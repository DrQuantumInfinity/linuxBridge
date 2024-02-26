#pragma once

#include <stdint.h>

class TimerTick
{
    public:
        bool HasElapsed(void);
        uint32_t GetRemaining(void);
        void SetFromNow(uint32_t msFromNow);
        void Increment(uint32_t numMs);
        void Disable(void);

    private:
        uint32_t _ms;
};