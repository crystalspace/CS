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
struct iRenderView;
struct iCamera;

class csImposterMesh
{
private:
  csMeshWrapper *parent_mesh;// Who is this an imposter for.
  csImposterProcTex *tex;    // Texture which is drawn on rect
  csPoly3D cutout;           // Rect for cardboard cutout version
  bool     ready;            // Whether texture must be redrawn
  float    incidence_dist;   // Angle of incidence to camera last time rendered

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

  float CalcIncidenceAngleDist (iCamera *camera);
  void FindImposterRectangle (iCamera *camera);
  void SetImposterReady (bool r);

  friend class csImposterProcTex;

public:
  csImposterMesh (csEngine* engine, csMeshWrapper *parent);
  ~csImposterMesh ();

  bool CheckIncidenceAngle (iRenderView *rview, float tolerance);

  //returns the imposter billboard
  csRenderMesh** GetRenderMesh (iRenderView *rview);

  bool GetImposterReady () { return ready; }

  void SetIncidenceDist (float d) { incidence_dist=d; }
  float GetIncidenceDist () { return incidence_dist;  }
};

#endif // __CS_IMPMESH_H__
