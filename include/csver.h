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

#ifndef __CS_CSVER_H__
#define __CS_CSVER_H__

#define CS_VERSION_MAJOR "0.19"	/* NOTE: Update docs/texinfo/version.txi  */
#define CS_VERSION_MINOR "dev"	/* whenever updating these two values.    */
#define CS_RELEASE_DATE "Sun, 18-Feb-2001"

#if !defined(CS_PLATFORM_NAME)
#  define CS_PLATFORM_NAME "MysteryPlatform"
#  warning Unable to identify platform name using CS_PLATFORM_NAME.
#endif
#if !defined(CS_PROCESSOR_NAME)
#  define CS_PROCESSOR_NAME "MysteryProcessor"
#  warning Unable to identify processor name using CS_PROCESSOR_NAME.
#endif
#if !defined(CS_COMPILER_NAME)
#  define CS_COMPILER_NAME "MysteryCompiler"
#  warning Unable to identify compiler name using CS_COMPILER_NAME.
#endif

#define CS_VERSION_NUMBER CS_VERSION_MAJOR " r" CS_VERSION_MINOR

#define CS_VERSION CS_VERSION_NUMBER \
  " [" CS_PLATFORM_NAME "-" CS_PROCESSOR_NAME "-" CS_COMPILER_NAME "]"

#endif // __CS_CSVER_H__
