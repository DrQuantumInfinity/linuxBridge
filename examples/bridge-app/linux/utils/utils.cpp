/*******************************************************************************************
File: utils.cpp

********************************************************************************************/


#include "utils.h"
#include <ctype.h>	//lint !e537
#include <stdlib.h>	//lint !e537
#include <string.h>	//lint !e537
#include <stdarg.h>	//lint !e537
#include <stdio.h>


/**************************************************************************
 *                                  Constants
 **************************************************************************/
const char* const stringMonthsLong[12] = {
   "January",
   "February",
   "March",
   "April",
   "May",
   "June",
   "July",
   "August",
   "September",
   "October",
   "November",
   "December",
};
const char* const stringMonthsShort[12] = {
   "Jan",
   "Feb",
   "Mar",
   "Apr",
   "May",
   "Jun",
   "Jul",
   "Aug",
   "Sep",
   "Oct",
   "Nov",
   "Dec"
};
/**************************************************************************
 *                                  Types
 **************************************************************************/
 /**************************************************************************
  *                                  Variables
  **************************************************************************/
  /**************************************************************************
   *                                  Prototypes
   **************************************************************************/
   /**************************************************************************
    *                                  Global Functions
    **************************************************************************/
bool CpuIsLittleEndian(void)
{
   uint16_t n = 1;
   char* p = (char*)&n;
   return (bool)(*p == 1);
}

int32_t antoi(char const* pStr, uint32_t maxChars)
{
   char buffer[12];
   maxChars = MIN(sizeof(buffer) - 1, maxChars);
   memcpy(buffer, pStr, maxChars);
   buffer[maxChars] = '\0';
   return atoi(buffer);
}

uint32_t GetStringIndexFromTable(char const* const* ppTable, char const* pSearchStr, uint32_t maxIndex)
{
   uint32_t stringIndex;
   for (stringIndex = 0; stringIndex < maxIndex; stringIndex++)
   {
      if (strncmp(pSearchStr, ppTable[stringIndex], strlen(ppTable[stringIndex])) == STR_CMP_MATCH)
      {
         break;
      }
   }
   return stringIndex;
}

int stricmp(char const* pStr1, char const* pStr2)
{
   uint32_t len1 = (uint32_t)strlen(pStr1);
   uint32_t len2 = (uint32_t)strlen(pStr2);

   return strnicmp(pStr1, pStr2, (len1 > len2) ? len1 : len2);
}
int strnicmp(char const* pStr1, char const* pStr2, uint32_t len)
{
   char c1, c2;
   while (len--)
   {
      c1 = *pStr1++;
      c2 = *pStr2++;
      c1 = (char)toupper((int)c1);
      c2 = (char)toupper((int)c2);

      if (c1 < c2)
         return -1;
      else if (c1 > c2)
         return 1;
      else if (c1 == '\0')
         return 0;
   }
   return 0;
}
char* stristr(char* pStr1, char* pStr2)
{
   char c1, c2;
   uint32_t len2 = (uint32_t)strlen(pStr2);

   c1 = (char)toupper((int)*pStr2);
   while (*pStr1)
   {
      c2 = (char)toupper((int)*pStr1);
      if (c1 == c2)
      {
         if (strnicmp(pStr1, pStr2, len2) == 0)
         {
            return pStr1;
         }
      }
      pStr1++;
   }
   return NULL;
} //lint !e818

char const* strTrimFront(char const* pStr)
{
   if (pStr)
   {
      while ((*pStr != '\0') && isspace(*pStr))
      {
         pStr++;
      }
   }
   return pStr;
}
char* strTrimTail(char* pStr)
{
   if (pStr)
   {
      char* pEnd = &pStr[(strlen(pStr) - 1)];
      while (pEnd > pStr)
      {
         if (!isspace(*pEnd))
         {
            pEnd[1] = '\0';
            break;
         }
         pEnd--;
      }
   }
   return pStr;
}
char* strTrim(char* pStr)
{
   if (pStr)
   {
      while ((*pStr != '\0') && isspace(*pStr))
      {
         pStr++;
      }
      pStr = strTrimTail(pStr);
   }
   return pStr;
}

char* Strxtoa(uint32_t v, char* pStr, int32_t r, int32_t isNeg)
{
   char* pStart = pStr;
   char buf[33], * p;

   p = buf;

   do
   {
      *p++ = "0123456789abcdef"[(v % (uint32_t)r) & 0xf];
   } while (v /= (uint32_t)r); //lint !e720

   if (isNeg)
   {
      *p++ = '-';
   }

   do
   {
      *pStr++ = *--p;
   } while (buf != p);

   *pStr = '\0';

   return pStart;
}
char* Stritoa(int32_t v, char* pStr, int32_t r)
{
   if ((r == 10) && (v < 0))
   {
      return Strxtoa((uint32_t)(-v), pStr, r, 1);
   }
   return Strxtoa((uint32_t)(v), pStr, r, 0);
}

void strncatf(char* pDest, uint32_t maxLen, char const* pFormat, ...)
{
   va_list pArg;
   uint32_t len = (uint32_t)strlen(pDest);
   if (maxLen > len)
   {
      va_start(pArg, pFormat);
      vsnprintf(pDest + len, maxLen - len, pFormat, pArg); //lint !e534
      va_end(pArg);
   }
}

void StringifyTime(char* pDest, uint32_t destLen, char const* pFormat, time_t const* pData)
{
   struct tm ltm;
   localtime_r(pData, &ltm);

   int32_t strLen;
   do
   {
      char curChar = pFormat[0];
      char nxtChar = pFormat[1];
      switch (curChar)
      {
         case 'y':   strLen = snprintf(pDest, destLen, "%02u", ltm.tm_year - 100);                                          break;
         case 'Y':   strLen = snprintf(pDest, destLen, "20%02u", ltm.tm_year - 100);                                        break;
         case 'M':   strLen = snprintf(pDest, destLen, "%02u", ltm.tm_mon + 1);                                             break;
         case 'B':   strLen = snprintf(pDest, destLen, " %s ", stringMonthsLong[ltm.tm_mon]);                               break;
         case 'b':   strLen = snprintf(pDest, destLen, " %s ", stringMonthsShort[ltm.tm_mon]);                              break;
         case 'd':   strLen = snprintf(pDest, destLen, "%02u", ltm.tm_mday);                                                break;
         case 'D':   strLen = snprintf(pDest, destLen, "%02u/%02u/%02u", ltm.tm_year - 100, ltm.tm_mon + 1, ltm.tm_mday);   break;
         case 'h':   strLen = snprintf(pDest, destLen, "%02u", ltm.tm_hour);                                                break;
         case 'm':   strLen = snprintf(pDest, destLen, "%02u", ltm.tm_min);                                                 break;
         case 's':   strLen = snprintf(pDest, destLen, "%02u", ltm.tm_sec);                                                 break;
         case 'T':   strLen = snprintf(pDest, destLen, "%02u:%02u:%02u", ltm.tm_hour, ltm.tm_min, ltm.tm_sec);              break;
         default:    strLen = 1;    *pDest = curChar;                                                                       break;
      }
      destLen -= (uint32_t)strLen;
      pDest += strLen;

      if ((curChar == 'y' || curChar == 'Y' || curChar == 'M' || curChar == 'd') &&
         (nxtChar == 'y' || nxtChar == 'Y' || nxtChar == 'M' || nxtChar == 'd') &&
         destLen)
      {
         destLen--;
         *pDest = '/';
         pDest++;
      }
      else if ((curChar == 'y' || curChar == 'Y' || curChar == 'M' || curChar == 'd' || curChar == 'D') &&
         (nxtChar == 'h' || nxtChar == 'm' || nxtChar == 's' || nxtChar == 'T') &&
         destLen)
      {
         destLen--;
         *pDest = ' ';
         pDest++;
      }
      else if ((curChar == 'h' || curChar == 'm' || curChar == 's' || curChar == 'T') &&
         (nxtChar == 'y' || nxtChar == 'Y' || nxtChar == 'M' || nxtChar == 'd' || nxtChar == 'D') &&
         destLen)
      {
         destLen--;
         *pDest = ' ';
         pDest++;
      }
      else if ((curChar == 'h' || curChar == 'm' || curChar == 's') &&
         (nxtChar == 'h' || nxtChar == 'm' || nxtChar == 's') &&
         destLen)
      {
         destLen--;
         *pDest = ':';
         pDest++;
      }
      pFormat++;
   } while (*pFormat != '\0' && destLen);
}


uint32_t aton(char const* pAddr)
{
   // Example: 192.168.1.120 = 0x7801ABC0
   uint32_t retAddr = 0;
   char const* pStart = pAddr;
   for (uint32_t i = 0; i < 4; i++)
   {
      retAddr |= ((uint32_t)atoi(pStart) & 0xFF) << (uint32_t)(8 * i);
      pStart = strchr(pStart, '.');
      if (pStart)
      {
         pStart++; // move past the '.' char
      }
      else
      {
         break;
      }
   }
   return retAddr;
}


void macToa(char* pDest, char const* pMac)
{
#define UINT_FIELD(b)		(((uint32_t)b) & 0xFF)
   sprintf(pDest, "%02X:%02X:%02X:%02X:%02X:%02X",
      UINT_FIELD(pMac[0]), UINT_FIELD(pMac[1]), UINT_FIELD(pMac[2]),
      UINT_FIELD(pMac[3]), UINT_FIELD(pMac[4]), UINT_FIELD(pMac[5]));
}
void atoMac(char* pDest, char const* pMac)
{
   for (uint32_t i = 0; i < 6; i++)
   {
      pDest[i] = (uint8_t)strtoul(&pMac[i * 3], NULL, 16); // each field is 3 digits (i.e. 00:11:22:33:44:55)
   }
}
bool stringIsValidIp(char const* pIp)
{
   bool retVal = true;
   uint32_t ipLength = (uint32_t)strlen(pIp);
   int32_t digit;
   if (ipLength <= strlen("xxx.xxx.xxx.xxx"))
   {
      for (uint8_t ipDigit = 0; ipDigit < 4; ipDigit++)
      {
         digit = atoi(pIp);
         if (!isdigit(*pIp) || digit < 0 || digit > 255)
         {
            retVal = false;
            break;
         }

         if (digit >= 100)
         {
            pIp += 3;
         }
         else if (digit >= 10)
         {
            pIp += 2;
         }
         else
         {
            pIp += 1;
         }

         if (ipDigit < 3 && *pIp != '.')
         {
            retVal = false;
            break;
         }
         pIp++;
      }
   }
   else
   {
      retVal = false;
   }
   return retVal;
}


//http://stackoverflow.com/questions/746171/best-algorithm-for-bit-reversal-from-msb-lsb-to-lsb-msb-in-c
//http://graphics.stanford.edu/~seander/bithacks.html
uint32_t BitReverseWord(uint32_t x)
{
   x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
   x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
   x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
   x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
   return ((x >> 16) | (x << 16));
}

uint8_t BitReverseByte(uint8_t x)
{
   x = (uint8_t)(((x & 0xaa) >> 1) | ((x & 0x55) << 1));
   x = (uint8_t)(((x & 0xcc) >> 2) | ((x & 0x33) << 2));
   return ((uint8_t)(x >> 4) | (uint8_t)(x << 4)); //lint !e734
}

//http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
uint32_t BitRoundToNextPowerOf2(uint32_t v)
{
   v--;
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   v |= v >> 8;
   v |= v >> 16;
   v++;
   return v;
}

//http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
uint8_t BitsSetInWord(uint32_t x)
{
   x = x - ((x >> 1) & 0x55555555);
   x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
   x = ((x >> 4) + x) & 0x0F0F0F0F;
   x = ((x >> 8) + x) & 0x00FF00FF;
   x = ((x >> 16) + x) & 0x0000FFFF;
   return (uint8_t)x;
}
uint8_t BitsSetInHalfWord(uint16_t x)
{
   x = x - ((x >> 1) & 0x5555);
   x = ((x >> 2) & 0x3333) + (x & 0x3333);
   x = ((x >> 4) + x) & 0x0F0F;
   x = ((x >> 8) + x) & 0x00FF;
   return (uint8_t)x;
}
uint8_t BitsSetInByte(uint8_t x)
{
   x = x - ((x >> 1) & 0x55);
   x = ((x >> 2) & 0x33) + (x & 0x33);
   x = ((x >> 4) + x) & 0x0F;
   return x;
}

// "maxUlps" is the "Units in the Last Place", which specifies how big an error the caller is willing
// to accept in terms of the value of the least significant digits of the FP number's representation.
// AKA how many representable floats we are willing to accept between A and B
bool Fp32AlmostEqual(float A, float B, int32_t maxUlps)
{
   // Make sure maxUlps is non-negative and small enough that the
   // default NAN won't compare as equal to anything.
   if ((maxUlps > 0) && (maxUlps < (4 * 1024 * 1024))) return false;

   // Make aInt lexicographically ordered as a twos-complement int
   int32_t aInt = *(int32_t*)&A; //lint !e740
   if (aInt < 0)
      aInt = (int32_t)0x80000000 - aInt;

   // Make bInt lexicographically ordered as a twos-complement int
   int32_t bInt = *(int32_t*)&B; //lint !e740
   if (bInt < 0)
      bInt = (int32_t)0x80000000 - bInt;

   int32_t intDiff = ABS(aInt - bInt);
   return (bool)(intDiff <= maxUlps);
}

void SwitchBytesInString(char* pDest, char const* pSrc)
{
   uint8_t numChars = (uint8_t)strlen((char const*)pSrc);
   uint8_t tempChar;
   bool oddChars = false;

   if (IS_ODD(numChars))
   {
      numChars--;
      oddChars = true;
   }

   for (uint8_t i = 0; i < numChars; i += 2)
   {
      tempChar = pSrc[i];
      pDest[i] = pSrc[i + 1];
      pDest[i + 1] = tempChar;
   }
   if (oddChars)
   {
      pDest[numChars + 1] = pSrc[numChars];
      pDest[numChars] = 'F';
      pDest[numChars + 2] = '\0';
   }
   else
   {
      pDest[numChars + 1] = '\0';
   }
}

void CopyToContiguousArray(uint8_t* pDest, SRC_ELEMENT const* pSrc, uint32_t numElements)
{
   while (numElements--)
   {
      memcpy(pDest, pSrc->pData, pSrc->length);
      pDest += pSrc->length;
      pSrc++;
   }
}
uint32_t SrcElementsGetLength(SRC_ELEMENT const* pSrc, uint32_t numElements)
{
   uint32_t totalLength = 0;
   while (numElements-- > 0)
   {
      totalLength += pSrc->length;
      pSrc++;
   }
   return totalLength;
}
uint32_t SrcElementsMemMem(SRC_ELEMENT const* pHaystack, uint32_t numHaystacks, void const* pNeedle, uint32_t needleLength)
{
   uint32_t needleLocation = 0;
   uint32_t needleIndex = 0;
   while (numHaystacks--)
   {
      uint32_t stackSize = pHaystack->length;
      uint32_t stackIndex;
      for (stackIndex = 0; stackIndex < stackSize; stackIndex++)
      {
         if (((uint8_t*)pHaystack->pData)[stackIndex] == ((uint8_t*)pNeedle)[needleIndex])
         {
            needleIndex++;
            if (needleIndex == needleLength)
            {
               numHaystacks = 0;
               break;
            }
         }
         else
         {
            stackIndex -= needleIndex;
            needleIndex = 0;
         }
      }
      needleLocation += stackIndex;
      pHaystack++;
   }
   return needleLocation - (needleLength - 1);
}
bool SrcElementsMemCpy(SRC_ELEMENT const* pHaystack, uint32_t numHaystacks, void* pDest, uint32_t copyLength)
{
   while (numHaystacks-- && copyLength)
   {
      uint32_t currentCopyLength = MIN(copyLength, pHaystack->length);
      copyLength -= currentCopyLength;
      memcpy(pDest, pHaystack->pData, currentCopyLength);
      pDest = (uint8_t*)pDest + currentCopyLength;
      pHaystack++;
   }
   return copyLength == 0;
}
void SrcElementsMoveToStartOffset(SRC_ELEMENT** ppHaystack, uint32_t* pNumHaystacks, uint32_t startOffset)
{
   while (*pNumHaystacks)
   {
      if ((*ppHaystack)->length > startOffset)
      {
         (*ppHaystack)->pData = (uint8_t*)((*ppHaystack)->pData) + startOffset;
         (*ppHaystack)->length -= startOffset;
         break;
      }
      else
      {
         startOffset -= (*ppHaystack)->length;
         (*ppHaystack)++;
         (*pNumHaystacks)--;
      }
   }
}

uint32_t AsciiHexToBytes(uint8_t* pDest, uint32_t destLen, char const* pSrc, char const** pSrcEnd)
{
   uint32_t destCount;
   uint8_t msb, lsb;
   for (destCount = 0; (*pSrc != '\0') && (destCount < destLen); destCount++, pSrc += 2)
   {
      // trim whitespace around pairs (if any)
      while (isspace(*pSrc))
      {
         pSrc++;
      }
      msb = (uint8_t)tolower(pSrc[0]);
      msb -= isdigit(msb) ? '0' : ('a' - 0xa);
      lsb = (uint8_t)tolower(pSrc[1]);
      lsb -= isdigit(lsb) ? '0' : ('a' - 0xa);
      if ((msb > 0xf) || (lsb > 0xf))
      {
         break;
      }
      *pDest++ = (uint8_t)((msb << 4) | lsb);
   }
   if (pSrcEnd)
   {
      *pSrcEnd = pSrc;
   }
   return destCount;
}

void* memmem(void const* pHaystack, uint32_t haystackSize, void const* pNeedle, uint32_t needleLength)
{
   const void* pNextSearch = memchr(pHaystack, *(char*)pNeedle, haystackSize);
   while (pNextSearch)
   {
      haystackSize -= (uint32_t)((char*)pNextSearch - (char*)pHaystack);
      pHaystack = pNextSearch;
      if (haystackSize >= needleLength)
      {
         if (memcmp(pHaystack, pNeedle, needleLength) == MEM_CMP_MATCH)
         {
            break;
         }
         pNextSearch = memchr((char*)pHaystack + 1, *(char*)pNeedle, haystackSize);
      }
      else
      {
         pNextSearch = NULL;
         break;
      }
   }
   return (void*)pNextSearch;
}

void memset32(void* pDest, void const* pMemsetWord, uint32_t bufferSize)
{
   uint32_t i;
   uint32_t numWhole32BitWords = bufferSize & (~(sizeof(uint32_t) - 1));
   for (i = 0; i < numWhole32BitWords; i += sizeof(uint32_t))
   {
      memcpy(((char*)pDest) + i, pMemsetWord, sizeof(uint32_t));
   }
   memcpy(((char*)pDest) + i, pMemsetWord, bufferSize - i);
}
uint32_t memdif(void const* pSrc1, void const* pSrc2, uint32_t startIndex, uint32_t compareLength)
{
   uint32_t byteIdx = MAX_UINT32;
   for (byteIdx = startIndex; byteIdx < compareLength; byteIdx++)
   {
      if (((uint8_t*)pSrc1)[byteIdx] != ((uint8_t*)pSrc2)[byteIdx])
      {
         break;
      }
   }
   return byteIdx;
}

/**************************************************************************
 *                                 Private Functions
 **************************************************************************/
