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
#include "csutil/typedvec.h"
#include "csutil/garray.h"
#include "csengine/engine.h"
#include "csengine/rdrprior.h"
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

void csRenderQueueSet::Add (iMeshWrapper *mesh)
{
  long pri = mesh->GetRenderPriority ();

  // look if the desired priority queue exists, and possibly

  // extend the list of queues
  for (int i = Queues.Length (); i <= pri; i++) Queues[i] = NULL;

  // look if the desired queue exists, and create it if not
  if (!Queues[pri]) Queues[pri] = new csMeshVectorNodelete ();

  // add the mesh wrapper
  Queues[pri]->Push (mesh);
}

void csRenderQueueSet::Remove (iMeshWrapper *mesh)
{
  long pri = mesh->GetRenderPriority ();

  // look if the queue of the mesh exists
  if (pri < Queues.Length () && Queues[pri] != NULL)
  {
    // delete the object from the queue
    Queues[pri]->Delete (mesh);
  }
}

void csRenderQueueSet::RemoveUnknownPriority (iMeshWrapper *mesh)
{
  for (int i = 0; i < Queues.Length (); i++)
  {
    if (Queues[i])
    {
      int n = Queues[i]->Find (mesh);
      if (n != -1)
      {
        Queues[i]->Delete (n);
        return ;
      }
    }
  }
}

struct comp_mesh_comp
{
  float z;
  iMeshWrapper *mesh;
};

static CS_DECLARE_GROWING_ARRAY (comp_mesh_z, comp_mesh_comp);

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

void csRenderQueueSet::Sort (iRenderView *rview, int priority)
{
  if (!Queues[priority]) return ;

  int rendsort = csEngine::current_engine->GetRenderPrioritySorting (
      priority);
  if (rendsort == CS_RENDPRI_NONE) return ;

  csMeshVectorNodelete *v = Queues[priority];
  if (v->Length () > comp_mesh_z.Limit ())
    comp_mesh_z.SetLimit (v->Length ());

  const csReversibleTransform &camtrans = rview->GetCamera ()->GetTransform ();
  int i;
  for (i = 0; i < v->Length (); i++)
  {
    iMeshWrapper *mesh = v->Get (i);
    csVector3 rad, cent;
    mesh->GetRadius (rad, cent);

    csReversibleTransform tr_o2c = camtrans *
      mesh->GetMovable ()->GetFullTransform ().GetInverse ();
    csVector3 tr_cent = tr_o2c.Other2This (cent);
    comp_mesh_z[i].z = rendsort == CS_RENDPRI_FRONT2BACK ? tr_cent.z : -tr_cent.z;
    comp_mesh_z[i].mesh = mesh;
  }

  qsort (
    comp_mesh_z.GetArray (),
    v->Length (),
    sizeof (comp_mesh_comp),
    comp_mesh);

  for (i = 0; i < v->Length (); i++)
  {
    (*v)[i] = comp_mesh_z[i].mesh;
  }

  return ;
}
