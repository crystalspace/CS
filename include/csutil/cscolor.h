/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_CSCOLOR_H__
#define __CS_CSCOLOR_H__

/**\file
 * Floating-point color
 */

#include "csextern.h"

/**
 * A class used to represent a color in RGB space.
 * This class is similar to csRGBpixel and csRGBcolor except that
 * it uses floating-point values to store R,G,B values.
 */
class csColor
{
public:
#if !defined(__STRICT_ANSI__) && !defined(SWIG)
  union
  {
    struct 
    {
#endif
      /// Red (0..1)
      float red;
      /// Green (0..1)
      float green;
      /// Blue (0..1)
      float blue;
#if !defined(__STRICT_ANSI__) && !defined(SWIG)
    };
    /// All components
    float m[3];
  };
#endif

public:
  /// Initialize a color object (contents undefined)
  csColor () { }
  /// Initialize a color object with given R,G,B components
  csColor (float r, float g, float b) : red (r), green (g), blue (b)
  {}
  /// Initialize a color object with same content in R,G,B components
  csColor (float v) : red (v), green (v), blue (v)
  {}
  /// Initialize a color object with an existing color
  csColor (const csColor& c)
  { red = c.red; green = c.green; blue = c.blue; }
  /// Set color to given R,G,B components
  void Set (float r, float g, float b)
  { red = r; green = g; blue = b; }
  /// Set color to given color.
  void Set (const csColor& c)
  { red = c.red; green = c.green; blue = c.blue; }
  /// Clamp color to given R,G,B values
  void Clamp (float r, float g, float b)
  {
    if (red > r) red = r;
    if (green > g) green = g;
    if (blue > b) blue = b;
  }
  /// Make sure color components are not negative.
  void ClampDown ()
  {
    if (red < 0) red = 0;
    if (green < 0) green = 0;
    if (blue < 0) blue = 0;
  }
  /// Check if color is all black (red green and blue all 0)
  bool IsBlack () const
  {
    return (red == 0 && green == 0 && blue == 0);
  }
  /// Check if color is all black (red green and blue all below threshold)
  bool IsBlack (float threshold) const
  {
    return (red < threshold && green < threshold && blue < threshold);
  }
  /// Assign one color object to another.
  csColor& operator= (const csColor& c)
  { red = c.red; green = c.green; blue = c.blue; return *this; }
  /// Multiply this color by a scalar value.
  csColor& operator*= (float f)
  { red *= f; green *= f; blue *= f; return *this; }
  /// Add another color to this one.
  csColor& operator+= (const csColor& c)
  { red += c.red; green += c.green; blue += c.blue; return *this; }
  /// Subtract another color to this one.
  csColor& operator-= (const csColor& c)
  { red -= c.red; green -= c.green; blue -= c.blue; return *this; }
  /// Multiply another color with this one.
  csColor& operator*= (const csColor& c)
  { red *= c.red; green *= c.green; blue *= c.blue; return *this; }
  /// Multiply this color by a scalar, return result
  csColor operator * (const float f)
  {
    csColor ret;
    ret.red = red*f;
    ret.green = green*f;
    ret.blue = blue*f;
    return ret;
  }
  /// Compare equality of two colors
  bool operator== (const csColor& c) const
  { return red == c.red && green == c.green && blue == c.blue; }
  /// Compare inequality of two colors
  bool operator!= (const csColor& c) const
  { return red != c.red || green != c.green || blue != c.blue; }
  /// Add given R,G,B components to color.
  void Add (float r, float g, float b)
  { red += r; green += g; blue += b; }
  /// Subtract given R,G,B components from color.
  void Subtract (float r, float g, float b)
  { red -= r; green -= g; blue -= b; }
  /// Return luminance of pixel (assuming sRGB color space)
  float Luminance() const
  { return red*0.2126f + green*0.7152f + blue*0.0722f; }

  /// Returns n-th component of the color.
#ifdef __STRICT_ANSI__
  inline float operator[] (size_t n) const { return !n?red:n&1?green:blue; }
#else
  inline float operator[] (size_t n) const { return m[n]; }
#endif

  /// Returns n-th component of the color.
#ifdef __STRICT_ANSI__
  inline float & operator[] (size_t n) { return !n?red:n&1?green:blue; }
#else
  inline float & operator[] (size_t n) { return m[n]; }
#endif
};

/// Divide a color by a scalar.
inline csColor operator/ (const csColor& v, float f)
{ f = 1.0f/f; return csColor(v.red*f, v.green*f, v.blue*f); }
/// Multiply two colors.
inline csColor operator* (const csColor& v1, const csColor& v2)
{
  return csColor (v1.red * v2.red,
  		  v1.green * v2.green,
		  v1.blue * v2.blue);
}


/// Multiply a color by a scalar value.
inline csColor operator* (const csColor& s, float f)
{ csColor c (s); c *= f; return c; }

/// Multiply a scalar value by a color.
inline csColor operator* (float f, const csColor& s)
{ csColor c (s); c *= f; return c; }

/// Add two colors.
inline csColor operator+ (const csColor& s1, const csColor& s2)
{ csColor c (s1); c += s2; return c; }
/// Subtract two colors.
inline csColor operator- (const csColor& s1, const csColor& s2)
{ csColor c (s1); c -= s2; return c; }

/**
 * A class used to represent a color in RGBA space.
 */
class csColor4 : public csColor
{
public:
  /// Alpha value
  float alpha;

  /// Initialize a color object (contents undefined)
  csColor4 () { }
  /// Initialize a color object with given R,G,B,A components
  csColor4 (float r, float g, float b, float a = 1.0f) : csColor (r, g, b)
  { alpha = a; }
  csColor4 (const csColor& c) : csColor (c), alpha (1.0f) { }
  void Set (const csColor& c)
  {
    red = c.red;
    green = c.green;
    blue = c.blue;
    alpha = 1.0f;
  }
  void Set (const csColor4& c)
  {
    red = c.red;
    green = c.green;
    blue = c.blue;
    alpha = c.alpha;
  }
  void Set (float r, float g, float b)
  {
    red = r;
    green = g;
    blue = b;
    alpha = 1.0f;
  }
  void Set (float r, float g, float b, float a)
  {
    red = r;
    green = g;
    blue = b;
    alpha = a;
  }
  /// Assign one color object to another.
  csColor4& operator= (const csColor4& c)
  {
    red = c.red;
    green = c.green;
    blue = c.blue;
    alpha = c.alpha;
    return *this;
  }
  /// Assign one color object to another/.
  csColor4& operator= (const csColor& c)
  { red = c.red; green = c.green; blue = c.blue; alpha = 1.0f; return *this; }
  /// Multiply this color by a scalar value.
  csColor4& operator*= (float f)
  { red *= f; green *= f; blue *= f; alpha *= f; return *this; }
  /// Add another color to this one.
  csColor4& operator+= (const csColor4& c)
  {
    red += c.red;
    green += c.green;
    blue += c.blue;
    alpha += c.alpha;
    return *this;
  }
  /// Add another color to this one.
  csColor4& operator+= (const csColor& c)
  { red += c.red; green += c.green; blue += c.blue; return *this; }
  /// Subtract another color to this one.
  csColor4& operator-= (const csColor4& c)
  {
    red -= c.red;
    green -= c.green;
    blue -= c.blue;
    alpha -= c.alpha;
    return *this;
  }
  /// Subtract another color to this one.
  csColor& operator-= (const csColor& c)
  { red -= c.red; green -= c.green; blue -= c.blue; return *this; }
  /// Compare equality of two colors
  bool operator== (const csColor4& c) const
  {
    return red == c.red &&
    	   green == c.green &&
	   blue == c.blue &&
	   alpha == c.alpha;
  }
  /// Compare inequality of two colors
  bool operator!= (const csColor4& c) const
  {
    return red != c.red ||
    	   green != c.green ||
	   blue != c.blue ||
	   alpha != c.alpha;
  }

  /// Returns n-th component of the color.
  inline float operator[] (size_t n) const
  { return (n == 3) ? alpha : csColor::operator[] (n); }

  /// Returns n-th component of the color.
  inline float & operator[] (size_t n)
  { return (n == 3) ? alpha : csColor::operator[] (n); }
};


/// Divide a color by a scalar.
inline csColor4 operator/ (const csColor4& v, float f)
{ f = 1.0f/f; return csColor4(v.red*f, v.green*f, v.blue*f, v.alpha*f); }
/// Multiply two colors.
inline csColor4 operator* (const csColor4& v1, const csColor4& v2)
{
  return csColor4 (v1.red * v2.red,
    v1.green * v2.green,
    v1.blue * v2.blue,
    v1.alpha * v2.alpha);
}


/// Multiply a color by a scalar value.
inline csColor4 operator* (const csColor4& s, float f)
{ csColor4 c (s); c *= f; return c; }

/// Multiply a scalar value by a color.
inline csColor4 operator* (float f, const csColor4& s)
{ csColor4 c (s); c *= f; return c; }

/// Add two colors.
inline csColor4 operator+ (const csColor4& s1, const csColor4& s2)
{ csColor4 c (s1); c += s2; return c; }
/// Subtract two colors.
inline csColor4 operator- (const csColor4& s1, const csColor4& s2)
{ csColor4 c (s1); c -= s2; return c; }

#endif // __CS_CSCOLOR_H__
