/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __IMATER_H__
#define __IMATER_H__

#include "csutil/scf.h"

struct iTextureHandle;

SCF_VERSION (iMaterial, 0, 0, 1);

/**
 * This class represents a material as seen from the engine
 * view. You need to register this to the texture manager to get
 * a handle to an internal compiled material.
 * This interface is a stub for later implementation. Currently
 * the only function is to get a texture from it.
 */
struct iMaterial : public iBase
{
  /**
   * Get a texture from the material (stub).
   */
  virtual iTextureHandle* GetTexture () = 0;
};

SCF_VERSION (iMaterialHandle, 0, 0, 1);

/**
 * This class represents a material handle (compiled material)
 * for the 3D rasterizer.
 * This interface is a stub for later implementation. Currently
 * the only function is to get a texture from it.
 */
struct iMaterialHandle : public iBase
{
  /**
   * Get a texture from the material (stub).
   */
  virtual iTextureHandle* GetTexture () = 0;
};

#endif // __IMATER_H__
