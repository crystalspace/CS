/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_ITEXTURE_IFIRE_H__
#define __CS_ITEXTURE_IFIRE_H__

/**\file
 * Interface to the 'fire' procedural texture properties.
 */

/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/scf.h"

class csGradient;

/**
 * Interface to the 'fire' procedural texture.
 * \todo 'GetPalette()' maybe.
 */
struct iFireTexture : public virtual iBase
{
  SCF_INTERFACE(iFireTexture, 2, 0, 0);

  /**
   * Set animation parameter: possible burn (0..)
   * Try possburn 3=wood,90=oil,255=max.
   */
  virtual void SetPossibleBurn (int possburn) = 0;
  /// Get possible burn.
  virtual int GetPossibleBurn() = 0;

  /**
   * Set animation parameter: additional burn (0..)
   * Try addburn=1..5 or so
   */
  virtual void SetAdditionalBurn (int addburn) = 0;
  /// Get additional burn.
  virtual int GetAdditionalBurn() = 0;
  
  /**
   * Set animation parameter: continued burn (0..)
   * Try 80.
   */
  virtual void SetContinuedBurn (int contburn) = 0;
  /// Get continued burn.
  virtual int GetContinuedBurn() = 0;
  
  /**
   * Set animation parameter: smoothing factor (0..)
   * Try 2.
   */
  virtual void SetSmoothing (int smoothing) = 0;
  /// Get smoothing factor.
  virtual int GetSmoothing() = 0;
  
  /**
   * Set animation parameter: burning down param
   * Try 3*256/height.
   */
  virtual void SetExtinguish (int extinguish) = 0;
  /// Get burning down param.
  virtual int GetExtinguish() = 0;
  
  /// Set single flame mode
  virtual void SetSingleFlameMode (bool enable) = 0;
  /// Get single flame mode
  virtual bool GetSingleFlameMode() = 0;
  
  /**
   * Set animation parameter: 1/2 size of flame base, from middle bottom 
   * sideways.
   */
  virtual void SetHalfBase (int halfbase) = 0;
  /// Get 1/2 size of flame base.
  virtual int GetHalfBase() = 0;
  
  /**
   * Set whether to smooth the whole image again after calculating an 
   * iteration.
   * \param amount Size of the square used for averaging. 
   *  0 = disable smoothing.
   */
  virtual void SetPostSmoothing (int amount) = 0;
  /// Get post smoothing value.
  virtual int GetPostSmoothing () = 0;
  
  /**
   * Set the colors used by the flame.
   * Position 0 = darkest areas (background, actually), 
   * position 1 = brightest areas.
   */
  virtual void SetPalette (const csGradient gradient) = 0;
};

/** @} */

#endif
