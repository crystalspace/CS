/*
    Copyright (C) 2006 by Jorrit Tyberghein

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
#include "csgeom/math3d.h"
#include "csutil/array.h"
#include "cstool/procmesh.h"
#include <math.h>

#include "iutil/objreg.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/texture.h"
#include "iengine/engine.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "iengine/mesh.h"
#include "iengine/camera.h"
#include "iengine/rendermanager.h"

csMeshOnTexture::csMeshOnTexture (iObjectRegistry* object_reg)
{
  engine = csQueryRegistry<iEngine> (object_reg);
  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  view.AttachNew (new csView (engine, g3d));
  view->SetAutoResize (false);
  view->GetMeshFilter().SetFilterMode(CS::Utility::MESH_FILTER_INCLUDE);
  cur_w = cur_h = -1;
}

csMeshOnTexture::~csMeshOnTexture ()
{
}

void csMeshOnTexture::ScaleCamera (iMeshWrapper* mesh, int txtw, int txth)
{
  UpdateView (txtw, txth);
  csBox3 mesh_box = mesh->GetWorldBoundingBox ();
  csVector3 mesh_center = mesh_box.GetCenter ();
  iPerspectiveCamera* camera = view->GetPerspectiveCamera ();
  float aspect = float (camera->GetFOV ());
  float shift_x = camera->GetShiftX ();
  float shift_y = camera->GetShiftY ();
  int i;
  float maxz = -100000000.0f;
  for (i = 0 ; i < 8 ; i++)
  {
    csVector3 corner = mesh_box.GetCorner (i) - mesh_center;
    float z = (corner.x * aspect) / (1.0f - shift_x);
    if (z < 0) z = (corner.x * aspect) / (float (txtw) - shift_x);
    z += corner.z;
    if (z > maxz) maxz = z;

    z = (corner.y * aspect) / (1.0f - shift_y);
    if (z < 0) z = (corner.y * aspect) / (float (txth) - shift_y);
    z += corner.z;
    if (z > maxz) maxz = z;
  }

  csVector3 cam_pos = mesh_center;
  cam_pos.z -= maxz;
  for (i = 0 ; i < 8 ; i++)
  {
    csVector3 corner = mesh_box.GetCorner (i) - cam_pos;
    csVector2 p = view->GetCamera()->Perspective (corner);
  }

  view->GetCamera()->GetTransform ().Identity ();
  view->GetCamera()->GetTransform ().SetOrigin (cam_pos);
}

void csMeshOnTexture::ScaleCamera (iMeshWrapper* mesh, float distance)
{
  iMovable* movable = mesh->GetMovable ();
  csVector3 mesh_pos = movable->GetFullPosition ();
  iCamera* camera = view->GetCamera ();
  const csVector3& cam_pos = camera->GetTransform ().GetOrigin ();

  camera->GetTransform ().LookAt (mesh_pos-cam_pos, csVector3 (0, 1, 0));

  csVector3 new_cam_pos = mesh_pos + distance * (cam_pos-mesh_pos).Unit ();
  camera->GetTransform ().SetOrigin (new_cam_pos);
}

void csMeshOnTexture::UpdateView (int w, int h)
{
  if (cur_w != w || cur_h != h)
  {
    view->GetCamera ()->SetViewportSize (w, h);
    view->SetRectangle (0, 0, w, h);
    view->UpdateClipper ();
    view->GetPerspectiveCamera ()->SetPerspectiveCenter (0.5f, 0.5f);
    view->GetPerspectiveCamera ()->SetFOV (1, 1);
    cur_w = w;
    cur_h = h;
  }
}

bool csMeshOnTexture::Render (iMeshWrapper* mesh, iTextureHandle* handle,
    bool persistent, int color)
{
  int w, h;
  handle->GetRendererDimensions (w, h);
  UpdateView (w, h);

  view->GetMeshFilter().Clear();
  view->GetMeshFilter().AddFilterMesh(mesh);
  view->GetCamera()->SetSector(mesh->GetMovable()->GetSectors()->Get(0));

  csRef<iRenderManagerTargets> rmTargets = scfQueryInterface<iRenderManagerTargets>(engine->GetRenderManager());
  rmTargets->RegisterRenderTarget(handle, 
                                  view, 
                                  0, 
                                  iRenderManagerTargets::updateOnce 
                                  | ((persistent || color != -1) ? 0 : iRenderManagerTargets::clearScreen));

  ///@TODO Color doesn't work at the moment.
  /// The next line should probably move to SimpleTreeRenderer::RenderContextStack() somewhere.
  //if ((!persistent) && color != -1) g3d->GetDriver2D()->Clear (color);

  rmTargets->MarkAsUsed(handle);

  return true;
}

