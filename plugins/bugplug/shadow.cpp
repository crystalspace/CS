/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "shadow.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/movable.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"

IMPLEMENT_IBASE (csShadow)
  IMPLEMENTS_INTERFACE (iMeshObject)
IMPLEMENT_IBASE_END

csShadow::csShadow ()
{
  CONSTRUCT_IBASE (NULL);
  wrap = NULL;
  shadow_mesh = NULL;
  do_bbox = true;
  do_rad = true;
}

csShadow::~csShadow ()
{
  CS_ASSERT (wrap == NULL);
}

bool csShadow::DrawTest (iRenderView* rview, iMovable*)
{
  if (!shadow_mesh) return false;
  // See if we are in the same sector as the mesh.
  iMovable* shadow_movable = shadow_mesh->GetMovable ();
  iSector* sector = rview->GetCamera ()->GetSector ();
  int i;
  for (i = 0 ; i < shadow_movable->GetSectorCount () ; i++)
  {
    iSector* sec = shadow_movable->GetSector (i);
    if (sec == sector)
    {
      return true;
    }
  }
  return false;
}

bool csShadow::Draw (iRenderView* rview, iMovable*, csZBufMode)
{
  if (!shadow_mesh) return false;
  iMovable* shadow_movable = shadow_mesh->GetMovable ();

  iGraphics3D* G3D = rview->GetGraphics3D ();
  G3D->BeginDraw (CSDRAW_2DGRAPHICS);

  csReversibleTransform tr_o2c = rview->GetCamera ()->GetTransform ()
    	* shadow_movable->GetFullTransform ().GetInverse ();
  float fov = G3D->GetPerspectiveAspect ();
  if (do_bbox)
  {
    int bbox_color = G3D->GetTextureManager ()->FindRGB (0, 255, 255);
    csBox3 bbox;
    shadow_mesh->GetMeshObject ()->GetObjectBoundingBox (bbox);
    csVector3 vxyz = tr_o2c * bbox.GetCorner (BOX_CORNER_xyz);
    csVector3 vXyz = tr_o2c * bbox.GetCorner (BOX_CORNER_Xyz);
    csVector3 vxYz = tr_o2c * bbox.GetCorner (BOX_CORNER_xYz);
    csVector3 vxyZ = tr_o2c * bbox.GetCorner (BOX_CORNER_xyZ);
    csVector3 vXYz = tr_o2c * bbox.GetCorner (BOX_CORNER_XYz);
    csVector3 vXyZ = tr_o2c * bbox.GetCorner (BOX_CORNER_XyZ);
    csVector3 vxYZ = tr_o2c * bbox.GetCorner (BOX_CORNER_xYZ);
    csVector3 vXYZ = tr_o2c * bbox.GetCorner (BOX_CORNER_XYZ);
    G3D->DrawLine (vxyz, vXyz, fov, bbox_color);
    G3D->DrawLine (vXyz, vXYz, fov, bbox_color);
    G3D->DrawLine (vXYz, vxYz, fov, bbox_color);
    G3D->DrawLine (vxYz, vxyz, fov, bbox_color);
    G3D->DrawLine (vxyZ, vXyZ, fov, bbox_color);
    G3D->DrawLine (vXyZ, vXYZ, fov, bbox_color);
    G3D->DrawLine (vXYZ, vxYZ, fov, bbox_color);
    G3D->DrawLine (vxYZ, vxyZ, fov, bbox_color);
    G3D->DrawLine (vxyz, vxyZ, fov, bbox_color);
    G3D->DrawLine (vxYz, vxYZ, fov, bbox_color);
    G3D->DrawLine (vXyz, vXyZ, fov, bbox_color);
    G3D->DrawLine (vXYz, vXYZ, fov, bbox_color);
  }
  if (do_rad)
  {
    int rad_color = G3D->GetTextureManager ()->FindRGB (0, 255, 0);
    csVector3 radius, r;
    radius = shadow_mesh->GetMeshObject ()->GetRadius ();
    csVector3 trans_o = tr_o2c * csVector3 (0);
    r.Set (radius.x, 0, 0);
    G3D->DrawLine (trans_o-r, trans_o+r, fov, rad_color);
    r.Set (0, radius.y, 0);
    G3D->DrawLine (trans_o-r, trans_o+r, fov, rad_color);
    r.Set (0, 0, radius.z);
    G3D->DrawLine (trans_o-r, trans_o+r, fov, rad_color);
  }

  G3D->BeginDraw (CSDRAW_3DGRAPHICS);
  return true;
}

void csShadow::SetShadowMesh (iMeshWrapper* sh)
{
  shadow_mesh = sh;
}

bool csShadow::AddToEngine (iEngine* engine)
{
  if (wrap) { engine->RemoveMesh (wrap); wrap = NULL; }
  if (engine->GetSectorCount () <= 0) return false;
  wrap = engine->CreateMeshObject (this, "_@Shadow@_");
  iMovable* movable = wrap->GetMovable ();
  int i;
  for (i = 0 ; i < engine->GetSectorCount () ; i++)
  {
    iSector* sec = engine->GetSector (i);
    movable->AddSector (sec);
  }
  movable->UpdateMove ();
  return true;
}

void csShadow::RemoveFromEngine (iEngine* engine)
{
  if (wrap)
  {
    engine->RemoveMesh (wrap);
    wrap = NULL;
  }
}

