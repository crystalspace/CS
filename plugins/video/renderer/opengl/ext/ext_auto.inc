/*  -*- Mode: C++; -*- */
/*
    Copyright (C) 1998 by Jorrit Tyberghein
    File Created by Gary Haussmann 1/12/2000

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

/* the purpose of this file is to include platform/OS/whatever-specific
 * code for detecting OpenGL extensions.  While the standard is
 * moving toward a glGetProcAddress() type interface, there are
 * still other versions.  With typical unix installations you can refer to the
 * extension functions and let the loader worry about it; with Windows
 * you need to explicitly declare the extension function prototypes
 * and resolve them with wglGetProcAddress().
 *
 * If you have code to detect and load OpenGL extensions for a specific
 * installation, add a module to this directory and refer to it from
 * this file so that your code is loaded when compiled on the
 * relevant platform.
 */

// win32 extension detection by Robert Bergkvist
#if defined(OS_WIN32)
#include "ext_w32.inc"

// Mesa 3.1/Linux extension detection by Gary Haussmann
// if you get CS to compile and detect extensions on another
// Mesa platform besides LINUX plz add a test here
#elif (MESA_MAJOR_VERSION >= 3) && (MESA_MINOR_VERSION >= 1)
#include "ext_mesa.inc"

#elif defined(OS_LINUX) && defined(COMP_GCC)
#include "ext_mesa.inc"

#elif defined(OS_MACOS)
#include "ext_mesa.inc"

// Default no extensions...
#else
#undef USE_EXTENSIONS
#undef USE_MULTITEXTURE
void csGraphics3DOGLCommon::DetectExtensions()
{
  SysPrintf(MSG_INITIALIZATION,
    "OpenGL extensions not supported on this platform!\n");
  return;
}
#define _DEFINED_DETECTION_METHOD
#endif

// default detection function.  Maybe we should make
// some sort of base/subclass hierarchy for extension
// detection, but that seems like a needless mess.  Of
// course this module is kind of a mess :)
#ifndef _DEFINED_DETECTION_METHOD
#error Opengl extension detection not supported on this platform.
#endif
