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

#include "csutil/blockallocator.h"
#include "cstool/rendermeshholder.h"
#include "ivideo/rendermesh.h"

struct RenderMeshBlockAlloc : public csBlockAllocator<csRenderMesh>
{
public:
  RenderMeshBlockAlloc () : csBlockAllocator<csRenderMesh> (100) {}
};
CS_IMPLEMENT_STATIC_VAR (GetRMAlloc, RenderMeshBlockAlloc, ())

csRenderMeshHolder::csRenderMeshPtr::csRenderMeshPtr()
{
  ptr = GetRMAlloc()->Alloc();// new csRenderMesh;
}

csRenderMeshHolder::csRenderMeshPtr::csRenderMeshPtr (
  csRenderMeshPtr const& other)
{
  ptr = GetRMAlloc()->Alloc();
  *ptr = *other.ptr;
}

csRenderMeshHolder::csRenderMeshPtr::~csRenderMeshPtr()
{
  GetRMAlloc()->Free (ptr);
}

csRenderMesh*& csRenderMeshHolder::GetUnusedMesh (bool& created,
                                                  uint frameNumber)
{
  return meshes.GetUnusedData (created, frameNumber).ptr;
}
