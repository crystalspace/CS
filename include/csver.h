/*
    Copyright (C) 1998-2002 by Jorrit Tyberghein
  
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

#ifndef __CS_CSVER_H__
#define __CS_CSVER_H__

/**\file
 * Crystal Space Version Information
 */

// *** NOTE ***
// Also update CS/configure.ac and CS/docs/texinfo/version.txi when updating
// the version number.

/// Major version
#define CS_VERSION_MAJOR CS_VER_QUOTE(0.99)
/// Minor version (release, or "dev" for CVS version)
#define CS_VERSION_MINOR CS_VER_QUOTE(0)
/// Date of release
#define CS_RELEASE_DATE  CS_VER_QUOTE(Mon 7-Jul-2004)

#define CS_VER_QUOTE(X) #X

#if !defined(CS_PLATFORM_NAME)
/// Name of the platform CS is compiled for (i.e. Win32)
#  define CS_PLATFORM_NAME "MysteryPlatform"
#  if defined(CS_COMPILER_GCC)
#  warning Unable to identify platform name using CS_PLATFORM_NAME.
#  elif defined(CS_COMPILER_MSVC)
#  pragma message("Unable to identify platform name using CS_PLATFORM_NAME.")
#  endif
#endif
#if !defined(CS_PROCESSOR_NAME)
/// Name of the processor CS is compiled for (i.e. X86)
#  define CS_PROCESSOR_NAME "MysteryProcessor"
#  if defined(CS_COMPILER_GCC)
#  warning Unable to identify processor name using CS_PROCESSOR_NAME.
#  elif defined(CS_COMPILER_MSVC)
#  pragma message("Unable to identify processor name using CS_PROCESSOR_NAME.")
#  endif
#endif
#if !defined(CS_COMPILER_NAME)
/// Name of the compiler CS is compiled with (i.e. GCC)
#  define CS_COMPILER_NAME "MysteryCompiler"
#  if defined(CS_COMPILER_GCC)
#  warning Unable to identify compiler name using CS_COMPILER_NAME.
#  elif defined(CS_COMPILER_MSVC)
#  pragma message("Unable to identify compiler name using CS_COMPILER_NAME.")
#  endif
#endif

/// A complete version number
#define CS_VERSION_NUMBER CS_VERSION_MAJOR " r" CS_VERSION_MINOR

/// A complete version string, including platform, processor and compiler
#define CS_VERSION CS_VERSION_NUMBER \
  " [" CS_PLATFORM_NAME "-" CS_PROCESSOR_NAME "-" CS_COMPILER_NAME "]"

#endif // __CS_CSVER_H__
