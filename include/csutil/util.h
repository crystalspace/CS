/*
    Copyright (C) 1998 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_UTIL_H__
#define __CS_UTIL_H__

#include <stdio.h>
#include "csextern.h"
#include "csunicode.h"

/**\file
 */

/**\addtogroup util
 * @{ */
 
/**
 * Allocate a new char [] and copy the string into the newly allocated 
 * storage.
 * This is a handy method for copying strings, in fact it is the C++ analogue
 * of the strdup() function from string.h (strdup() is not present on some
 * platforms). To free the pointer the caller should call delete[].
 */
CS_CRYSTALSPACE_EXPORT char *csStrNew (const char *s);
/**
 * Allocate a new char [] and copy an UTF-8 version of the string into 
 * the newly allocated storage.
 */
CS_CRYSTALSPACE_EXPORT char *csStrNew (const wchar_t *s);
/**
 * Allocate a new widechar [] and the string into the newly allocated storage.
 */
CS_CRYSTALSPACE_EXPORT wchar_t* csStrNewW (const wchar_t *s);
/**
 * Allocate a new widechar [] and copy the string converted from UTF-8 into 
 * the newly allocated storage.
 */
CS_CRYSTALSPACE_EXPORT wchar_t* csStrNewW (const char *s);

/**
 * Perform case-insensitive string comparison. Returns a negative number if \p
 * str1 is less than \p str2, zero if they are equal, or a positive number if
 * \p str1 is greater than \p str2. For best portability, use function rather
 * than strcasecmp() or stricmp().
 */
CS_CRYSTALSPACE_EXPORT int csStrCaseCmp(char const* str1, char const* str2);

/**
 * Perform case-insensitive string comparison of the first \p n characters of
 * \p str1 and \p str2. Returns a negative number if the n-character prefix of
 * \p str1 is less than \p str2, zero if they are equal, or a positive number
 * if the prefix of \p str1 is greater than \p str2. For best portability, use
 * function rather than strncasecmp() or strnicmp().
 */
CS_CRYSTALSPACE_EXPORT int csStrNCaseCmp(char const* str1, char const* str2,
  size_t n);

/**
 * Helper class to convert widechar* to char*(UTF-8) strings for use
 * as function parameters.
 * Use of this helper class is more convenient than an a 
 * csStrNewW() / delete[], but essentially does the same.
 * \code
 *   wchar_t* wstr = L"Hello World";
 *    ... 
 *   iNativeWindow* natwin =  ... ;
 *     natwin->SetTitle (csWtoC (wstr));
 * \endcode
 */
struct csWtoC
{
private:
  char* s;
public:
  /**
   * Constructor.
   * Stores an UTF-8 converted version of \p ws internally.   
   */
  csWtoC (const wchar_t* ws)
  { s = csStrNew (ws); }
  /**
   * Deletes the internally stored string.
   */
  ~csWtoC ()
  { delete[] s; }
  /**
   * Retrieve the converted string.
   */
  operator const char* () const
  { return s; }
};

/**
 * Helper class to convert (UTF-8)widechar* to char* strings for use
 * as function parameters.
 * Use of this helper class is more convenient than an a 
 * csStrNewW() / delete[], but essentially does the same.
 */
struct csCtoW
{
private:
  wchar_t* ws;
public:
  /**
   * Constructor.
   * Stores an UTF-16 converted version of \p s internally.   
   */
  csCtoW (const char* s)
  { ws = csStrNewW (s); }
  /**
   * Deletes the internally stored string.
   */
  ~csCtoW ()
  { delete[] ws; }
  /**
   * Retrieve the converted string.
   */
  operator const wchar_t* () const
  { return ws; }
};

/**
 * Expand a filename if it contains shortcuts.
 * Currently the following macros are recognised and expanded:
 * <pre>
 * '.', '~', '..', 'drive:' (on DOS/Win32/OS2)
 * </pre>
 * The returned filename is always absolute, i.e. it always starts
 * from root. Return a string allocated with csStrNew().
 */
CS_CRYSTALSPACE_EXPORT char *csExpandName (const char *iName);

/**
 * Split a pathname into separate path and name.
 */
CS_CRYSTALSPACE_EXPORT void csSplitPath (const char *iPathName, char *oPath,
  size_t iPathSize, char *oName, size_t iNameSize);

/**
 * Perform shell-like filename \e globbing (pattern matching).
 * The special token \p * matches zero or more characters, and the token \p ? 
 * matches exactly one character. Examples: "*a*.txt", "*a?b*", "*"
 * Character-classes \p [a-z] are not understood by this function.
 * \remark If you want case-insensitive comparison, convert \p fName and
 *   \p fMask to upper- or lower-case first.
 */
CS_CRYSTALSPACE_EXPORT bool csGlobMatches (const char *fName, const char *fMask);

/**
 * Finds the smallest number that is a power of two and is larger or
 * equal to n.
 */
CS_CRYSTALSPACE_EXPORT int csFindNearestPowerOf2 (int n);

/// Returns true if n is a power of two.
CS_CRYSTALSPACE_EXPORT bool csIsPowerOf2 (int n);

/// Find the log2 of 32bit argument.
static inline int csLog2 (int n)
{
  const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
  const unsigned int S[] = {1, 2, 4, 8, 16};
  int i;

  register unsigned int c = 0; // result of log2(v) will go here
  for (i = 4; i >= 0; i--) // unroll for speed...
  {
    if (n & b[i])
    {
      n >>= S[i];
      c |= S[i];
    } 
  }
  return c;
}

/**
 * Given \p src and \p dest, which are already allocated, copy \p source to \p
 * dest.  But, do not copy \p search, instead replace that with \p replace
 * string.  \p max is size in bytes of \p dest.
 */
CS_CRYSTALSPACE_EXPORT void csReplaceAll (char *dest, const char *src,
  const char *search, const char *replace, int max);

/**
 * Given \p src and \p dest, which are already allocated, copy \p source to \p
 * dest.  But, do not copy \p search, instead replace that with \p replace
 * string.  \p max is size in bytes of \p dest.
 * \deprecated Use csReplaceAll() instead.
 */
/* CS_DEPRECATED_METHOD */
CS_CRYSTALSPACE_EXPORT inline void csFindReplace (char *dest, const char *src,
  const char *search, const char *replace, int max)
{ csReplaceAll(dest, src, search, replace, max); }

/** @} */
  
#endif // __CS_UTIL_H__
