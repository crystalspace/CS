/*
  Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein

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
#include "plugins/engine/3d/impprctx.h"
#include "csgeom/box.h"

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
  csBox2   screen_rect;      // Rectangle occupied by imposter on screen
  csBox3   camera_box;       // Bounding box of object being impostered

public:
  csImposterMesh (csMeshWrapper *parent);

  float CalcIncidenceAngleDist (iRenderView *rview);
  bool CheckIncidenceAngle (iRenderView *rview,float tolerance);
  void FindImposterRectangle (const iCamera *camera);
  void Draw (iRenderView *rview);

  bool GetImposterReady () { return ready; }
  void SetImposterReady (bool r) { ready=r; }

  void SetIncidenceDist (float d) { incidence_dist=d; }
  float GetIncidenceDist () { return incidence_dist;  }

  csMeshWrapper *GetParent () { return parent_mesh; }
};

#endif // __CS_IMPMESH_H__
