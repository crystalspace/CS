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

#ifndef __CS_LODTERRAIN_H__
#define __CS_LODTERRAIN_H__

#include "csgeom/transfrm.h"
#include "csobject/csobject.h"
#include "csengine/terrain.h"
#include "igraph3d.h"
#include "csutil/cscolor.h"

class csMaterialWrapper;
struct iRenderView;

/**
 * This object encapsulates a terrain surface so that it
 * can be used in a csSector. This class uses the LOD method.
 */
class csLODTerrain : public csTerrain
{
private:
// this array holds the Quadtree
    char  **quadTree;
// HeightMap
    float **heightMap;
// array of the terrain rouphness error
// this table is needed to do d2-propagtion
// (don't know if we need this or not?? it's a lot faster without, but doesn't
// take care of terrain rouphness).
    float **d2Table;

// dimwnsion of the heightfield    
    int  dim;

// only one material supported by now
    csMaterialWrapper *material;
// position of the camera
    csVector3 position;

// this constant is the minimum global resolution    
    int lod_C;
// this is the desired global resolution
// this can be changed dynamically to change framerate
    float lod_c;
// constant for d2 propagation
    float K;
// terrain height scale
    int scale;

// 3d mesh    
    G3DTriangleMesh *g3dmesh;
// the vertices, texels and triangles of the mesh
    csVector3 *vertices;
    csVector2 *texels;
    csTriangle *triangles;

// main calculation functions    
    void resetQuadTree(void);
    void setupQuadTree(int x, int z, int width);
    float setupd2Table(int x, int y, int width);

// and the draw functions    
    void draw(int x, int z, int width);
    void drawTriangle(int x1, int z1, int x2, int z2, int x3, int z3);
    
public:
  /// Constructor
  csLODTerrain();
  
  /// Destructor
  virtual ~csLODTerrain();

  /// Set a directional light.
  virtual void SetDirLight (const csVector3& /*dirl*/, const csColor& /*dirc*/) { }
  /// Disable directional light.
  virtual void DisableDirLight () { }

  /// Loads and initializes the heightmap
  virtual bool Initialize (const void *heightMapFile, unsigned long size);
  /// main draw function
  virtual void Draw (iRenderView* rview, bool use_z_buf);
  /// puts us on top of terrain (should be changed)
  virtual int CollisionDetect (csTransform *p);
  
  /// set the meterial of the terrain
  virtual void SetMaterial (int /*i*/, csMaterialWrapper *mat) { material = mat; }
  /// Get the number of materials required/supported.
  virtual int GetNumMaterials () { return 1; }
  /// Set the c constant (higher=more triangles, lower=less)
  void SetLODLevel (float l) { if(l!=0) lod_c = l; }
  /// Set the amount of triangles
  virtual void SetDetail (unsigned int /*detail*/) { } //@@@?
  /// Scale of the terrain height
  void SetScale (int s) { scale = s; }
  /// Number of triangles drawn
  int GetTNum (void) { return g3dmesh->num_triangles; }
  
  CSOBJTYPE;
};

#endif // __CS_LODTERRAIN_H__

