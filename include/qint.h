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

#if !defined(OS_NEXT) && (defined(PROC_INTEL) || defined(PROC_M68K))
#define CS_USE_FAST_FLOAT_TO_INT
#else
#undef  CS_USE_FAST_FLOAT_TO_INT
#endif

#if !defined(CS_USE_FAST_FLOAT_TO_INT)

#define QRound(x) ((int)((x)+.5))
#define QInt(x)   ((int)(x))
#define QInt16(x) ((int)((x)*65536.))

#else

#ifdef PROC_INTEL
/*
//#pragma aux RoundToInt=\
//        "fistp DWORD [eax]"\
//        parm nomemory [eax] [8087]\
//        modify exact [8087];
*/

// This is 'stolen' from someone (I don't remember who anymore). It
// is a nice and fast way to convert a floating point number to int
// (only works on a i386 type processor).
// It is equivalent to 'i=(int)(f+.5)'. 
#define FIST_MAGIC ((((65536.0 * 65536.0 * 16)+(65536.0 * 0.5))* 65536.0))
inline long QuickRound (float inval)
{
  double dtemp = FIST_MAGIC + inval;
  return ((*(long *)&dtemp) - 0x80000000);
}

inline long QuickInt (float inval)
{
  double dtemp = FIST_MAGIC + (inval-.4999);
  return ((*(long *)&dtemp) - 0x80000000);
}

// This is my own invention derived from the previous one. This converts
// a floating point number to a 16.16 fixed point integer. It is
// equivalent to 'i=(int)(f*65536.)'.
#define FIST_MAGIC2 ((((65536.0 * 16)+(0.5))* 65536.0))
inline long QuickInt16 (float inval)
{
  double dtemp = FIST_MAGIC2 + inval;
  return ((*(long *)&dtemp) - 0x80000000);
}
#endif //PROC_INTEL

#ifdef PROC_M68K

#define FIST_MAGIC ((((65536.0 * 65536.0 * 16)+(65536.0 * 0.5))* 65536.0))
inline long QuickRound (float inval)
{
  double dtemp = FIST_MAGIC + inval;
  return (*(((long *)&dtemp) + 1)) - 0x80000000;
}
    
inline long QuickInt (float inval)
{
  double dtemp = FIST_MAGIC + (inval-.4999);
  return (*(((long *)&dtemp) + 1)) - 0x80000000;
}
	
#define FIST_MAGIC2 ((((65536.0 * 16)+(0.5))* 65536.0))
inline long QuickInt16 (float inval)
{
  double dtemp = FIST_MAGIC2 + inval;
  return (*(((long *)&dtemp) + 1)) - 0x80000000;
}
#endif

#define QRound(x) QuickRound(x)
#define QInt(x)   QuickInt(x)
#define QInt16(x) QuickInt16(x)

#endif // CS_USE_FAST_FLOAT_TO_INT

// @@@ I don't know if there is a better way to convert
// a floating point to 8:24 fixed point (one with constants
// like the tricks above instead of the multiplication).
#define QInt24(x) (QInt16(((x)*256.)))

#endif // __QINT_H__
