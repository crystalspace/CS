/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>

/// Return a random number.
long csRndNum (long minRange, long maxRange);

/**
 * Allocate a new char [] and copy the string into newly allocated storage.
 * This is a handy method for copying strings, in fact it is the C++ analogue
 * of the strdup() function from string.h (strdup() is not present on some
 * platforms). To free the pointer the caller should call delete[].
 */
extern char *csStrNew (const char *s);

/**
 * Compute all combinations of m by n. Calls the callback function with
 * a vector of numbers from 0 to m - 1; numbers never appears twice in vector.
 * Callback returns false for continuation and true to break the loop.
 */
extern void csCombinations (int m, int n,
  bool (*callback) (int *vector, int count,
  void *arg), void *arg);

/**
 * Expand a filename if it contains shortcuts.
 * Currently the following macros are recognised and expanded:
 * <pre>
 * '.', '~', '..', 'drive:' (on DOS/Win32/OS2)
 * </pre>
 * The returned filename is always absolute, i.e. it always starts
 * from root. Return a string allocated with csStrNew().
 */
extern char *csExpandName (const char *iName);

/**
 * Split a pathname into separate path and name. Path delimiters are either
 * '/', PATH_SEPARATOR and, for OS/2, MS-DOS and Win32 targets, ':'.
 */
extern void csSplitPath (const char *iPathName, char *oPath, size_t iPathSize,
  char *oName, size_t iNameSize);

/**
 * This is a really simple function that does very nice
 * "filename against filemask" comparisons. It understands
 * even such things like "*a*.txt" or "*a?b*" or even "*"
 * (wish I DOS could do it ...). No "[]" wildcards though :-)
 * <p>
 * NOTE: If you want case-insensitive comparison, upcase strings first.
 */
extern bool csGlobMatches (const char *fName, const char *fMask);

/// Swap two integer variables
static inline void csSwapInt (int &a, int &b)
{
  int tmp = a;
  a = b; b = tmp;
}

/// Swap two floating-point variables
static inline void csSwapFloat (float &a, float &b)
{
  float tmp = a;
  a = b; b = tmp;
}

/// Return the argument squared
static inline float csSquareFloat (float x)
{
  return x*x;
}

/// Byte swap 32 bit data.
static inline unsigned long csByteSwap32bit( const unsigned long value )
{
  return ((value >> 24 ) & 0x000000FF ) | ((value >> 8) & 0x0000FF00)
  	| ((value << 8) & 0x00FF0000) | (( value << 24) & 0xFF000000);
}

/// Byte swap 16 bit data.
static inline unsigned short csByteSwap16bit( const unsigned short value )
{
  return (( value >> 8 ) & 0x000000FF ) | (( value << 8 ) & 0x0000FF00 );
}

/// Byte swap 32 bit data in a buffer
void csByteSwap32bitBuffer( register unsigned long* const place,
	register unsigned long count );

/// Byte swap 16 bit data in a buffer
void csByteSwap16bitBuffer( register unsigned short* const place,
	register unsigned long count );

/**
 * Finds the smallest number that is a power of two and is larger or
 * equal to n.
 */
int csFindNearestPowerOf2 (int n);

/// returns true if n is a power of two
bool csIsPowerOf2 (int n);

/// Find the log2 of argument
static inline int csLog2 (int n)
{
  int l = 31; unsigned x = 0x80000000;
  while (x && !(n & x))
    l--, x >>= 1;
  return l;
}

/**
 * given src and dest, which are already allocated, copy source to dest.
 * But, do not copy 'search', instead replace that with 'replace' string.
 * max is size of dest.
 */
void csFindReplace (char *dest, const char *src, const char *search,
  const char *replace, int max);

#endif // __UTIL_H__
