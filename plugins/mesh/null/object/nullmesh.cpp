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

csNullmeshMeshObject::csNullmeshMeshObject (csNullmeshMeshFactory* factory,
                                            iMeshObjectType* parent)
  : scfImplementationType(this), logparent (0), nullmesh_type (parent),
  vis_cb (0)
{
  csNullmeshMeshObject::factory = factory;
  radius = factory->GetRadius ();
  factory->GetBoundingBox (box);
}

csNullmeshMeshObject::~csNullmeshMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
}

void csNullmeshMeshObject::SetRadius (float radius)
{
  csNullmeshMeshObject::radius = radius;
  box.Set (-radius, -radius, -radius, radius, radius, radius);
  ShapeChanged ();
}

void csNullmeshMeshObject::SetBoundingBox (const csBox3& box)
{
  csNullmeshMeshObject::box = box;
  radius = csQsqrt (csSquaredDist::PointPoint (box.Max (), box.Min ())) / 2.0;
  ShapeChanged ();
}

void csNullmeshMeshObject::GetObjectBoundingBox (csBox3& bbox)
{
  bbox = box;
}

const csBox3& csNullmeshMeshObject::GetObjectBoundingBox ()
{
  return box;
}

void csNullmeshMeshObject::SetObjectBoundingBox (const csBox3& bbox)
{
  box = bbox;
  ShapeChanged ();
}

bool csNullmeshMeshObject::HitBeamOutline (const csVector3& /*start*/,
                                           const csVector3& /*end*/, 
                                           csVector3& /*isect*/, float* /*pr*/)
{
  // @@@ TODO
  return false;
}

bool csNullmeshMeshObject::HitBeamObject (const csVector3& /*start*/,
                                          const csVector3& /*end*/, 
                                          csVector3& /*isect*/, float* /*pr*/,
                                          int* /*polygon_idx*/, iMaterialWrapper**)
{
  // @@@ TODO
  return false;
}

void csNullmeshMeshObject::GetRadius (float& rad, csVector3& cent)
{
  rad = radius;
  cent.Set (box.GetCenter ());
}


//----------------------------------------------------------------------
csNullmeshMeshFactory::csNullmeshMeshFactory (csNullmeshMeshObjectType* type)
: scfImplementationType(this), nullmesh_type (type)
{
}


void csNullmeshMeshFactory::SetRadius (float radius)
{
  csNullmeshMeshFactory::radius = radius;
  box.Set (-radius, -radius, -radius, radius, radius, radius);
}

void csNullmeshMeshFactory::SetBoundingBox (const csBox3& box)
{
  csNullmeshMeshFactory::box = box;
  radius = csQsqrt (csSquaredDist::PointPoint (box.Max (), box.Min ())) / 2.0;
}

csPtr<iMeshObject> csNullmeshMeshFactory::NewInstance ()
{
  csRef<csNullmeshMeshObject> cm;
  cm.AttachNew (new csNullmeshMeshObject (this, nullmesh_type));

  csRef<iMeshObject> im = scfQueryInterface<iMeshObject> (cm);
  return csPtr<iMeshObject> (im);
}


//----------------------------------------------------------------------


SCF_IMPLEMENT_FACTORY (csNullmeshMeshObjectType)


csNullmeshMeshObjectType::csNullmeshMeshObjectType (iBase* pParent)
  : scfImplementationType (this, pParent)
{
}

csNullmeshMeshObjectType::~csNullmeshMeshObjectType ()
{
}

csPtr<iMeshObjectFactory> csNullmeshMeshObjectType::NewFactory ()
{
  csNullmeshMeshFactory* cm = new csNullmeshMeshFactory (this);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

