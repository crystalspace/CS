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

#ifndef __CYGWIN_H__
#define __CYGWIN_H__

/* because win32.h needs it... */
#define __CSOSDEFS_H__

// So many things require this. IF you have an issue with something defined
// in it then undef that def here.
#if 0

// @@@ this needs testing...

#define WINVER 0x0400

// From the w32api header files:

#if defined(__i686__) && !defined(_M_IX86)
#define _M_IX86 600
#elif defined(__i586__) && !defined(_M_IX86)
#define _M_IX86 500
#elif defined(__i486__) && !defined(_M_IX86)
#define _M_IX86 400
#elif defined(__i386__) && !defined(_M_IX86)
#define _M_IX86 300
#endif
#if defined(_M_IX86) && !defined(_X86_)
#define _X86_
#endif

#ifdef __GNUC__
#ifndef NONAMELESSUNION
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95) 
#define _ANONYMOUS_UNION __extension__
#define _ANONYMOUS_STRUCT __extension__
#else
#if defined(__cplusplus)
#define _ANONYMOUS_UNION __extension__
#endif /* __cplusplus */
#endif /* __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95) */
#endif /* NONAMELESSUNION */
#endif /* __GNUC__ */

#ifndef _ANONYMOUS_UNION
#define _ANONYMOUS_UNION
#define _UNION_NAME(x) x
#define DUMMYUNIONNAME	u
#define DUMMYUNIONNAME2	u2
#define DUMMYUNIONNAME3	u3
#define DUMMYUNIONNAME4	u4
#define DUMMYUNIONNAME5	u5
#define DUMMYUNIONNAME6	u6
#define DUMMYUNIONNAME7	u7
#define DUMMYUNIONNAME8	u8
#else
#define _UNION_NAME(x)
#define DUMMYUNIONNAME
#define DUMMYUNIONNAME2
#define DUMMYUNIONNAME3
#define DUMMYUNIONNAME4
#define DUMMYUNIONNAME5
#define DUMMYUNIONNAME6
#define DUMMYUNIONNAME7
#define DUMMYUNIONNAME8
#endif
#ifndef _ANONYMOUS_STRUCT
#define _ANONYMOUS_STRUCT
#define _STRUCT_NAME(x) x
#define DUMMYSTRUCTNAME	s
#define DUMMYSTRUCTNAME2 s2
#define DUMMYSTRUCTNAME3 s3
#else
#define _STRUCT_NAME(x)
#define DUMMYSTRUCTNAME
#define DUMMYSTRUCTNAME2
#define DUMMYSTRUCTNAME3
#endif

#include <excpt.h>
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <malloc.h>

#define WINGDIAPI DECLSPEC_IMPORT

#else

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif

#undef min
#undef max
#undef DeleteFile

/* Assert */
#ifdef _DEBUG
  #include <assert.h>
  #define ASSERT(expression) assert(expression)
  #define VERIFY_SUCCESS(expression) assert(SUCCEEDED(expression))
  #define VERIFY_RESULT(expression, result) assert(expression == result)
  #ifndef CS_DEBUG
    #define CS_DEBUG
  #endif
#else
  #define ASSERT(expression)
  #define VERIFY_SUCCESS(expression) expression
  #define VERIFY_RESULT(expression, result) expression
#endif

// The 2D graphics driver used by software renderer on this platform
#define CS_SOFTWARE_2D_DRIVER "crystalspace.graphics2d.directdraw"
//#define CS_SOFTWARE_2D_DRIVER "crystalspace.graphics2d.glwin32"
#define CS_OPENGL_2D_DRIVER "crystalspace.graphics2d.glwin32"

// The sound driver
#define CS_SOUND_DRIVER "crystalspace.sound.driver.waveout"

// SCF symbol export facility.
#undef CS_EXPORTED_FUNCTION
#define CS_EXPORTED_FUNCTION extern "C" __declspec(dllexport)

#if defined (CS_SYSDEF_PROVIDE_DIR) || defined (CS_SYSDEF_PROVIDE_GETCWD) || defined (CS_SYSDEF_PROVIDE_MKDIR)
#  include <dirent.h>
#endif

#ifdef CS_SYSDEF_PROVIDE_PATH
#    include <dirent.h>
#    define __NEED_GENERIC_ISDIR
#endif

#ifdef CS_SYSDEF_PROVIDE_MKDIR
#    define MKDIR(path)	mkdir(path, 0755)
#    undef CS_SYSDEF_PROVIDE_MKDIR
#endif

#ifdef CS_SYSDEF_PROVIDE_SOCKETS
#include <winsock.h>
   typedef int socklen_t;
   typedef SOCKET csNetworkSocket;
#  define CS_NET_SOCKET_INVALID INVALID_SOCKET
#  define CS_IOCTLSOCKET ioctlsocket
#  define CS_CLOSESOCKET closesocket
#  define EWOULDBLOCK WSAEWOULDBLOCK
#  define CS_GETSOCKETERROR ::WSAGetLastError()
#  undef CS_SYSDEF_PROVIDE_SOCKETS
#endif

#ifdef CS_SYSDEF_PROVIDE_SELECT
#  undef CS_SYSDEF_PROVIDE_SELECT
#endif

#ifdef CS_SYSDEF_PROVIDE_ACCESS
#  include <io.h>
#endif

#ifdef CS_SYSDEF_PROVIDE_TEMP
#  include <unistd.h>
#  define TEMP_FILE "cslud.tmp", (unsigned long)getpid()
#  define TEMP_DIR  "/tmp"
#endif // CS_SYSDEF_PROVIDE_TEMP

#if defined (PROC_X86)
#  define CS_LITTLE_ENDIAN
#else
#  error "Please define a suitable CS_XXX_ENDIAN macro in win32/csosdefs.h!"
#endif

/* Cygwin should implement it's own winMain...
   but how to get ModuleHandle from then? */
#define CS_IMPLEMENT_PLATFORM_APPLICATION \
HINSTANCE ModuleHandle = NULL; \
int ApplicationShow = SW_SHOWNORMAL;

/* plugins */
#if !defined(CS_STATIC_LINKED)
#define CS_IMPLEMENT_PLATFORM_PLUGIN \
HINSTANCE ModuleHandle = NULL; \
int ApplicationShow = SW_SHOWNORMAL; \
extern "C" BOOL WINAPI \
DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpvReserved*/) \
{ \
  if (fdwReason == DLL_PROCESS_ATTACH) \
    ModuleHandle = hinstDLL; \
  return TRUE; \
}
#endif // !CS_STATIC_LINKED

#endif // __CSOSDEFS_H__
