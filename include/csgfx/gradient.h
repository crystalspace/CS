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

#ifndef __CS_CSGFX_GRADIENT_H__
#define __CS_CSGFX_GRADIENT_H__

/**\file
 * Simple color gradient implementation
 */

#include "csextern.h"

#include "ivaria/gradient.h"

#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "csutil/scf_implementation.h"
#include "csutil/scfarray.h"
#include "csgfx/rgbpixel.h"

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
 * For some examples see the iGradient documentation.
 */
class CS_CRYSTALSPACE_EXPORT csGradient :
  public scfImplementation1<csGradient, iGradient>
{
protected:
  /// The entries in this gradient.
  csArray<csGradientShade> shades;
  struct scfGradientShadesArray : public scfArrayWrapConst<iGradientShades,
    csArray<csGradientShade> >
  {
    scfGradientShadesArray (csGradient* parent) : 
      scfArrayWrapConst<iGradientShades, csArray<csGradientShade> > (
      	parent->shades, parent) {}
  };
public:
  /// Construct an empty gradient.
  csGradient ();
  /// Construct with \p first at position 0 and \p last at 1.
  csGradient (csColor4 first, csColor4 last);
  
  /// Add a shade
  //@{
  /// Add a shade
  void AddShade (const csGradientShade& shade);
  void AddShade (const csColor4& color, float position);
  void AddShade (const csColor4& left, const csColor4& right, 
    float position);
  //@}
  
  /// Clear all shades
  void Clear ();
  
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
  bool Render (csRGBcolor* pal, size_t count, float begin = 0.0f, 
    float end = 1.0f) const;
  
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
  bool Render (csRGBpixel* pal, size_t count, float begin = 0.0f, 
    float end = 1.0f) const;

  /// Get the array of shades
  csPtr<iGradientShades> GetShades ();
};


#endif // __CS_CSGFX_GRADIENT_H__
