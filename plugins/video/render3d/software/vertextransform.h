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

#ifndef __CS_VERTEXTRANSFORM_H__
#define __CS_VERTEXTRANSFORM_H__

#include "sft3dcom.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{

  class VertexTransform : public iVertexTransform
  {
    csRef<csRenderBuffer> clipBuffers[activeBufferCount];
    csDirtyAccessArray<float> clippedData[activeBufferCount];
    csDirtyAccessArray<iRenderBuffer*> bufferPtrs;
    csDirtyAccessArray<csTriangle> trisArray;

    /// Current near plane.
    csPlane3 near_plane;
    /// Is near plane used?
    bool do_near_plane;
  public:
    VertexTransform();

    virtual void SetNearPlane (const csPlane3& near_plane)
    {
      this->near_plane = near_plane;
      do_near_plane = true;
    }
    virtual void DisableNearPlane ()
    {
      do_near_plane = false;
    }

    virtual void TransformVertices (const csReversibleTransform& object2world,
      const csReversibleTransform& world2cam, iRenderBuffer* inIndices,
      csRenderMeshType meshType, const VerticesLTN& inBuffers,
      size_t inIndexStart, size_t inIndexEnd,
      csTriangle*& outTriangles, size_t& outTrianglesCount,
      VerticesLTN& outBuffers, size_t& rangeStart, size_t& rangeEnd);
  };

}
CS_PLUGIN_NAMESPACE_END(Soft3D)

#endif // __CS_VERTEXTRANSFORM_H__
