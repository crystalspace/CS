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
#include "csutil/set.h"



namespace lighter
{
  class RadObject;
  struct RadObjectVertexData;

  /**
  * Primitive in the radiosity world.
  * Represents a single primitive (face) in the radiosity world.
  */
  class RadPrimitive
  {
  public:
    // Constructors
    inline RadPrimitive (RadObjectVertexData &dataHolder) 
      : vertexData(dataHolder),
      uFormVector (0), vFormVector (0), illuminationColor (0,0,0),
      reflectanceColor (1.0f,1.0f,1.0f), uPatches (0), vPatches (0), minCoord (0),
      minUV (0,0), maxUV (0,0), originalPrim (0), radObject (0), lightmapID (0)
    {
    }

    ///Split primitive into two. Front side is kept in current
    bool Split (const csPlane3& plane, RadPrimitive &back);

    /// Calculate min-max UV-coords
    void ComputeMinMaxUV (csVector2 &min, csVector2 &max) const;

    /// Calculate min-max UV-coords as integers
    inline void ComputeMinMaxUV (int &minU, int &maxU, int &minV, int &maxV) const
    {
      csVector2 min, max;
      ComputeMinMaxUV (min, max);
      minU = (int)floor (min.x);
      minV = (int)floor (min.y);
      maxU = (int)floor (max.x);
      maxV = (int)floor (max.y);
    }

    /// Fix down the min/max to ints..
/*    inline void QuantizeUVs ()
    {
      //TODO Move to per object
      for (uint i = 0; i < lightmapUVs.GetSize (); i++)
      {
        csVector2 &uv = lightmapUVs[i];
        uv.x = int(uv.x+0.5);
        uv.y = int(uv.y+0.5);
      }
    }*/

    /// Remap (in linear fashion) the UVs
    void RemapUVs (csVector2 &move);

    /// Calculate and save primitive plane
    void ComputePlane ();

    /// Calculate center
    const csVector3 GetCenter () const;

    /// Calculate area, forwarder
    float GetArea () const;

    /// Get min/max extent in given dimension
    void GetExtent (uint dimension, float &min, float &max) const;

    /// Calculate the u/v form vectors
    void ComputeUVTransform ();

    /// Prepare the primitive, create patches and elements
    void Prepare (uint uResolution, uint vResolution);

    /// Prepare the primitive wihtout creating new patches (just make sure elements are right)
    void PrepareNoPatches ();

    /// Build a 
    csPoly3D BuildPoly3D () const;

    /// Classify with respect to another plane
    int Classify (const csPlane3 &plane) const;

    /// Return triangles (triangulated version)
    csArray<csTriangle> BuildTriangulated() const;

    // Data accessors
    /*
    inline Vector3DArray& GetVertices () { return vertices; }
    inline const Vector3DArray& GetVertices () const { return vertices; }

    inline Vector2DArray& GetUVs () { return lightmapUVs; }
    inline const Vector2DArray& GetUVs () const { return lightmapUVs; }

    inline IntDArray& GetExtraData () { return extraData; }
    inline const IntDArray& GetExtraData () const { return extraData; }
*/
    inline SizeTDArray& GetIndexArray () { return indexArray; }
    inline const SizeTDArray& GetIndexArray () const { return indexArray; }

    inline RadObjectVertexData& GetVertexData () { return vertexData; }
    inline const RadObjectVertexData& GetVertexData () const { return vertexData; }

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
    //Vector2DArray lightmapUVs;

    // Extra per-vertex data (meshtype specific)
    //IntDArray extraData;

    /// Vertex data holder
    RadObjectVertexData& vertexData;

    /// Index array for this primitive
    SizeTDArray indexArray;

    /// Computed plane
    csPlane3 plane;

    /// Mapping vectors
    csVector3 uFormVector;
    csVector3 vFormVector;

    /// Color
    csColor illuminationColor;
    csColor reflectanceColor;

    /// Elements
    FloatDArray elementAreas;

    /// Patches
    RadPatchArray patches;
    uint uPatches;
    uint vPatches;

    /// Min/max data
    csVector3 minCoord;
    csVector2 minUV;
    csVector2 maxUV;

    /// Pointer to unchanged prim
    RadPrimitive *originalPrim;

    /// Original object
    RadObject* radObject;

    /// Lightmap id
    uint lightmapID;
  };

  typedef csArray<RadPrimitive> RadPrimitiveArray;
  typedef csArray<RadPrimitive*> RadPrimitivePtrArray;
  typedef csSet<RadPrimitive*> RadPrimitivePtrSet;
}

template<>
class csHashComputer<lighter::RadPrimitive*> : public csHashComputerIntegral<lighter::RadPrimitive*> {};

#endif
