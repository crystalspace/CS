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

#ifndef __SYSDEFS_H__
#define __SYSDEFS_H__

#define OK_TO_INCLUDE_DEFS_IM_A_FRIEND
#include "def.h"
#undef OK_TO_INCLUDE_DEFS_IM_A_FRIEND

/*
    This include file should be included from every source file.
    Just before #include directive it can contain several #define's
    that specify what the source file requires.

    The following variables can be defined:

    #define SYSDEF_CASE
      Define the UPPERCASE() and LOWERCASE() macros.

    #define SYSDEF_PATH
      Include definition of PATH_SEPARATOR character, MAXPATHLEN and
      APPEND_SLASH macros.

    #define SYSDEF_MKDIR
      Include definition for MKDIR()

    #define SYSDEF_GETCWD
      Include definition for getcwd()

    #define SYSDEF_TEMP
      Include definitions for TEMP_DIR and TEMP_FILE.

    #define SYSDEF_DIR
      Include definitions required for opendir(), readdir(), closedir()
      and isdir().
 
    #define SYSDEF_UNLINK
      Include definitions required for unlink()

    #define SYSDEF_ACCESS
      Include definitions required for access()

    #define SYSDEF_ALLOCA
      Include definition for alloca()

    #define SYSDEF_GETOPT
      for getopt() and GNU getopt_long()

    #define SYSDEF_SOCKETS
      for TCP/IP sockets definitions

    The system-dependent include files can undefine some or all SYSDEF_xxx
    macros to avoid further definitions in this file. For example, if a
    system-dependent file defines everything needed for SYSDEF_GETOPT it
    should #undefine SYSDEF_GETOPT to avoid including util/gnu/getopt.h
    at the bottom of this file.
*/

#if defined (SYSDEF_DIR) && !defined (SYSDEF_PATH)
#  define SYSDEF_PATH
#endif

#ifdef OS_WIN32
#  include "cssys/win32/csosdefs.h"
#endif

#if defined (COMP_WCC) && defined (OS_DOS)
#  include "cssys/wcc/csosdefs.h"
#endif

#if defined (COMP_GCC) && defined (OS_DOS)
#  include "cssys/djgpp/csosdefs.h"
#endif

#if defined (OS_MACOS)
#  include "cssys/mac/csosdefs.h"
#endif

#if defined (OS_UNIX) && !defined(OS_BE)
#  include "cssys/unix/csosdefs.h"
#endif

#if defined (OS_AMIGAOS)
#  include "cssys/amiga/csosdefs.h"
#endif

#if defined (OS_OS2)
#  include "cssys/os2/csosdefs.h"
#endif

#if defined (OS_BE)
#  include "cssys/be/csosdefs.h"
#endif

#if defined (OS_NEXT)
#  include "cssys/next/csosdefs.h"
#endif

//--//--//--//--//--/ Allow system-dependent header files to override these --//

#ifdef SYSDEF_CASE
// Convert a character to upper case
#  ifndef UPPERCASE
#    define UPPERCASE(c) ((c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c)
#  endif
// Convert a character to lower case
#  ifndef LOWERCASE
#    define LOWERCASE(c) ((c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c)
#  endif
#endif // SYSDEF_CASE

#ifdef SYSDEF_PATH
// Path separator character
#  ifndef PATH_SEPARATOR
#    if defined(OS_MACOS)
#      define PATH_SEPARATOR '/'
#    elif defined(OS_OS2) || defined(OS_DOS) || defined(OS_WIN32)
#      define PATH_SEPARATOR '\\'
#    else
#      define PATH_SEPARATOR '/'
#    endif
#  endif
// Maximal path length
#  ifndef MAXPATHLEN
#    ifdef _MAX_FNAME
#      define MAXPATHLEN _MAX_FNAME
#    else
#      define MAXPATHLEN 256
#    endif
#  endif
#  define APPEND_SLASH(str,len)			\
     if ((len)					\
      && (str[len - 1] != '/')			\
      && (str[len - 1] != PATH_SEPARATOR))	\
     {						\
       str[len++] = PATH_SEPARATOR;		\
       str[len] = 0;				\
     } /* endif */
#endif // SYSDEF_PATH

#ifdef SYSDEF_TEMP
// Directory for temporary files
#  ifndef TEMP_DIR
#    if defined(OS_UNIX)
#      define TEMP_DIR "/tmp/"
#    else
#      define TEMP_DIR ""
#    endif
#  endif
// Name for temporary file
#  ifndef TEMP_FILE
#    if defined(OS_UNIX)
#      include <unistd.h>
#      define TEMP_FILE "cs%lud.tmp", (unsigned long)getpid()
#    else
#      define TEMP_FILE "$cs$.tmp"
#    endif
#  endif
#endif // SYSDEF_TEMP

#ifdef SYSDEF_MKDIR
// How to make a directory (not entire path, only the last on the path)
#  ifndef MKDIR
#    if defined(OS_WIN32) || (defined(OS_DOS) && !defined(COMP_GCC))
#      define MKDIR(path) _mkdir (path)
#    else
#      define MKDIR(path) mkdir (path, 0644)
#    endif
#  endif
#endif // SYSDEF_MKDIR


#ifdef SYSDEF_GETCWD
#  if defined(OS_MACOS)
#    include <unix.h>
#  else
#    if !defined(COMP_VC) && !defined(COMP_BC)
#      include <unistd.h>
#    endif
#  endif
#endif // SYSDEF_GETCWD

#ifdef SYSDEF_DIR
// For systems without opendir()
#  ifdef __NEED_OPENDIR_PROTOTYPE
#    if defined(OS_MACOS)
       typedef char DIR;
       typedef struct dirent {
	  char d_name[ MAXPATHLEN ];
       } dirent;
#    else
       struct DIR;
       struct dirent;
#    endif
     extern "C" DIR *opendir (const char *name);
     extern "C" dirent *readdir (DIR *dirp);
     extern "C" int closedir (DIR *dirp);
     //extern "C" void seekdir (DIR *dirp, long off);
     //extern "C" long telldir (DIR *dirp);
     //extern "C" void rewinddir (DIR *dirp);
#  endif
#  ifdef __NEED_GENERIC_ISDIR
#    if !defined (OS_UNIX) && !defined (OS_MACOS) && !defined (OS_AMIGAOS)
#      include <io.h>
#    endif
#    if defined(OS_MACOS)
#      include <stat.h>
#    else
#      include <sys/types.h>
#      if !(defined (OS_WIN32) && defined (COMP_WCC))
#        include <dirent.h>
#      endif
#      include <sys/stat.h>
#    endif
     static inline bool isdir (const char *path, struct dirent *de)
     {
       char fullname [MAXPATHLEN];
       int pathlen = strlen (path);
       memcpy (fullname, path, pathlen + 1);
       APPEND_SLASH (fullname, pathlen);
       strcat (&fullname [pathlen], de->d_name);
       struct stat st;
       stat (fullname, &st);
       return ((st.st_mode & S_IFMT) == S_IFDIR);
     }
#  endif
#endif // SYSDEF_DIR

#ifdef SYSDEF_UNLINK
#  if defined (OS_MACOS)
#    include <unix.h>
#  else
#    if !defined(COMP_VC) && !defined(COMP_BC)
#      include <unistd.h>
#    endif
#  endif
#endif

#ifdef SYSDEF_ALLOCA
#  if defined(COMP_WCC) || defined (COMP_VC) || defined(COMP_BC)
#    include <malloc.h>
#  elif defined(COMP_GCC) && defined(OS_DOS)
#    include <stdlib.h>
#  elif defined(OS_BSD)
#    include <stdlib.h>
#  else
#    include <alloca.h>
#  endif
#endif

#ifdef SYSDEF_ACCESS
#  if !defined (OS_MACOS) && !defined(COMP_VC) && !defined(COMP_BC)
#    include <unistd.h>
#  endif
#  ifndef F_OK
#    define F_OK 0
#  endif
#  ifndef R_OK
#    define R_OK 2
#  endif
#  ifndef W_OK
#    define W_OK 4
#  endif
#endif

#ifdef SYSDEF_GETOPT
#  ifndef __STDC__
#    define __STDC__ 1
#  endif
#  include "support/gnu/getopt.h"
#endif

#ifdef SYSDEF_SOCKETS
#  if !defined (OS_MACOS)
#    include <unistd.h>
#  endif
#  include <sys/types.h>
#  include <sys/socket.h>
#  if defined (OS_UNIX)
#    define BSD_COMP 1
#    include <sys/ioctl.h>
#  endif
#  include <netinet/in.h>
#  include <netdb.h>
#endif

// Fatal exit routine (which can be replaced if neccessary)
extern void (*fatal_exit) (int errorcode, bool canreturn);

#endif // __SYSDEFS_H__
