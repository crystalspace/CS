/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
  
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

#ifndef __CS_TERRAIN_H__
#define __CS_TERRAIN_H__

#include "csgeom/transfrm.h"
#include "csobject/csobject.h"
#include "csutil/cscolor.h"

class csMaterialWrapper;
class csVector3;
struct iRenderView;

/**
 * This object encapsulates a terrain surface so that it
 * can be used in a csSector. It is an abstract class. Specific
 * implementations of this need to be made.
 */
class csTerrain : public csObject
{
public:
  /**
   * Create an empty terrain.
   */
  csTerrain ();

  /// Destructor.
  virtual ~csTerrain ();

  /// Set a directional light.
  virtual void SetDirLight (const csVector3& dirl, const csColor& dirc) = 0;
  /// Disable directional light.
  virtual void DisableDirLight () = 0;

  /// Load the heightmap.
  virtual bool Initialize (const void* heightMapFile, unsigned long size) = 0;

  /**
   * Draw this terrain given a view and transformation.
   */
  virtual void Draw (iRenderView* rview, bool use_z_buf = true) = 0;

  /// Set a material for this surface.
  virtual void SetMaterial (int i, csMaterialWrapper *material) = 0;
  /// Get the number of materials required/supported.
  virtual int GetNumMaterials () = 0;
  /// Set the amount of triangles
  virtual void SetDetail (unsigned int detail) = 0;

  /**
   * If current transformation puts us below the terrain at the given
   * x,z location then we have hit the terrain.  We adjust position to
   * be at level of terrain and return 1.  If we are above the terrain we
   * return 0.
   */
  virtual int CollisionDetect (csTransform *p) = 0;

  CSOBJTYPE;
};

#endif // __CS_TERRAIN_H__
