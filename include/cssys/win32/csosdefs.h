/*
    Copyright (C) 1998 by Jorrit Tyberghein
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

#ifndef __CSOSDEFS_H__
#define __CSOSDEFS_H__

// For GUI applications, use "csMain" instead of "main".
// For console applications, use regular "main".
#ifndef CONSOLE
#  define main csMain
#endif

#if defined(COMP_VC)
#pragma warning(disable:4244)   // conversion from 'double' to 'float'
#pragma warning(disable:4305)   // conversion from 'const double' to 'float'
#pragma warning(disable:4018)   // Signed unsigned warnings
#pragma warning(disable:4805)   // unsafe mix of bool and int.
#pragma warning(disable:4800)   // Forcing value to bool
#pragma warning(disable:4514)   // Removal of unreferenced inline function
#pragma warning(disable:4097)   // use of xxx as synonym for a classname
#pragma warning(disable:4127)   // conditional expression is constant
#pragma warning(disable:4189)   // local variable is intialized but not referenced
#pragma warning(disable:4706)   // Assignmet in conditional expression
#pragma warning(disable:4611)   // interaction between _setjmp and C++ destructors not portable
#pragma warning(disable:4710)   // function not inlined
#pragma warning(disable:4201)   // structure/ union without name. (Only relevant on MSVC 5.0)
#endif

#if defined(COMP_WCC) || defined(COMP_BC)
// The WATCOM C++ compiler does not accept a 'main' routine
// in a program which already contains WinMain. This is a 'fix'.
#define main csMain
#endif

// The 2D graphics driver used by software renderer on this platform
#define SOFTWARE_2D_DRIVER "crystalspace.graphics2d.directdraw"
#define OPENGL_2D_DRIVER "crystalspace.graphics2d.defaultgl"
#define GLIDE_2D_DRIVER	"crystalspace.graphics2d.glidewin"
#define SOUND_DRIVER "crystalspace.sound.driver.waveout"

#if defined (SYSDEF_DIR) || defined (SYSDEF_GETCWD) || defined (SYSDEF_MKDIR)
#  include <direct.h>
#endif

#ifdef COMP_BC
#  define strcasecmp(s1,s2)    stricmp(s1,s2)
#  define strncasecmp(s1,s2,n) strnicmp(s1,s2,n)
#endif

#ifdef SYSDEF_PATH
// Maximal path length
#  ifndef MAXPATHLEN
#    ifdef _MAX_FNAME
#      define MAXPATHLEN _MAX_FNAME
#    else
#      define MAXPATHLEN 256
#    endif
#  endif
#endif

#ifdef SYSDEF_DIR
   /// Directory read functions
#  if !(defined(COMP_BC) || defined(COMP_WCC))
#    define __NEED_OPENDIR_PROTOTYPE
#    include <io.h>
     /// Directory entry
     struct dirent
     {
       char d_name [MAXPATHLEN + 1];	/* File name, 0 terminated */
       long d_size;			/* File size (bytes) */
       unsigned d_attr;			/* File attributes (Windows-specific) */
     };
     /// Directory handle
     struct DIR
     {
       bool valid;
       long handle;
       dirent de;
       _finddata_t fd;
     };
     static inline bool isdir (const char *path, dirent *de)
     {
       (void)path;
       return !!(de->d_attr & _A_SUBDIR);
     }
#  endif
#endif

#ifdef SYSDEF_SOCKETS
#  define _WINSOCKAPI_
#  include <winsock2.h>
#  undef SYSDEF_SOCKETS
#endif

#ifdef SYSDEF_ACCESS
#  include <io.h>
#endif

#endif // __CSOSDEFS_H__
