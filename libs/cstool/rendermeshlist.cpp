/*
  Copyright (C) 2003 by Marten Svanfeldt

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

#include "csgeom/transfrm.h"

#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "cstool/rendermeshlist.h"


#include "iengine/engine.h"
#include "ivideo/rendermesh.h"


csRenderMeshList::csRenderMeshList (iEngine* engine)
{
  csRenderMeshList::engine = engine;
}

csRenderMeshList::~csRenderMeshList ()
{
}

void csRenderMeshList::AddRenderMeshes (csRenderMesh** meshes, int num,
                                   long renderPriority,
				   csZBufMode z_buf_mode)
{
  renderMeshListInfo* entry;

  //check if we have rp or need to add it
  if (renderPriority > renderList.Length () - 1 || renderList.Get
    ((uint32)renderPriority) == 0)
  {
    entry = new renderMeshListInfo;
    entry->renderPriority = renderPriority;
    if (engine)
    {
      entry->sortingOption = engine->GetRenderPrioritySorting (renderPriority);
    }
    else
    {
      entry->sortingOption = 0;
    }

    renderList.Put ((uint32)renderPriority, entry);
  }
  else
  {
    entry = renderList.Get ((uint32)renderPriority);
  }

  for (int i = 0; i < num; ++i)
  {
    meshes[i]->z_buf_mode = z_buf_mode;
    entry->meshList.Push (meshes[i]);
  }
}

void csRenderMeshList::Empty ()
{
  csPDelArray < renderMeshListInfo >::Iterator it = renderList.GetIterator ();

  while (it.HasNext ())
  {
    renderMeshListInfo* listEnt = it.Next ();
    if (0 == listEnt)
      continue;

    listEnt->meshList.Empty ();
  }
}

static int SortMeshBack2Front (void const* item1,
                        void const* item2)
{
  csRenderMesh* m1 = *(csRenderMesh**) item1;
  csRenderMesh* m2 = *(csRenderMesh**) item2;

  const csReversibleTransform& t1 = m1->object2camera;
  const csReversibleTransform& t2 = m2->object2camera;
  if (t1.GetOrigin ().z < t2.GetOrigin().z)
    return -1;
  else if (t1.GetOrigin ().z > t2.GetOrigin().z)
    return 1;
  return 0;
}

static int SortMeshFront2Back (void const* item1,
                        void const* item2)
{
  csRenderMesh* m1 = *(csRenderMesh**) item1;
  csRenderMesh* m2 = *(csRenderMesh**) item2;

  const csReversibleTransform& t1 = m1->object2camera;
  const csReversibleTransform& t2 = m2->object2camera;
  if (t1.GetOrigin ().z < t2.GetOrigin().z)
    return 1;
  else if (t1.GetOrigin ().z > t2.GetOrigin().z)
    return -1;
  return 0;
}

static int SortMeshMaterial (void const* item1,
                             void const* item2)
{
  csRenderMesh* m1 = *(csRenderMesh**) item1;
  csRenderMesh* m2 = *(csRenderMesh**) item2;

  if (m1->portal != 0 && m2->portal == 0)
    return 1;
  else if (m2->portal != 0 && m1->portal == 0)
    return -1;
  else
  {
    if (m1->material > m2->material)
      return 1;
    if (m1->material < m2->material)
      return -1;
  }
  return 0;
}

int csRenderMeshList::SortMeshLists ()
{
  int numObjects = 0;
  csPDelArray < renderMeshListInfo >::Iterator it = renderList.GetIterator ();
  while (it.HasNext ())
  {
    renderMeshListInfo* listEnt = it.Next ();
    if (0 == listEnt)
      continue;

    if (listEnt->sortingOption == CS_RENDPRI_BACK2FRONT)
    {
      listEnt->meshList.Sort (SortMeshBack2Front);
    }
    else if (listEnt->sortingOption == CS_RENDPRI_FRONT2BACK)
    {
      listEnt->meshList.Sort (SortMeshFront2Back);
    }
    else
    {
      listEnt->meshList.Sort (SortMeshMaterial);
    }
    numObjects += listEnt->meshList.Length ();
  }
  return numObjects;
}

void csRenderMeshList::GetSortedMeshes (csRenderMesh** meshes)
{
  csPDelArray < renderMeshListInfo >::Iterator it = renderList.GetIterator ();
  while (it.HasNext ())
  {
    renderMeshListInfo* listEnt = it.Next ();
    if (listEnt)
    {
      int numObjects = listEnt->meshList.Length ();
      for (int j = 0 ; j < numObjects ; j++)
        *meshes++ = listEnt->meshList[j];
    }
  }
}

