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
  lastMesh = 0;
  nextShrink = 0;
}

csRenderMeshHolderSingle::~csRenderMeshHolderSingle ()
{
  while (meshes.Length () > 0)
  {
    csRenderMesh *mesh = meshes.Pop ();
    delete mesh;
  }
}

csRenderMesh*& csRenderMeshHolderSingle::GetUnusedMesh (bool& created,
							uint frameNumber)
{
  created = false;
  if ((meshes.Length() == 0) || 
    (meshes[lastMesh]->lastFrame == frameNumber))
  {
    lastMesh = -1;
    //check the list
    int i;
    for(i = 0; i<meshes.Length (); i++)
    {
      if (meshes[i]->lastFrame != frameNumber)
      {
        lastMesh = i;
        break;
      }
    }
    if (lastMesh == -1)
    {
      lastMesh = meshes.Push (new csRenderMesh);
      created = true;
      nextShrink = frameNumber + 1;
    }
  }

  // Conserve some memory
  if (!created && (frameNumber <= nextShrink))
  {
    meshes.ShrinkBestFit ();
  }

  csRenderMesh*& mesh = meshes[lastMesh];
  mesh->lastFrame = frameNumber;
  return mesh;
}

//---------------------------------------------------------------------------

csRenderMeshHolderMultiple::csRenderMeshHolderMultiple (bool deleteMeshes) :
  clearQueue (0, 4)
{
  rmHolderListIndex = rmHolderList.Push (
    new csDirtyAccessArray<csRenderMesh*>);
  csRenderMeshHolderMultiple::deleteMeshes = deleteMeshes;
}

csRenderMeshHolderMultiple::~csRenderMeshHolderMultiple ()
{
  while (rmHolderList.Length() > 0)
  {
    csDirtyAccessArray<csRenderMesh*>* holder = rmHolderList.Pop();
    if (deleteMeshes)
    {
      for (int j = 0; j < holder->Length(); j++)
      {
	csRenderMesh* rm = (*holder)[j];
	delete rm;
      }
    }
    delete holder;
  }
}

csDirtyAccessArray<csRenderMesh*>& 
csRenderMeshHolderMultiple::GetUnusedMeshes(uint frameNumber)
{
  csDirtyAccessArray<csRenderMesh*>* rmH = rmHolderList[rmHolderListIndex];

  if (rmH->Length() > 0 && ((*rmH)[0]->lastFrame == frameNumber))
  {
    rmHolderListIndex = -1;
    //find an empty rmH
    for(int i = 0; i < rmHolderList.Length(); i++)
    {
      rmH = rmHolderList[i];
      if ((rmH->Length() == 0) || ((*rmH)[0]->lastFrame != frameNumber))
      {
        rmHolderListIndex = i;
        break;
      }
    }
    if (rmHolderListIndex == -1)
    {
      rmH = new csDirtyAccessArray<csRenderMesh*>;
      rmHolderListIndex = rmHolderList.Push (rmH);
    }
  }

  if (clearQueue.Length() > 0)
  {
    int i = 0;
    while (i < clearQueue.Length())
    {
      csDirtyAccessArray<csRenderMesh*>* clearArray = clearQueue[i];
      if ((clearArray->Length() == 0) || ((*clearArray)[0]->lastFrame != frameNumber))
      {
	if (deleteMeshes)
	{
	  for (int j = 0; j < clearArray->Length(); j++)
	  {
	    csRenderMesh* rm = (*clearArray)[j];
	    delete rm;
	  }
	}
	delete clearArray;
	clearQueue.DeleteIndex (i);
      }
      else
	i++;
    }
    if (clearQueue.Length() == 0)
      clearQueue.ShrinkBestFit();
  }

  return *rmH;
}

void csRenderMeshHolderMultiple::Clear()
{
  clearQueue.SetCapacity (rmHolderList.Length());
  while (rmHolderList.Length() > 0)
  {
    clearQueue.Push (rmHolderList.Pop ());
  }

  rmHolderListIndex = rmHolderList.Push (
    new csDirtyAccessArray<csRenderMesh*>);
}
