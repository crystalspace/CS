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

#ifndef __CS_CSOSDEFS_H__
#define __CS_CSOSDEFS_H__

#ifdef CS_BUILD_SHARED_LIBS
  #define CS_EXPORT_SYM __declspec(dllexport)
  #define CS_IMPORT_SYM __declspec(dllimport)
#else
  #define CS_EXPORT_SYM
  #define CS_IMPORT_SYM
#endif // CS_BUILD_SHARED_LIBS

#if defined(CS_COMPILER_MSVC)
  #pragma warning(disable:4097)   // use of xxx as synonym for a classname
  #pragma warning(disable:4099)   // type seen as both 'struct' and `class'
  #pragma warning(disable:4100)   // Use of void* as a formal function parameter
  #pragma warning(disable:4102)   // 'label' : unreferenced label
  #pragma warning(disable:4146)   // unary minus operator applied to unsigned type, result still unsigned
  #pragma warning(disable:4201)   // structure/ union without name. (Only relevant on MSVC 5.0)
  #pragma warning(disable:4244)   // conversion from 'double' to 'float'
  #pragma warning(disable:4251)   // class needs to have dll-interface to be used by clients
  #pragma warning(disable:4275)   // non-DLL-interface used as base for DLL-interface
  #pragma warning(disable:4291)   // no matching operator delete found
  #pragma warning(disable:4312)	  // 'variable' : conversion from 'type' to 'type' of greater size
  #pragma warning(disable:4390)   // Empty control statement
  #pragma warning(disable:4505)   // 'function' : unreferenced local function has been removed
  #pragma warning(disable:4611)   // interaction between _setjmp and C++ destructors not portable
  #pragma warning(disable:4702)   // Unreachable Code
  #pragma warning(disable:4706)   // Assignment in conditional expression
  #pragma warning(disable:4710)   // function not inlined
  #pragma warning(disable:4711)   // function 'function' selected for inline expansion
  #pragma warning(disable:4786)   // identifier was truncated to '255' characters in the browser information (VC6)
  #pragma warning(disable:4800)   // Forcing value to bool
  #pragma warning(disable:4805)   // unsafe mix of bool and int.

#if (_MSC_VER < 1300)
  #pragma warning(disable:4248)   // MSVC6 gives bogus "protected constructor"
				  // for csHash::*Iterator, even though csHash
				  // is friend.
  #pragma warning(disable:4503)   // 'identifier' : decorated name length exceeded, name was truncated
#endif

  #pragma warning(default:4265)   // class has virtual functions, but destructor is not virtual

  #pragma inline_depth (255)
  #pragma inline_recursion (on)
  #pragma auto_inline (on)

  #pragma intrinsic (memset, memcpy, memcmp)
  #pragma intrinsic (strcpy, strcmp, strlen, strcat)
  #pragma intrinsic (abs, fabs)

  #if defined(__CRYSTAL_SPACE__) && !defined(CS_DEBUG)
    #pragma code_seg("CSpace")	  // Just for fun :)
    // However, doing this in debug builds prevents Edit & Continue from
    // functioning properly :/
  #endif
#endif

#ifndef WINVER
#define WINVER 0x0400
#endif

// Although MSVC6 generally supports templated functions within templated
// classes, nevertheless it crashes and burns horribly when arguments to those
// functions are function-pointers or functors.  In fact, such usage triggers a
// slew of bugs, mostly "internal compiler error" but also several other
// Worse, the bugs manifest in "random" locations throughout the project, often
// in completely unrelated code.  Consequently, instruct csArray<> to avoid
// such usage for MSVC6.
#if defined(CS_COMPILER_MSVC) && (_MSC_VER < 1300)
#define CSARRAY_INHIBIT_TYPED_KEYS
#endif

// So many things require this. IF you have an issue with something defined
// in it then undef that def here.

#if defined(CS_COMPILER_GCC)

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

#else

#if !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_IX86)
#define _X86_
#endif

#if !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_AMD64)
#define _AMD64_
#endif

#if !defined(_X86_) && !defined(_M_IX86) && !defined(_AMD64_) && defined(_M_IA64)
#if !defined(_IA64_)
#define _IA64_
#endif 
#endif

#endif

#ifndef __CYGWIN32__
#include <excpt.h>
#endif
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <malloc.h>

#ifndef WINGDIAPI
#define WINGDIAPI DECLSPEC_IMPORT
#endif

#undef min
#undef max
#undef DeleteFile
#undef Yield

/*
  LONG_PTR is used in the Win32 canvases, but it's only defined in newer 
  Platform or DirectX SDKs (in BaseTsd.h).
  Ergo, on older SDKs, we have to define ourselves. One indicator for the
  presence of LONG_PTR seems to be if the __int3264 macro is #defines.
  So, if it's not, we define LONG_PTR.
 */
#ifndef __int3264
  typedef LONG LONG_PTR;
#endif

#if defined(_DEBUG) || defined(CS_DEBUG)
  #include <assert.h>
  #define ASSERT(expression) assert(expression)
  #define VERIFY_SUCCESS(expression) assert(SUCCEEDED(expression))
  #define VERIFY_RESULT(expression, result) assert(expression == result)
  #ifndef CS_DEBUG
    #define CS_DEBUG
  #endif

  #undef  DEBUG_BREAK
  #define DEBUG_BREAK ::DebugBreak()
  
  #if defined(CS_COMPILER_MSVC) 
    #include <crtdbg.h>

    #if defined(CS_EXTENSIVE_MEMDEBUG)
      #define malloc(size) 	_malloc_dbg ((size), _NORMAL_BLOCK, __FILE__, __LINE__)
      #define free(ptr) 		_free_dbg ((ptr), _NORMAL_BLOCK)
      #define realloc(ptr, size) 	_realloc_dbg ((ptr), (size), _NORMAL_BLOCK, __FILE__, __LINE__)
      #define calloc(num, size)	_calloc_dbg ((num), (size), _NORMAL_BLOCK, __FILE__, __LINE__)

      // heap consistency check is on by default, leave it
      #define CS_WIN32_MSVC_DEBUG_GOOP \
        _CrtSetDbgFlag (_CrtSetDbgFlag (_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF)
    #else
      // turn heap consistency check off
      #define CS_WIN32_MSVC_DEBUG_GOOP \
        _CrtSetDbgFlag ((_CrtSetDbgFlag (_CRTDBG_REPORT_FLAG) & ~_CRTDBG_ALLOC_MEM_DF) | \
	  _CRTDBG_LEAK_CHECK_DF)
    #endif
  #endif

#else
  #define ASSERT(expression)
  #define VERIFY_SUCCESS(expression) expression
  #define VERIFY_RESULT(expression, result) expression
#endif

#ifdef CS_WIN32_MSVC_DEBUG_GOOP
  #define CS_INITIALIZE_PLATFORM_APPLICATION CS_WIN32_MSVC_DEBUG_GOOP
#endif

#ifdef CS_SYSDEF_PROVIDE_HARDWARE_MMIO

// Defines that this platform supports hardware memory-mapped i/o
#define CS_HAS_MEMORY_MAPPED_IO 1

/// Windows specific memory-mapped I/O stuff.
struct mmioInfo
{
    /// Handle to the mapped file 
    HANDLE hMappedFile;

    /// Handle to the mapping
    HANDLE hFileMapping;
    
    /// Whether to close the handles
    bool close;
  
    /// Base pointer to the data
    unsigned char *data;

    /// File size
    unsigned int file_size;
};

#endif

// The 2D graphics driver used by software renderer on this platform
#define CS_SOFTWARE_2D_DRIVER "crystalspace.graphics2d.directdraw"
#define CS_OPENGL_2D_DRIVER "crystalspace.graphics2d.glwin32"

// The sound driver
#define CS_SOUND_DRIVER "crystalspace.sound.driver.waveout"

// SCF symbol export facility.
#undef CS_EXPORTED_FUNCTION
#define CS_EXPORTED_FUNCTION extern "C" __declspec(dllexport)

#if defined (CS_SYSDEF_PROVIDE_DIR) || defined (CS_SYSDEF_PROVIDE_GETCWD) || defined (CS_SYSDEF_PROVIDE_MKDIR)
#ifndef __CYGWIN32__
#  include <direct.h>
#endif
#endif

#if defined (CS_COMPILER_BCC)
#  define strcasecmp stricmp
#  define strncasecmp strnicmp
#endif

#if defined (CS_COMPILER_MSVC)
#  define strcasecmp _stricmp
#  define strncasecmp _strnicmp
#endif

// Maximal path length
#ifdef _MAX_FNAME
#  define CS_MAXPATHLEN _MAX_FNAME
#else
#  define CS_MAXPATHLEN 260 /* not 256 */
#endif
#define PATH_SEPARATOR '\\'
#define PATH_DELIMITER ';'

#if defined (__CYGWIN32__) && defined(CS_SYSDEF_PROVIDE_MKDIR)
#    define MKDIR(path) mkdir(path, 0755)
#    undef CS_SYSDEF_PROVIDE_MKDIR
#endif

#ifdef CS_SYSDEF_VFS_PROVIDE_CHECK_VAR
  #define CS_PROVIDES_VFS_VARS 1
#  ifdef CS_CSUTIL_LIB
  extern CS_EXPORT_SYM const char* csCheckPlatformVFSVar(const char* VarName);
#  else
  extern CS_IMPORT_SYM const char* csCheckPlatformVFSVar(const char* VarName);
#  endif // CS_CSUTIL_LIB
#endif

#ifdef CS_SYSDEF_PROVIDE_EXPAND_PATH
  #define CS_PROVIDES_EXPAND_PATH 1

  inline void
  csPlatformExpandPath(const char* path, char* buffer, int bufsize)
  {
    GetFullPathName (path, bufsize, buffer, 0);
  }

#endif

// Although CS_COMPILER_GCC has opendir, readdir, CS' versions are preferred.
#if defined(CS_SYSDEF_PROVIDE_DIR)
// Directory read functions
  #define __NEED_OPENDIR_PROTOTYPE
  #include <io.h>

  // Directory entry
  struct dirent
  {
    char d_name [CS_MAXPATHLEN + 1]; // File name, 0 terminated
    size_t d_size; // File size (bytes)
    long dwFileAttributes; // File attributes (Windows-specific)
  };
  // Directory handle
  struct DIR;

# ifdef CS_CSUTIL_LIB
  extern "C" CS_EXPORT_SYM DIR *opendir (const char *name);
  extern "C" CS_EXPORT_SYM dirent *readdir (DIR *dirp);
  extern "C" CS_EXPORT_SYM int closedir (DIR *dirp);
  extern "C" CS_EXPORT_SYM bool isdir (const char *path, dirent *de);
# else
  extern "C" CS_IMPORT_SYM DIR *opendir (const char *name);
  extern "C" CS_IMPORT_SYM dirent *readdir (DIR *dirp);
  extern "C" CS_IMPORT_SYM int closedir (DIR *dirp);
  extern "C" CS_IMPORT_SYM bool isdir (const char *path, dirent *de);
# endif // CS_BUILD_SHARED_LIBS
#endif

#ifdef CS_SYSDEF_PROVIDE_SELECT
#  undef CS_SYSDEF_PROVIDE_SELECT
#endif

#ifdef CS_SYSDEF_PROVIDE_ACCESS
#  include <io.h>
#endif

#if defined (CS_COMPILER_BCC) || defined (__CYGWIN32__)
#  define GETPID() getpid()
#else
#  define GETPID() _getpid()
#endif

#ifdef CS_SYSDEF_PROVIDE_TEMP
# ifdef __CYGWIN32__
#   include <unistd.h>
#   define TEMP_FILE "cs%lu.tmp", (unsigned long)getpid()
#   define TEMP_DIR  "/tmp"
# else
#   include <process.h>
#   define TEMP_FILE "%x.cs", GETPID()
#   define TEMP_DIR win32_tempdir()
   // This is the function called by TEMP_DIR macro
   static inline char *win32_tempdir()
   {
     char *tmp;
     if ((tmp = getenv ("TMP")) != 0)
       return tmp;
     if ((tmp = getenv ("TEMP")) != 0)
       return tmp;
     return "";
   }
# endif
#endif // CS_SYSDEF_PROVIDE_TEMP

// Microsoft Visual C++ compiler includes a very in-efficient 'memcpy'.
// This also replaces the older 'better_memcpy', which was also not as
// efficient as it could be ergo... heres a better solution.
#if defined(CS_COMPILER_MSVC) && (_MSC_VER < 1300)
#include <memory.h>
#define memcpy fast_mem_copy
static inline void* fast_mem_copy (void *dest, const void *src, int count)
{
    __asm
    {
      mov		eax, count
      mov		esi, src
      mov		edi, dest
      xor		ecx, ecx

      // Check for 'short' moves
      cmp		eax, 16
      jl		do_short
		
      // Move enough bytes to align 'dest'
      sub		ecx, edi
      and		ecx, 3
      je		skip
      sub		eax, ecx
      rep		movsb

      skip:
        mov		ecx, eax
        and		eax, 3
        shr		ecx, 2
        rep		movsd
        test	eax, eax
        je		end

      do_short:
        mov		ecx, eax
        rep		movsb

      end:
    }

    return dest;
}
#endif

#ifdef CS_COMPILER_BCC
// Major hack due to pow failures in CS for Borland, removing this
// causes millions of strings to print out -- Brandon Ehle
#define pow(arga, argb) ( (!arga && !argb)?0:pow(arga, argb) )
// Dunno why this is in CS -- Brandon Ehle
#define DEBUG_BREAK
#endif

#if defined (CS_PROCESSOR_X86)
#  define CS_LITTLE_ENDIAN
#else
#  error "Please define a suitable CS_XXX_ENDIAN macro in win32/csosdefs.h!"
#endif

#if defined(CS_COMPILER_BCC)
  // The Borland C++ compiler does not accept a 'main' routine
  // in a program which already contains WinMain. This is a work-around.
  #undef main
  #define main csMain
#endif

// cygwin has no _beginthread and _endthread functions
#ifdef __CYGWIN32__
#ifndef _beginthread
#define _beginthread(func, stack, ptr)	CreateThread (0, 0, \
	  LPTHREAD_START_ROUTINE(func), ptr, CREATE_SUSPENDED, 0)
#endif
#ifndef _endthread
#define _endthread()  {}
#endif
#endif

// just to avoid windows.h inclusion
#define csSW_SHOWNORMAL 1

#if defined(CS_COMPILER_BCC)
  #define CS_WIN32_ARGC _argc
  #define CS_WIN32_ARGV _argv
#else
  #define CS_WIN32_ARGC __argc
  #define CS_WIN32_ARGV __argv
#endif

#ifdef __CYGWIN32__
#if !defined(CS_IMPLEMENT_PLATFORM_APPLICATION)
#define CS_IMPLEMENT_PLATFORM_APPLICATION
#endif

#else // __CYGWIN32__

/*
 if the EXE is compiled as a GUI app,
 a WinMain is needed. But if compiled
 as a console app it's not used but main() is
 instead. 
 */

#if !defined(CS_IMPLEMENT_PLATFORM_APPLICATION)
#define CS_IMPLEMENT_PLATFORM_APPLICATION                              \
int main (int argc, char* argv[]);                                     \
int WINAPI WinMain (HINSTANCE hApp, HINSTANCE prev, LPSTR cmd, int show)\
{                                                                      \
  (void)hApp;                                                          \
  (void)show;                                                          \
  (void)prev;                                                          \
  (void)cmd;                                                           \
  int ret = main(CS_WIN32_ARGC, CS_WIN32_ARGV);                        \
  return ret;                                                          \
}
#endif // CS_IMPLEMENT_PLATFORM_APPLICATION

#endif // __CYGWIN32__

#if !defined(CS_STATIC_LINKED)

#if !defined(CS_IMPLEMENT_PLATFORM_PLUGIN)
#define CS_IMPLEMENT_PLATFORM_PLUGIN                                   \
int _cs_main(int argc, char* argv[])                                   \
{                                                                      \
         return 0;                                                     \
}                                                                      \
extern "C" BOOL WINAPI                                                 \
DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpvReserved*/)  \
{                                                                      \
          return TRUE;                                                 \
}                                                                      \
CS_EXPORTED_FUNCTION const char* plugin_compiler()                     \
{                                                                      \
         return CS_COMPILER_NAME;                                      \
}
#endif // CS_IMPLEMENT_PLATFORM_PLUGIN

#endif // CS_STATIC_LINKED

#endif // __CS_CSOSDEFS_H__
