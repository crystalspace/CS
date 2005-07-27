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

#include "csextern.h"

#include "csgeom/vector3.h"
#include "csutil/parray.h"

#include "ivideo/graph3d.h"

struct iEngine;
struct iMeshWrapper;
struct iObjectRegistry;
struct iRenderView;

struct csRenderMesh;


/**
 * This class is used when we need to store, sort and then render a list of
 * rendermeshes.
 */
class CS_CRYSTALSPACE_EXPORT csRenderMeshList
{
public:
  /**
   * Constructor. 
   * It needs objectregistry becouse the meshsorter needs to get the 
   * renderpriorities sortingoptions from the engine.
   */
  csRenderMeshList (iEngine* engine);
  
  /**
   * Destructor. Clean up the list
   */
  ~csRenderMeshList ();

  /**
   * Add a new set of rendermeshes to the lists
   */
  void AddRenderMeshes (csRenderMesh** meshes, int num, long renderPriority,
	csZBufMode z_buf_mode, iMeshWrapper* mesh);

  /**
   * Sort the list of meshes by render priority and within every render
   * priority. Return number of total meshes.
   */
  size_t SortMeshLists (iRenderView *rview);

  /**
   * After sorting the meshes fetch them with this function.
   */
  void GetSortedMeshes (csRenderMesh** meshes, iMeshWrapper** imeshes);

  /**
   * Empty the meshlist. It will still hold the list of renderpriorities.
   */
  void Empty ();

private:
  struct meshListEntry
  {
    csRenderMesh* rm;
    iMeshWrapper* mesh;

    meshListEntry (csRenderMesh* mesh, iMeshWrapper* imesh) 
      : rm(mesh), mesh(imesh) {}
  };

  /// This struct contains one entry in the RP infoqueue
  struct renderMeshListInfo 
  {
    /// RP number
    long renderPriority;

    /// Sorting options
    int sortingOption;

    /// list of rendermeshes
    csArray<meshListEntry> meshList;

  };

  csPDelArray < renderMeshListInfo > renderList;
  iEngine* engine;

  static int SortMeshMaterial (meshListEntry const& me1, meshListEntry const& me2);
  static int SortMeshBack2Front (meshListEntry const& me1, meshListEntry const& me2);
  static int SortMeshFront2Back (meshListEntry const& me1, meshListEntry const& me2);
  static csVector3 sort_CameraPosition;
};

#endif //__CS_RENDERMESHLIST_H__
