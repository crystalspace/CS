/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter
	      (C) 2004 by Marten Svanfeldt

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

#include "cstool/rendermeshholder.h"
#include "ivideo/rendermesh.h"
  
csRenderMeshHolderSingle::csRenderMeshHolderSingle ()
{
  lastMesh = meshes.Push (new csRenderMesh);
}

csRenderMeshHolderSingle::~csRenderMeshHolderSingle ()
{
  while (meshes.Length () > 0)
  {
    csRenderMesh *mesh = meshes.Pop ();
    CS_ASSERT (mesh->inUse == false);
    delete mesh;
  }
}

csRenderMesh*& csRenderMeshHolderSingle::GetUnusedMesh()
{
  if (meshes[lastMesh]->inUse == true)
  {
    lastMesh = -1;
    //check the list
    int i;
    for(i = 0; i<meshes.Length (); i++)
    {
      if (meshes[i]->inUse == false)
      {
        lastMesh = i;
        break;
      }
    }
    if (lastMesh == -1)
    {
      lastMesh = meshes.Push (new csRenderMesh);
    }
  }

  return meshes[lastMesh];
}

//---------------------------------------------------------------------------

csRenderMeshHolderMultiple::csRenderMeshHolderMultiple ()
{
  rmHolderList.Push (new rmHolder);
  rmHolderListIndex = 0;
}

csRenderMeshHolderMultiple::~csRenderMeshHolderMultiple ()
{
  while (rmHolderList.Length() > 0)
  {
    rmHolder* holder = rmHolderList.Pop();
    for (int j = 0; j < holder->renderMeshes.Length(); j++)
    {
      csRenderMesh* rm = holder->renderMeshes[j];
      CS_ASSERT (rm->inUse == false);
      delete rm;
    }
    delete holder;
  }
}

csDirtyAccessArray<csRenderMesh*>& 
csRenderMeshHolderMultiple::GetUnusedMeshes()
{
  rmHolder *rmH = rmHolderList[rmHolderListIndex];

  if (rmH->renderMeshes.Length() > 0 && rmH->renderMeshes[0]->inUse)
  {
    rmHolderListIndex = -1;
    //find an empty rmH
    for(int i = 0; i < rmHolderList.Length(); i++)
    {
      rmH = rmHolderList[i];
      csRenderMesh *m = rmH->renderMeshes[0];
      if ((rmH->renderMeshes.Length() == 0) || !rmH->renderMeshes[0]->inUse)
      {
        rmHolderListIndex = i;
        break;
      }
    }
    if (rmHolderListIndex == -1)
    {
      rmH = new rmHolder;
      rmHolderListIndex = rmHolderList.Push (rmH);
    }
  }

  return rmH->renderMeshes;
}
