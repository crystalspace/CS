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
    csArray<size_t> splits;

    size_t SplitVertex (size_t oldIndex)
    {
      size_t ret = ObjectBaseVertexData::SplitVertex (oldIndex);
      splits.Push (oldIndex);
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

    ObjectVertexData& operator= (const ObjectBaseVertexData& other)
    {
      ObjectBaseVertexData::operator= (other);
      lightmapUVs.SetSize (positions.GetSize());
      return *this;
    }
  };
}

#endif // __VERTEXDATA_H__
