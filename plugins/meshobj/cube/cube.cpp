/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "plugins/meshobj/cube/cube.h"
#include "imovable.h"
#include "irview.h"
#include "igraph3d.h"
#include "igraph2d.h"
#include "imater.h"
#include "icamera.h"
#include "iclip2.h"

IMPLEMENT_IBASE (csCubeMeshObject)
  IMPLEMENTS_INTERFACE (iMeshObject)
IMPLEMENT_IBASE_END

csCubeMeshObject::csCubeMeshObject (csCubeMeshObjectFactory* factory)
{
  CONSTRUCT_IBASE (NULL);
  csCubeMeshObject::factory = factory;
}

csCubeMeshObject::~csCubeMeshObject ()
{
}

bool csCubeMeshObject::Draw (iRenderView* rview, iMovable* movable)
{
// @@@ TODO:
//     - Far plane clipping (iCamera)
//     - Portal plane clipping
//     - Portal frustum clipping
//     - Frustum clipping
//     - Z fill vs Z use
 
  if (!factory->GetMaterialWrapper ())
  {
    printf ("INTERNAL ERROR: cube used without material!\n");
    return false;
  }
  iMaterialHandle* mat = factory->GetMaterialWrapper ()->GetMaterialHandle ();
  if (!mat)
  {
    printf ("INTERNAL ERROR: cube used without valid material handle!\n");
    return false;
  }
  float size = factory->GetSize ();
  iGraphics3D* g3d = rview->GetGraphics3D ();
  iGraphics2D* g2d = rview->GetGraphics2D ();
  iCamera* camera = rview->GetCamera ();

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csReversibleTransform tr_o2c = camera->GetTransform ()
    	* movable->GetTransform ().GetInverse ();
  g3d->SetObjectToCamera (&tr_o2c);
  g3d->SetClipper (rview->GetClipper ()->GetClipPoly (), rview->GetClipper ()->GetNumVertices ());
  // @@@ This should only be done when aspect changes...
  float fov = camera->GetFOV ();
  g3d->SetPerspectiveAspect (fov);
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);

  float s = size/2;
  csVector3 xyz (-s, -s, -s);
  csVector3 Xyz ( s, -s, -s);
  csVector3 xYz (-s,  s, -s);
  csVector3 XYz ( s,  s, -s);
  csVector3 xyZ (-s, -s,  s);
  csVector3 XyZ ( s, -s,  s);
  csVector3 xYZ (-s,  s,  s);
  csVector3 XYZ ( s,  s,  s);
  xyz = tr_o2c * xyz;
  Xyz = tr_o2c * Xyz;
  xYz = tr_o2c * xYz;
  XYz = tr_o2c * XYz;
  xyZ = tr_o2c * xyZ;
  XyZ = tr_o2c * XyZ;
  xYZ = tr_o2c * xYZ;
  XYZ = tr_o2c * XYZ;

  g3d->DrawLine (xyz, Xyz, fov, 0xffff);
  g3d->DrawLine (Xyz, XYz, fov, 0xffff);
  g3d->DrawLine (XYz, xYz, fov, 0xffff);
  g3d->DrawLine (xYz, xyz, fov, 0xffff);

  g3d->DrawLine (xyZ, XyZ, fov, 0xffff);
  g3d->DrawLine (XyZ, XYZ, fov, 0xffff);
  g3d->DrawLine (XYZ, xYZ, fov, 0xffff);
  g3d->DrawLine (xYZ, xyZ, fov, 0xffff);

  g3d->DrawLine (xyz, xyZ, fov, 0xffff);
  g3d->DrawLine (Xyz, XyZ, fov, 0xffff);
  g3d->DrawLine (xYz, xYZ, fov, 0xffff);
  g3d->DrawLine (XYz, XYZ, fov, 0xffff);

  return true;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csCubeMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_EMBEDDED_INTERFACE (iCubeMeshObject)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csCubeMeshObjectFactory::CubeMeshObject)
  IMPLEMENTS_INTERFACE (iCubeMeshObject)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_FACTORY (csCubeMeshObjectFactory)

EXPORT_CLASS_TABLE (cube)
  EXPORT_CLASS (csCubeMeshObjectFactory, "crystalspace.meshobj.cube",
    "Crystal Space Cube Mesh Object")
EXPORT_CLASS_TABLE_END

csCubeMeshObjectFactory::csCubeMeshObjectFactory (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
  CONSTRUCT_EMBEDDED_IBASE (scfiCubeMeshObject);
  size = 1;
  material = NULL;
}

csCubeMeshObjectFactory::~csCubeMeshObjectFactory ()
{
}

bool csCubeMeshObjectFactory::Initialize (iSystem*)
{
  return true;
}

iMeshObject* csCubeMeshObjectFactory::NewInstance ()
{
  return new csCubeMeshObject (this);
}

