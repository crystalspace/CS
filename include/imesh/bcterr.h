/*
    Copyright (C) 2002 by Jorrit Tyberghein and Ryan Surkamp

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

#ifndef __IMESH_BCTERR_H_
#define __IMESH_BCTERR_H_

#include "csutil/scf.h"

struct iEngine;
struct iImage;
struct iMaterialWrapper;
struct iMaterialList;
struct iLoaderContext;
class csVector3;
class csVector2;
class csColor;
class csTransform;
class csSharedLODMesh;
class csBCTerrBlock;
class csBox3;



SCF_VERSION (iBCTerrState, 0, 0, 2);

/**
 * This interface describes the API for the Bezier Curve terrain object.
 */
struct iBCTerrState : public iBase
{
  /// Set Number of Blocks across 2 axis.
  virtual void SetSize (int x, int z) = 0;
  /// Set topleft corner.
  virtual void SetTopLeftCorner (const csVector3& topleft) = 0;
  /// Set Block info.
  virtual void SetBlockMaterial (int x_block, int z_block,
  	iMaterialWrapper* mat) = 0;
  /// Used to create control points, needs size to be set.
  virtual void SetHeightMap (iImage* im) = 0;
  /// Test the height at point, returns y + 2.0f.
  virtual int HeightTest (csVector3 *point) = 0;
  /// Set Block Material by grid number 0 = start 
  virtual void SetBlockMaterialNum (int num, iMaterialWrapper* mat) = 0;
  /// CameraHeightTest should return a point that a camera can use
  virtual int CameraHeightTest (csVector3 *point) = 0;
  /// Test a Bounding Box's path against the terrain
  virtual void CollideBBox (const csBox3 bbox, const csVector3 start,
      csVector3 &end) = 0;
  /// Test a ray against the mesh
  virtual void CollideRay (const csVector3 start, csVector3 &end) = 0;
};

SCF_VERSION (iBCTerrFactoryState, 0, 0, 2);

/**
 * This interface describes the API for the Bezier Curve terrain Factory object.
 */
struct iBCTerrFactoryState : public iBase
{
  /// Bezier Curves have shared edges pre-computed.  Set edge resolution here.
  virtual void SetMaxEdgeResolution (int res) = 0;
  virtual int GetMaxEdgeResolution () = 0;
  /// Set Block Size.
  virtual void SetBlockSize (float x, float z) = 0;

  /// Add LOD info.
  virtual void AddLOD (float distance, int increments) = 0;
  /// Set LOD Distance.
  virtual void SetLODDistance (int lod_level, float distance) = 0;
  /// newheight = startheight + HeightFromHeightMap * multiplier.
  virtual void SetMultiplier (float m ) = 0;
  /// Will return NULL if unavailable CPU time or no free shared meshes.
  virtual csSharedLODMesh* GetSharedMesh (int level, csBCTerrBlock *owner) = 0;
  virtual void FreeShared (csBCTerrBlock *owner, int level ) = 0;
  /// AddTime -> used to free lod in time increments / also cpu limiter.
  virtual void AddTime (csTicks time) = 0;
  virtual csVector2* GetSize () = 0;
  virtual float GetMultiplier () = 0;
  /// Create a shared LODMesh that isn't shared. 
  virtual csSharedLODMesh* CreateFreeMesh (bool wavy) = 0;
  virtual void SetFocusPoint (const csVector3 focus) = 0;
  virtual csVector2* GetLODUV (int lod_level) = 0;
  virtual float* GetLODDistances () = 0;
  virtual void SetDefaultMaterial ( iMaterialWrapper* mat ) = 0;
  virtual iMaterialWrapper* GetDefaultMaterial () = 0;
  /// Set number of LOD levels
  virtual int GetUserLOD () = 0;
  virtual float GetSystemDistance () = 0;
  /// Set distance shared edge usage stops
  virtual void SetSystemDistance (float new_dist) = 0;
};

#endif // __IMESH_BCTERR_H_

