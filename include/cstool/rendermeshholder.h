/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_CSTOOL_RENDERMESHHOLDER_H__
#define __CS_CSTOOL_RENDERMESHHOLDER_H__

#include "csextern.h"

#include "csutil/array.h"
#include "csutil/garray.h"

struct csRenderMesh;

/// @@@ Document me!
class CS_CSTOOL_EXPORT csRenderMeshHolderSingle
{
  csArray<csRenderMesh*> meshes;
  int lastMesh;
public:
  csRenderMeshHolderSingle ();
  ~csRenderMeshHolderSingle ();

  csRenderMesh*& GetUnusedMesh();
};

/// @@@ Document me!
class CS_CSTOOL_EXPORT csRenderMeshHolderMultiple
{
  struct rmHolder
  {
    csDirtyAccessArray<csRenderMesh*> renderMeshes;
  };
  csArray<rmHolder*> rmHolderList;
  int rmHolderListIndex;
public:
  csRenderMeshHolderMultiple ();
  ~csRenderMeshHolderMultiple ();

  csDirtyAccessArray<csRenderMesh*>& GetUnusedMeshes ();
};

#endif // __CS_CSTOOL_RENDERMESHHOLDER_H__
