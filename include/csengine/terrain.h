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
#include "csengine/rview.h"
#include "csobject/csobject.h"

class ddgTBinMesh;
class ddgTBinTree;
class ddgHeightMap;
class ddgBBox;
class ddgVBuffer;
class csTextureHandle;
class csVector3;

class ddgColor3
{
public:
  /// Data
  unsigned char v[3];
  /// Set values
  void set( ddgColor3 *c ) { v[0] = c->v[0]; v[1] = c->v[1]; v[2] = c->v[2]; }
  /// Set values
  void set( ddgColor3 c ) { v[0] = c.v[0]; v[1] = c.v[1]; v[2] = c.v[2]; }
  /// Set values
  void set( unsigned char r, unsigned char g, unsigned char b ) { v[0] = r; v[1] = g; v[2] = b; }
};

/**
 * This object encapsulates a terrain surface so that it
 * can be used in a csSector.
 */
class csTerrain : public csObject
{
private:
  ///
  ddgTBinMesh* mesh;
  ///
  ddgHeightMap* height;
  ///
  ddgBBox* clipbox;
  ///
  ddgVBuffer *vbuf;
  /// Terrain handle.
  csTextureHandle *_textureMap;
  /// World to camera transformation matrix.
  double wtoc[16];
  /// Colours used at various altitudes.
  ddgColor3 _cliff, _beach, _grass, _trees, _rock, _snow;
  /// Angle/Altitude at which color takes effect.
  float	_cliffangle, _beachalt, _grassalt, _treealt, _rockalt, _snowalt;
  /// Texture scale factor.
  float _texturescale;
  /// Terrains location offset in world space.
  csVector3	_pos;
  /// Terrains size in world space.
  csVector3 _size;
public:
  /**
   * Create an empty terrain.
   */
  csTerrain ();

  /// Destructor.
  virtual ~csTerrain ();

  ///
  ddgTBinMesh* GetMesh () { return mesh; }
  ///
  ddgHeightMap* GetHeightMap () { return height; }
  ///
  ddgBBox* GetBBox () { return clipbox; }

  /// Load the heightmap.
  bool Initialize (const void* heightmap, unsigned long size);

  /**
   * Draw this terrain given a view and transformation.
   */
  void Draw (csRenderView& rview, bool use_z_buf = true);

  /// Set the texture for this surface.
  void SetTexture (csTextureHandle *texture) { _textureMap = texture; }
  /// Set the amount of triangles
  void SetDetail(unsigned int detail);
  /// Choose a color for a vertex.
  void classify( csVector3 *p, ddgColor3 *c);
  /// Put a triangle into the vertex buffer.
  bool PushTriangle(ddgTBinTree *bt, unsigned int tvc, ddgVBuffer *vbuf);
  /**
   * If current transformation puts us below the terrain at the given x,z location
   * then we have hit the terrain.  We adjust position to be at level of terrain
   * and return 1.  If we are above the terrain we return 0.
   */
  int CollisionDetect( csTransform *p );

  CSOBJTYPE;
};

#endif // __CS_TERRAIN_H__
