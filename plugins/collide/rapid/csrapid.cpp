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
#include "csrapid.h"
#include "rapcol.h"
#include "ivaria/polymesh.h"

CS_IMPLEMENT_PLUGIN

IMPLEMENT_IBASE (csRapidCollider)
  IMPLEMENTS_INTERFACE (iCollider)
IMPLEMENT_IBASE_END

csRapidCollider::csRapidCollider (iPolygonMesh* mesh)
{
  CONSTRUCT_IBASE (NULL);

  csRapidCollider::mesh = mesh;
  mesh->IncRef ();
  collider = new csRAPIDCollider (mesh);
}

csRapidCollider::~csRapidCollider ()
{
  if (mesh) mesh->DecRef ();
  delete collider;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csRapidCollideSystem)
  IMPLEMENTS_INTERFACE (iCollideSystem)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csRapidCollideSystem)

EXPORT_CLASS_TABLE (rapid)
  EXPORT_CLASS (csRapidCollideSystem, "crystalspace.collisiondetection.rapid",
    "Crystal Space RAPID CD System")
EXPORT_CLASS_TABLE_END

csRapidCollideSystem::csRapidCollideSystem (iBase *pParent)
{
  CONSTRUCT_IBASE (pParent);
}

bool csRapidCollideSystem::Initialize (iSystem*)
{
  return true;
}

iCollider* csRapidCollideSystem::CreateCollider (iPolygonMesh* mesh)
{
  csRapidCollider* col = new csRapidCollider (mesh);
  return col;
}

bool csRapidCollideSystem::Collide (iCollider* collider1, const csTransform* trans1,
  	iCollider* collider2, const csTransform* trans2)
{
  csRapidCollider* col1 = (csRapidCollider*)collider1;
  csRapidCollider* col2 = (csRapidCollider*)collider2;
  return col1->GetPrivateCollider ()->Collide (*(col2->GetPrivateCollider ()),
  	trans1, trans2);
}

csCollisionPair* csRapidCollideSystem::GetCollisionPairs ()
{
  return csRAPIDCollider::GetCollisions ();
}

int csRapidCollideSystem::GetCollisionPairCount ()
{
  return csRAPIDCollider::numHits;
}

void csRapidCollideSystem::ResetCollisionPairs ()
{
  csRAPIDCollider::CollideReset ();
  csRAPIDCollider::numHits = 0;
}

