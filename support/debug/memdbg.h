/*
    Copyright (C) 1999 by Andrew Zabolotny
    Cross-platform memory debugger: private include file

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

#ifndef __MEMDBG_H__
#define __MEMDBG_H__

/// The 'address' type
typedef void *address;

/*
 * First we resolve a few platform dependent issues.
 * We should know the following things that depend on platform we're running on:
 *
 * - You also may define the GET_CALL_ADDRESS macro if the version provided
 *   below (that is supposed to work on most platforms) won't work.
 * - You also may want to define the uint32 macro that stands for an unsigned
 *   32-bits integer.
 */

#if !defined (MAX_ADDRESS)
#  define MAX_ADDRESS address (-1)
#endif

#if !defined (uint32)
#  define uint32 unsigned long
#endif

#if !defined (DEBUG_BREAK)
#  define DEBUG_BREAK
#endif

/**
 * Some C runtime libraries provide an alternative name for malloc() and
 * company. If this is the case, we'll be able to debug malloc/free's as
 * well as new/delete calls.
 */
#if defined (OS_LINUX)
#  define DEBUG_MALLOC
#  define MALLOC  __libc_malloc
#  define CALLOC  __libc_calloc
#  define FREE    __libc_free
#  define REALLOC __libc_realloc
#elif defined (OS_OS2)
#  define DEBUG_MALLOC
#  define MALLOC  _malloc
#  define CALLOC  _calloc
#  define FREE    _free
#  define REALLOC _realloc
#else
#  define MALLOC  malloc
#  define FREE    free
#  define REALLOC realloc
#endif

/**
 * This is a trick that should work on x86 platforms.
 * Since the standard C calling convention says that the address of
 * function arguments should increase from first to last argument, we
 * suppose that the previous machine word on stack contains the return
 * address. This way:
 *
 * +---+----+----+---+
 * |ret|arg0|arg1|...|
 * +---+----+----+---+
 *
 * By going sizeof (address) bytes backward from the address of arg0
 * and taking the value found there we should be able to get the
 * return address.
 *
 * GCC versions above 2.8.0 have a extremely useful function in this
 * regard. It is fully platform-independent, thus we prefer it over
 * other methods.
 */
#if (__GNUC__ >= 2) && (__GNUC_MINOR__ >= 8)
#  define GET_CALL_ADDRESS(firstarg)		\
     address addr = (address)__builtin_return_address (0);
#elif defined (PROC_X86)
#  define GET_CALL_ADDRESS(firstarg)		\
     address addr = ((address *)&firstarg) [-1];
#else
#  error "Please define a GET_CALL_ADDRESS macro suitable for your platform!"
#endif

/******************************************************************************
 ************************* Memory debugger mode flags *************************
 ******************************************************************************/

/**
 * Fill memory after allocation with garbage?
 * This is highly recommended as it is not too expensive but allows to
 * detect cases when memory is used without being initialized first.
 * If MDF_FREEFILL flag is not used, the memory is also filled before
 * being freed.
 * Option: a
 */
#define MDF_ALLOCFILL	0x00000001
/**
 * Do we want to fill freed memory with garbage and leave it non-freed?
 * Upon exiting the application checks the contents of these blocks to
 * remain intact, otherwise it generates information on changed blocks.
 * WARNING: This is quite expensive in memory terms!
 * Option: f
 */
#define MDF_FREEFILL	0x00000002
/**
 * Do we want debugger breakpoints when detecting serious erros?
 * The debugger will break inside operator new or delete right before
 * exiting back to user program. By stepping a few you can detect
 * the place where error occured.
 * Option: d
 */
#define MDF_DEBUGBREAK	0x00000004
/**
 * Do we want memory debugger to print an information string each
 * time new or delete is called? It is recommended you to redirect
 * stdout when using this flag as output to console will immensely
 * slow down your program.
 * Option: v
 */
#define MDF_VERBOSE	0x00000008
/**
 * Do we want a summary sheet at the end of program?
 * The sheet will list the summary number of memory allocations,
 * memory frees, the peak memory consumption and lots of other
 * useful information.
 * Option: s
 */
#define MDF_SUMMARY	0x00000010
/**
 * Do we want a list of unfreed memory blocks at the end of program?
 * The list will also contain the location where the corresponding
 * memory block was allocated.
 * Option: l
 */
#define MDF_LISTUNFREED	0x00000020
/**
 * Detect writes past the block bounds? This is implemented by allocating
 * slightly bigger blocks than actually requested, and by filling those
 * inter-block spaces with some well-known value. When block is freed, the
 * space between blocks is checked to contains same well-known value.
 * Option: b
 */
#define MDF_CHECKBOUNDS	0x00000040
/**
 * Redirect (append) output from console to a log file called memdbg.log.
 * Option: L
 */
#define MDF_LOGFILE	0x00000080

/// Default memory debugger flags
#define MDF_DEFAULT	MDF_ALLOCFILL | MDF_LISTUNFREED | MDF_SUMMARY | \
			MDF_CHECKBOUNDS | MDF_LOGFILE

/// Load executable memory map from memdbg.map
void mdbLoadMap ();
/// Query file name and line number given address
void mdbGetPos (address addr, const char *&file, const char *&func, int &line);
/// Return a string description of a location
char *mdbLocation (address addr);

#endif // __MEMDBG_H__
