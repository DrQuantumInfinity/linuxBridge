/*******************************************************************************************
File: utils.h

********************************************************************************************/

#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdint.h>
#include <ctime>


/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define  NUM_HRS_PER_DAY	(24uL)

#define  NUM_MIN_PER_HOUR	(60uL)
#define  NUM_MIN_PER_DAY	(NUM_MIN_PER_HOUR  * NUM_HRS_PER_DAY)

#define  NUM_SEC_PER_MIN	(60uL)
#define  NUM_SEC_PER_HOUR	(NUM_SEC_PER_MIN * NUM_MIN_PER_HOUR)
#define  NUM_SEC_PER_DAY	(NUM_SEC_PER_HOUR  * NUM_HRS_PER_DAY)

#define  NUM_MS_PER_SEC		(1000uL)
#define  NUM_US_PER_SEC		(1000000uL)
#define  NUM_NS_PER_SEC		(1000000000uL)

#define  MHZ					1000000uL
#define  KHZ					1000uL

#define	_PI					((float)3.141592654f)

#define STR_CMP_MATCH		(0)
#define MEM_CMP_MATCH		(0)

#define BIT0    ((uint8_t)0x01)
#define BIT1    ((uint8_t)0x02)
#define BIT2    ((uint8_t)0x04)
#define BIT3    ((uint8_t)0x08)
#define BIT4    ((uint8_t)0x10)
#define BIT5    ((uint8_t)0x20)
#define BIT6    ((uint8_t)0x40)
#define BIT7    ((uint8_t)0x80)
#define BIT8    ((uint16_t)0x0100)
#define BIT9    ((uint16_t)0x0200)
#define BIT10   ((uint16_t)0x0400)
#define BIT11   ((uint16_t)0x0800)
#define BIT12   ((uint16_t)0x1000)
#define BIT13   ((uint16_t)0x2000)
#define BIT14   ((uint16_t)0x4000)
#define BIT15   ((uint16_t)0x8000)
#define BIT16   ((uint32_t)0x00010000)
#define BIT17   ((uint32_t)0x00020000)
#define BIT18   ((uint32_t)0x00040000)
#define BIT19   ((uint32_t)0x00080000)
#define BIT20   ((uint32_t)0x00100000)
#define BIT21   ((uint32_t)0x00200000)
#define BIT22   ((uint32_t)0x00400000)
#define BIT23   ((uint32_t)0x00800000)
#define BIT24   ((uint32_t)0x01000000)
#define BIT25   ((uint32_t)0x02000000)
#define BIT26   ((uint32_t)0x04000000)
#define BIT27   ((uint32_t)0x08000000)
#define BIT28   ((uint32_t)0x10000000)
#define BIT29   ((uint32_t)0x20000000)
#define BIT30   ((uint32_t)0x40000000)
#define BIT31   ((uint32_t)0x80000000)

#define MIN_INT8   ((int8_t)   0x80)
#define MAX_INT8   ((int8_t)   0x7F)
#define MAX_UINT8  ((uint8_t)  0xFF)
#define MIN_INT16  ((int16_t)  0x8000)
#define MAX_INT16  ((int16_t)  0x7FFF)
#define MAX_UINT16 ((uint16_t) 0xFFFF)
#define MIN_INT32  ((int32_t)  0x80000000)
#define MAX_INT32  ((int32_t)  0x7FFFFFFF)
#define MAX_UINT32 ((uint32_t) 0xFFFFFFFF)
#define MIN_INT64  ((int64_t)  0x8000000000000000)
#define MAX_INT64  ((int64_t)  0x7FFFFFFFFFFFFFFF)
#define MAX_UINT64 ((uint64_t) 0xFFFFFFFFFFFFFFFF)

/**************************************************************************
 *                                  Pre-processor Macros
 **************************************************************************/

// Misc:
#define MEMBER_SIZE(type, field)	(sizeof(((type *)0)->field))
#define NELEMENTS(array)  			(sizeof (array) / sizeof (array[0]))
#define NELEMENTS2(array)  		(sizeof (array) / sizeof (array[0][0]))
#define OFFSET_OF(type, field)   ((UINT64)&(((type *)0)->field))

#define QUOTE_VAR_NAME(x) 	#x
#define QUOTE_VAR_VALUE(x) QUOTE_VAR_NAME(x)

#ifndef ASSERT_ACTIVE
#define ASSERT_ACTIVE		(1)
#endif
#if ASSERT_ACTIVE
#define ASSERT(x)				{if (!(x))	return;}
#define ASSERT_RET(x, ret)	{if (!(x))	return (ret);}
#else
#define ASSERT(x)
#define ASSERT_RET(x, ret)
#endif

#define IS_EVEN(x)	(((uint8_t)(x)&1)==0)
#define IS_ODD(x)		(((uint8_t)(x))&1)
#define NEXT_EVEN(x)	((x)+(IS_ODD(x)?1:0))

// Math-related:
#ifndef MAX
#define MAX(x, y)		( ((x) > (y))? (x) : (y) )
#endif
#ifndef MIN
#define MIN(x, y)		( ((x) < (y))? (x) : (y) )
#endif
#ifndef ABS
#define ABS(x)			( ((x) < 0)? -(x) : (x) )
#endif
#ifndef ABS_DIF
#define ABS_DIF(x,y)	( ((x) >= (y))? ((x) - (y)) : ((y) - (x)) )
#endif
#define CLAMP(x,low,high)	(((x) > (high))? (high) : (((x) < (low))? (low) : (x)))

#define DIVIDE_ROUND_UP(top, bottom)   (((top) + (bottom) - 1)/(bottom))

#define DOUBLE(x)		( 2 * (x) )
#define SQUARE(x)		( (x) * (x) )
#define CUBE(x)		( (x) * (x) * (x) )


// Binary helpers:
#define NUM_BYTES_FOR_BITS(sizeType, numBits)	((numBits + ((sizeType * 8)-1)) / (sizeType * 8))
#define NUM_TO_BIT_ARR_INDEX(arr, num)	((num) >> (3 * sizeof(arr[0])))			// divide by 8
#define NUM_TO_BIT(arr, num)				(1u << ((num) & (sizeof(arr[0] * 8) - 1)))
#define NUM_TO_BIT_SET(arr, num)			{arr[NUM_TO_BIT_ARR_INDEX(arr, num)] |=  NUM_TO_BIT(arr, num);}
#define NUM_TO_BIT_CLR(arr, num)			{arr[NUM_TO_BIT_ARR_INDEX(arr, num)] &= ~NUM_TO_BIT(arr, num);}
#define NUM_TO_BIT_TST(arr, num)			(arr[NUM_TO_BIT_ARR_INDEX(arr, num)] &   NUM_TO_BIT(arr, num))
#define BIT_TO_NUM(arr, index, bit)		(((arrIndex) * (sizeof(arr[0]) * 8)) + (bit))

//lint -emacro((572,778), NEXT_POWER_OF_2)
#define NEXT_POWER_OF_2(x) (NEXT_B32((x)-1) + 1)
// Example usage: bytesRequired = NEXT_POWER_OF_2(115)	// 128

#define BIT(n)								(1U << (n))
#define BIT_SET(var, mask)				{ (var) |=  (mask); }
#define BIT_CLEAR(var, mask)			{ (var) &= ~(mask); }
#define BIT_TOGGLE(var, mask)			{ var = ((var) & (mask)) ? (var) & ~(mask) : (var) | (mask); }
#define BIT_IS_SET(var, mask)			((((var) & (mask)) == (mask)) ? (TRUE) : (FALSE))
#define BIT_IS_CLEAR(var, mask)		(((var) & (mask)) ? (FALSE) : (TRUE))
#define BIT_IS_ANY_SET(var, mask)	(((var) & (mask)) ? (TRUE) : (FALSE))
#define BIT_IS_ANY_CLEAR(var, mask)	((((var) & (mask)) != (mask)) ? (TRUE) : (FALSE))
#define BIT_MASK(_BIT)					((_BIT) - 1)
#define COMBINE_NIBBLES(upper, lower)	( ((upper) << 4) | ((lower) & 0x0f) )

//lint -emacro(778, B8, B16, B32) -esym(778, B8, B16, B32)
#define B8(d) 				((uint8_t)B8__(HEX__(d)))
#define B16(dmsb,dlsb)	(((uint16_t)B8(dmsb)<<8) + B8(dlsb))
#define B32(dmsb,db2,db3,dlsb) 		(((unsigned long)B8(dmsb)<<24) \
												+ ((unsigned long)B8(db2)<<16) \
												+ ((unsigned long)B8(db3)<<8) \
												+ B8(dlsb))
// Example usage of B8, B16 and B32:
// B8(01010101) // 85
// B16(10101010,01010101) // 43,605
// B32(10000000,11111111,10101010,01010101) // 2,164,238,933
// reg = ( (B8(010) << 5) | (B8(11) << 3) | (B8(101) << 0) )

#if _ASSERT_ACTIVE
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((char *)__FILE__, __LINE__, #expr))
#else
#define assert_param(expr)
#endif

/*************************************************************************
*                                   ENDIAN
**************************************************************************/
#define RD_LE16(p)	((uint16_t)(((uint16_t)((p)[1])<<8) + (uint16_t)((p)[0])))
#define WR_LE16(p,v)	{ (p)[0]=(uint8_t)(v&0xff); \
							  (p)[1]=(uint8_t)((v>>8)&0xff); }

#define RD_BE16(p)	(uint16_t)(((uint16_t)((p)[0])<<8) + (uint16_t)((p)[1]))
#define WR_BE16(p,v)	{ (p)[1]=(uint8_t)(v&0xff); \
							  (p)[0]=(uint8_t)((v>>8)&0xff); }

#define RD_LE24(p)	((uint32_t)(((uint32_t)((p)[2])<<16) + ((uint32_t)((p)[1])<<8) + (uint32_t)((p)[0])))
#define WR_LE24(p,v)	{ (p)[0]=(uint8_t)(v&0xff); \
							  (p)[1]=(uint8_t)((v>>8)&0xff); \
							  (p)[2]=(uint8_t)((v>>16)&0xff); }

#define RD_BE24(p)	((uint32_t)(((uint32_t)((p)[0])<<16) + ((uint32_t)((p)[1])<<8) + (uint32_t)((p)[2])))
#define WR_BE24(p,v)	{ (p)[2]=(uint8_t)(v&0xff); \
							  (p)[1]=(uint8_t)((v>>8)&0xff); \
							  (p)[0]=(uint8_t)((v>>16)&0xff); }

#define RD_LE32(p)	((uint32_t)(((uint32_t)((p)[3])<<24) + ((uint32_t)((p)[2])<<16) + \
										 ((uint32_t)((p)[1])<<8)  + (uint32_t)((p)[0])))
#define WR_LE32(p,v)	{ (p)[0]=(uint8_t)(v&0xff); \
							  (p)[1]=(uint8_t)((v>>8)&0xff); \
							  (p)[2]=(uint8_t)((v>>16)&0xff); \
							  (p)[3]=(uint8_t)((v>>24)&0xff); }

#define RD_BE32(p)	((uint32_t)(((uint32_t)((p)[0])<<24) + ((uint32_t)((p)[1])<<16) + \
										 ((uint32_t)((p)[2])<<8)  + (uint32_t)((p)[3])))
#define WR_BE32(p,v)	{ (p)[3]=(uint8_t)(v&0xff); \
							  (p)[2]=(uint8_t)((v>>8)&0xff); \
							  (p)[1]=(uint8_t)((v>>16)&0xff); \
							  (p)[0]=(uint8_t)((v>>24)&0xff); }

#ifndef SWAP_2
#define SWAP_2(x) ((uint16_t)((x) << 8) | ((uint16_t)(x) >> 8))
#endif
#ifndef SWAP_4
#define SWAP_4(x) ((uint32_t)((x) << 24) | (((x) << 8) & 0x00FF0000) | (((x) >> 8) & 0x0000FF00) | ((x) >> 24))
#endif


/**************************************************************************
 *                                  "Private" helpers to the macros
 **************************************************************************/
#define NEXT_B2(x)	( (x) | ( (x) >> 1) )
#define NEXT_B4(x)	( NEXT_B2(x) | ( NEXT_B2(x) >> 2) )
#define NEXT_B8(x)	( NEXT_B4(x) | ( NEXT_B4(x) >> 4) )
#define NEXT_B16(x)	( NEXT_B8(x) | ( NEXT_B8(x) >> 8) )
#define NEXT_B32(x)	(NEXT_B16(x) | (NEXT_B16(x) >>16) )

//lint -emacro(778, B8__) -esym(778, B8__)
#define HEX__(n)	0x##n##LU
#define B8__(x)	((x&0x0000000FLU)?1:0) \
						+((x&0x000000F0LU)?2:0) \
						+((x&0x00000F00LU)?4:0) \
						+((x&0x0000F000LU)?8:0) \
						+((x&0x000F0000LU)?16:0) \
						+((x&0x00F00000LU)?32:0) \
						+((x&0x0F000000LU)?64:0) \
						+((x&0xF0000000LU)?128:0)
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef struct
{
	uint32_t length;
	void* pData;
}SRC_ELEMENT;

typedef struct
{
   uint32_t numSrcElements;
   SRC_ELEMENT* pSrc;
}SRC_ELEMENT_LIST;
/**************************************************************************
 *                                  Variables
 **************************************************************************/
extern const char *const stringMonthsLong[12];
extern const char *const stringMonthsShort[12];
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
bool CpuIsLittleEndian(void);

int32_t antoi(char const *pStr, uint32_t maxChars);
uint32_t GetStringIndexFromTable (char const *const *ppTable, char const *pSearchStr, uint32_t maxIndex);

int stricmp(char const *pStr1, char const *pStr2);
int strnicmp(char const *pStr1, char const *pStr2, uint32_t num);

char * stristr(char *pStr1, char *pStr2);
char const * strTrimFront(char const *pStr);		// Removes leading whitespace
char * strTrimTail(char *pStr);	//lint -esym(534, strTrimTail)
char * strTrim(char *pStr);
char * Strxtoa(uint32_t v,char *pStr, int32_t r, int32_t isNeg);
char * Stritoa(int32_t v, char *pStr, int32_t r);
void strncatf(char *pDest, uint32_t maxLen, char const *pFormat, ...);
void StringifyTime(char *pDest, uint32_t destLen, char const *pFormat, time_t const *pData);

uint32_t aton(char const *pAddr);
void macToa(char *pDest, char const *pMac);
void atoMac(char *pDest, char const *pMac);
bool stringIsValidIp(char const *pIp);

void CopyToContiguousArray(uint8_t *pDest, SRC_ELEMENT const *pSrc, uint32_t numElements);
uint32_t SrcElementsGetLength(SRC_ELEMENT const *pSrc, uint32_t numElements);
uint32_t SrcElementsMemMem(SRC_ELEMENT const *pHaystack, uint32_t numHaystacks, void const *pNeedle, uint32_t needleLength);
bool SrcElementsMemCpy(SRC_ELEMENT const *pHaystack, uint32_t numHaystacks, void *pDest, uint32_t copyLength);
void SrcElementsMoveToStartOffset(SRC_ELEMENT **ppHaystack, uint32_t *pNumHaystacks, uint32_t startOffset);

// Convert ASCII-Hex to array. Whitespace allowed. Optional pSrcEnd points to next char (like strtol)
uint32_t AsciiHexToBytes(uint8_t *pDest, uint32_t destLen, char const *pSrc, char const **pSrcEnd);
void SwitchBytesInString(char *pDest, char const *pSrc);

uint32_t BitReverseWord(uint32_t x);
uint8_t BitReverseByte(uint8_t x);
uint32_t BitRoundToNextPowerOf2(uint32_t v);
uint8_t BitsSetInWord(uint32_t x);
uint8_t BitsSetInHalfWord(uint16_t x);
uint8_t BitsSetInByte(uint8_t x);

void *memmem(void const *pHaystack, uint32_t haystackSize, void const *pNeedle, uint32_t needleLength);
void memset32(void *pDest, void const *pMemsetWord, uint32_t bufferSize);
uint32_t memdif(void const *pSrc1, void const *pSrc2, uint32_t startIndex, uint32_t compareLength);

void assert_failed(char const *pFileName, uint32_t lineNumber, char const *pMsg);

// "maxUlps" is the "Units in the Last Place", which specifies how big an error the caller is willing
// to accept in terms of the value of the least significant digits of the FP number's representation.
// AKA how many representable floats we are willing to accept between A and B
bool Fp32AlmostEqual(float A, float B, int32_t maxUlps);

#endif
