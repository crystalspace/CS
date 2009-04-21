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

#include "cssysdef.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "csutil/csunicode.h"
#include "csutil/csuctransform.h"
#include "csutil/util.h"

static const size_t shortStringChars = 64;

#if !defined(CS_HAVE_WCSLEN)

static size_t wcslen(wchar_t const* s)
{
  size_t n = 0;
  while (*s++ != 0)
    n++;
  return n;
}

#endif

char *csStrNew (const char *s)
{
  if (s)
  {
    size_t sl = strlen (s) + 1;
    char *r = new char [sl];
    memcpy (r, s, sl);
    return r;
  }
  else
    return 0;
}

char *csStrNew (const wchar_t *s)
{
  if (!s) return 0;

  utf8_char buf[shortStringChars];
  static const size_t bufChars = sizeof (buf) / sizeof (utf8_char);
  size_t charsNeeded;

  if ((charsNeeded = 
    csUnicodeTransform::WCtoUTF8 (buf, bufChars, s, (size_t)-1)) > bufChars)
  {
    utf8_char* newbuf = new utf8_char[charsNeeded];
    csUnicodeTransform::WCtoUTF8 (newbuf, charsNeeded, s, (size_t)-1);
    return (char*)newbuf;
  }
  else
  {
    return csStrNew ((char*)buf);
  }
}

wchar_t* csStrNewW (const wchar_t *s)
{
  if (s)
  {
    size_t sl = wcslen (s) + 1;
    wchar_t *r = new wchar_t [sl];
    memcpy (r, s, sl * sizeof (wchar_t));
    return r;
  }
  else
    return 0;
}

wchar_t* csStrNewW (const char *s)
{
  if (!s) return 0;

  wchar_t buf[shortStringChars];
  static const size_t bufChars = sizeof (buf) / sizeof (wchar_t);
  size_t charsNeeded;

  if ((charsNeeded = csUnicodeTransform::UTF8toWC (buf, bufChars, 
    (utf8_char*)s, (size_t)-1)) > bufChars)
  {
    wchar_t* newbuf = new wchar_t[charsNeeded];
    csUnicodeTransform::UTF8toWC (newbuf, charsNeeded, (utf8_char*)s, 
      (size_t)-1);
    return newbuf;
  }
  else
  {
    return csStrNewW (buf);
  }
}

namespace CS
{
  char* StrDup (const char *s)
  {
    if (s)
    {
      size_t sl = strlen (s) + 1;
      char *r = (char*)cs_malloc (sl);
      memcpy (r, s, sl);
      return r;
    }
    else
      return 0;
  }

  char* StrDup (const wchar_t *s)
  {
    if (!s) return 0;

    utf8_char buf[shortStringChars];
    static const size_t bufChars = sizeof (buf) / sizeof (utf8_char);
    size_t charsNeeded;

    if ((charsNeeded = 
      csUnicodeTransform::WCtoUTF8 (buf, bufChars, s, (size_t)-1)) > bufChars)
    {
      utf8_char* newbuf = (utf8_char*)cs_malloc (charsNeeded);
      csUnicodeTransform::WCtoUTF8 (newbuf, charsNeeded, s, (size_t)-1);
      return (char*)newbuf;
    }
    else
    {
      return StrDup ((char*)buf);
    }
  }

  wchar_t* StrDupW (const wchar_t *s)
  {
    if (s)
    {
      size_t sl = wcslen (s) + 1;
      wchar_t *r = (wchar_t*)cs_malloc (sl * sizeof (wchar_t));
      memcpy (r, s, sl * sizeof (wchar_t));
      return r;
    }
    else
      return 0;
  }

  wchar_t* StrDupW (const char *s)
  {
    if (!s) return 0;

    wchar_t buf[shortStringChars];
    static const size_t bufChars = sizeof (buf) / sizeof (wchar_t);
    size_t charsNeeded;

    if ((charsNeeded = csUnicodeTransform::UTF8toWC (buf, bufChars, 
      (utf8_char*)s, (size_t)-1)) > bufChars)
    {
      wchar_t* newbuf = (wchar_t*)cs_malloc (charsNeeded * sizeof (wchar_t));
      csUnicodeTransform::UTF8toWC (newbuf, charsNeeded, (utf8_char*)s, 
        (size_t)-1);
      return newbuf;
    }
    else
    {
      return StrDupW (buf);
    }
  }
}

// These functions are not inline in <csutil/util.h> because the underlying
// str{n}casecmp() is not compatible with gcc's "-ansi -pedantic".
int csStrCaseCmp(char const* s1, char const* s2)
{
  return strcasecmp(s1, s2);
}
int csStrNCaseCmp(char const* s1, char const* s2, size_t n)
{
  return strncasecmp(s1, s2, n);
}

#if defined (CS_PLATFORM_DOS)
  #define IS_PATH_SEPARATOR(c)	\
    (((c) == CS_PATH_SEPARATOR) || ((c) == '/') || ((c) == ':'))
#else
  #define IS_PATH_SEPARATOR(c)	\
    (((c) == CS_PATH_SEPARATOR) || ((c) == '/'))
#endif

void csSplitPath (const char *iPathName, char *oPath, size_t iPathSize,
  char *oName, size_t iNameSize)
{
  size_t sl = strlen (iPathName);
  size_t maxl = sl;
  while (sl && (!IS_PATH_SEPARATOR (iPathName [sl - 1])))
    sl--;

  if (iPathSize)
  {
    if (sl >= iPathSize)
    {
      memcpy (oPath, iPathName, iPathSize - 1);
      oPath [iPathSize - 1] = 0;
    }
    else
    {
      memcpy (oPath, iPathName, sl);
      oPath [sl] = 0;
    }
  }

  if (iNameSize)
  {
    if (maxl - sl >= iNameSize)
    {
      memcpy (oName, &iPathName [sl], iNameSize - 1);
      oName [iNameSize - 1] = 0;
    }
    else
      memcpy (oName, &iPathName [sl], maxl - sl + 1);
  }
}

bool csGlobMatches (const char *fName, const char *fMask)
{
  while (*fName || *fMask)
  {
    if (*fMask == '*')
    {
      while (*fMask == '*')
        fMask++;
      if (!*fMask)
        return true;		// mask = "something*"
      while (*fName && *fName != *fMask)
        fName++;
      if (!*fName)
        return false;		// "*C" - fName does not contain C
    }
    else
    {
      if ((*fMask != '?')
       && (*fName != *fMask))
        return false;
      if (*fMask)
        fMask++;
      if (*fName)
        fName++;
    }
  }
  return (!*fName) && (!*fMask);
}

/// Find the log2 of 32bit argument.
int csLog2 (int v)
{
  static const char LogTable256[] = 
  {
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
  };

  unsigned int r;     // r will be lg(v)
  register unsigned int t, tt; // temporaries

  if ((tt = v >> 16))
  {
    r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
  }
  else 
  {
    r = (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
  }

  return r;
}


/**
 * given src and dest, which are already allocated, copy source to dest.
 * But, do not copy 'search', instead replace that with 'replace' string.
 * max is size of dest
*/
void csReplaceAll(char *dest, const char *src, const char *search,
  const char *replace, int max)
{
  const char *found;
  const char *srcpos = src;
  char *destpos = dest;
  size_t searchlen = strlen (search);
  size_t replacelen = strlen (replace);
  destpos[0] = 0;
  size_t sizeleft = max;
  /// find and replace occurrences
  while( (found=strstr(srcpos, search)) != 0 )
  {
    // copy string before it
    int beforelen = found - srcpos;
    sizeleft -= beforelen;
    if(sizeleft <= 0) { destpos[0]=0; return; }
    memcpy (destpos, srcpos, beforelen);
    destpos += beforelen;
    srcpos += beforelen;
    destpos[0]=0;
    // add replacement
    sizeleft -= replacelen;
    if(sizeleft <= 0) { destpos[0]=0; return; }
    strcpy(destpos, replace);
    destpos += replacelen;
    // skip replaced string
    srcpos += searchlen;
  }
  // add remainder of string
  size_t todo = strlen (srcpos);
  sizeleft -= todo;
  if(sizeleft <= 0) { destpos[0]=0; return; }
  strcpy (destpos, srcpos);
  destpos += todo;
  destpos[0]=0;
}
