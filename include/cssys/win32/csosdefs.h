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
  #pragma warning(disable:4702)   // Unreachable Code
  #pragma warning(disable:4512)   // Could not generate assignment operator
  #pragma warning(disable:4100)   // Use of void* as a formal function parameter
#endif

// So many things require this. IF you have an issue with something defined
// in it then undef that def here.
#include <windows.h>
#include <malloc.h>
#undef min
#undef max
#undef GetCurrentTime
#undef DeleteFile

// For GUI applications, use "csMain" instead of "main".
// For console applications, use regular "main".
#ifndef CONSOLE
  #define main csMain
#endif

#if defined(COMP_BC)
  // The Borland C++ compiler does not accept a 'main' routine
  // in a program which already contains WinMain. This is a work-around.
  #define main csMain
#endif

#ifdef _DEBUG
  #include <assert.h>
  #define ASSERT(expression) assert(expression)
  #define VERIFY_SUCCESS(expression) assert(SUCCEEDED(expression))
  #define VERIFY_RESULT(expression, result) assert(expression == result)
  #define CS_DEBUG
#else
  #define ASSERT(expression)
  #define VERIFY_SUCCESS(expression) expression
  #define VERIFY_RESULT(expression, result) expression
#endif

//COM Helpers. (DirectX still requires COM...03/31/2000 -- PEG)
#define FINAL_RELEASE(d) if (d != NULL) { d->Release(); d = NULL; }

// The 2D graphics driver used by software renderer on this platform
#define SOFTWARE_2D_DRIVER "crystalspace.graphics2d.directdraw"
#define OPENGL_2D_DRIVER "crystalspace.graphics2d.glwin32"
#define GLIDE_2D_DRIVER	"crystalspace.graphics2d.glidewin"
#define SOUND_DRIVER "crystalspace.sound.driver.waveout"

// SCF symbol export facility.
#undef SCF_EXPORT_FUNCTION
#define SCF_EXPORT_FUNCTION extern "C" __declspec(dllexport)

#if defined (SYSDEF_DIR) || defined (SYSDEF_GETCWD) || defined (SYSDEF_MKDIR)
#  include <direct.h>
#  if defined(COMP_BC) || defined(COMP_GCC)
#    ifdef __CYGWIN32__
#	include <sys/dirent.h>
#    else
#    	include <dirent.h>
#    endif
#  endif
#endif

#if defined (COMP_BC)
#  define strcasecmp stricmp
#  define strncasecmp strnicmp
#endif

#if defined (COMP_VC)
#  define strcasecmp _stricmp
#  define strncasecmp _strnicmp
#endif

// Maximal path length
#ifdef SYSDEF_PATH
#  ifndef MAXPATHLEN
#    ifdef _MAX_FNAME
#      define MAXPATHLEN _MAX_FNAME
#    else
#      define MAXPATHLEN 260 /* not 256 */
#    endif
#  endif
#endif

// COMP_GCC has generic opendir(), readdir(), closedir()

#if defined(SYSDEF_DIR) || defined(SYSDEF_PATH)
// Directory read functions
# if !defined(COMP_GCC)	  
#  if !defined(COMP_BC)
    #define __NEED_OPENDIR_PROTOTYPE
    #include <io.h>

    // Directory entry
    struct dirent
    {
      char d_name [MAXPATHLEN + 1]; // File name, 0 terminated
      long d_size; // File size (bytes)
      unsigned d_attr; // File attributes (Windows-specific)
    };
    // Directory handle
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

    extern "C" DIR *opendir (const char *name);
    extern "C" dirent *readdir (DIR *dirp);
    extern "C" int closedir (DIR *dirp);
#  endif // end if !defined(COMP_BC)
# endif
#endif

#ifdef SYSDEF_PATH
#  if defined(COMP_BC) || defined(COMP_GCC)
#    define __NEED_GENERIC_ISDIR
#  else
#    define __NO_GENERIC_ISDIR
     static inline bool isdir (char *path, dirent *de)
     {
       (void)path;
       return !!(de->d_attr & _A_SUBDIR);
     }
#  endif
#endif

#ifdef SYSDEF_SOCKETS
   typedef int socklen_t;
   typedef SOCKET csNetworkSocket;
#  define CS_NET_SOCKET_INVALID INVALID_SOCKET
#  define CS_IOCTLSOCKET ioctlsocket
#  define CS_CLOSESOCKET closesocket
#  define EWOULDBLOCK WSAEWOULDBLOCK
#  define CS_GETSOCKETERROR ::WSAGetLastError()
#  undef SYSDEF_SOCKETS
#endif

#ifdef SYSDEF_SELECT
#  undef SYSDEF_SELECT
#endif

#ifdef SYSDEF_ACCESS
#  include <io.h>
#endif

#ifdef COMP_BC
#  define GETPID() getpid()
#else
#  define GETPID() _getpid()
#endif

#ifdef SYSDEF_TEMP
#  include <process.h>
#  define TEMP_FILE "%x.cs", GETPID()
#  define TEMP_DIR win32_tempdir()
   // This is the function called by TEMP_DIR macro
   static inline char *win32_tempdir()
   {
     char *tmp;
     if ((tmp = getenv ("TMP")) != NULL)
       return tmp;
     if ((tmp = getenv ("TEMP")) != NULL)
       return tmp;
     return "";
   }
#endif // SYSDEF_TEMP

// The Microsoft Visual C compiler miserably crashes on boxclip.inc
// because of memcpy(). This is a replacement for memcpy() which is
// supposed to fix the problem.
#ifdef COMP_VC
#  define memcpy better_memcpy
static inline void *better_memcpy (void *dst, const void *src, size_t len)
{
  _asm		mov	esi,src
  _asm		mov	edi,dst
  _asm		mov	ecx,len
  _asm		mov	al,cl
  _asm		shr	ecx,2
  _asm		cld
  _asm		rep	movsd
  _asm		mov	cl,al
  _asm		and	cl,3
  _asm		rep	movsb
  return dst;
}
#endif

#ifdef COMP_BC
// Major hack due to pow failures in CS for Borland, removing this
// causes millions of strings to print out -- Brandon Ehle
#define pow(arga, argb) ( (!arga && !argb)?0:pow(arga, argb) )
// Dunno why this is in CS -- Brandon Ehle
#define DEBUG_BREAK
#endif

#if defined (PROC_INTEL)
#  define CS_LITTLE_ENDIAN
#else
#  error "Please define a suitable CS_XXX_ENDIAN macro in win32/csosdefs.h!"
#endif

#endif // __CSOSDEFS_H__
