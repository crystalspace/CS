/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "csutil/dirtyaccessarray.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/rdrprior.h"
#include "iengine/mesh.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/engine.h"

csRenderQueueSet::csRenderQueueSet ()
{
}

csRenderQueueSet::~csRenderQueueSet ()
{
}

void csRenderQueueSet::ClearVisible ()
{
  size_t i;
  for (i = 0 ; i < visible.Length () ; i++)
    if (visible[i])
      visible[i]->SetLength (0);
}

void csRenderQueueSet::AddVisible (csMeshWrapper *mesh, uint32 frustum_mask)
{
  long pri = mesh->GetRenderPriority ();

  // look if the desired priority queue exists, and possibly
  // extend the list of visible.
  if ((size_t)pri >= visible.Length ())
    visible.SetLength (pri+1);

  // look if the desired queue exists, and create it if not
  if (!visible[pri])
  {
    csArrayMeshMask* mvnd = new csArrayMeshMask ();
    visible.Put (pri, mvnd);
  }

  // add the mesh wrapper
  visible[pri]->Push (csMeshWithMask (mesh, frustum_mask));
}

struct comp_mesh_comp
{
  float z;
  csMeshWithMask mesh_with_mask;
};

typedef csDirtyAccessArray<comp_mesh_comp> engine3d_comp_mesh_z;
CS_IMPLEMENT_STATIC_VAR (GetStaticComp_Mesh_Comp, engine3d_comp_mesh_z,())

static int comp_mesh (const void *el1, const void *el2)
{
  comp_mesh_comp *m1 = (comp_mesh_comp *)el1;
  comp_mesh_comp *m2 = (comp_mesh_comp *)el2;
  if (m1->z < m2->z)
    return -1;
  else if (m1->z > m2->z)
    return 1;
  else
    return 0;
}

void csRenderQueueSet::SortAll (csArrayMeshMask& meshes, iRenderView* rview)
{
  int tot_objects = 0;
  size_t priority;
  for (priority = 0 ; priority < visible.Length () ; priority++)
  {
    Sort (rview, (int)priority);
    csArrayMeshMask* v = visible[priority];
    if (v)
      tot_objects += (int)v->Length ();
  }
  if (!tot_objects) return;

  for (priority = 0 ; priority < visible.Length () ; priority++)
  {
    csArrayMeshMask* v = visible[priority];
    if (v)
      for (size_t i = 0 ; i < v->Length () ; i++)
        meshes.Push ((*v)[i]);
  }
}

void csRenderQueueSet::Sort (iRenderView *rview, int priority)
{
  static engine3d_comp_mesh_z &comp_mesh_z = *GetStaticComp_Mesh_Comp ();
  if (!visible[priority]) return ;

  int rendsort = csEngine::current_engine->GetRenderPrioritySorting (
      priority);
  if (rendsort == CS_RENDPRI_NONE) return ;

  csArrayMeshMask *v = visible[priority];
  if (v->Length () > comp_mesh_z.Length ())
    comp_mesh_z.SetLength (v->Length ());

  const csReversibleTransform &camtrans = rview->GetCamera ()->GetTransform ();
  size_t i;
  for (i = 0; i < v->Length (); i++)
  {
    csMeshWithMask& mesh_with_mask = v->Get (i);
    csMeshWrapper *mesh = mesh_with_mask.mesh;
    csVector3 rad, cent;
    mesh->GetRadius (rad, cent);

    csReversibleTransform tr_o2c = camtrans;
    iMovable* movable = mesh->GetMovable ();
    if (!movable->IsFullTransformIdentity ())
      tr_o2c /= movable->GetFullTransform ();
    csVector3 tr_cent = tr_o2c.Other2This (cent);
    comp_mesh_z[i].z = rendsort == CS_RENDPRI_FRONT2BACK
    	? tr_cent.z
	: -tr_cent.z;
    comp_mesh_z[i].mesh_with_mask = mesh_with_mask;
  }

  qsort (
    comp_mesh_z.GetArray (),
    v->Length (),
    sizeof (comp_mesh_comp),
    comp_mesh);

  for (i = 0; i < v->Length (); i++)
  {
    (*v)[i] = comp_mesh_z[i].mesh_with_mask;
  }

  return ;
}

