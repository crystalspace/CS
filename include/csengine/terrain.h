/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef TERRAIN_H
#define TERRAIN_H

#include "csgeom/transfrm.h"
#include "csengine/rview.h"
#include "csobject/csobject.h"
class csTextureHandle;

class ddgTBinMesh;
class ddgHeightMap;
class ddgBBox;
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
  /// Terrain handle.
  csTextureHandle *_textureMap;
  /// World to camera transformation matrix.
  double wtoc[16];
  /// Colours used at various altitudes.
  ddgColor3 _cliff, _beach, _grass, _trees, _rock, _snow;
  /// Angle/Altitude at which color takes effect.
  float	_cliffangle, _beachalt, _grassalt, _treealt, _rockalt, _snowalt;

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
  bool Initialize (char* heightmap);

  /**
   * Draw this terrain given a view and transformation.
   */
  void Draw (csRenderView& rview, bool use_z_buf = true);

  /// Set the texture for this surface.
  void SetTexture (csTextureHandle *texture) { _textureMap = texture; }
  /// Choose a color for a vertex.
  void classify( csVector3 *p, ddgColor3 *c);

  CSOBJTYPE;
};

#endif /*TERRAIN_H*/

