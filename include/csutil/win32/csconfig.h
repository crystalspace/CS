/*
  This header is used by CS_COMPILER_MSVC and CS_COMPILER_BCC for
  CS_PLATFORM_WIN32 builds.  It is not used for CS_COMPILER_GCC builds under
  normal circumstances since GCC builds are performed in concert with
  invocation of the CS configure script which generates a suitable csconfig.h
  file.
*/
#ifndef __CS_WIN32_CSCONFIG_H__
#define __CS_WIN32_CSCONFIG_H__

#define CS_PACKAGE_NAME "crystalspace"

#define CS_PLATFORM_WIN32
#if !defined(CS_PLATFORM_NAME)
#  define CS_PLATFORM_NAME "Win32"
#endif

#define CS_PROCESSOR_X86
#if !defined(CS_PROCESSOR_NAME)
#  define CS_PROCESSOR_NAME "X86"
#endif

#if defined(__BORLANDC__)
#  define CS_COMPILER_BCC
#  if !defined(CS_COMPILER_NAME)
#    define CS_COMPILER_NAME "Borland"
#  endif
#  define CS_USE_CUSTOM_ISDIR
#elif defined(__MINGW32__) || defined(__CYGWIN32__)
#  define CS_COMPILER_GCC
#  if !defined(CS_COMPILER_NAME)
#    define CS_COMPILER_NAME "GCC"
#  endif
#else
#  define CS_COMPILER_MSVC
#  if !defined(CS_COMPILER_NAME)
#    define CS_COMPILER_NAME "VisualC"
#  endif
#endif

#if !defined (CS_COMPILER_GCC)
#  define CS_USE_MMX
#endif

#define CS_HAS_WCHAR_H
#define CS_WCHAR_T_SIZE 2

#define CS_USE_FAKE_SOCKLEN_TYPE

#define CS_EMBED_PLUGIN_META

//#define CS_BUILD_SHARED_LIBS

#ifdef _WIN64
  #define CS_PROCESSOR_SIZE	64
#else
  #define CS_PROCESSOR_SIZE	32
#endif

//#define CS_REF_TRACKER
//#define CS_MEMORY_TRACKER

#if defined(CS_COMPILER_MSVC) && (_MSC_VER >= 1400)
  #define _CRT_SECURE_NO_DEPRECATE
	/* In VC8, a lot of CRT methods were marked "deprecated" b/c they're 
	   deemed "insecure". Silence those warnings. 
	   NB: This is here b/c it needs to be set before any standard headers
	   are included. */
#endif

#endif // __CS_WIN32_CSCONFIG_H__
