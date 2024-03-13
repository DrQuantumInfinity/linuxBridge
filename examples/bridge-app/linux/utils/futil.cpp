/*******************************************************************************************
Provides file utilities

(c) Copyright 2010, SkyTrac Systems Inc. All rights reserved.
This source code contains confidential, trade secret material. Any attempt or participation
in deciphering, decoding, reverse engineering or in any way altering the source code is
strictly prohibited, unless the prior written consent of SkyTrac Systems is obtained.
********************************************************************************************/


#include "futil.h"

#include <string.h>
#include <ctype.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
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
uint8_t FutilDriveNum(char const *pName)
{
   uint8_t drv;
   if (pName[0] == '/')
   {
      drv = pName[1] - '0';
   }
   else if (isdigit(pName[0]))
   {
      drv = pName[0] - '0';
   }
   else
   {
      drv = 0;
   }
   return drv;
}

bool FutilSplitName(F_PATH_NAME *pDest, char const *pName)
{
   char const *pPos;
   char const *pPathEndPos;
   char ch;

   pDest->name[0] = '\0';
   pDest->path[0] = '\0';

   // get path and filename:
   pPathEndPos = NULL;
   pPos = pName;
   while ((ch = *pName++) != '\0')
   {
      if ((ch == '/') || (ch == '\\'))
         pPathEndPos = pName;
   }
   if (pPathEndPos)
   {
      if ((void*)pDest != (void*)pName)
      {
         if ((uint32_t)(pPathEndPos - pPos) < sizeof(pDest->path))
         {
            strncpy(pDest->path, pPos, (uint32_t)(pPathEndPos - pPos));
            pDest->path[(uint32_t)(pPathEndPos - pPos)] = '\0';
         }
      }
      else
         pDest->path[pPos - pName] = '\0';
      pPos = pPathEndPos;
   }
   if ((void*)pDest != (void*)pName)
      strncpy(pDest->name, pPos, sizeof(pDest->name));

   return true;
}

char const *FutilFileName(char const *pName)
{
   char const *pPathEndPos;
   char ch;

   // find last folder delimiter / or \:
   pPathEndPos = pName;
   while ((ch = *pName++) != '\0')
   {
      if ((ch == '/') || (ch == '\\'))
         pPathEndPos = pName;
   }
   return pPathEndPos;
}

char const *FutilFileExtension(char const *pName)
{
   char const *pLastDot = NULL;
   char ch;

   // find last '.':
   while ((ch = *pName++) != '\0')
   {
      if (ch == '.')
         pLastDot = pName;
   }
   if (!pLastDot)
   {
      pLastDot = pName;
   }
   return pLastDot;
}


bool FutilFilenameIsValid(char const *pName, bool wildCardOkay)
{
   char ch;

   if (!pName || (*pName == F_DELETED_CHAR))
      return false;

   while ((ch = *pName++) != '\0')
   {
      if (((uint8_t)ch < 0x20) || (ch == 0x22) ||
          (ch == '|') || (ch == '<') || (ch == '>') ||
          (ch == '/') || (ch == '\\') || (ch == ':'))
      {
         return false;
      }
      if (!wildCardOkay && ((ch == '?') || (ch == '*')))
      {
         return false;
      }
   }
   return true;
}

// Derived from Alessandro Cantatore (http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html)
/*
# The code mentioned below is the copyright property of Alessandro Felice Cantatore with the exception
  of those algorithms which are, as specified in the documentation, property of the respective owners.
# You are not allowed to patent or copyright this code as your!
# If you work for Microsoft you are not allowed to use this code!
# If you work for IBM you can use this code only if the target OS is OS/2 or linux.
# You are not allowed to use this code for projects or programs owned or used by the army, the
  department of defense or the government of any country violating human rights or international laws
  (including those western countries pretending to "export" democracy).
If you are unsure just check what Amnesty International reports about your country.
# You are not allowed to use this code in programs used in activities causing harm to living beeings
  or to the environment.
# You are free to use the algorithms described below in your programs, or modify them to suit your
  needs, provided that you include their source, including these notes and my name, (you are not
  required to include the whole source of your program) in your program package.
# You are not allowed to sell this code, but you can include it into a commercial program.
# If you have any doubt ask the author.
*/
bool FutilWildcardNameIsMatch(char const *pat, char const *str)
{
   char const *s;
   char const *p;
   bool star = false;

loopStart:
   for (s = str, p = pat; *s; ++s, ++p)
   {
      switch (*p)
      {
      case '?':
         if (*s == '.') goto starCheck; //lint !e801
         break;
      case '*':
         star = true;
         str = s, pat = p;
         if (!*++pat) return true;
         goto loopStart; //lint !e801
      default:
         if (toupper(*s) != toupper(*p))
            goto starCheck; //lint !e801
         break;
      } /* endswitch */
   } /* endfor */
   if (*p == '*') ++p;
   return (bool)(!*p);

starCheck:
   if (!star) return false;
   str++;
   goto loopStart; //lint !e801
}

bool FutilNameIsOnlyDots(char const *pName)
{
   char ch;
   while ((ch = *pName++) != '\0')
   {
      if (ch != '.')
         return false;
   }
   return true;
}

// Strips the last folder from the current path. Will return the folder name that was removed, so
// the incoming string may require room for one extra '\0' character
char *FutilUpDir(char *pPath)
{
   size_t len = strlen(pPath);
   bool trailingSlash = false;
   char *pRetName = NULL;
   char *pSlash;

   if ((pPath[len - 1] == '\\') || (pPath[len - 1] == '/'))
   {
      pPath[len - 1] = '\0';
      trailingSlash = true;
   }

   pSlash = strrchr(pPath, '\\');
   if (!pSlash)
      pSlash = strrchr(pPath, '/');

   if (pSlash)
   {
      if (trailingSlash)
      {
         pSlash++;
         memmove(pSlash + 1, pSlash, (uint32_t)(len - (uint32_t)(pSlash - pPath)));
      }
      pRetName = pSlash + 1;
      *pSlash = '\0';
   }
   else if (trailingSlash)
      pPath[len - 1] = '/'; // restore original string

   return pRetName;
}


/**************************************************************************
 *                                 Private Functions
 **************************************************************************/
