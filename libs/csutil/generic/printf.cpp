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
#include "csutil/csstring.h"
#include "csutil/csuctransform.h"
#include "csutil/sysfunc.h"

// Replacement for printf(); exact same prototype/functionality as printf()
int csPrintf(char const* str, ...)
{
  va_list args;
  va_start (args, str);
  int const rc = csPrintfV (str, args);
  va_end (args);
  return rc;
}

static int csFPutStr (FILE* file, const char* str)
{
//#ifdef CS_HAVE_FPUTWS
#if 0
  // Jorrit: Disabled for now since on some linuxes it appears
  // that fputws is buggy.
  size_t wstrSize = strlen (str) + 1;
  CS_ALLOC_STACK_ARRAY(wchar_t, wstr, wstrSize);
  
  csUnicodeTransform::UTF8toWC (wstr, wstrSize, (utf8_char*)str, (size_t)-1);
    
  return fputws (wstr, file);
#else
  // Use a cheap UTF8-to-ASCII conversion.
  const utf8_char* ch = (utf8_char*)str;
  
  int n = 0;
  while (*ch != 0)
  {
    utf8_char type = *ch & 0xc0;
    
    if (type <= 0x40)
      fputc ((char)*ch, file);
    else if (type == 0x80)
      fputc ('?', file);
      
    ch++; n++;
  }
  
  return n;
#endif
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
  return csFPutStr (stderr, str);
}

