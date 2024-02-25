#include "freertos/FreeRTOS.h"

class TimerTick
{
    public:
        bool HasElapsed(void);
        TickType_t GetRemaining(void);
        void SetFromNow(uint32_t msFromNow);
        void Increment(uint32_t numMs);
        void Disable(void);

    private:
        TickType_t _tick;
};