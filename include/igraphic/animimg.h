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

#ifndef __CS_IGRAPHIC_ANIMIMG_H__
#define __CS_IGRAPHIC_ANIMIMG_H__

/**\file
 * Animated image interface
 */

/**
 * \addtogroup gfx2d
 * @{
 */

#include "csutil/scf.h"

class csRect;

/**
 * If an image file has animation capabilities, this interface is exhibited.
 */
struct iAnimatedImage : public virtual iBase
{
  SCF_INTERFACE (iAnimatedImage, 2, 0, 0);

  /**
   * Update the image data.
   * \param time Time that passed since the last call to Animate().
   * \param dirtyrect If not 0, the area that has changed is filled in.
   * \return Whether any image data has changed at all.
   */
  virtual bool Animate (csTicks time, csRect* dirtyrect = 0) = 0;

  /**
   * Is this image really animated?
   * E.g. returns false if an animation has just 1 frame.
   */
  virtual bool IsAnimated () = 0;
};

/** @} */

#endif
