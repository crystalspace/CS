/*
    Generic Console Output Support
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

static int cs_fputsn (FILE* file, const char* str, size_t len)
{
//#ifdef CS_HAVE_FPUTWS
#if 0
  // Jorrit: Disabled for now since on some linuxes it appears
  // that fputws is buggy.
  size_t wstrSize = len + 1;
  CS_ALLOC_STACK_ARRAY(wchar_t, wstr, wstrSize);
  
  csUnicodeTransform::UTF8toWC (wstr, wstrSize, (utf8_char*)str, len);
    
  return fputws (wstr, file);
#else
  // Use a cheap UTF8-to-ASCII conversion.
  const utf8_char* ch = (utf8_char*)str;
  
  int n = 0;
  while (len-- > 0)
  {
    utf8_char type = *ch & 0xc0;
    
    if (type <= 0x40)
    {
      if (fputc ((char)*ch, file) == EOF) return EOF;
      n++;
    }
    else if (type == 0xc0)
    {
      if (fputc ('?', file) == EOF) return EOF;
      n++;
    }
      
    ch++; 
  }
  
  return n;
#endif
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
    if (isTTY && (cmdClass == csAnsiParser::classFormat))
    {
      // Only let formatting codes through
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

int csFPutErr (const char* str)
{
  int ret = csFPutStr (stderr, str);
  fflush (stderr);
  return ret;
}

int csPrintfErr (const char* str, ...)
{
  va_list args;
  va_start (args, str);
  int const rc = csPrintfErrV (str, args);
  va_end (args);
  return rc;
}

int csPrintfErrV (const char* str, va_list args)
{
  csString temp;
  temp.FormatV (str, args);
  
  return csFPutErr (temp);
}
