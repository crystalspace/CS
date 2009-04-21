/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2003 by Frank Richter
              (C) 2006 by Christopher Nelson

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

#ifndef __CS_IVARIA_GRADIENT_H__
#define __CS_IVARIA_GRADIENT_H__

/**\file
 * Simple color gradient interfaces
 */

#include "iutil/array.h"

#include "csutil/cscolor.h"
#include "csgfx/rgbpixel.h"

/**
 * An entry in an iGradient gradient.
 */
struct csGradientShade
{
  /// Color of the left side
  csColor4 left;
  /// Color of the right side
  csColor4 right;
  /// Position in the gradient
  float position;
  
  /// Construct with all values set to 0
  csGradientShade () : left (0, 0, 0, 1.0f), right (0, 0, 0, 1.0f), 
    position (0) {}
      
  /// Construct supplying all values
  csGradientShade (const csColor4& left_color, const csColor4& right_color, 
    float pos)  : left (left_color), right (right_color), position (pos) {}
  /// Construct supplying all values
  explicit csGradientShade (const csColor& left_color, 
    const csColor& right_color, float pos) : left (left_color), 
    right (right_color), position (pos) {}
      
  /**
   * Construct using a color and position. Both left and right will have the 
   * value of color.
   */
  csGradientShade (const csColor4& color, float pos) : left (color), 
    right (color), position (pos) {}
  /**
   * Construct using a color and position. Both left and right will have the 
   * value of color.
   */
  explicit csGradientShade (const csColor& color, float pos) : 
    left (color), right (color), position (pos) {}

  bool operator== (const csGradientShade& other)
  { 
    return (left == other.left) && (right == other.right)
      && (position == other.position);
  }
};

/**
 * An array of gradient shades.
 */
struct iGradientShades : public iArrayReadOnly<csGradientShade>
{
  SCF_IARRAYREADONLY_INTERFACE(iGradientShades);
};

/**
 * A simple color gradient.
 * If you ever have worked with an image creation/manipulation program with
 * a slightly higher niveau than Windows Paint then you probably know what 
 * this is.
 *
 * Colors(here called 'shades') can be placed at arbitrary positions; although 
 * commonly a range of [0;1] is used, negative positions and positions larger 
 * than 1 are supported.
 *
 * Shades contain actually two colors, a 'left' and 'right' one. You can think
 * of this as, when approaching from one side, you'll get closer and closer to
 * the respective color. If you step over a shade, you have the other color, 
 * but you're getting farther and farther from it (and towards the next color) 
 * when moving on. This feature can be used for sharp transitions; for smooth 
 * ones they are simply set to the same value.
 *
 * This interface is implemented by csGradient.
 *
 * Examples:
 * \code
 * csRef<iGradient> grad;
 * // Rainbow-ish
 * grad->AddShade (csColor4 (1.0f, 0.0f, 0.0f, 1.0f), 0.0f);
 * grad->AddShade (csColor4 (1.0f, 1.0f, 0.0f, 1.0f), 0.2f);
 * grad->AddShade (csColor4 (0.0f, 1.0f, 0.0f, 1.0f), 0.4f);
 * grad->AddShade (csColor4 (0.0f, 1.0f, 1.0f, 1.0f), 0.6f);
 * grad->AddShade (csColor4 (0.0f, 0.0f, 1.0f, 1.0f), 0.8f);
 * grad->AddShade (csColor4 (1.0f, 0.0f, 1.0f, 1.0f), 1.0f);
 *
 * // German flag
 * grad->Clear ();
 * grad->AddShade (csColor4 (0.0f, 0.0f, 0.0f, 1.0f), 0.0f);
 * grad->AddShade (csColor4 (0.0f, 0.0f, 0.0f, 1.0f), 
 *   csColor4 (1.0f, 0.0f, 0.0f, 1.0f), 0.33f);
 * grad->AddShade (csColor4 (1.0f, 0.0f, 0.0f, 1.0f), 
 *  csColor4 (1.0f, 1.0f, 0.0f, 1.0f),
 *  0.66f);
 * grad->AddShade (csColor4 (1.0f, 1.0f, 0.0f, 1.0f), 1.0f);
 * \endcode
 * \todo More shade management (e.g. getting, deleting of single shades.)
 */
struct iGradient : public virtual iBase
{
  SCF_INTERFACE(iGradient, 0, 0, 1);
  
  //@{
  /// Add a shade
  virtual void AddShade (const csGradientShade& shade) = 0;
  virtual void AddShade (const csColor4& color, float position) = 0;
  virtual void AddShade (const csColor4& left, const csColor4& right, 
    float position) = 0;
  //@}
  
  /// Clear all shades
  virtual void Clear () = 0;
  
  /**
   * Interpolate the colors over a part of the gradient.
   * \param pal Array of csRGBcolor the gradient should be rendered to.
   * \param count Number of \p palette entries to render.
   * \param begin Start position. Can be anywhere in the gradient.
   * \param end End position. Can be anywhere in the gradient.
   *
   * \remark At least 1 shade has to be present in the gradient to
   *  have this function succeed.
   * \remark Makes heavy use of floating point calculations, so you
   *  might want to use this function in a precalc phase.
   * \remark \p begin doesn't have to be smaller than \p end.
   * \remark \p begin and \p end can both lie completely 'outside'
   *  the gradient (i.e. both smaller/large than the first resp. last
   *  shade's position.)
   */
  virtual bool Render (csRGBcolor* pal, size_t count, float begin = 0.0f, 
    float end = 1.0f) const = 0;
  
  /**
   * Interpolate the colors over a part of the gradient.
   * \param pal Array of csRGBpixel the gradient should be rendered to.
   * \param count Number of \p palette entries to render.
   * \param begin Start position. Can be anywhere in the gradient.
   * \param end End position. Can be anywhere in the gradient.
   *
   * \remark At least 1 shade has to be present in the gradient to
   *  have this function succeed.
   * \remark Makes heavy use of floating point calculations, so you
   *  might want to use this function in a precalc phase.
   * \remark \p begin doesn't have to be smaller than \p end.
   * \remark \p begin and \p end can both lie completely 'outside'
   *  the gradient (i.e. both smaller/large than the first resp. last
   *  shade's position.)
   */  
  virtual bool Render (csRGBpixel* pal, size_t count, float begin = 0.0f, 
    float end = 1.0f) const = 0;

  /// Get the array of shades
  virtual csPtr<iGradientShades> GetShades () = 0;
};


#endif // __CS_IVARIA_GRADIENT_H__
