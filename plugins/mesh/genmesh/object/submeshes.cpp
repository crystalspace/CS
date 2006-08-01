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

#include "cssysdef.h"

#include "submeshes.h"

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{
  static int SubmeshSubmeshCompare (const SubMesh& A, const SubMesh& B)
  {
    const char* a = A.GetName();
    const char* b = B.GetName();
    if (a == 0) return (b == 0) ? 0 : 1;
    if (b == 0) return -1;
    return strcmp (a, b);
  }

  bool SubMeshesContainer::AddSubMesh (iRenderBuffer* indices, 
    iMaterialWrapper *material, const char* name, uint mixmode)
  {
    SubMesh subMesh;
    subMesh.material = material;
    subMesh.MixMode = mixmode;
    subMesh.index_buffer = indices;
    subMesh.SetName (name);

    subMesh.bufferHolder.AttachNew (new csRenderBufferHolder);
    subMesh.bufferHolder->SetRenderBuffer(CS_BUFFER_INDEX,
      indices);

    subMeshes.InsertSorted (subMesh, SubmeshSubmeshCompare);
    /* @@@ FIXME: Prolly do some error checking, like sanity of
     * indices */
    return true;
  }

  static int SubmeshStringCompare (const SubMesh& A, const char* const& b)
  {
    const char* a = A.GetName();
    if (a == 0) return (b == 0) ? 0 : 1;
    if (b == 0) return -1;
    return strcmp (a, b);
  }

  size_t SubMeshesContainer::FindSubMesh (const char* name) const
  {
    return subMeshes.FindSortedKey (
      csArrayCmp<SubMesh, const char*> (name, &SubmeshStringCompare));
  }
}
CS_PLUGIN_NAMESPACE_END(Genmesh)
