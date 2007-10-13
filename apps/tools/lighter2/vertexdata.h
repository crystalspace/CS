/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __VERTEXDATA_H__
#define __VERTEXDATA_H__

namespace lighter
{
  /**
   * Hold per object vertex data
   */
  struct ObjectBaseVertexData
  {
    typedef csDirtyAccessArray<csVector2> Vector2Array;
    typedef csDirtyAccessArray<csVector3> Vector3Array;
    Vector3Array positions;
    Vector3Array normals;
    Vector2Array uvs;

    typedef csDirtyAccessArray<float> FloatArray;
    size_t customDataTotalComp;
    FloatArray customData;

    ObjectBaseVertexData() : customDataTotalComp (0) {}

    size_t AddCustomData (size_t numComps)
    {
      CS_ASSERT(positions.GetSize() == 0);
      size_t r = customDataTotalComp;
      customDataTotalComp += numComps;
      return r;
    }

    void ResizeCustomData ()
    {
      customData.SetSize (customDataTotalComp * positions.GetSize());
    }

    float* GetCustomData (size_t vert, size_t comp)
    {
      return customData.GetArray() + (vert * customDataTotalComp) + comp;
    }

    const float* GetCustomData (size_t vert, size_t comp) const
    {
      return customData.GetArray() + (vert * customDataTotalComp) + comp;
    }

    //Helper functions

    /// Split one vertex, duplicate it and return index for new one
    size_t SplitVertex (size_t oldIndex)
    {
      size_t index = positions.Push (positions[oldIndex]);
      normals.Push (normals[oldIndex]);
      uvs.Push (uvs[oldIndex]);
      
      customData.SetSize (customData.GetSize() + customDataTotalComp);
      const float* cds = GetCustomData (oldIndex, 0);
      float* cdd = GetCustomData (index, 0);
      for (size_t c = 0; c < customDataTotalComp; c++)
        *cdd++ = *cds++;

      return index;
    }

    /// Interpolate between two vertices with t
    size_t InterpolateVertex (size_t i0, size_t i1, float t)
    {
      const csVector3& p0 = positions[i0];
      const csVector3& p1 = positions[i1];
      size_t index = positions.Push (p0 - (p1 - p0) * t);

      const csVector3& n0 = normals[i0];
      const csVector3& n1 = normals[i1];
      csVector3 newNormal (n0 - (n1 - n0) * t);
      newNormal.Normalize ();
      normals.Push (newNormal);

      const csVector2& uv0 = uvs[i0];
      const csVector2& uv1 = uvs[i1];
      uvs.Push (csLerp (uv0, uv1, t));

      customData.SetSize (customData.GetSize() + customDataTotalComp);
      const float* cds1 = GetCustomData (i0, 0);
      const float* cds2 = GetCustomData (i1, 0);
      float* cdd = GetCustomData (index, 0);
      for (size_t c = 0; c < customDataTotalComp; c++)
      {
        *cdd++ = csLerp (*cds1++, *cds2++, t);
      }

      return index;
    }

    /// Transform all vertex positions and normal
    void Transform (const csReversibleTransform& transform)
    {
      for(size_t i = 0; i < positions.GetSize (); ++i)
      {
        positions[i] = transform.This2Other (positions[i]);
        normals[i] = transform.This2OtherRelative (normals[i]);
      }
    }
  };

  struct ObjectFactoryVertexData : public ObjectBaseVertexData
  {
    struct SplitInfo
    {
      size_t i0, i1;
      float t;

      SplitInfo (size_t i0, size_t i1, float t) : i0 (i0), i1 (i1),t (t) {}
    };
    csArray<SplitInfo> splits;

    size_t SplitVertex (size_t oldIndex)
    {
      size_t ret = ObjectBaseVertexData::SplitVertex (oldIndex);
      splits.Push (SplitInfo (oldIndex, (size_t)~0, 0));
      return ret;
    }
    size_t InterpolateVertex (size_t i0, size_t i1, float t)
    {
      size_t ret = ObjectBaseVertexData::InterpolateVertex (i0, i1, t);
      splits.Push (SplitInfo (i0, i1, t));
      return ret;
    }
  };

  struct ObjectVertexData : public ObjectBaseVertexData
  {
    Vector2Array lightmapUVs;

    size_t SplitVertex (size_t oldIndex)
    {
      ObjectBaseVertexData::SplitVertex (oldIndex);
      return lightmapUVs.Push (lightmapUVs[oldIndex]);
    }

    size_t InterpolateVertex (size_t i0, size_t i1, float t)
    {
      ObjectBaseVertexData::InterpolateVertex (i0, i1, t);
      const csVector2& l0 = lightmapUVs[i0];
      const csVector2& l1 = lightmapUVs[i1];
      return lightmapUVs.Push (l0 - (l1 - l0) * t);
    }

    ObjectVertexData& operator= (const ObjectBaseVertexData& other)
    {
      ObjectBaseVertexData::operator= (other);
      lightmapUVs.SetSize (positions.GetSize());
      return *this;
    }
  };
}

#endif // __VERTEXDATA_H__
