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
#include "thingmsh.h"
#include "iutil/objreg.h"
#include "iengine/engine.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csThingMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iThingEnvironment)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingMeshObjectType::eiThingEnvironment)
  SCF_IMPLEMENTS_INTERFACE (iThingEnvironment)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csThingMeshObjectType)

SCF_EXPORT_CLASS_TABLE (thing)
  SCF_EXPORT_CLASS (csThingMeshObjectType, "crystalspace.mesh.object.thing",
    "Crystal Space Thing Mesh Type")
SCF_EXPORT_CLASS_TABLE_END

csThingMeshObjectType::csThingMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiThingEnvironment);
  parent_type = NULL;
  te = NULL;
}

csThingMeshObjectType::~csThingMeshObjectType ()
{
  if (te) te->DecRef ();
}

iMeshObjectFactory* csThingMeshObjectType::NewFactory ()
{
  if (!parent_type)
  {
    iEngine* engine = CS_QUERY_REGISTRY (object_reg, iEngine);
    CS_ASSERT (engine != NULL);
    parent_type = engine->GetThingType ();
    engine->DecRef ();
  }
  return parent_type->NewFactory ();
}

iThingEnvironment* csThingMeshObjectType::TE ()
{
  if (te) return te;
  if (!parent_type)
  {
    iEngine* engine = CS_QUERY_REGISTRY (object_reg, iEngine);
    CS_ASSERT (engine != NULL);
    parent_type = engine->GetThingType ();
    engine->DecRef ();
  }
  te = SCF_QUERY_INTERFACE (parent_type, iThingEnvironment);
  return te;
}


