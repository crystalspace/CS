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

#ifndef __CS_RENDERMESHLIST_H__
#define __CS_RENDERMESHLIST_H__

#include "csutil/array.h"
#include "csutil/parray.h"
#include "csutil/ref.h"

struct iEngine;
struct iObjectRegistry;
class csRenderMesh;

/**
 * This class is used when we need to store, sort and then render a list of
 * rendermeshes. The meshes will be sorted according to shader on pointervalue
 */
class csRenderMeshList
{
public:
  /**
   * Constructor. 
   * It needs objectregistry becouse the meshsorter needs to get the 
   * renderpriorities sortingoptions from the engine.
   */
  csRenderMeshList (iObjectRegistry *objreg);
  
  /**
   * Destructor. Clean up the list
   */
  ~csRenderMeshList ();

  /**
   * Add a new set of rendermeshes to the lists
   */
  void AddRenderMeshes (csRenderMesh** meshes, int num, long renderPriority);

  /**
   * Get a sorted list of meshes, sorted by renderpriority, and if needed
   * do a sort within the RP. The array should be deleted by delete [] when
   * you are finished with it.
   */
  csRenderMesh** GetSortedMeshList (int &n);

private:
  /// This struct contains one entry in the RP infoqueue
  struct renderMeshListInfo 
  {
    /// RP number
    long renderPriority;

    /// Sorting options
    int sortingOption;

    /// list of rendermeshes
    csArray <csRenderMesh*> meshList;

  };

  /// Function used when sorting renderList
  static int CompareMeshListInfo (void const* item1, 
    void const* item2);

  csPDelArray < renderMeshListInfo > renderList;
  csRef<iEngine> engine;
};

#endif //__CS_RENDERMESHLIST_H__
