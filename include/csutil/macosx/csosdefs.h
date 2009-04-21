//=============================================================================
//
//	Copyright (C)1999-2009 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// csutil/macosx/csosdefs.h
//
//	MacOS/X-specific interface to common functionality.
//
//-----------------------------------------------------------------------------
#ifndef CSUTIL_MACOSX_CSOSDEFS_H
#define CSUTIL_MACOSX_CSOSDEFS_H

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <dirent.h>

#if defined(CS_UNIVERSAL_BINARY)
#undef CS_BIG_ENDIAN
#undef CS_LITTLE_ENDIAN
#if defined(__BIG_ENDIAN__)
#define CS_BIG_ENDIAN
#elif defined(__LITTLE_ENDIAN__)
#define CS_LITTLE_ENDIAN
#else
#error Unknown endianess for Mac OS X universal binary build
#endif
#endif

#if defined(CS_UNIVERSAL_BINARY)
#undef CS_PROCESSOR_X86
#undef CS_PROCESSOR_POWERPC
#undef CS_PROCESSOR_NAME
#if defined(__ppc__)
#define CS_PROCESSOR_POWERPC
#define CS_PROCESSOR_NAME "powerpc"
#elif defined(__i386__)
#define CS_PROCESSOR_X86
#define CS_PROCESSOR_NAME "x86"
#else
#error Unknown host CPU type for Mac OS X universal binary build
#endif
#endif

#define CS_HAVE_POSIX_MMAP
#define CS_USE_CUSTOM_ISDIR
#define CS_PATH_DELIMITER ':'
#define CS_PATH_SEPARATOR '/'

#define CS_MKDIR(p) mkdir(p,0755)

#undef  CS_OPENGL_2D_DRIVER
#define CS_OPENGL_2D_DRIVER "crystalspace.graphics2d.glosx"

#undef  CS_SOUND_DRIVER
#define CS_SOUND_DRIVER "crystalspace.sound.driver.coreaudio"

#undef  CS_SNDSYS_DRIVER
#define CS_SNDSYS_DRIVER "crystalspace.sndsys.software.driver.coreaudio"

#endif // CSUTIL_MACOSX_CSOSDEFS_H
