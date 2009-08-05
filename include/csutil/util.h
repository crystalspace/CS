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
#include "csutil/csunicode.h"

/**\file
 * Miscellaneous utilities
 */

/**\addtogroup util
 * @{ */

namespace CS
{
  /**
   * Allocate a char string with cs_malloc() and copy the string into the 
   * newly allocated storage.
   * This is a handy method for copying strings, in fact it is an analogue
   * of the strdup() function from string.h, but using cs_malloc(). (Also,
   * strdup() is not present on some platforms). To free the pointer the 
   * caller should call cs_free().
   */
  CS_CRYSTALSPACE_EXPORT char* StrDup (const char *s);
  /**
   * Allocate a char string with cs_malloc() and copy an UTF-8 version of the 
   * string into the newly allocated storage.
   * \sa StrDup(const char*)
   */
  CS_CRYSTALSPACE_EXPORT char* StrDup (const wchar_t *s);
  /**
   * Allocate a wide char string with cs_malloc() and copy the string into 
   * the newly allocated storage.
   * \sa StrDup(const char*)
   */
  CS_CRYSTALSPACE_EXPORT wchar_t* StrDupW (const wchar_t *s);
  /**
   * Allocate a wide char string with cs_malloc() and copy the string 
   * converted from UTF-8 into the newly allocated storage.
   * \sa StrDup(const char*)
   */
  CS_CRYSTALSPACE_EXPORT wchar_t* StrDupW (const char *s);
}

///\internal Helper function needed by the csStrNew variants below
CS_CRYSTALSPACE_EXPORT size_t cs_wcslen (wchar_t const* s);

/**
 * Allocate a new char [] and copy the string into the newly allocated 
 * storage.
 * This is a handy method for copying strings, in fact it is the C++ analogue
 * of the strdup() function from string.h (strdup() is not present on some
 * platforms). To free the pointer the caller should call delete[].
 * \sa CS::StrDup which is slightly more efficient
 */
inline char *csStrNew (const char *s)
{
  if (!s) return 0;
  size_t sl = strlen (s) + 1;
  char *r = new char [sl];
  memcpy (r, s, sl);
  return r;
}
/**
 * Allocate a new char [] and copy an UTF-8 version of the string into 
 * the newly allocated storage.
 * \sa csStrNew(const char*)
 * \sa CS::StrDup which is slightly more efficient
 */
inline char *csStrNew (const wchar_t *s)
{
  if (!s) return 0;
  char* cs = CS::StrDup (s);
  size_t sl = strlen (cs) + 1;
  char *r = new char [sl];
  memcpy (r, cs, sl);
  cs_free (cs);
  return r;
}
/**
 * Allocate a new widechar [] and copy the string into the newly allocated 
 * storage.
 * \sa csStrNew(const char*)
 * \sa CS::StrDupW which is slightly more efficient
 */
inline wchar_t* csStrNewW (const wchar_t *s)
{
  if (!s) return 0;
  size_t sl = cs_wcslen (s) + 1;
  wchar_t *r = new wchar_t [sl];
  memcpy (r, s, sl * sizeof (wchar_t));
  return r;
}
/**
 * Allocate a new widechar [] and copy the string converted from UTF-8 into 
 * the newly allocated storage.
 * \sa csStrNew(const char*)
 * \sa CS::StrDupW which is slightly more efficient
 */
inline wchar_t* csStrNewW (const char *s)
{
  if (!s) return 0;
  wchar_t* ws = CS::StrDupW (s);
  size_t sl = cs_wcslen (ws) + 1;
  wchar_t *r = new wchar_t [sl];
  memcpy (r, ws, sl * sizeof (wchar_t));
  cs_free (ws);
  return r;
}

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
 * Helper class to convert wchar_t* to char* (UTF-8 encoded) strings for use
 * as function parameters.
 * Use of this helper class is more convenient than a csStrNew() / delete[]
 * pair, but essentially does the same (with the convenience of automatic
 * cleanup).
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
  { s = CS::StrDup (ws); }
  /**
   * Deletes the internally stored string.
   */
  ~csWtoC ()
  { cs_free (s); }
  /**
   * Retrieve the converted string.
   */
  operator const char* () const
  { return s; }
};

/**
 * Helper class to convert char* (UTF-8 encoded )to wchar_t* strings for use
 * as function parameters.
 * Use of this helper class is more convenient than a csStrNewW() / delete[]
 * pair, but essentially does the same (with the convenience of automatic
 * cleanup).
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
  { ws = CS::StrDupW (s); }
  /**
   * Deletes the internally stored string.
   */
  ~csCtoW ()
  { cs_free (ws); }
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
CS_CRYSTALSPACE_EXPORT bool csGlobMatches (const char *fName,
	const char *fMask);

/**
 * Finds the smallest number that is a power of two and is larger or
 * equal to n.
 */
static inline int csFindNearestPowerOf2 (int n)
{
  int v=n;

  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;

  return v;
}

/// Returns true if n is a power of two.
static inline bool csIsPowerOf2 (int n)
{
  return !(n & (n - 1)) && n;	// (n-1) ^ n >= n;
}

/// Find the log2 of 32bit argument.
CS_CRYSTALSPACE_EXPORT int csLog2 (int n);

/**
 * Given \p src and \p dest, which are already allocated, copy \p source to \p
 * dest.  But, do not copy \p search, instead replace that with \p replace
 * string.  \p max is size in bytes of \p dest.
 */
CS_CRYSTALSPACE_EXPORT void csReplaceAll (char *dest, const char *src,
  const char *search, const char *replace, int max);


/** @} */
  
#endif // __CS_UTIL_H__
