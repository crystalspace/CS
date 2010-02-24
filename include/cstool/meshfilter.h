/*
    Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CSTOOL_MESHFILTER_H__
#define __CSTOOL_MESHFILTER_H__

#include "csutil/set.h"
#include "csutil/ref.h"
#include "iengine/mesh.h"

namespace CS
{
namespace Utility
{

  enum MeshFilterMode
  {
    MESH_FILTER_EXCLUDE,
    MESH_FILTER_INCLUDE
  };

  class CS_CRYSTALSPACE_EXPORT MeshFilter
  {
  public:
    MeshFilter();
    ~MeshFilter();
    
    void AddFilterMesh (iMeshWrapper* mesh, bool addChildren=false);
    void RemoveFilterMesh (iMeshWrapper* mesh);

    bool IsMeshFiltered (iMeshWrapper* mesh) const;

    void SetFilterMode (MeshFilterMode mode)
    {
      filterMode = mode;
    }

    MeshFilterMode GetFilterMode () const
    {
      return filterMode;
    }

    void Clear ();

  private:
    MeshFilterMode filterMode;
    // FIXME: csRef<> prolly not a good idea
    csSet<csRef<iMeshWrapper> > filteredMeshes;
  };

}
}

#endif
