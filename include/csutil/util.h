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
long RndNum(long minRange, long maxRange);

/// Allocate a new char [] and copy the string into newly allocated storage
extern char *strnew (const char *s);

/**
 * Compute all combinations of m by n. Calls the callback function with
 * a vector of numbers from 0 to m - 1; numbers never appears twice in vector.
 * Callback returns false for continuation and true to break the loop.
 */
extern void Combinations (int m, int n, bool (*callback) (int *vector, int count,
  void *arg), void *arg);

/**
 * Expand a filename if it contains shortcuts ('.', '~', '..' etc)<p>
 * Return a newly-allocated with new [] "char *".
 */
extern char *expandname (char *iName);
/**
 * Split a pathname into separate path and name. Path delimiters are either
 * '/', PATH_SEPARATOR and, for OS/2, MS-DOS and Win32 targets, ':'.
 */
extern void splitpath (char *iPathName, char *iPath, size_t iPathSize,
  char *iName, size_t iNameSize);

/// Swap two integer variables
inline void iSwap (int &a, int &b)
{
  int tmp = a;
  a = b; b = tmp;
}

/// Swap two floating-point variables
inline void fSwap (float &a, float &b)
{
  float tmp = a;
  a = b; b = tmp;
}

/// Return the argument squared
inline float fSquare (float x)
{
  return x*x;
}

/// Byte swap 32 bit data.
inline unsigned long ByteSwap32bit( const unsigned long value )
{
  return ((value >> 24 ) & 0x000000FF ) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | (( value << 24) & 0xFF000000);
}

/// Byte swap 16 bit data.
inline unsigned short ByteSwap16bit( const unsigned short value )
{
  return (( value >> 8 ) & 0x000000FF ) | (( value << 8 ) & 0x0000FF00 );
}

/// Byte swap 32 bit data in a buffer
void ByteSwap32bitBuffer( register unsigned long* const place, register unsigned long count );

/// Byte swap 16 bit data in a buffer
void ByteSwap16bitBuffer( register unsigned short* const place, register unsigned long count );

/// finds the smallest number that is a power of two and is larger or equal to n
int FindNearestPowerOf2 (int n);

/// returns true if n is a power of two
bool IsPowerOf2 (int n);

#endif // __UTIL_H__
