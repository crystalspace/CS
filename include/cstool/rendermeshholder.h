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

/**\file
 * Helper classes to retrieve unused csRenderMeshes and arrays of them.
 */

#include "csextern.h"

#include "csutil/array.h"
#include "csutil/blockallocator.h"
#include "csutil/dirtyaccessarray.h"

#include "framedataholder.h"

struct csRenderMesh;

/**
 * Helper class to retrieve an unused csRenderMesh.
 * \remark Uses csFrameDataHolder internally. See it's documentation for some
 *  additional information.
 */
class CS_CRYSTALSPACE_EXPORT csRenderMeshHolderSingle
{
  struct CS_CRYSTALSPACE_EXPORT csRenderMeshPtr
  {
    csRenderMesh* ptr;

    csRenderMeshPtr ();
    csRenderMeshPtr (csRenderMeshPtr const& other);
    ~csRenderMeshPtr ();
  };
  csFrameDataHolder<csRenderMeshPtr> meshes;
public:
  /**
   * Retrieve a csRenderMesh which was not yet used this frame.
   * \param created \a True if a new csRenderMesh was allocated, \a False
   *  if one was reused. Can be used to determine whether all or only a
   *  subset of the csRenderMesh values have to be initialized.
   * \param frameNumber Current frame number - used to determine unused 
   *  meshes.
   */
  csRenderMesh*& GetUnusedMesh (bool& created, uint frameNumber);
};

/**
 * Helper class to retrieve an unused array of csRenderMesh*.
 * Manages a list of csRenderMesh* array and returns one whose first contained
 * csRenderMesh has it's \a inUse member set to false (the assumption is that 
 * when one mesh is unused, all are).
 */
class CS_CRYSTALSPACE_EXPORT csRenderMeshHolderMultiple
{
  csArray<csDirtyAccessArray<csRenderMesh*>*> rmHolderList;
  csArray<csDirtyAccessArray<csRenderMesh*>*> clearQueue;
  size_t rmHolderListIndex;
public:
  /**
   * Whether to delete the rendermeshes contained in the managed
   * arrays.
   */
  bool deleteMeshes;

  /**
   * Construct a new render mesh array holder.
   * \param deleteMeshes Whether to delete the contained rendermeshes.
   */
  csRenderMeshHolderMultiple (bool deleteMeshes = true);
  ~csRenderMeshHolderMultiple ();

  /**
   * Retrieve an unused array of csRenderMesh*.
   * \param frameNumber Current frame number - used to determine unused 
   *  meshes.
   */
  csDirtyAccessArray<csRenderMesh*>& GetUnusedMeshes (uint frameNumber);

  /**
   * Instruct the mesh holder to throw away all currently used csRenderMesh*
   * arrays, effectively using clean arrays on subsequent calls when an 
   * unused array is needed.
   * \remark The arrays are *not* immediately deleted, this is delayed until
   *  they really aren't used any more (determined by the frame number).
   *  This has to be considered if manual mesh deletion was enabled.
   */
  void Clear();
};

#endif // __CS_CSTOOL_RENDERMESHHOLDER_H__
