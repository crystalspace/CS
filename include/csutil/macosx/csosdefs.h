#ifndef __OSX_csosdefs_h
#define __OSX_csosdefs_h
//=============================================================================
//
//	Copyright (C)1999-2005 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// csosdefs.h
//
//	MacOS/X-specific interface to common functionality.
//
//-----------------------------------------------------------------------------

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <dirent.h>

#define CS_HAVE_POSIX_MMAP
#define CS_USE_CUSTOM_ISDIR
#define CS_PATH_DELIMITER ':'
#define CS_PATH_SEPARATOR '/'

#define CS_MKDIR(p) mkdir(p,0755)

#undef  CS_SOFTWARE_2D_DRIVER
#define CS_SOFTWARE_2D_DRIVER "crystalspace.graphics2d.coregraphics"

#undef  CS_OPENGL_2D_DRIVER
#define CS_OPENGL_2D_DRIVER "crystalspace.graphics2d.glosx"

#undef  CS_SOUND_DRIVER
#define CS_SOUND_DRIVER "crystalspace.sound.driver.coreaudio"

#undef  CS_SNDSYS_DRIVER
#define CS_SNDSYS_DRIVER "crystalspace.sndsys.software.driver.coreaudio"

#endif // __OSX_csosdefs_h
