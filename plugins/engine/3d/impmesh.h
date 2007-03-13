/*
  Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein
  Rewritten during Sommer of Code 2006 by Christoph "Fossi" Mewes

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
#ifndef __CS_IMPMESH_H__
#define __CS_IMPMESH_H__

#include "csutil/nobjvec.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "csgeom/poly3d.h"
#include "csgeom/box.h"
#include "ivideo/rendermesh.h"
#include "cstool/rendermeshholder.h"
#include "iengine/mesh.h"

#include "impprctx.h"

class csVector3;
class csMatrix3;
class csMovable;
class csMeshWrapper;
class csMeshFactoryWrapper;
class csImposterMesh;
struct iRenderView;
struct iCamera;

/**
 * Class representing the factory/imposter relation.
 */
class csImposterFactory
{
private:
  csImposterMesh* imposter_mesh;	// @@@ Temporary.
  csMeshFactoryWrapper* meshfact;

public:
  csImposterFactory (csMeshFactoryWrapper* meshfact)
    : imposter_mesh (0), meshfact (meshfact) { }

  /**
   * Get a valid imposter mesh for a given mesh.
   * If possible this will reuse the previous imposter mesh if there is
   * one and if it is still usable.
   */
  csImposterMesh* GetImposterMesh (csMeshWrapper* mesh,
      csImposterMesh* old_imposter_mesh, iRenderView* rview);
};

/**
 * Class representing the mesh/imposter relation.
 */
class csImposterMesh
{
private:
  csMeshWrapper *parent_mesh;// Who is this an imposter for.
  csImposterProcTex *tex;    // Texture which is drawn on rect
  csPoly3D cutout;           // Rect for cardboard cutout version
  bool     ready;            // Whether texture must be redrawn

  //saved values for update checking
  csVector3 meshLocalDir;
  csVector3 cameraLocalDir;

  //screen bounding box helper
  csScreenBoxResult res;

  //rendermeshholder for this mesh
  csRenderMeshHolder rmHolder;

  //flag that indices have been updated
  bool dirty;

  //current height and width of the billboard
  float height, width;

  //imposter material
  iMaterialWrapper *impostermat;

  //direction the imposter is facing in world coordinates
  csVector3 imposterDir;

  //convenience shortcut
  csEngine *engine;

  // Get the current rotation vectors.
  void GetLocalViewVectors (iCamera *cam);

  void FindImposterRectangle (iCamera *camera);
  void SetImposterReady (bool r, iRenderView* rview);

  friend class csImposterProcTex;

public:
  csImposterMesh (csEngine* engine, csMeshWrapper *parent);
  ~csImposterMesh ();

  bool CheckUpdateNeeded (iRenderView *rview, float tolerance,
      	float camtolerance);

  csMeshWrapper* GetParentMesh () const { return parent_mesh; }

  //returns the imposter billboard
  csRenderMesh** GetRenderMesh (iRenderView *rview);

  bool GetImposterReady (iRenderView* rview);
};

#endif // __CS_IMPMESH_H__
