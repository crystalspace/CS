/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "csgeom/box.h"
#include "nullmesh.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "igeom/clip2d.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iutil/objreg.h"
#include "qsqrt.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csNullmeshMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iNullMeshState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullmeshMeshObject::MeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullmeshMeshObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullmeshMeshObject::NullMeshState)
  SCF_IMPLEMENTS_INTERFACE (iNullMeshState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csNullmeshMeshObject::csNullmeshMeshObject (iMeshObjectFactory* factory)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshObjectFactory);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiNullMeshState);
  csNullmeshMeshObject::factory = factory;
  logparent = NULL;
  vis_cb = NULL;
  shapenr = 0;
  radius = .1;
}

csNullmeshMeshObject::~csNullmeshMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
}

bool csNullmeshMeshObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  iCamera* camera = rview->GetCamera ();

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csReversibleTransform tr_o2c = camera->GetTransform () / movable->GetFullTransform ();

  csSphere sphere;
  sphere.SetCenter (csVector3 (0, 0, 0));
  sphere.SetRadius (radius);
  int clip_portal, clip_plane, clip_z_plane;
  if (rview->ClipBSphere (tr_o2c, sphere, clip_portal, clip_plane,
  	clip_z_plane) == false)
    return false;
  return true;
}

void csNullmeshMeshObject::UpdateLighting (iLight**, int, iMovable*)
{
  return;
}

bool csNullmeshMeshObject::Draw (iRenderView* rview, iMovable* /*movable*/,
	csZBufMode /*mode*/)
{
  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;
  return true;
}

void csNullmeshMeshObject::GetObjectBoundingBox (csBox3& bbox, int /*type*/)
{
  bbox.Set (-radius, -radius, -radius, radius, radius, radius);
}

bool csNullmeshMeshObject::HitBeamOutline (const csVector3& /*start*/,
  const csVector3& /*end*/, csVector3& /*isect*/, float* /*pr*/)
{
  // @@@ TODO
  return false;
}

bool csNullmeshMeshObject::HitBeamObject (const csVector3& /*start*/,
  const csVector3& /*end*/, csVector3& /*isect*/, float* /*pr*/)
{
  // @@@ TODO
  return false;
}

void csNullmeshMeshObject::GetRadius (csVector3& rad, csVector3& cent)
{
  rad.Set (radius);
  cent.Set (0);
}

csPtr<iMeshObject> csNullmeshMeshObject::MeshObjectFactory::NewInstance ()
{
  csNullmeshMeshObject* cm = new csNullmeshMeshObject (
  	(iMeshObjectFactory*)this);
  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (cm, iMeshObject));
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csNullmeshMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullmeshMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csNullmeshMeshObjectType)

SCF_EXPORT_CLASS_TABLE (nullmesh)
  SCF_EXPORT_CLASS (csNullmeshMeshObjectType, "crystalspace.mesh.object.nullmesh",
    "Crystal Space Null Mesh Type")
SCF_EXPORT_CLASS_TABLE_END

csNullmeshMeshObjectType::csNullmeshMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csNullmeshMeshObjectType::~csNullmeshMeshObjectType ()
{
}

csPtr<iMeshObjectFactory> csNullmeshMeshObjectType::NewFactory ()
{
  csNullmeshMeshObject* cm = new csNullmeshMeshObject (NULL);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

