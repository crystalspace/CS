/*
  This header is used by COMP_VC and COMP_BC for OS_WIN32 builds.  It is
  not used for COMP_GCC builds under normal circumstances since GCC builds
  are performed in concert with invocation of the CS configure script which
  generates a suitable volatile.h file.
*/
#ifndef __CS_WIN32_VOLATILE_H__
#define __CS_WIN32_VOLATILE_H__

#define OS_WIN32
#if !defined(CS_PLATFORM_NAME)
#  define CS_PLATFORM_NAME "Win32"
#endif

#define PROC_X86
#if !defined(CS_PROCESSOR_NAME)
#  define CS_PROCESSOR_NAME "X86"
#endif

#if defined(__BORLANDC__)
#  define COMP_BC
#  if !defined(CS_COMPILER_NAME)
#    define CS_COMPILER_NAME "Borland"
#  endif
#  define __NEED_GENERIC_ISDIR
#elif defined(__MINGW32__) || defined(__CYGWIN32__)
#  define COMP_GCC
#  if !defined(CS_COMPILER_NAME)
#    define CS_COMPILER_NAME "GCC"
#  endif
#else
#  define COMP_VC
#  if !defined(CS_COMPILER_NAME)
#    define CS_COMPILER_NAME "VisualC"
#  endif
#endif

#if !defined (COMP_GCC)
#  define DO_MMX
#endif

#define CS_RGBCOLOR_SANE
#define CS_RGBPIXEL_SANE

#define CS_HAS_WCHAR_H
#define CS_WCHAR_T_SIZE 2

#define CS_EMBED_PLUGIN_META

//#define CS_BUILD_SHARED_LIBS

#ifdef _WIN64
  #define CS_PLATFORM_IS_64BITS
#endif

//#define CS_REF_TRACKER

#endif // __CS_WIN32_VOLATILE_H__
