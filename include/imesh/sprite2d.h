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

#ifndef __IMESH_SPRITE2D_H__
#define __IMESH_SPRITE2D_H__

#include "csutil/scf.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"

struct iMaterialWrapper;

struct csSprite2DVertex
{
  csVector2 pos;
  csColor color_init;
  csColor color;
  float u, v;
};

TYPEDEF_GROWING_ARRAY (csColoredVertices, csSprite2DVertex);

SCF_VERSION (iSprite2DFactoryState, 0, 0, 1);

/**
 * This interface describes the API for the sprite factory mesh object.
 */
struct iSprite2DFactoryState : public iBase
{
  /// Set material of sprite.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of sprite.
  virtual iMaterialWrapper* GetMaterialWrapper () = 0;
  /// Set mix mode.
  virtual void SetMixMode (UInt mode) = 0;
  /// Get mix mode.
  virtual UInt GetMixMode () = 0;

  /**
   * Set true if this sprite needs lighting (default).
   * Otherwise the given colors are used.
   * If lighting is disabled then the color_init array
   * is copied to the color array.
   */
  virtual void SetLighting (bool l) = 0;

  /// Return the value of the lighting flag.
  virtual bool HasLighting () = 0;
};

SCF_VERSION (iSprite2DState, 0, 0, 1);

/**
 * This interface describes the API for the sprite factory mesh object.
 * iSprite2DState inherits from iSprite2DFactoryState.
 */
struct iSprite2DState : public iSprite2DFactoryState
{
  /// Get the vertex array.
  virtual csColoredVertices& GetVertices () = 0;
  /** 
   * Set vertices to form a regular n-polygon around (0,0),
   * optionally also set u,v to corresponding coordinates in a texture.
   * Large n approximates a circle with radius 1. n must be > 2. 
   */
  virtual void CreateRegularVertices (int n, bool setuv) = 0;
};

#endif

