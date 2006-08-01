/*
  Copyright (C) 2006 by Jorrit Tyberghein
            (C) 2006 by Frank Richter

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

#ifndef __CS_SUBMESHES_H__
#define __CS_SUBMESHES_H__

#include "cstool/rendermeshholder.h"
#include "csutil/ref.h"
#include "csutil/util.h"

#include "iengine/material.h"
#include "ivideo/rndbuf.h"

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{
  struct SubMesh
  {
  protected:
    const char* name;
  public:
    csRef<iRenderBuffer> index_buffer;
    csRef<iMaterialWrapper> material;
    csRenderMeshHolder rmHolder;
    csRef<csRenderBufferHolder> bufferHolder;

    // Override mixmode from parent.
    uint MixMode;

    SubMesh () : name (0)
    { }
    ~SubMesh()
    {
      delete[] name;
    }
    const char* GetName() const { return name; }
    void SetName (const char* newName)
    {
      delete[] name;
      name = csStrNew (newName);
    }
  };

  class SubMeshesContainer
  {
    csArray<SubMesh> subMeshes;
  public:
    void ClearSubMeshes ()
    { subMeshes.DeleteAll(); }
    bool AddSubMesh (iRenderBuffer* indices, iMaterialWrapper *material, 
      const char* name, uint mixmode = (uint)~0);
    size_t FindSubMesh (const char* name) const;
    void DeleteSubMesh (size_t index)
    { subMeshes.DeleteIndex (index); }
    size_t GetSubMeshCount () const
    { return subMeshes.GetSize(); }
    iRenderBuffer* GetSubMeshIndices (size_t index) const
    { return (index < subMeshes.GetSize()) ? subMeshes[index].index_buffer : 0; }
    iMaterialWrapper* GetSubMeshMaterial (size_t index) const
    { return (index < subMeshes.GetSize()) ? subMeshes[index].material : 0; }
    const char* GetSubMeshName (size_t index) const
    { return (index < subMeshes.GetSize()) ? subMeshes[index].GetName() : 0; }
    uint GetSubMeshMixmode (size_t index) const
    { return (index < subMeshes.GetSize()) ? subMeshes[index].MixMode : (uint)~0; }

    size_t GetSize() const
    { return subMeshes.GetSize(); }
    SubMesh& operator[](size_t index)
    { return subMeshes[index]; }
  };
  
}
CS_PLUGIN_NAMESPACE_END(Genmesh)

#endif // __CS_SUBMESHES_H__
