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
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define CS_SYSDEF_PROVIDE_GETCWD
#define CS_SYSDEF_PROVIDE_EXPAND_PATH
#include "cssysdef.h"

#include "csutil/csunicode.h"
#include "csutil/csuctransform.h"
#include "csutil/util.h"

static const size_t shortStringChars = 64;

#if defined(CS_USE_FAKE_WCSLEN)

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

#if defined (CS_PLATFORM_DOS)
  #define IS_PATH_SEPARATOR(c)	\
    (((c) == PATH_SEPARATOR) || ((c) == '/') || ((c) == ':'))
#else
  #define IS_PATH_SEPARATOR(c)	\
    (((c) == PATH_SEPARATOR) || ((c) == '/'))
#endif

#ifndef CS_PROVIDES_EXPAND_PATH
// generic csExpandName for all platforms

#ifdef CS_COMPILER_BCC
static int __getcwd (char drive, char *buffer, int buffersize) {
  _getdcwd(drive, buffer, buffersize);
  return strlen(buffer);
}
#endif

#if defined (CS_PLATFORM_DOS)
// We need a function to retrieve current working directory on specific drive

static int __getcwd (char drive, char *buffer, int buffersize)
{
  unsigned int old_drive, num_drives;
  _dos_getdrive (&old_drive);
  _dos_setdrive (drive, &num_drives);
  getcwd (buffer, buffersize);
  _dos_setdrive (old_drive, &num_drives);
  return strlen (buffer);
}

#endif // defined (CS_PLATFORM_DOS)

char *csExpandName (const char *iName)
{
  char outname [CS_MAXPATHLEN + 1];
  int inp = 0, outp = 0, namelen = strlen (iName);
  while ((outp < CS_MAXPATHLEN)
      && (inp < namelen))
  {
    char tmp [CS_MAXPATHLEN + 1];
    int ptmp = 0;
    while ((inp < namelen) && (!IS_PATH_SEPARATOR(iName[inp]) ))
      tmp [ptmp++] = iName [inp++];
    tmp [ptmp] = 0;

    if ((ptmp > 0)
     && (outp == 0)
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
     && ((inp >= namelen)
      || (iName [inp] != ':'))
#endif
        )
    {
      getcwd (outname, sizeof (outname));
      outp = strlen (outname);
      if (strcmp (tmp, "."))
        outname [outp++] = PATH_SEPARATOR;
    } /* endif */

#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
    // If path starts with '/' (root), get current drive
    if ((ptmp == 0)
     && (outp == 0))
    {
      getcwd (outname, sizeof (outname));
      if (outname [1] == ':')
        outp = 2;
    } /* endif */
#endif

    if (strcmp (tmp, "..") == 0)
    {
      while ((outp > 0)
          && ((outname [outp - 1] == '/')
           || (outname [outp - 1] == PATH_SEPARATOR)
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
           || (outname [outp - 1] == ':')
#endif
             )
            )
        outp--;
      while ((outp > 0)
          && (outname [outp - 1] != '/')
          && (outname [outp - 1] != PATH_SEPARATOR)
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
          && (outname [outp - 1] != ':')
#endif
            )
        outp--;
    }
    else if (strcmp (tmp, ".") == 0)
    {
      // do nothing
    }
    else if (strcmp (tmp, "~") == 0)
    {
      // strip all output path; start from scratch
      strcpy (outname, "~/");
      outp = 2;
    }
    else
    {
      memcpy (&outname [outp], tmp, ptmp);
      outp += ptmp;
      if (inp < namelen)
      {
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
        if ((inp == 1)
         && (iName [inp] == ':'))
          if ((iName [inp + 1] == '/')
           || (iName [inp + 1] == PATH_SEPARATOR))
            outname [outp++] = ':';
          else
            outp += __getcwd (iName [inp - 1], outname + outp - 1,
	      sizeof (outname) - outp + 1) - 1;
        if ((outname [outp - 1] != '/')
         && (outname [outp - 1] != PATH_SEPARATOR))
#endif
        outname [outp++] = PATH_SEPARATOR;
      }
    } /* endif */
    while ((inp < namelen)
        && ((iName [inp] == '/')
         || (iName [inp] == PATH_SEPARATOR)
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
         || (iName [inp] == ':')
#endif
           )
          )
      inp++;
  } /* endwhile */

  char *ret = new char [outp + 1];
  memcpy (ret, outname, outp);
  ret [outp] = 0;
  return ret;
}

#else
// platform has it's own path expansion routines

char *csExpandName (const char *iName)
{
  char outname [CS_MAXPATHLEN + 1];
  csPlatformExpandPath (iName, outname, sizeof(outname));
  return csStrNew (outname);
}

#endif

void csSplitPath (const char *iPathName, char *oPath, size_t iPathSize,
  char *oName, size_t iNameSize)
{
  size_t sl = strlen (iPathName);
  size_t maxl = sl;
  while (sl && (!IS_PATH_SEPARATOR (iPathName [sl - 1])))
    sl--;

  if (iPathSize)
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

  if (iNameSize)
    if (maxl - sl >= iNameSize)
    {
      memcpy (oName, &iPathName [sl], iNameSize - 1);
      oName [iNameSize - 1] = 0;
    }
    else
      memcpy (oName, &iPathName [sl], maxl - sl + 1);
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

// don't forget to revert the changes above
#if defined(__CYGWIN__)
#define CS_PLATFORM_WIN32
#endif

// finds the smallest number that is a power of two and is larger or equal to n
int csFindNearestPowerOf2 (int n)
{
  int w=1;

  while (n > w)  w <<= 1;

  return w;
}

// returns true if n is a power of two
bool csIsPowerOf2 (int n)
{
  if (n <= 0)
    return false;
  return !(n & (n - 1));	// (n-1) ^ n >= n;
}

/**
 * given src and dest, which are already allocated, copy source to dest.
 * But, do not copy 'search', instead replace that with 'replace' string.
 * max is size of dest
*/
void csFindReplace(char *dest, const char *src, const char *search,
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
    strncpy(destpos, srcpos, beforelen);
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
