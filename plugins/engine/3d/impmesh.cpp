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

#include "cssysdef.h"
#include "csqsqrt.h"
#include "plugins/engine/3d/impmesh.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/engine.h"
#include "iengine/portal.h"
#include "csutil/debug.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"

csImposterMesh::csImposterMesh (csMeshWrapper *parent)
{
  parent_mesh = parent;
  tex = new csImposterProcTex (this);
  ready	= false;
  incidence_dist = 0;
}

float csImposterMesh::CalcIncidenceAngleDist (iRenderView *rview)
{
  // Jorrit, not sure about this correctness but it compiles at least. -Keith

  // Calculate angle of incidence vs. the camera
  iCamera* camera = rview->GetCamera ();
  csReversibleTransform obj = (parent_mesh->GetCsMovable()).csMovable::GetTransform ();
  csReversibleTransform cam = camera->GetTransform ();
  csReversibleTransform seg = obj / cam;  // Matrix Math Magic!
  csVector3 straight(0,0,1);
  csVector3 pt = seg * straight;
  return csSquaredDist::PointPoint (straight, pt);
}

bool csImposterMesh::CheckIncidenceAngle (iRenderView *rview, float tolerance)
{
  float const dist2 = CalcIncidenceAngleDist(rview);
  float diff = dist2 - incidence_dist;
  if (diff < 0) diff = -diff;

  // If not ok, mark for redraw of imposter
  if (diff > tolerance)
  {
    SetImposterReady (false);
    return false;
  }
  return true;
}

void csImposterMesh::FindImposterRectangle (const iCamera *camera)
{
  // Called from csImposterProcTex during Anim.
  //  (Recalc of texture causes recalc of imposter poly also.)

  // Rotate camera to look at object directly.
  // Get screen bounding box, modified to also return depth of
  //  point of max width or height in the box.
  // Rotate camera back to original lookat
  // Project screen bounding box, at the returned depth to
  //  the camera transform to rotate it around where we need it
  // Save as csPoly3d for later rendering
#if 0
  parent_mesh->GetScreenBoundingBox (camera,sbox,cbox);
#endif
}

void csImposterMesh::Draw (iRenderView *rview)
{
#if 0  
  iGraphics3D *g3d = rview->GetG3D();
  G3DPolygonDP poly;
  memset (&poly, 0, sizeof(poly));

  csMatrix3 m_cam2tex;
  csVector3 v_cam2tex;

  poly.mixmode = CS_FX_COPY;
  poly.use_fog = false;
  poly.do_fullbright = false;
  poly.plane.m_cam2tex = &m_cam2tex;
  poly.plane.v_cam2tex = &v_cam2tex;

  //  poly.mat_handle = tex->GetMaterial ();
  poly.flat_color_r = 128;
  poly.flat_color_g = 128;
  poly.flat_color_b = 128;

  m_cam2tex = pol.m_obj2tex * o2c.GetT2O ();
  v_cam2tex = o2c.Other2This (pol.v_obj2tex);

  poly.poly_texture = 0; // pol.poly_texture; when imposter is rendered correctly

  // project screen bounding box into poly vertex list here

  // ok now actually draw it
  g3d->DrawPolygon (poly);
#endif
}
