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

#ifndef __CS_DDGTERRAIN_H__
#define __CS_DDGTERRAIN_H__

#include "csgeom/transfrm.h"
#include "csengine/terrain.h"
#include "csobject/csobject.h"
#include "csutil/cscolor.h"

class ddgTBinMesh;
class ddgTBinTree;
class ddgHeightMap;
class ddgVArray;
class ddgContext;
class csMaterialWrapper;
class csVector3;
typedef int ddgVBIndex;

/**
 * This object encapsulates a DDG terrain surface.
 */
class csDDGTerrain : public csTerrain
{
private:
  ///
  ddgTBinMesh* mesh;
  ///
  ddgHeightMap* heightMap;
  ///
  ddgVArray *vbuf;
  ///
  ddgContext *context;
  /// Terrain handle.
  csMaterialWrapper **_materialMap;
  /// Texture scale factor.
  float _texturescale;
  /// Terrain's location offset in world space.
  csVector3 _pos;
  /// Terrain's size in world space.
  csVector3 _size;

  /**
   * A direction vector for a directional light hitting
   * the terrain.
   */
  csVector3 dirlight;
  /// Color for the directional light.
  csColor dircolor;
  /// If true then a directional light is used.
  bool do_dirlight;

public:
  /**
   * Create an empty terrain.
   */
  csDDGTerrain ();

  /// Destructor.
  virtual ~csDDGTerrain ();

  /// Set a directional light.
  virtual void SetDirLight (const csVector3& dirl, const csColor& dirc);
  /// Disable directional light.
  virtual void DisableDirLight () { do_dirlight = false; }

  ///
  ddgContext* GetContext () { return context; }
  ///
  ddgTBinMesh* GetMesh () { return mesh; }
  ///
  ddgHeightMap* GetHeightMap () { return heightMap; }

  /// Load the heightmap.
  virtual bool Initialize (const void* heightMapFile, unsigned long size);

  /**
   * Draw this terrain given a view and transformation.
   */
  virtual void Draw (iRenderView* rview, bool use_z_buf = true);

  /// Set a material for this surface.
  virtual void SetMaterial (int i, csMaterialWrapper *material)
  {
    _materialMap[i] = material;
  }
  /// Get the number of materials required.
  virtual int GetNumMaterials ();
  /// Set the amount of triangles
  virtual void SetDetail (unsigned int detail);
  /// Put a triangle into the vertex buffer.
  bool drawTriangle (ddgTBinTree *bt, ddgVBIndex tvc, ddgVArray *vbuf);

  /**
   * If current transformation puts us below the terrain at the given
   * x,z location then we have hit the terrain.  We adjust position to
   * be at level of terrain and return 1.  If we are above the terrain we
   * return 0.
   */
  virtual int CollisionDetect (csTransform *p);

  CSOBJTYPE;
};

#endif // __CS_DDGTERRAIN_H__
