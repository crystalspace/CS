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

IMPLEMENT_IBASE (csCubeMeshObject)
  IMPLEMENTS_INTERFACE (iMeshObject)
IMPLEMENT_IBASE_END

csCubeMeshObject::csCubeMeshObject ()
{
  CONSTRUCT_IBASE (NULL);
}

csCubeMeshObject::~csCubeMeshObject ()
{
}

bool csCubeMeshObject::Draw (iRenderView* rview)
{
  printf ("Draw\n");
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csCubeMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csCubeMeshObjectFactory)

EXPORT_CLASS_TABLE (cube)
  EXPORT_CLASS (csCubeMeshObjectFactory, "crystalspace.meshobj.cube",
    "Crystal Space Cube Mesh Object")
EXPORT_CLASS_TABLE_END

csCubeMeshObjectFactory::csCubeMeshObjectFactory (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
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
  return new csCubeMeshObject ();
}

