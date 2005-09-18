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

#ifndef __RADPRIMITIVE_H__
#define __RADPRIMITIVE_H__

#include "common.h"
#include "radpatch.h"


namespace lighter
{
  class RadObject;

  /**
  * Primitive in the radiosity world.
  * Represents a single primitive (face) in the radiosity world.
  */
  class RadPrimitive : public csPoly3D
  {
  public:
    // Constructors
    inline RadPrimitive () 
      : uFormVector (0), vFormVector (0), illuminationColor (0,0,0),
      reflectanceColor (0,0,0), uPatches (0), vPatches (0), minCoord (0),
      minUV (0,0), maxUV (0,0), originalPrim (0), radObject (0), lightmapID (0)
    {
    }

    // Transform the primitive with given transform
    inline void Transform (const csReversibleTransform& trans)
    {
      for (unsigned int i = 0; i < vertices.GetSize (); i++)
      {
        vertices[i] = trans.This2Other (vertices[i]);
      }
    }

    // Split primitive into two. Front side is kept in current
    bool Split (const csPlane3& plane, RadPrimitive &back);

    // Calculate min-max UV-coords
    inline void ComputeMinMaxUV (csVector2 &min, csVector2 &max) const
    {
      min = lightmapUVs[0];
      max = lightmapUVs[0];
      for (uint i = 1; i < lightmapUVs.GetSize (); i++)
      {
        const csVector2 &uv = lightmapUVs[i];
        min.x = csMin (min.x, uv.x);
        min.y = csMin (min.y, uv.y);

        max.x = csMax (max.x, uv.x);
        max.y = csMax (max.y, uv.y);
      }
    }

    // Calculate min-max UV-coords as integers
    inline void ComputeMinMaxUV (int &minU, int &maxU, int &minV, int &maxV) const
    {
      csVector2 min, max;
      ComputeMinMaxUV (min, max);
      minU = (int)floor (min.x);
      minV = (int)floor (min.y);
      maxU = (int)floor (max.x);
      maxV = (int)floor (max.y);
    }

    // Remap (in linear fashion) the UVs
    inline void RemapUVs (csVector2 &move)
    {
      for (uint i = 0; i < lightmapUVs.GetSize (); i++)
      {
        csVector2 &uv = lightmapUVs[i];
        uv += move;
      }
    }

    // Renormalize UVs given picture size
    inline void RenormalizeUVs (int uSize, int vSize)
    {
      for (uint i = 0; i < lightmapUVs.GetSize (); i++)
      {
        csVector2 &uv = lightmapUVs[i];
        uv.x /= (float)uSize;
        uv.y /= (float)vSize;
      }
    }

    // Calculate and save primitive plane
    inline void ComputePlane ()
    {
      plane = csPoly3D::ComputePlane ();
    }

    // Calculate center
    const csVector3 GetCenter () const
    {
      return csPoly3D::GetCenter ();
    }

    // Calculate area, forwarder
    inline float GetArea () const
    {
      return csPoly3D::GetArea ();
    }

    // Get min/max extent in given dimension
    inline void GetExtent (uint dimension, float &min, float &max) const
    {
      min = FLT_MAX;
      max = -FLT_MAX;
      for (unsigned int i = 0; i < vertices.GetSize (); i++)
      {
        float val = vertices[i][dimension];
        min = csMin(min, val);
        max = csMax(max, val);
      }
    }

    // Set 3d->2d mapping scales
    void SetLightmapMapping (float uScale = 1.0f, float vScale = 1.0f);

    // Calculate the u/v form vectors
    void ComputeUVTransform ();

    // Prepare the primitive, create patches and elements
    void Prepare (uint uResolution, uint vResolution);

    // Prepare the primitive wihtout creating new patches (just make sure elements are right)
    void PrepareNoPatches ();

    // Data accessors
    inline Vector3DArray& GetVertices () { return vertices; }
    inline const Vector3DArray& GetVertices () const { return vertices; }

    inline Vector2DArray& GetUVs () { return lightmapUVs; }
    inline const Vector2DArray& GetUVs () const { return lightmapUVs; }

    inline IntDArray& GetExtraData () { return extraData; }
    inline const IntDArray& GetExtraData () const { return extraData; }

    inline const csPlane3& GetPlane () const { return plane; }

    inline const csVector3& GetuFormVector () const { return uFormVector; }
    inline const csVector3& GetvFormVector () const { return vFormVector; }

    inline const csColor& GetIlluminationColor () const { return illuminationColor; }
    inline const csColor& GetReflectanceColor () const { return reflectanceColor; }

    inline const FloatDArray& GetElementAreas () const { return elementAreas; }

    inline const RadPatchArray& GetPatches () const { return patches; }
    inline RadPatchArray& GetPatches () { return patches; }
    inline uint GetuPatches () const { return uPatches; }
    inline uint GetvPatches () const { return vPatches; }

    inline const csVector3& GetMinCoord () const { return minCoord; }
    inline const csVector2& GetMinUV () const { return minUV; }
    inline const csVector2& GetMaxUV () const { return maxUV; }

    inline const RadPrimitive* GetOriginalPrimitive () const { return originalPrim; }
    inline void SetOriginalPrimitive (RadPrimitive *p) { originalPrim = p; }

    inline const uint GetLightmapID () const { return lightmapID; }
    inline void SetLightmapID (uint id) { lightmapID = id; }

    inline const RadObject* GetRadObject () const { return radObject; }
    inline RadObject* GetRadObject () { return radObject; }
    inline void SetRadObject (RadObject *obj) { radObject = obj; }

  protected:
    // Lightmap texture coordinates
    Vector2DArray lightmapUVs;

    // Extra per-vertex data (meshtype specific)
    IntDArray extraData;

    // Computed plane
    csPlane3 plane;

    // Mapping vectors
    csVector3 uFormVector;
    csVector3 vFormVector;

    // Color
    csColor illuminationColor;
    csColor reflectanceColor;

    // Elements
    FloatDArray elementAreas;

    // Patches
    RadPatchArray patches;
    uint uPatches;
    uint vPatches;

    // Min/max data
    csVector3 minCoord;
    csVector2 minUV;
    csVector2 maxUV;

    // Pointer to unchanged prim
    RadPrimitive *originalPrim;

    // Original object
    RadObject* radObject;

    // Lightmap id
    uint lightmapID;
  };

  typedef csArray<RadPrimitive> RadPrimitiveArray;
  typedef csArray<RadPrimitive*> RadPrimitivePtrArray;
}

#endif
