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
#include "iengine/material.h"
#include "iengine/camera.h"
#include "igeom/clip2d.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iutil/objreg.h"
#include "csqsqrt.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csNullmeshMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iNullMeshState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iNullFactoryState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullmeshMeshObject::MeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullmeshMeshObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullmeshMeshObject::NullFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iNullFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullmeshMeshObject::NullMeshState)
  SCF_IMPLEMENTS_INTERFACE (iNullMeshState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csNullmeshMeshObject::csNullmeshMeshObject (csNullmeshMeshObject* factory,
                                            iMeshObjectType* parent)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshObjectFactory);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiNullFactoryState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiNullMeshState);
  if (factory)
  {
    csNullmeshMeshObject::factory = &(factory->scfiMeshObjectFactory);
    radius = factory->radius;
    box = factory->box;
  }
  else
  {
    csNullmeshMeshObject::factory = 0;
    radius = 0.0001f;
    box.Set (-radius, -radius, -radius, radius, radius, radius);
  }
  logparent = 0;
  logparent_factory = 0;
  nullmesh_type = parent;
  vis_cb = 0;
}

csNullmeshMeshObject::~csNullmeshMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMeshObjectFactory);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiNullFactoryState);
  SCF_DESTRUCT_IBASE ();
}

void csNullmeshMeshObject::SetRadius (float radius)
{
  csNullmeshMeshObject::radius = radius;
  box.Set (-radius, -radius, -radius, radius, radius, radius);
  scfiObjectModel.ShapeChanged ();
}

void csNullmeshMeshObject::SetBoundingBox (const csBox3& box)
{
  csNullmeshMeshObject::box = box;
  radius = csQsqrt (csSquaredDist::PointPoint (box.Max (), box.Min ())) / 2.0;
  scfiObjectModel.ShapeChanged ();
}

void csNullmeshMeshObject::GetObjectBoundingBox (csBox3& bbox)
{
  bbox = box;
}

void csNullmeshMeshObject::SetObjectBoundingBox (const csBox3& bbox)
{
  box = bbox;
  scfiObjectModel.ShapeChanged ();
}

bool csNullmeshMeshObject::HitBeamOutline (const csVector3& /*start*/,
  const csVector3& /*end*/, csVector3& /*isect*/, float* /*pr*/)
{
  // @@@ TODO
  return false;
}

bool csNullmeshMeshObject::HitBeamObject (const csVector3& /*start*/,
  const csVector3& /*end*/, csVector3& /*isect*/, float* /*pr*/,
  int* /*polygon_idx*/)
{
  // @@@ TODO
  return false;
}

void csNullmeshMeshObject::GetRadius (csVector3& rad, csVector3& cent)
{
  rad.Set (radius);
  cent.Set (box.GetCenter ());
}

csPtr<iMeshObject> csNullmeshMeshObject::MeshObjectFactory::NewInstance ()
{
  csNullmeshMeshObject* cm = new csNullmeshMeshObject (scfParent, 
    scfParent->nullmesh_type);
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


csNullmeshMeshObjectType::csNullmeshMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csNullmeshMeshObjectType::~csNullmeshMeshObjectType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csNullmeshMeshObjectType::NewFactory ()
{
  csNullmeshMeshObject* cm = new csNullmeshMeshObject (0, this);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

