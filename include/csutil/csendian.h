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

#ifndef __CS_CSENDIAN_H__
#define __CS_CSENDIAN_H__

/**\file
 * Helpers for dealing with endian conversions.
 */
/**\addtogroup util
 * @{
 */

#include <math.h>
#include "cstypes.h"

#define csQroundSure(x) (int ((x) + ((x < 0) ? -0.5 : +0.5)))

/**
 * Methods to unconditionally swap the byte order of specifically sized types.
 */
struct csSwapBytes
{
private:
  struct Swap8
  {
    uint8 b1, b2, b3, b4, b5, b6, b7, b8;
  };
public:
  //@{
  /// Swap byte order
  static CS_FORCEINLINE uint16 Swap (uint16 s) 
  { return (s >> 8) | (s << 8); }
  static CS_FORCEINLINE int16  Swap (int16 s)
  { return (int16)Swap ((uint16)s); }
  static CS_FORCEINLINE uint32 Swap (uint32 l)
  { return (l >> 24) | ((l >> 8) & 0xff00) | ((l << 8) & 0xff0000) | (l << 24); }
  static CS_FORCEINLINE int32  Swap (int32 l)
  { return (int32)Swap ((uint32)l); }
  static CS_FORCEINLINE uint64 Swap (uint64 l)
  {
    uint64 r;
    Swap8 *p1 = (Swap8 *)&l;
    Swap8 *p2 = (Swap8 *)&r;
    p2->b1 = p1->b8;
    p2->b2 = p1->b7;
    p2->b3 = p1->b6;
    p2->b4 = p1->b5;
    p2->b5 = p1->b4;
    p2->b6 = p1->b3;
    p2->b7 = p1->b2;
    p2->b8 = p1->b1;
    return r;
  }
  static CS_FORCEINLINE int64  Swap (int64 l)
  { return (int64)Swap ((uint64)l); }
  
  static CS_FORCEINLINE uint16 UInt16 (uint16 x) { return Swap (x); }
  static CS_FORCEINLINE int16  Int16  (int16 x)  { return Swap (x); }
  static CS_FORCEINLINE uint32 UInt32 (uint32 x) { return Swap (x); }
  static CS_FORCEINLINE int32  Int32  (int32 x)  { return Swap (x); }
  static CS_FORCEINLINE uint64 UInt64 (uint64 x) { return Swap (x); }
  static CS_FORCEINLINE int64  Int64  (int64 x)  { return Swap (x); }
  //@}
};

#ifdef CS_BIG_ENDIAN
struct csBigEndian
#else
/**
 * Little endian to native conversion routines.
 * \remarks Since conversion from and to native representation is the same
 *  operation, all methods can be used for either direction.
 */
struct csLittleEndian
#endif
{
  //@{
  /// Convert specifically sized type from or to little endian.
  static CS_FORCEINLINE uint16 Convert (uint16 x) { return x; }
  static CS_FORCEINLINE int16  Convert (int16 x)  { return x; }
  static CS_FORCEINLINE uint32 Convert (uint32 x) { return x; }
  static CS_FORCEINLINE int32  Convert (int32 x)  { return x; }
  static CS_FORCEINLINE uint64 Convert (uint64 x) { return x; }
  static CS_FORCEINLINE int64  Convert (int64 x)  { return x; }
  
  static CS_FORCEINLINE uint16 UInt16 (uint16 x) { return Convert (x); }
  static CS_FORCEINLINE int16  Int16  (int16 x)  { return Convert (x); }
  static CS_FORCEINLINE uint32 UInt32 (uint32 x) { return Convert (x); }
  static CS_FORCEINLINE int32  Int32  (int32 x)  { return Convert (x); }
  static CS_FORCEINLINE uint64 UInt64 (uint64 x) { return Convert (x); }
  static CS_FORCEINLINE int64  Int64  (int64 x)  { return Convert (x); }
  //@}
};

#ifdef CS_LITTLE_ENDIAN
/**
 * Big endian to native conversion routines.
 * \remarks Since conversion from and to native representation is the same
 *  operation, all methods can be used for either direction.
 */
struct csBigEndian
#else
struct csLittleEndian
#endif
{
public:
  //@{
  /// Convert specifically sized type from or to big endian.
  static CS_FORCEINLINE uint16 Convert (uint16 s) 
  { return csSwapBytes::Swap (s); }
  static CS_FORCEINLINE int16  Convert (int16 s)
  { return csSwapBytes::Swap (s); }
  static CS_FORCEINLINE uint32 Convert (uint32 l)
  { return csSwapBytes::Swap (l); }
  static CS_FORCEINLINE int32  Convert (int32 l)
  { return csSwapBytes::Swap (l); }
  static CS_FORCEINLINE uint64 Convert (uint64 l)
  { return csSwapBytes::Swap (l); }
  static CS_FORCEINLINE int64  Convert (int64 l)
  { return csSwapBytes::Swap (l); }
  
  static CS_FORCEINLINE uint16 UInt16 (uint16 x) { return Convert (x); }
  static CS_FORCEINLINE int16  Int16  (int16 x)  { return Convert (x); }
  static CS_FORCEINLINE uint32 UInt32 (uint32 x) { return Convert (x); }
  static CS_FORCEINLINE int32  Int32  (int32 x)  { return Convert (x); }
  static CS_FORCEINLINE uint64 UInt64 (uint64 x) { return Convert (x); }
  static CS_FORCEINLINE int64  Int64  (int64 x)  { return Convert (x); }
  //@}
};

/**
 * Convert IEEE 32-bit floats from or to native machine floats.
 */
struct csIEEEfloat
{
#ifdef CS_IEEE_DOUBLE_FORMAT
  /// Convert native to IEEE
  static CS_FORCEINLINE uint32 FromNative (float f)
  { return *(uint32*)&f; }
  /// Convert IEEE to native
  static CS_FORCEINLINE float ToNative (uint32 f)
  { return *(float*)&f; }
#else
  #error Do not know how to convert to IEEE floats
#endif
};

/**
 * Sized data type access helpers.
 * On some platforms, certain data types can only be accessed when correctly 
 * aligned (e.g. uint32 can only be read from addresses aligned to 4 bytes).
 * This routines assist accessing sized types from arbitrary memory positions
 * (e.g. when parsing files from memory) by working around the alignment
 * requirements on platforms that have such.
 */
struct csGetFromAddress
{
  //@{
  /// Get specifically sized type from unaligned memory address
  static CS_FORCEINLINE uint16 UInt16 (const void *buff)
  {
  #ifdef CS_STRICT_ALIGNMENT
    uint16 s; memcpy (&s, buff, sizeof (s));
    return s;
  #else
    return *(uint16 *)buff;
  #endif
  }
  static CS_FORCEINLINE int16  Int16 (const void *buff)
  { return (int16)UInt16 (buff); }
  static CS_FORCEINLINE uint32 UInt32 (const void *buff)
  {
  #ifdef CS_STRICT_ALIGNMENT
    uint32 s; memcpy (&s, buff, sizeof (s));
    return s;
  #else
    return *(uint32 *)buff;
  #endif
  }
  static CS_FORCEINLINE int32  Int32 (const void *buff)
  { return (int32)UInt32 (buff); }
  static CS_FORCEINLINE uint64 UInt64 (const void *buff)
  {
  #ifdef CS_STRICT_ALIGNMENT
    uint64 s; memcpy (&s, buff, sizeof (s));
    return s;
  #else
    return *(uint64 *)buff;
  #endif
  }
  static CS_FORCEINLINE int64  Int64 (const void *buff)
  { return (int64)UInt64 (buff); }
  //@}
};

/**
 * Sized data type access helpers.
 * On some platforms, certain data types can only be manipulated when correctly 
 * aligned (e.g. uint32 can only be Written to addresses aligned to 4 bytes).
 * This routines assist manipulating sized types at arbitrary memory positions
 * (e.g. when constructing files in memory) by working around the alignment
 * requirements on platforms that have such.
 */
struct csSetToAddress
{
  //@{
  /// Set specifically sized type at unaligned memory address
  static CS_FORCEINLINE void UInt16 (void *buff, uint16 s)
  {
  #ifdef CS_STRICT_ALIGNMENT
    memcpy (buff, &s, sizeof (s));
  #else
    *((uint16 *)buff) = s;
  #endif
  }
  static CS_FORCEINLINE void Int16  (void *buff, int16 s)
  { UInt16 (buff, (uint16)s); }
  static CS_FORCEINLINE void UInt32 (void *buff, uint32 s)
  {
  #ifdef CS_STRICT_ALIGNMENT
    memcpy (buff, &s, sizeof (s));
  #else
    *((uint32 *)buff) = s;
  #endif
  }
  static CS_FORCEINLINE void Int32  (void *buff, int32 s)
  { UInt32 (buff, (uint32)s); }
  static CS_FORCEINLINE void UInt64 (void *buff, uint64 s)
  {
  #ifdef CS_STRICT_ALIGNMENT
    memcpy (buff, &s, sizeof (s));
  #else
    *((uint64 *)buff) = s;
  #endif
  }
  static CS_FORCEINLINE void Int64  (void *buff, int64 s)
  { UInt64 (buff, (uint64)s); }
  //@}
};

/**\name Deprecated endian conversion routines.
 * It is recommended to use csLittleEndian, csBigEndian, csIEEEfloat, 
 * csGetFromAddress or csSetToAddress for the tasks below.
 * @{ */

/// Convert a longlong from big-endian to machine format
static inline uint64 csBigEndianLongLong (uint64 l)
{ return csBigEndian::Convert (l); }

/// Convert a long from big-endian to machine format
static inline uint32 csBigEndianLong (uint32 l)
{ return csBigEndian::Convert (l); }

/// Convert a short from big-endian to machine format
static inline uint16 csBigEndianShort (uint16 s)
{ return csBigEndian::Convert (s); }

/// Convert a big-endian floating-point number to machine format
static inline float csBigEndianFloat (float f)
{ 
  uint32 u = csBigEndian::Convert (*(uint32*)&f); 
  return *(float*)&u;
}

/// Convert a longlong from little-endian to machine format
static inline uint64 csLittleEndianLongLong (uint64 l)
{ return csLittleEndian::Convert (l); }

/// Convert a long from little-endian to machine format
static inline uint32 csLittleEndianLong (uint32 l)
{ return csLittleEndian::Convert (l); }

/// Convert a short from little-endian to machine format
static inline uint16 csLittleEndianShort (uint16 s)
{ return csLittleEndian::Convert (s); }

/// Convert a little-endian floating-point number to machine format
static inline float csLittleEndianFloat (float f)
{
  uint32 u = csLittleEndian::Convert (*(uint32*)&f); 
  return *(float*)&u;
}

/*
    To be able to painlessly transfer files betwen platforms, we should
    avoid using native floating-point format. Here are a couple of routines
    that are guaranteed to work on all platforms.

    The floating point is converted to a fixed 1.7.25 bits format
    (one bit sign, 7 bits exponent, 25 bits mantissa) and back,
    so that we can binary store floating-point number without
    cross-platform problems. If you wonder why 1+7+25 = 33 while we
    only have 32 bits, we'll ommit the most significant bit of mantissa
    since it is always 1 (we use normalized numbers). This increases the
    precision twice.

    For double, we use one bit sign, 15 bits exponent, 49 bits mantissa.
*/

/**
 * Convert a float to a cross-platform 32-bit format (no endianess
 * adjustments!)
 */
static inline int32 csFloatToLong (float f)
{
  int exp;
  int32 mant = csQroundSure (frexp (f, &exp) * 0x1000000);
  int32 sign = mant & 0x80000000;
  if (mant < 0) mant = -mant;
  if (exp > 63) exp = 63; else if (exp < -64) exp = -64;
  return sign | ((exp & 0x7f) << 24) | (mant & 0xffffff);
}

/// Convert a 32-bit cross-platform float to native format (no endianess adjustments!)
static inline float csLongToFloat (int32 l)
{
  int exp = (l >> 24) & 0x7f;
  if (exp & 0x40) exp = exp | ~0x7f;
  float mant = float (l & 0x00ffffff) / 0x1000000;
  if (l & 0x80000000) mant = -mant;
  return (float) ldexp (mant, exp);
}

/* Implementation note: csDoubleToLongLong() and csLongLongToDouble()
 *
 * We avoid use of CONST_INT64() because 64-bit constants are illegal with g++
 * under -ansi -pedantic, and we want this header to be useful to external
 * projects which use -ansi -pedantic.  Instead, we use bit shifts, such as (1
 * << 59), and construct `mask' manually.
 */

/// Convert a double to a cross-platform 64-bit format (no endianess adjustments!)
static inline int64 csDoubleToLongLong (double d)
{
  int exp;
  int64 mant = (int64) (frexp (d, &exp) * ((int64)1 << 48));
  int64 sign = mant & ((int64)1 << 59);
  if (mant < 0) mant = -mant;
  if (exp > 32767) exp = 32767; else if (exp < -32768) exp = -32768;
  int64 const mask = ((uint64)0xffff << 32) | (uint64)0xffffffff;
  return sign | ((int64 (exp) & 0x7fff) << 48) | (mant & mask);
}

/// Convert a 64-bit cross-platform double to native format (no endianess adjustments!)
static inline double csLongLongToDouble (int64 i)
{
  int exp = (i >> 48) & 0x7fff;
  if (exp & 0x4000) exp = exp | ~0x7fff;
  int64 const mask = ((uint64)0xffff << 32) | (uint64)0xffffffff;
  double mant = double (i & mask) / ((int64)1 << 48);
  if (i & ((int64)1 << 59)) mant = -mant;
  return ldexp (mant, exp);
}

/* *\name Floating point conversions
 * The following routines are used for converting floating-point numbers
 * into 16-bit shorts and back. This is useful for low-precision data.
 * They use the 1.4.12 format. The range of numbers that can be represented
 * in this format is from 2^-8 to 2^7. The precision for numbers near to
 * 2^-8 (0.00390625) is near 0.000001, for numbers near 2^7 (128) is near 0.03.
 * @{ */

/// Convert a float to a cross-platform 16-bit format (no endianess adjustments!)
static inline short csFloatToShort (float f)
{
  int exp;
  long mant = csQroundSure (frexp (f, &exp) * 0x1000);
  long sign = mant & 0x8000;
  if (mant < 0) mant = -mant;
  if (exp > 7) mant = 0x7ff, exp = 7; else if (exp < -8) mant = 0, exp = -8;
  return short(sign | ((exp & 0xf) << 11) | (mant & 0x7ff));
}

/// Convert a 16-bit cross-platform float to native format (no endianess adjustments!)
static inline float csShortToFloat (short s)
{
  int exp = (s >> 11) & 0xf;
  if (exp & 0x8) exp = exp | ~0xf;
  float mant = float ((s & 0x07ff) | 0x0800) / 0x1000;
  if (s & 0x8000) mant = -mant;
  return (float) ldexp (mant, exp);
}

/* * @} */

/// Swap the bytes in a uint64 value.
static inline uint64 csConvertEndian (uint64 l)
{ return csLittleEndianLongLong (l); }

/// Swap the bytes in a int64 value.
static inline int64 csConvertEndian (int64 l)
{ return csLittleEndianLongLong (l); }

/// Swap the bytes in a uint32 value.
static inline uint32 csConvertEndian (uint32 l)
{ return csLittleEndianLong (l); }

/// Swap the bytes in a int32 value.
static inline int32 csConvertEndian (int32 l)
{ return csLittleEndianLong (l); }

/// Swap the bytes in a int16 value.
static inline int16 csConvertEndian (int16 s)
{ return csLittleEndianShort (s); }

/// Swap the bytes in a uint16 value.
static inline uint16 csConvertEndian (uint16 s)
{ return csLittleEndianShort (s); }

/// Swap the bytes in a float value.
static inline float csConvertEndian (float f)
{ return csLittleEndianFloat (f); }

/// Read a little-endian short from address
inline uint16 csGetLittleEndianShort (const void *buff)
{
  return csGetFromAddress::UInt16 (buff);
}

/// Read a little-endian long from address
inline uint32 csGetLittleEndianLong (const void *buff)
{
  return csGetFromAddress::UInt32 (buff);
}

/// Read a little-endian 32-bit float from address
inline float csGetLittleEndianFloat32 (const void *buff)
{ uint32 l = csGetLittleEndianLong (buff); return csLongToFloat (l); }

/// Read a little-endian 16-bit float from address
inline float csGetLittleEndianFloat16 (const void *buff)
{ uint16 s = csGetLittleEndianShort (buff); return csShortToFloat (s); }

/** @} */

/** @} */

#endif // __CS_CSENDIAN_H__
