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

// So many things require this. IF you have an issue with something defined
// in it then undef that def here.
#include <windows.h>
#include <malloc.h>
#undef min
#undef max
#undef GetCurrentTime
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
#undef SCF_EXPORT_FUNCTION
#define SCF_EXPORT_FUNCTION extern "C" __declspec(dllexport)

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

#define CS_WIN32_ARGC __argc
#define CS_WIN32_ARGV __argv

/* Cygwin should implement it's own winMain...
   but how to get ModuleHandle from then? */
#define CS_IMPLEMENT_APPLICATION \
HINSTANCE ModuleHandle = NULL; \
int ApplicationShow = SW_SHOWNORMAL;

/* plugins */
#if !defined(CS_STATIC_LINKED)
#define CS_IMPLEMENT_PLUGIN \
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
