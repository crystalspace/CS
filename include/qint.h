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

#ifndef __QINT_H__
#define __QINT_H__

#if (defined (PROC_INTEL) || defined (PROC_M68K)) && !defined (OS_NEXT) && !defined(OS_BE)

// These are 'stolen' from someone (I don't remember who anymore). It
// is a nice and fast way to convert a floating point number to int
// (only tested on i386 and m68k type processors).
// This constant is used for computing 'i=(int)(f+.5)'. 
#define FIST_MAGIC1 ((((65536.0 * 65536.0 * 16.0) + (65536.0 * 0.5)) * 65536.0))
// This constant is used for computing 'i=(int)(f*65536+.5)'. 
#define FIST_MAGIC2 ((((65536.0 * 16.0) + (0.5)) * 65536.0))
// This constant is used for computing 'i=(int)(f*65536+.5)'. 
#define FIST_MAGIC3 ((((65536.0 * 16.0 / 256.0)+(0.5 / 256.0)) * 65536.0))

static inline long QRound (float inval)
{
#if defined (COMP_GCC) && defined (PROC_INTEL)
  long ret;
  asm ("fistpl %0" : "=m" (ret) : "t" (inval) : "st");
  return ret;
#else
  double dtemp = FIST_MAGIC1 + inval;
  return ((*(long *)&dtemp) - 0x80000000);
#endif
}

static inline long QInt (float inval)
{
#if defined (COMP_GCC) && defined (PROC_INTEL)
  long ret;
  asm ("fistpl %0" : "=m" (ret) : "t" (double (inval) - (0.5 - FLT_EPSILON)) : "st");
  return ret;
#else
  // The magic constant you see has been deduced experimentally:
  // it is enough to round "1 - FLT_EPSILON" to 0, and "1" to 1.
  double dtemp = FIST_MAGIC1 + (double (inval) - 0.4997558593749999722444);
  return ((*(long *)&dtemp) - 0x80000000);
#endif
}

inline long QInt16 (float inval)
{
#if defined (COMP_GCC) && defined (PROC_INTEL)
  long ret;
  asm ("fistpl %0" : "=m" (ret) : "t" (inval * 0x10000) : "st");
  return ret;
#else
  double dtemp = FIST_MAGIC2 + inval;
  return ((*(long *)&dtemp) - 0x80000000);
#endif
}

inline long QInt24 (float inval)
{
#if defined (COMP_GCC) && defined (PROC_INTEL)
  long ret;
  asm ("fistpl %0" : "=m" (ret) : "t" (inval * 0x1000000) : "st");
  return ret;
#else
  double dtemp = FIST_MAGIC3 + inval;
  return ((*(long *)&dtemp) - 0x80000000);
#endif
}
    
#else

#define QRound(x) ((int)((x)+.5))
#define QInt(x)   ((int)(x))
#define QInt16(x) ((int)((x)*65536.))
#define QInt24(x) (QInt16(((x)*256.)))
	
#endif

#endif // __QINT_H__
