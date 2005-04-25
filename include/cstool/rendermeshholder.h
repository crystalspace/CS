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
class CS_CRYSTALSPACE_EXPORT csRenderMeshHolder
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

#endif // __CS_CSTOOL_RENDERMESHHOLDER_H__
