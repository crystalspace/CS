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
#include "csengine/sector.h"
#include "csengine/meshobj.h"
#include "csengine/light.h"
#include "igraph3d.h"

IMPLEMENT_CSOBJTYPE (csMeshWrapper, csSprite)

csMeshWrapper::csMeshWrapper (csObject* theParent, iMeshObject* mesh)
	: csSprite (theParent), bbox (NULL)
{
  bbox.SetOwner (this);
  ptree_obj = &bbox;
  csMeshWrapper::mesh = mesh;
  mesh->IncRef ();
  draw_cb = NULL;
}

csMeshWrapper::csMeshWrapper (csObject* theParent)
	: csSprite (theParent), bbox (NULL)
{
  bbox.SetOwner (this);
  ptree_obj = &bbox;
  csMeshWrapper::mesh = NULL;
  draw_cb = NULL;
}

void csMeshWrapper::SetMeshObject (iMeshObject* mesh)
{
  if (mesh) mesh->IncRef ();
  if (csMeshWrapper::mesh) mesh->DecRef ();
  csMeshWrapper::mesh = mesh;
}

csMeshWrapper::~csMeshWrapper ()
{
  if (mesh) mesh->DecRef ();
}

void csMeshWrapper::UpdateInPolygonTrees ()
{
  bbox.RemoveFromTree ();

  // If we are not in a sector which has a polygon tree
  // then we don't really update. We should consider if this is
  // a good idea. Do we only want this object updated when we
  // want to use it in a polygon tree? It is certainly more
  // efficient to do it this way when the object is currently
  // moving in normal convex sectors.
  int i;
  csPolygonTree* tree = NULL;
  csVector& sects = movable.GetSectors ();
  for (i = 0 ; i < sects.Length () ; i++)
  {
    tree = ((csSector*)sects[i])->GetStaticTree ();
    if (tree) break;
  }
  if (!tree) return;

  csBox3 b;
  mesh->GetObjectBoundingBox (b);

  // This transform should be part of the object class and not just calculated
  // every time we need it. @@@!!!
  csTransform trans = movable.GetFullTransform ().GetInverse ();

  bbox.Update (b, trans, this);

  // Here we need to insert in trees where this sprite lives.
  for (i = 0 ; i < sects.Length () ; i++)
  {
    tree = ((csSector*)sects[i])->GetStaticTree ();
    if (tree)
    {
      // Temporarily increase reference to prevent free.
      bbox.GetBaseStub ()->IncRef ();
      tree->AddObject (&bbox);
      bbox.GetBaseStub ()->DecRef ();
    }
  }
}

void csMeshWrapper::UpdateMove ()
{
  csSprite::UpdateMove ();
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    csSprite* spr = (csSprite*)children[i];
    spr->GetMovable ().UpdateMove ();
  }
}

void csMeshWrapper::Draw (csRenderView& rview)
{
  if (draw_cb) draw_cb (this, &rview, draw_cbData);
  iRenderView* irv = QUERY_INTERFACE (&rview, iRenderView);
  iMovable* imov = QUERY_INTERFACE (&movable, iMovable);
  if (mesh->DrawTest (irv, imov))
  {
    UpdateDeferedLighting (movable.GetFullPosition ());
    mesh->Draw (irv, imov);
  }
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    csSprite* spr = (csSprite*)children[i];
    spr->Draw (rview);
  }
}

void csMeshWrapper::NextFrame (cs_time current_time)
{
  mesh->NextFrame (current_time);
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    csSprite* spr = (csSprite*)children[i];
    spr->NextFrame (current_time);
  }
}

void csMeshWrapper::UpdateLighting (csLight** lights, int num_lights)
{
  defered_num_lights = 0;
  if (num_lights <= 0) return;
  // @@@ Can't we avoid this allocation?
  iLight** ilights = new iLight* [num_lights];
  int i;
  for (i = 0 ; i < num_lights ; i++)
  {
    ilights[i] = QUERY_INTERFACE (lights[i], iLight);
  }
  iMovable* imov = QUERY_INTERFACE (&movable, iMovable);
  mesh->UpdateLighting (ilights, num_lights, imov);
  delete ilights;

  for (i = 0 ; i < children.Length () ; i++)
  {
    csSprite* spr = (csSprite*)children[i];
    spr->UpdateLighting (lights, num_lights);
  }
}


bool csMeshWrapper::HitBeamObject (const csVector3& /*start*/,
  const csVector3& /*end*/, csVector3& /*isect*/, float* /*pr*/)
{
  return false;
}

void csMeshWrapper::HardTransform (const csReversibleTransform& t)
{
  mesh->HardTransform (t);
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    csSprite* spr = (csSprite*)children[i];
    // @@@ Change as soon as csMeshWrapper becomes csSprite.
    if (spr->GetType () >= csMeshWrapper::Type)
      ((csMeshWrapper*)spr)->HardTransform (t);
  }
}


//--------------------------------------------------------------------------

IMPLEMENT_IBASE (csMeshFactoryWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iMeshFactoryWrapper)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csMeshFactoryWrapper::MeshFactoryWrapper)
  IMPLEMENTS_INTERFACE (iMeshFactoryWrapper)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_CSOBJTYPE (csMeshFactoryWrapper, csObject)

csMeshFactoryWrapper::csMeshFactoryWrapper (iMeshObjectFactory* meshFact)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csMeshFactoryWrapper::meshFact = meshFact;
  meshFact->IncRef ();
}

csMeshFactoryWrapper::csMeshFactoryWrapper ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csMeshFactoryWrapper::meshFact = NULL;
}

csMeshFactoryWrapper::~csMeshFactoryWrapper ()
{
  if (meshFact) meshFact->DecRef ();
}

void csMeshFactoryWrapper::SetMeshObjectFactory (iMeshObjectFactory* meshFact)
{
  if (meshFact) meshFact->DecRef ();
  csMeshFactoryWrapper::meshFact = meshFact;
  if (meshFact) meshFact->IncRef ();
}

csMeshWrapper* csMeshFactoryWrapper::NewMeshObject (csObject* parent)
{
  iMeshObject* mesh = meshFact->NewInstance ();
  csMeshWrapper* meshObj = new csMeshWrapper (parent, mesh);
  mesh->DecRef ();
  return meshObj;
}

