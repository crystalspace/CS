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
#include "csutil/csvector.h"
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

  // If the CS_ENTITY_CAMERA flag is set we automatically mark
  // the render priority with the camera flag.
  bool do_camera = mesh->GetFlags ().Check (CS_ENTITY_CAMERA);
  if (do_camera)
  {
    csEngine::current_engine->SetRenderPriorityCamera (pri, do_camera);
  }

  // look if the desired priority queue exists, and possibly
  // extend the list of queues
  if (pri >= Queues.Length ())
    Queues.SetLength (pri+1);

  // look if the desired queue exists, and create it if not
  if (!Queues[pri])
  {
    csMeshVectorNodelete* mvnd = new csMeshVectorNodelete ();
    Queues.Put (pri, mvnd);
  }

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

typedef csGrowingArray<comp_mesh_comp> engine3d_comp_mesh_z;
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

iMeshWrapper** csRenderQueueSet::SortAll (iRenderView* rview,
	int& tot_num, uint32 current_visnr)
{
  tot_num = 0;

  int tot_objects = 0;
  int priority;
  for (priority = 0 ; priority < Queues.Length () ; priority++)
  {
    Sort (rview, priority);
    csMeshVectorNodelete* v = Queues[priority];
    if (v)
      tot_objects += v->Length ();
  }
  if (!tot_objects) return NULL;

  iMeshWrapper** meshes = new iMeshWrapper* [tot_objects];
  for (priority = 0 ; priority < Queues.Length () ; priority++)
  {
    csMeshVectorNodelete* v = Queues[priority];
    if (v)
      for (int i = 0 ; i < v->Length () ; i++)
      {
        iMeshWrapper *sp = v->Get (i);
        if (sp->GetPrivateObject ()->GetVisibilityNumber () == current_visnr)
        {
          meshes[tot_num++] = sp;
        }
      }
  }

  return meshes;
}

void csRenderQueueSet::Sort (iRenderView *rview, int priority)
{
  static engine3d_comp_mesh_z &comp_mesh_z = *GetStaticComp_Mesh_Comp ();
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

    csReversibleTransform tr_o2c = camtrans;
    iMovable* movable = mesh->GetMovable ();
    if (!movable->IsFullTransformIdentity ())
      tr_o2c /= movable->GetFullTransform ();
    csVector3 tr_cent = tr_o2c.Other2This (cent);
    comp_mesh_z[i].z = rendsort == CS_RENDPRI_FRONT2BACK
    	? tr_cent.z
	: -tr_cent.z;
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

