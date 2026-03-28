
#include "assert.h"

#if _ASSERT_ACTIVE
#include "timer.h"
#include "futil.h"
#include "Log.h"
void AssertFailed(char const *pFileName, uint32_t lineNumber, char const *pMsg)
{
   pFileName = FutilFileName(pFileName);
   log_fatal("\n%s\n%d\n%s\n", pFileName, lineNumber, pMsg);
   for (uint32_t dummy = 0; dummy < 0xFFFFFFFE; dummy++)
   {
      TimerSleepMs(1000);
   }
} //lint !e715 !e818
#endif