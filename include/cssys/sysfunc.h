/*
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

#ifndef __CS_SYSFUNC_H__
#define __CS_SYSFUNC_H__

#include <stdarg.h>
#include <stdio.h>

/// CS version of printf
extern int csPrintf (const char* str, ...);
/// CS version of vprintf
extern int csPrintfV (const char* str, va_list arg);

/// Get the current tick count.
extern csTicks csGetTicks ();

/**
 * Get the installation path.<p>
 * This returns the path where the system has been installed to.
 * It has a limited use because mostly everything should be done
 * through VFS which is installation directory - independent; but
 * some initialization tasks still need this.
 */
extern bool csGetInstallPath (char *oInstallPath, size_t iBufferSize);

/**
 * This function will freeze your application for given number of 1/1000
 * seconds. The function is very inaccurate, so don't use it for accurate
 * timing. It may be useful when the application is idle, to explicitly
 * release CPU for other tasks in multi-tasking operating systems.
 */
extern void csSleep (int /*SleepTime*/);

#endif // __CS_SYSFUNC_H__

