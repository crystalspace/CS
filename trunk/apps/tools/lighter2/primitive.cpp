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

#include "common.h"

#include "primitive.h"
#include "object.h"

#include <algorithm>

namespace lighter
{

  //-------------------------------------------------------------------------

  const csVector3 PrimitiveBase::GetCenter () const
  {
    csVector3 centroid;
    centroid = vertexData->positions[triangle.a];
    centroid += vertexData->positions[triangle.b];
    centroid += vertexData->positions[triangle.c];
    
    return centroid / 3.0f;
  }

  float PrimitiveBase::GetArea () const
  {
    float area = csMath3::DoubleArea3 (
      vertexData->positions[triangle.a],
      vertexData->positions[triangle.b], 
      vertexData->positions[triangle.c]);
    return area/2.0f;
  }

  void PrimitiveBase::GetExtent (uint axis, float &min, float &max) const
  {
    min = FLT_MAX;
    max = -FLT_MAX;
    for (unsigned int i = 0; i < 3; ++i)
    {
      float val = vertexData->positions[triangle.a][axis];
      min = csMin(min, val);
      max = csMax(max, val);
    }
  }

  int PrimitiveBase::Classify (const csPlane3 &plane) const
  {
    size_t i;
    size_t front = 0, back = 0;

    for (i = 0; i < 3; i++)
    {
      float dot = plane.Classify (vertexData->positions[triangle[i]]);
      if (ABS (dot) < EPSILON) dot = 0;
      if (dot > 0)
        back++;
      else if (dot < 0)
        front++;
    }

    if (back == 0 && front == 0) return CS_POL_SAME_PLANE;
    if (back == 0) return CS_POL_FRONT;
    if (front == 0) return CS_POL_BACK;
    return CS_POL_SPLIT_NEEDED;
  }

  void PrimitiveBase::ComputePlane ()
  {
    //Setup a temporary array of our vertices
    const csVector3 vertices[] =
    {
      vertexData->positions[triangle.a],
      vertexData->positions[triangle.b],
      vertexData->positions[triangle.c]
    };

    plane = csPoly3D::ComputePlane (vertices, 3);
  }

  template<typename Array>
  static void ComputeMinMaxUVHelper (const Array& lightmapUVs, 
                                     const PrimitiveBase::TriangleType& triangle,
                                     csVector2 &min, csVector2 &max)
  {
    size_t index = triangle[0];
    min = lightmapUVs[index];
    max = lightmapUVs[index];
    for (uint i = 1; i < 3; ++i)
    {
      index = triangle[i];
      const csVector2 &uv = lightmapUVs[index];
      min.x = csMin (min.x, uv.x);
      min.y = csMin (min.y, uv.y);

      max.x = csMax (max.x, uv.x);
      max.y = csMax (max.y, uv.y);
    }
    max += csVector2(0.5f, 0.5f);
  }

  void PrimitiveBase::ComputeMinMaxUV (const Vector2Array& lightmapUVs,
                                       csVector2 &min, csVector2 &max) const
  {
    ComputeMinMaxUVHelper (lightmapUVs, triangle, min, max);
  }

  void PrimitiveBase::ComputeMinMaxUV (const ObjectVertexData::Vector2Array& lightmapUVs,
                                       csVector2 &min, csVector2 &max) const
  {
    ComputeMinMaxUVHelper (lightmapUVs, triangle, min, max);
  }

  //-------------------------------------------------------------------------

  void PrintElements (const csBitArray& bits, uint uc, uint vc)
  {
    csPrintf("\n");

    for (uint r = 0; r < vc; ++r)
    {
      int rowOffset= r*uc;
      for (uint c = 0; c < uc; ++c)
      {
        bool isSet = bits.IsBitSet (2*(rowOffset+c));
        bool isFull = bits.IsBitSet (2*(rowOffset+c)+1);
        csPrintf (isSet ? isFull ? "x" : "b" : "." );
      }
      csPrintf ("\n");
    }
  }

  struct Scanline
  {
    uint min, max;
  };

  void ScanConvert(Scanline* scanBuf, const csVector2& v1, const csVector2& v2, size_t vc)
  {
    uint fy, ly;

    const csVector2* f;
    const csVector2* l;

    // Skip horizontal edge (for now ;))
    if(v1.y >= v2.y - FLT_EPSILON &&
       v1.y <= v2.y + FLT_EPSILON) 
      return;

    // Increasing Y order
    if (v1.y < v2.y)
    {
      f = &v1;
      l = &v2;
    }
    else
    {
      f = &v2;
      l = &v1;
    }

    fy = (uint)floorf(f->y);
    ly = (uint)floorf(l->y+1);

    // Clip or reject
    if (fy >= vc) 
      return;
    if (ly < 0) 
      return;
    if (ly >= vc) 
      ly = (uint)vc;

    // Calculate slope
    float slopeX, x;
    x = f->x;
    slopeX = (l->x - f->x) / (l->y - f->y);

    if (fy< 0)
    {
      x += slopeX * (0 - fy);
      fy = 0;
    }


    for (uint i = fy; i < ly; ++i)
    {
      if (x < scanBuf[i].min)
        scanBuf[i].min = static_cast<uint>(x);
      if ((x+1) > scanBuf[i].max)
        scanBuf[i].max = static_cast<uint>(x+1);

      x += slopeX;
    }


  }

  void Primitive::Prepare ()
  {
    ComputeBaryCoeffs();
    if (radObject->lightPerVertex) return;

    // Reset current data
    
    // Compute min/max uv
    uint uc, vc;
    int maxu, minu, maxv, minv;
    
    ComputeMinMaxUV (minu, maxu, minv, maxv);

    minUV.x = minu; minUV.y = minv;
    maxUV.x = maxu; maxUV.y = maxv;
    
    ObjectVertexData& vdata = GetVertexData();

    // Min xyz
    csVector2 d = minUV - (vdata.lightmapUVs[triangle.a]);
    
    minCoord = vdata.positions[triangle.a]
      + uFormVector * d.x + vFormVector * d.y;
    
    uc = maxu - minu + 1;
    vc = maxv - minv + 1;

    countU = uc;
    countV = vc;

    elementClassification.SetSize (2*uc*vc);

    // Compute the lm-uv offsets, 
    csVector2 verts[] = {
      vdata.lightmapUVs[triangle.a] - minUV,
      vdata.lightmapUVs[triangle.b] - minUV,
      vdata.lightmapUVs[triangle.c] - minUV
    };  

    CS_ALLOC_STACK_ARRAY(Scanline, scanBuffer, vc);

    // Zero out the scan-buffer
    for (uint i = 0; i < vc; ++i)
    {
      scanBuffer[i].min = INT_MAX;
      scanBuffer[i].max = INT_MIN;
    }

    ScanConvert (scanBuffer, verts[0], verts[1], vc);
    ScanConvert (scanBuffer, verts[1], verts[2], vc);
    ScanConvert (scanBuffer, verts[2], verts[0], vc);

    uint maxRow = (uint)floorf(csMax(verts[0].y, csMax(verts[1].y, verts[2].y))+1);

    for (uint row = 0; row < maxRow; ++row)
    {
      // Get start and ends
      uint min = scanBuffer[row].min;
      uint max = scanBuffer[row].max;

      // Clip
      if ((max < 0) && (min >= uc)) continue;

      if (min < 0)
         min = 0;
      if (max >= uc)
        max = uc;

      size_t rowOffset = row*uc;

      for (uint col = min; col <= max; ++col)
      {
        float area = ComputeElementFraction (rowOffset+col);
        if (area > 0)
        {
          elementClassification.SetBit (2*(rowOffset+col));
          if (area > (1.0f - LITEPSILON))
            elementClassification.SetBit (2*(rowOffset+col)+1);
        }
      }
    }

    //PrintElements (elementClassification, uc, vc);
  }

  size_t Primitive::ComputeElementIndex (const csVector3& pt) const
  {
    size_t u = (size_t)(pt * uFormVector);
    size_t v = (size_t)(pt * vFormVector);

    return v*size_t (maxUV.x - minUV.x + 1) + u;
  }


  csVector3 Primitive::ComputeElementCenter (size_t index) const
  {
    size_t u, v;

    GetElementUV (index, u, v);

    return minCoord + uFormVector * ((float)u+0.5f) + 
      vFormVector * ((float)v+0.5f);
  }

  csVector3 Primitive::ComputeNormal (const csVector3& point) const
  {
    float lambda, my;
    ComputeBaryCoords (point, lambda, my);

    // Clamp lambda/my
    lambda = csClamp (lambda, 1.0f, 0.0f);
    my = csClamp (my, 1.0f, 0.0f);

    csVector3 norm;

    norm = lambda * vertexData->normals[triangle.a] + 
      my * vertexData->normals[triangle.b] + 
      (1 - lambda - my) * vertexData->normals[triangle.c];

    return norm.Unit ();
  }

  csVector2 Primitive::ComputeUV (const csVector3& point) const
  {
    float lambda, my;
    ComputeBaryCoords (point, lambda, my);

    // Clamp lambda/my
    lambda = csClamp (lambda, 1.0f, 0.0f);
    my = csClamp (my, 1.0f, 0.0f);

    csVector2 uv;

    uv = lambda * vertexData->uvs[triangle.a] + 
      my * vertexData->uvs[triangle.b] + 
      (1 - lambda - my) * vertexData->uvs[triangle.c];

    return uv;
  }

  void Primitive::ComputeCustomData (const csVector3& point, 
                                     size_t customData, 
                                     size_t numComps, float* out) const
  {
    float lambda, my;
    ComputeBaryCoords (point, lambda, my);

    // Clamp lambda/my
    lambda = csClamp (lambda, 1.0f, 0.0f);
    my = csClamp (my, 1.0f, 0.0f);

    float* va = vertexData->GetCustomData (triangle.a, customData);
    float* vb = vertexData->GetCustomData (triangle.b, customData);
    float* vc = vertexData->GetCustomData (triangle.c, customData);

    float oneMinusLambdaMy = (1 - lambda - my);
    for (size_t c = 0; c < numComps; c++)
      out[c] = lambda * va[c] + my * vb[c] + oneMinusLambdaMy * vc[c];
  }

  void Primitive::ComputeMinMaxUV (csVector2 &min, csVector2 &max) const
  {
    PrimitiveBase::ComputeMinMaxUV (GetVertexData().lightmapUVs, min, max);
  }

  /* Ok, this is a very strange method.. it is fetched from FSrad
   * and basically computes the world->uv-transform. Check following 
   * explanation from FSrad and see if you understand it better than I do ;) 
   */

  /* ------------------------------------------------------------------------
   * Warning: The following code is bizarre, to say the least. I'll try to 
   * explain it here:
   *
   * A polygon with multiple 'n' coordinates per vertex will exist in 'n' 
   * spaces simultaneously. In other words, a polygon that has a 
   * (1) 3D coordinate per vertex, 
   * (2) a 2D texture coordinate per vertex and 
   * (3) a 2D lightmap coordinate per vertex, will exist
   * in each of these three space simultaneously.
   *
   * Because these three spaces are seemingly arbitrary (chosen by an artist, 
   * for example) there is no immediate way to map from one to the next. For 
   * the following explanation, we are only concerned with two of all possible 
   * spaces: 3-space (the description of the polygon as it exists in 3D) and 
   * 2-space (the description of the polygon as it exists in 2D lightmap space.)
   *
   * For the sake of simplicity, I will refer to these two spaces as "3-space" 
   * and "2-space".
   *
   * Because this code was created for the purpose of lightmapping (at least, 
   * originally) our main focus is the ability to transform from 3-space into 
   * 2-space. In the end, we want this to be as quick as possible, so our 
   * final product will be two 3D vectors.
   *
   * These two vectors represent the direction that you must travel (in 
   * 3-space) in order to follow along each of the two axes in 2-space. These 
   * two vectors can (and should) be given a length that will represent the 
   * distance in 3-space needed to travel one unit in 2-space, along the 
   * associated vector.
   *
   * I hope I've made the academics happy, but for those of you that want it 
   * in plain ol' fashioned English, here's a clearer description: Given a 
   * point in 3-space that maps to the point [15.84, 12.43] in 2-space, if we 
   * add the U-vector (the first of our two 2 vectors) to that 3-space point, 
   * we will end up with a 3-space point that maps to the 2-space coordinate 
   * [16.84, 12.43] (one unit along the U-axis in the positive direction in 
   * textures-space.)
   *
   * Confused? If not, you soon will be, because the method by which we need 
   * to calculate this is rather bizarre. I'll try to be clear about it...
   *
   * Going back to an earlier portion of the text, remember that the polygon 
   * exists in multiple spaces simultaneously. Also note that these spaces are 
   * chosen arbitrarily. Because of this, we actually need to search for the 
   * solution, rather than simply calculate it.
   *
   * It's best to think of the solution (or the search for the solution) in 
   * terms of axes. There are three axes in 3-space and two axes in 2-space. 
   * Our result will be the two (3-space) vectors that point along the 
   * direction of the 2 (2-space) axes. Here's what it looks like:
   *
   *             /\
   *            / |\
   *           /  | \
   *          /   |  \
   *         /    |   \
   *        /     |    \
   *       /____________\_____ V-Vector
   *      /----___|      \
   *              |----___\
   *              |
   *              |
   *              U-Vector
   * 
   * What you're looking at is a 3D polygon as seen from a perpendicular view 
   * to the camera, with the two Vectors (U and V) shown as vertical and 
   * horizontal lines. This is the simplest case, these lines will probably 
   * often be at odd orientations, which is why we must search for them. 
   * Here's how we'll perform that search:
   *
   * 	point3	u1_3, u2_3, v1_3, v2_3;
   * 	point2	u1_2, u2_2, v1_2, v2_2;
   * 	double	maxUDist, maxVDist;
   *
   * 	for (each vertex)
   * 	{
   * 	  for (each edge not sharing that vertex)
   * 	  {
   * 	    // Intercept in U?
   *
   * 	    if (edge UVs form a line that intercepts the U value from the current vertex)
   * 	    {
   * 	      if (interceptDist > maxUDist)
   * 	      {
   * 	        maxUDist = interceptDist
   * 	        u1_3 = current vertex
   * 	        u2_3 = interceptPoint
   * 	        u1_2 = current vertex (UV)
   * 	        u2_2 = interceptPoint (UV)
   * 	      }
   * 	    }
   *
   * 	    // Intercept in V (this code is the same as above, but for the V component)?
   *
   * 	    if (edge UVs form a line that intercepts the V value from the current vertex)
   * 	    {
   * 	      if (interceptDist > maxVDist)
   * 	      {
   * 	        maxVDist = interceptDist
   * 	        v1_3 = current vertex
   * 	        v2_3 = interceptPoint
   * 	        v1_2 = current vertex (UV)
   * 	        v2_2 = interceptPoint (UV)
   * 	      }
   * 	    }
   * 	  }
   * 	}
   *
   * At this point, we have 8 points: four of which are points in 2-space (one 
   * point for each endpoint of a line in 3-space that follows the axis of the 
   * associated 2-space) and the four corresponding 2-space coordinates. From 
   * this, we are able to easily calculate the resulting vectors (see code below.)
   *
   * ---------------------------------------------------------------------------------------------------------------------------------
   */

  void Primitive::ComputeUVTransform ()
  {
    uFormVector.Set (0.0f);
    vFormVector.Set (0.0f);

    // Some temporary holders
    csVector3 u0_3d(0.0f), u1_3d(0.0f), v0_3d(0.0f), v1_3d(0.0f);
    csVector2 u0_2d(0.0f), u1_2d(0.0f), v0_2d(0.0f), v1_2d(0.0f);

    float maxuDist = -1, maxvDist = -1;

    //visit all vertices
    for (uint i = 0; i < 3; i++)
    {
      const csVector3& c3D = GetVertexData().positions[triangle[i]];
      const csVector2& c2D = GetVertexData().lightmapUVs[triangle[i]];

      size_t v0 = 2;

      // Visit all edges
      for (size_t v1 = 0; v1 < 3; v1++)
      {
        // Skip those that includes current vertex
        if (v0 != i && v1 != i)
        {
          const csVector3& v03D = GetVertexData().positions[triangle[v0]];
          const csVector3& v13D = GetVertexData().positions[triangle[v1]];

          const csVector2& v02D = GetVertexData().lightmapUVs[triangle[v0]];
          const csVector2& v12D = GetVertexData().lightmapUVs[triangle[v1]];

          //intercept u
          if ((v02D.x - LITEPSILON <= c2D.x && v12D.x + LITEPSILON >= c2D.x) || 
              (v02D.x + LITEPSILON >= c2D.x && v12D.x - LITEPSILON <= c2D.x))
          {
            float D = (c2D.x - v02D.x) / (v12D.x - v02D.x);
            csVector3 isec3D = v03D + (v13D - v03D) * D;
            csVector2 isec2D = v02D + (v12D - v02D) * D;
            float isecDist3D = (c3D-isec3D).Norm ();
            if (isecDist3D > maxuDist)
            {
              maxuDist = isecDist3D;
              u0_3d = c3D;
              u1_3d = isec3D;
              u0_2d = c2D;
              u1_2d = isec2D;
            }
          }

          //intercept v
          if ((v02D.y - LITEPSILON <= c2D.y && v12D.y + LITEPSILON >= c2D.y) || 
              (v02D.y + LITEPSILON >= c2D.y && v12D.y - LITEPSILON <= c2D.y))
          {
            float D = (c2D.y - v02D.y) / (v12D.y - v02D.y);
            csVector3 isec3D = v03D + (v13D - v03D) * D;
            csVector2 isec2D = v02D + (v12D - v02D) * D;
            float isecDist3D = (c3D-isec3D).Norm ();
            if (isecDist3D > maxvDist)
            {
              maxvDist = isecDist3D;
              v0_3d = c3D;
              v1_3d = isec3D;
              v0_2d = c2D;
              v1_2d = isec2D;
            }
          }
        }
        // Next
        v0 = v1;
      }
    }

    // Calc vectors
    if (maxuDist > 0) vFormVector = (u1_3d - u0_3d) / (u1_2d.y - u0_2d.y);
    if (maxvDist > 0) uFormVector = (v1_3d - v0_3d) / (v1_2d.x - v0_2d.x);
  }

  bool Primitive::PointInside (const csVector3& pt) const
  {
    float lambda, my;
    ComputeBaryCoords (pt, lambda, my);
    return (lambda >= 0.0f && my >= 0.0f && (lambda + my) < 1.0f);
  }

  void Primitive::ComputeBaryCoords (const csVector3& v, float& lambda, 
                                     float& my) const
  {
    csVector3 thirdPosToV = 
      vertexData->positions[triangle.c] - v;

    lambda = (lambdaCoeffTV * thirdPosToV);
    my = (myCoeffTV * thirdPosToV);
  }

  ElementProxy Primitive::GetElement (size_t index)
  {
    return ElementProxy(*this, index);
  }

  ElementProxy Primitive::GetElement (const csVector3& pt)
  {
    return GetElement (ComputeElementIndex (pt));
  }

  void Primitive::ComputeBaryCoeffs ()
  {
    const csVector3& v1 = vertexData->positions[triangle.a];
    const csVector3& v2 = vertexData->positions[triangle.b];
    const csVector3& v3 = vertexData->positions[triangle.c];

    csVector3 diff1_3 = v1 - v3;
    csVector3 diff2_3 = v2 - v3;

    /* In the divisor, one of both diff1_3 and diff2_3 .x, .y. or .z appear.
     * Pick one where the two are not both zero (can happen with coplanar
     * tris.) */
    int dividingCoord = 0;
    while ((fabsf (diff1_3[dividingCoord]) < SMALL_EPSILON)
      && (fabsf (diff2_3[dividingCoord]) < SMALL_EPSILON)
      && (dividingCoord < 2))
      dividingCoord++;

    float coeff1_3, coeff2_3;
    switch (dividingCoord)
    {
      case 0:
        {
          coeff1_3 = diff2_3[1] + diff2_3[2];
          coeff2_3 = diff1_3[1] + diff1_3[2];
          float lambda_factor = 1.0f / 
            (diff1_3[0]*coeff1_3 - diff2_3[0]*coeff2_3);
          float my_factor = 1.0f / 
            (diff2_3[0]*coeff2_3 - diff1_3[0]*coeff1_3);
          lambdaCoeffTV.Set (
            -coeff1_3 * lambda_factor, 
            diff2_3[0]*lambda_factor, 
            diff2_3[0]*lambda_factor);
          myCoeffTV.Set (
            -coeff2_3 * my_factor, 
            diff1_3[0]*my_factor, 
            diff1_3[0]*my_factor);
        }
        break;
      case 1:
        {
          coeff1_3 = diff2_3[0] + diff2_3[2];
          coeff2_3 = diff1_3[0] + diff1_3[2];
          float lambda_factor = 1.0f / 
            (diff1_3[1]*coeff1_3 - diff2_3[1]*coeff2_3);
          float my_factor = 1.0f / 
            (diff2_3[1]*coeff2_3 - diff1_3[1]*coeff1_3);
          lambdaCoeffTV.Set (
            diff2_3[1]*lambda_factor, 
            -coeff1_3 * lambda_factor, 
            diff2_3[1]*lambda_factor);
          myCoeffTV.Set (
            diff1_3[1]*my_factor, 
            -coeff2_3 * my_factor, 
            diff1_3[1]*my_factor);
        }
        break;
      case 2:
        {
          coeff1_3 = diff2_3[0] + diff2_3[1];
          coeff2_3 = diff1_3[0] + diff1_3[1];
          float lambda_factor = 1.0f / 
            (diff1_3[2]*coeff1_3 - diff2_3[2]*coeff2_3);
          float my_factor = 1.0f / 
            (diff2_3[2]*coeff2_3 - diff1_3[2]*coeff1_3);
          lambdaCoeffTV.Set (
            diff2_3[2]*lambda_factor, 
            diff2_3[2]*lambda_factor, 
            -coeff1_3 * lambda_factor);
          myCoeffTV.Set (
            diff1_3[2]*my_factor, 
            diff1_3[2]*my_factor, 
            -coeff2_3 * my_factor);
        }
        break;
    }
  }

  uint Primitive::RecomputeQuadrantOffset (size_t element, 
    const csVector2 inOffsets[4], csVector2 offsets[4]) const
  {
    memcpy (offsets, inOffsets, sizeof (csVector2) * 4);

    const ObjectVertexData& vdata = GetVertexData ();

    uint clipMask = 0;

    size_t u, v;
    GetElementUV(element, u, v);

    //PrintElements (elementClassification, countU, countV);
    float uvArea = csMath2::Area2 (vdata.lightmapUVs[triangle.a],
      vdata.lightmapUVs[triangle.b], vdata.lightmapUVs[triangle.c]);

    csVector2 elementCenter = minUV + csVector2(u+0.5f, v+0.5f);

    csVector2 edgeD[3];
    csVector2 edgeN[3];
    for (size_t e1 = 0; e1 < 3; ++e1)
    {
      size_t e2 = CS::Math::NextModulo3 (e1);

      csVector2 uv1 = vdata.lightmapUVs[triangle[e1]];
      csVector2 uv2 = vdata.lightmapUVs[triangle[e2]];

      // Possible violating edge        
      edgeD[e1] = uv2-uv1;
      edgeN[e1].Set (edgeD[e1].y, -edgeD[e1].x); 
      edgeN[e1] /= edgeN[e1].Norm();
    }
    
    for (size_t i = 0; i < 4; ++i)
    {
      csVector2 absOffset;
      absOffset = offsets[i] + elementCenter;

      size_t endOfLoop = 3;
      // Traverse the triangle edges, clip the offsets to the edge
      for (size_t e = 0; e < endOfLoop; ++e)
      {
        size_t e1 = e % 3;
        csVector2 uv1 = vdata.lightmapUVs[triangle[e1]];

        // Compute edge-point distance
        csVector2 pointUV1Offset = absOffset - uv1;  
        float dist = edgeN[e1] * pointUV1Offset;        

        if ((fabsf (dist) > EPSILON) && (dist*uvArea > 0))
        {
          clipMask |= (1 << i);
          float denom = edgeN[e1] * edgeN[e1];
          
          csVector2 lineOffset = edgeN[e1] * (dist / denom)*(1.0f + EPSILON);

          /* Make sure the point is not just on the edge, but also on the
             triangle */
          const float nd = (edgeD[e1].Norm());
          const float f = ((pointUV1Offset - lineOffset) * edgeD[e1]) / (nd * nd);
          if (f < LITEPSILON)
          {
            lineOffset -= edgeD[e1] * (-f + EPSILON);
          }
          else if (f > 1.0f-LITEPSILON)
          {
            lineOffset -= edgeD[e1] * (1.0f-f - EPSILON);
          }

          offsets[i] -= lineOffset;
          absOffset -= lineOffset;
        }
      }
    }

    return clipMask;
  }

  float Primitive::ComputeElementFraction (size_t index) const
  {
    const ObjectVertexData& vdata = GetVertexData ();

    size_t u, v;
    GetElementUV(index, u, v);

    csBox2 uvBox (u + minUV.x, v + minUV.y,
                  u + minUV.x + 1, v + minUV.y + 1);

    csVector2 verts[3] = {
      vdata.lightmapUVs[triangle[0]],
      vdata.lightmapUVs[triangle[1]],
      vdata.lightmapUVs[triangle[2]]
    };

    csVector2 outVerts[7];

    size_t numVert = 0;

    csBoxClipper clipper (uvBox);
    clipper.Clip(verts, 3, outVerts, numVert);

    float area = 0.0;
    if (numVert >= 3)
    {
      // triangulate the polygon, triangles are (0,1,2), (0,2,3), (0,3,4), etc..
      for (size_t i = 0; i < numVert - 2; ++i)
        area += fabsf(csMath2::Area2 (outVerts[0], outVerts[i + 1], outVerts[i + 2]));
    }
    
    return area / 2.0f;
  }
}


