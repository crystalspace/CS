/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
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

#ifdef __CS_CSSYSDEFS_H__
#error Do not include cssysdef.h from header files please!
#else
#define __CS_CSSYSDEFS_H__

#define CSDEF_FRIEND
#include "csdef.h"
#undef CSDEF_FRIEND

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
      Include definition for alloca() and ALLOC_STACK_ARRAY()

    #define SYSDEF_GETOPT
      For getopt() and GNU getopt_long()

    #define SYSDEF_SOCKETS
      For TCP/IP sockets definitions.  Specifically, should define the
      following macros, constants, typedefs, and prototypes:
	inet_addr(), gethostbyname(), ntohl(), etc.
	socket(), listen(), bind(), etc. -- the standard socket functions
	csNetworkSocket -- typedef or macro for socket descriptor type
	struct sockaddr -- standard socket address type (and cousins)
	socklen_t -- typedef or macro
	CS_NET_SOCKET_INVALID -- value representing invalid socket
	CS_CLOSESOCKET -- name of function to close a socket
	CS_IOCTLSOCKET -- name of "ioctl" function for sockets
	CS_GETSOCKETERROR -- name of function or variable for socket error code
	
    #define SYSDEF_SELECT
      Includes definitions required for select(), FD_* macros, and
      struct timeval.

    The system-dependent include files can undefine some or all SYSDEF_xxx
    macros to avoid further definitions in this file. For example, if a
    system-dependent file defines everything needed for SYSDEF_GETOPT it
    should #undefine SYSDEF_GETOPT to avoid including util/gnu/getopt.h
    at the bottom of this file.
*/

#if defined (SYSDEF_DIR) && !defined (SYSDEF_PATH)
#  define SYSDEF_PATH
#endif

/*
 * Pull in platform-specific overrides of the requested functionality.
 */
#include "cssys/csosdefs.h"

/*
 * Default definitions for requested functionality.  Platform-specific
 * configuration files may override these.
 */

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
#    if defined(OS_MACOS) || defined(__CYGWIN32__)
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
#      define MKDIR(path) mkdir (path, 0755)
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
// COMP_GCC has opendir, readdir 
# if !defined(COMP_GCC) || defined(OS_PS2)
#  if defined(__NEED_OPENDIR_PROTOTYPE) || defined(OS_PS2)
#    if defined(OS_MACOS) || defined(OS_PS2)
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
# endif
// Generic ISDIR needed for COMP_GCC
#  ifdef __NEED_GENERIC_ISDIR
#    if !defined (OS_UNIX) && !defined (OS_MACOS) && !defined(OS_PS2)
#      include <io.h>
#    endif
#    if defined(OS_MACOS)
#      include <stat.h>
#    else
#      include <sys/types.h>
#      if !defined(OS_WIN32) && !defined(OS_PS2)
#        include <dirent.h>
#      endif
#      if defined(__CYGWIN32__)
#        include <sys/dirent.h>
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
// Prototypes for dynamic stack memory allocation
#  if defined (COMP_VC) || defined(COMP_BC) || \
     (defined(COMP_GCC) && defined(OS_WIN32))
#    include <malloc.h>
#  elif defined(COMP_GCC) && defined(OS_DOS)
#    include <stdlib.h>
#  elif defined(COMP_GCC) && defined(OS_PS2)
#    include <malloc.h>
#  elif defined(OS_BSD)
#    include <stdlib.h>
#  else
#    include <alloca.h>
#  endif
#  if defined (COMP_GCC)
    // In GCC we are able to declare stack vars of dynamic size directly
#    define ALLOC_STACK_ARRAY(var,type,size) \
       type var [size]
#  else
#    define ALLOC_STACK_ARRAY(var,type,size) \
       type *var = (type *)alloca (size * sizeof (type))
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
#  include "cssys/getopt.h"
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
#    if !defined (OS_SOLARIS) && !defined (OS_BE)
#      include <arpa/inet.h>
#      include <sys/time.h>
#    endif
#  endif
#  include <netinet/in.h>
#  include <netdb.h>
#  if defined (CS_USE_FAKE_SOCKLEN_TYPE)
     typedef int socklen_t;
#  endif
#  if !defined (CS_IOCTLSOCKET)
#    define CS_IOCTLSOCKET ioctl
#  endif
#  if !defined (CS_CLOSESOCKET)
#    define CS_CLOSESOCKET close
#  endif
#  if !defined (CS_GETSOCKETERROR)
#    define CS_GETSOCKETERROR errno
#  endif
   typedef unsigned int csNetworkSocket;
#  if !defined (CS_NET_SOCKET_INVALID)
#    define CS_NET_SOCKET_INVALID ((csNetworkSocket)~0)
#  endif
#endif

#ifdef SYSDEF_SELECT
#  include <sys/select.h>
#endif

#ifdef CS_DEBUG
#  if !defined (DEBUG_BREAK)
#    if defined (PROC_X86)
#      if defined (COMP_GCC)
#        define DEBUG_BREAK	asm ("int $3")
#      else
#        define DEBUG_BREAK	_asm int 3
#      endif
#    else
#      define DEBUG_BREAK	{ static int x = 0; x /= x; }
#    endif
#  endif
#  if !defined (CS_ASSERT)
#    if defined (COMP_VC)
#      define  CS_ASSERT(x) assert(x)
#    else
#      include <stdio.h>
#      define CS_ASSERT(x)						\
         if (!(x))							\
         {								\
           fprintf (stderr, __FILE__ ":%d: failed assertion '" #x "'\n",\
             int(__LINE__));						\
           DEBUG_BREAK;							\
         }
#    endif
#  endif
#else
#  undef DEBUG_BREAK
#  define DEBUG_BREAK
#  undef CS_ASSERT
#  define CS_ASSERT(x)
#endif

// Check if the csosdefs.h defined either CS_LITTLE_ENDIAN or CS_BIG_ENDIAN
#if !defined (CS_LITTLE_ENDIAN) && !defined (CS_BIG_ENDIAN)
#  error No CS_XXX_ENDIAN macro defined in your OS-specific csosdefs.h!
#endif

// Adjust some definitions contained in volatile.h
#if defined (PROC_X86) && !defined (DO_NASM)
#  undef NO_ASSEMBLER
#  define NO_ASSEMBLER
#endif

#if !defined (PROC_X86) || defined (NO_ASSEMBLER)
#  undef DO_MMX
#  undef DO_NASM
#endif

// Use fast QInt and QRound on CPUs that are known to support it
#if !defined (CS_NO_IEEE_OPTIMIZATIONS)
#  if !defined (CS_IEEE_DOUBLE_FORMAT)
#    if defined (PROC_X86) || defined (PROC_M68K)
#      define CS_IEEE_DOUBLE_FORMAT
#    endif
#  endif
#endif

// Fatal exit routine (which can be replaced if neccessary)
extern void (*fatal_exit) (int errorcode, bool canreturn);

#endif // __CS_CSSYSDEFS_H__
