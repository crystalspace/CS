/*
    DOS support for Crystal Space 3D library
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Written by David N. Arnold <derek_arnold@fuse.net>
    Written by Andrew Zabolotny <bit@eltech.ru>

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

// In DOS we do not want printf() to work in graphics mode
// since it shows garbage on screen

#include <stdarg.h>
#include <stdio.h>

#include "stdarg.h"
#include "cssysdef.h"
#include "cssys/sysfunc.h"

extern bool EnablePrintf;

// to be called instead of printf (exact same prototype/functionality of printf)
int csPrintf (const char *str, ...)
{
  int rc = 0;
  if (EnablePrintf)
  {
    va_list arg;
    va_start (arg, str);
    rc = ::vprintf (str, arg);
    va_end (arg);
  }
  return rc;
}

// to be called instead of vprintf
int csPrintfV (const char *str, va_list arg)
{
  int rc = 0;
  if (EnablePrintf)
    rc = ::vprintf (str, arg);
  return rc;
}

