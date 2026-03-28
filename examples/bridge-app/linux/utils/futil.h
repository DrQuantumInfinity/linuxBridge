#pragma once
/*******************************************************************************************
FILE: futil.h

********************************************************************************************/

#include <stdint.h>


/**************************************************************************
 *                                  Constants
 **************************************************************************/

#ifndef F_DELETED_CHAR
#define F_DELETED_CHAR	((char)0xE5)
#endif

/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef struct
{
   char path[255]; // pathname /directory1/dir2/
   char name[255]; // long file name
} F_PATH_NAME;


/**************************************************************************
 *                                  Prototypes
 **************************************************************************/

// String functions:
uint8_t FutilDriveNum(char const *pName);
bool FutilSplitName(F_PATH_NAME *pDest, char const *pName);
char const *FutilFileName(char const *pName);
char const *FutilFileExtension(char const *pName);
bool FutilFilenameIsValid(char const *pName, bool wildCardOkay);
bool FutilWildcardNameIsMatch(char const *pWildName, char const *pName);
bool FutilNameIsOnlyDots(char const *pName);
char *FutilUpDir(char *pPath); // Strip last folder from path and return folder name
