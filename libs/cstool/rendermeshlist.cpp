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


csRenderMeshList::csRenderMeshList (iObjectRegistry *objreg)
{
  engine = CS_QUERY_REGISTRY(objreg, iEngine);
}

csRenderMeshList::~csRenderMeshList ()
{
}

void csRenderMeshList::AddRenderMeshes (csRenderMesh** meshes, int num, 
                                   long renderPriority)
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

/**
 * NOTE! This function sorts a bit unusual. First, put all elements
 * which is 0 at the end. (no elements really). Then sort according to RP
 */
int csRenderMeshList::CompareMeshListInfo (void const* item1,
                                           void const* item2)
{
  if (0 == item1 && 0 != item2)
  {
    return 1;
  }
  else if (0 != item1 && 0 == item2)
  {
    return -1;
  }
  else if (0 != item1 && 0 != item2)
  {
    //sort on rp
    renderMeshListInfo* it1 = (renderMeshListInfo*)item1;
    renderMeshListInfo* it2 = (renderMeshListInfo*)item2;
    if (it1->renderPriority < it2->renderPriority)
      return -1;
    else if (it1->renderPriority > it2->renderPriority)
      return 1;
  }

  return 0;
}

static int SortMeshBack2Front (void const* item1,
                        void const* item2)
{
  csRenderMesh* m1 = *(csRenderMesh**) item1;
  csRenderMesh* m2 = *(csRenderMesh**) item2;

  if (m1->transform->GetOrigin ().z < m2->transform->GetOrigin().z)
    return -1;
  else if (m1->transform->GetOrigin ().z > m2->transform->GetOrigin().z)
    return 1;
  return 0;
}

static int SortMeshFront2Back (void const* item1,
                        void const* item2)
{
  csRenderMesh* m1 = *(csRenderMesh**) item1;
  csRenderMesh* m2 = *(csRenderMesh**) item2;

  if (m1->transform->GetOrigin ().z < m2->transform->GetOrigin().z)
    return 1;
  else if (m1->transform->GetOrigin ().z > m2->transform->GetOrigin().z)
    return -1;
  return 0;
}

void csRenderMeshList::GetSortedMeshList (
	csArray<csRenderMesh*>& meshes)
{
  // iterate, and if needed, sort the lists
  // also sum up how many objects we have
  int numObjects = 0;
  renderList.Sort (CompareMeshListInfo);

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

    numObjects += listEnt->meshList.Length ();
  }

  meshes.DeleteAll ();
  int currentElement;
  
  it.Reset ();

  while (it.HasNext ())
  {
    renderMeshListInfo* listEnt = it.Next ();

    if (0 == listEnt)
      continue;

    for (currentElement = 0; currentElement < listEnt->meshList.Length (); 
      ++currentElement)
    {
      meshes.Push (listEnt->meshList.Get (currentElement));
    }
  }
}
