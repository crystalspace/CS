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

#include "cscom/com.h"

extern const IID IID_IHaloRasterizer;

class csVector3;
typedef void* HALOINFO;

/// IHaloRasterizer: used to render halos (aka "light globes").
interface IHaloRasterizer : public IUnknown
{
  /// Create a halo of the specified color. This must be destroyed using DestroyHalo.
  STDMETHOD (CreateHalo) (float r, float g, float b, HALOINFO* pRetVal) PURE;
  /// Destroy the halo.
  STDMETHOD (DestroyHalo) (HALOINFO haloInfo) PURE;
  /// Test to see if a halo would be visible (but don't attempt to draw it)
  STDMETHOD (TestHalo) (csVector3* pCenter) PURE;

  /// Draw the halo given a center point and an intensity. 
  STDMETHOD (DrawHalo) (csVector3* pCenter, float fIntensity, HALOINFO haloInfo) PURE;
};

#endif
  
