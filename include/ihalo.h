/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles

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

#ifndef __IHALO_H__
#define __IHALO_H__

#include "csutil/scf.h"

class csVector2;
class csVector3;

/// This is a handle to a halo
typedef void *csHaloHandle;

SCF_VERSION (iHaloRasterizer, 1, 0, 1);

/// iHaloRasterizer: used to render halos (aka "light globes").
struct iHaloRasterizer : public iBase
{
  /**
   * Create a halo of the specified color and return a handle.
   * The iFactor parameter specifies the light fading speed; the bigger
   * is it (normal range is 0..1) brighter is halo.
   * the iCross parameter is an 0..1 value that shows how much the
   * halo ressembles a cross (0) or an ideal circle (1).
   */
  virtual csHaloHandle CreateHalo (float iR, float iG, float iB, float iFactor,
    float iCross) = 0;
  /// Destroy the halo
  virtual void DestroyHalo (csHaloHandle iHalo) = 0;
  /// Test to see if a halo would be visible (but don't attempt to draw it)
  virtual bool TestHalo (csVector3 *iCenter) = 0;

  /**
   * Draw the halo given a center point and an intensity. 
   * The function returns "true" if the halo *center* is still visible.
   * If the function will return "false", the halo will slowly fade away
   * and will be destroyed when the intensity will reach zero.
   */
  virtual bool DrawHalo (csVector3 *iCenter, float iIntensity, csHaloHandle iHalo) = 0;

  /// Set up the 2D clipping polygon for DrawHalo and TestHalo.
  virtual void SetHaloClipper (csVector2 *iClipper, int iCount) = 0;
};

#endif
