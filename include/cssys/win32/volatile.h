/*
  This header is used by COMP_BC, COMP_VC and COMP_GCC for all OS_WIN32 builds.
  You can change these macros to suit your own needs.
  For a description of what each macro does, see mk/user.mak.
*/
#ifndef __CS_VOLATILE_H__
#define __CS_VOLATILE_H__

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
  #define DO_SOUND
  #define DO_MMX
  #define ZLIB_DLL
#endif

// if you don't want to use direct input, comment this out
// keyboard handler will default to window message handling.
#if defined(COMP_GCC)
//#	define DO_DINPUT_KEYBOARD
#else
#	undef DO_DINPUT_KEYBOARD
#endif

#endif // __CS_VOLATILE_H__
