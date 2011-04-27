/*
    Generic Console Output Support
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Copyright (c) 2004-2005 by Frank Richter

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

#include <stdarg.h>
#include <stdio.h>
#include "cssysdef.h"
#include "csgeom/math.h"
#include "csutil/ansiparse.h"
#include "csutil/csstring.h"
#include "csutil/csuctransform.h"
#include "csutil/sysfunc.h"

// Replacement for printf(); exact same prototype/functionality as printf()
int csPrintf (char const* str, ...)
{
  va_list args;
  va_start (args, str);
  int const rc = csPrintfV (str, args);
  va_end (args);
  return rc;
}

static char CheapToASCII (wchar_t ch)
{
  // Translate some often-used characters to their nearest ASCII equivalent
  switch (ch)
  {
  // Typographical single quotation marks
  case 0x2018:
  case 0x2019:
  case 0x201b:
    return '\'';
  // Typographical double quotation marks
  case 0x201c:
  case 0x201d:
  case 0x201f:
    return '"';
  }
  
  if (ch < 0x80)
    // Already ASCII
    return char (ch);
    // Not ASCII. Not much we can do
  else
    return '?';
}

static int cs_fputsn (FILE* file, const char* str, size_t len)
{
  size_t wstrSize = len + 1;
  CS_ALLOC_STACK_ARRAY(wchar_t, wstr, wstrSize);

  csUnicodeTransform::UTF8toWC (wstr, wstrSize, (utf8_char*)str, len);

  int n = 0;
  const wchar_t* wcsPtr = wstr;
#if defined(CS_HAVE_FPUTWS) && defined(CS_HAVE_FWIDE) \
  && defined(CS_HAVE_WCSNRTOMBS)
  if (fwide (file, 0) > 0)
  {
    return fputws (wstr, file);
  }
  else
  {
    size_t numwcs = wcslen (wstr);
    const wchar_t* wcsEnd = wcsPtr + numwcs;
    mbstate_t oldstate, mbs;
    memset (&mbs, 0, sizeof (mbs));
    char mbstr[64];
    size_t mbslen;
    while (numwcs > 0)
    {
      memset (mbstr, 0, sizeof (mbstr));
      memcpy (&oldstate, &mbs, sizeof (mbs));
      const wchar_t* wcsOld = wcsPtr;
      mbslen = wcsnrtombs (mbstr, &wcsPtr, numwcs, sizeof (mbstr) - 1, &mbs);
      if (mbslen == (size_t)-1)
      {
	if (errno == EILSEQ)
	{
	  /* At least on OS/X wcsPtr is not updated in case of a conversion
	     error, so kludge around it */
	  if (mbstr[0] == 0)
          {
	    // Catch char that couldn't be encoded, print ? instead
	    wchar_t wch = *wcsPtr;
	    if (fputc (CheapToASCII (wch), file) == EOF) return EOF;
	    wcsPtr++;
	    if (CS_UC_IS_HIGH_SURROGATE (wch))
	    {
	      if (CS_UC_IS_LOW_SURROGATE (*wcsPtr))
	        wcsPtr++;
	    }
      	    numwcs = wcsEnd - wcsPtr;
          }
	  else
          {
	    // Try converting only a substring
	    numwcs = csMin (numwcs-1, strlen (mbstr));
	    memcpy (&mbs, &oldstate, sizeof (mbs));
	    wcsPtr = wcsOld;
	  }
	  continue;
	}
	break;
      }
      else
      {
	if (fputs (mbstr, file) == EOF) return EOF;
	numwcs = (wcsPtr == 0) ? 0 : wcsEnd - wcsPtr;
      }
    }
    return (int)len;
  }
#endif
  // Use a cheap Wide-to-ASCII conversion.
  const wchar_t* ch = wcsPtr;
  
  while (len-- > 0)
  {
    wchar_t wch = *ch;
    if (fputc (CheapToASCII (wch), file) == EOF) return EOF;
    ch++;
    if (CS_UC_IS_HIGH_SURROGATE (wch))
    {
      if (CS_UC_IS_LOW_SURROGATE (*ch))
	ch++;
    }
    n++;
  }
  
  return n;
}

static int csFPutStr (FILE* file, const char* str)
{
  bool isTTY = isatty (fileno (file));
  
  int ret = 0;
  size_t ansiCommandLen;
  csAnsiParser::CommandClass cmdClass;
  size_t textLen;
  // Check for ANSI codes
  while (csAnsiParser::ParseAnsi (str, ansiCommandLen, cmdClass, textLen))
  {
    int rc;
    if (isTTY && (cmdClass != csAnsiParser::classNone)
      && (cmdClass != csAnsiParser::classUnknown))
    {
      // Only let known codes through
      rc = cs_fputsn (file, str, ansiCommandLen);
      if (rc == EOF)
	return EOF;
      ret += rc;
    }

    if (textLen > 0)
    {
      rc = cs_fputsn (file, str + ansiCommandLen, textLen);
      if (rc == EOF)
	return EOF;
      ret += rc;
    }

    str += ansiCommandLen + textLen;
  }
  return ret;
}

// Replacement for vprintf()
int csPrintfV(char const* str, va_list args)
{
  csString temp;
  temp.FormatV (str, args);
  
  return csFPutStr (stdout, temp);
}

int csFPrintf (FILE* file, const char* str, ...)
{
  va_list args;
  va_start (args, str);
  int const rc = csFPrintfV (file, str, args);
  va_end (args);
  return rc;
}

int csFPrintfV (FILE* file, const char* str, va_list args)
{
  csString temp;
  temp.FormatV (str, args);
  
  return csFPutStr (file, temp);
}

int csPrintfErrV (const char* str, va_list arg)
{
  int rc = csFPrintfV (stderr, str, arg);
  fflush (stderr);
  return rc;
}

int csPrintfErr (const char* str, ...)
{
  va_list args;
  va_start (args, str);
  int const rc = csFPrintfV (stderr, str, args);
  va_end (args);
  return rc;
}

