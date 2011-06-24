/*
  Copyright (C) 2008 by Frank Richter

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

#ifndef __MORPHTARGET_H__
#define __MORPHTARGET_H__

#include "imesh/animesh.h"

#include "csutil/csstring.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakref.h"

CS_PLUGIN_NAMESPACE_BEGIN(Animesh)
{
  class AnimeshObject;
  class AnimeshObjectFactory;

  class MorphTarget :
    public scfImplementation1<MorphTarget,
			      CS::Mesh::iAnimatedMeshMorphTarget>
  {
    friend class AnimeshObject;

    csWeakRef<AnimeshObjectFactory> parent;
    csRef<iRenderBuffer> offsets;
    csString name;
    csArray<size_t> subsetList;

  public:
    MorphTarget (AnimeshObjectFactory* parent, const char* name);

    bool SetVertexOffsets (iRenderBuffer* renderBuffer);
    iRenderBuffer* GetVertexOffsets () { return offsets; }
    void Invalidate ();
    const char* GetName() const { return name; }
    void AddSubset (const size_t subset);
    size_t GetSubset (const size_t index) const;
    size_t GetSubsetCount () const;
  };
}
CS_PLUGIN_NAMESPACE_END(Animesh)

#endif // __MORPHTARGET_H__
