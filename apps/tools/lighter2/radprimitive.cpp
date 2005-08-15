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

#include "crystalspace.h"
#include "radprimitive.h"

namespace lighter
{

  bool RadPrimitive::Split (const csPlane3& splitPlane, RadPrimitive &back)
  {
    // Do a split
    
    // Classify all points
    FloatArray classification;
    classification.SetSize (vertices.GetSize ());
    
    bool n = false, p = false;
    uint i;

    for (i = 0; i < vertices.GetSize (); i++)
    {
      float c = splitPlane.Classify (vertices[i]);
      if (c > LITEPSILON) p = true;
      else if (c < LITEPSILON) n = true;

      classification.Push (c);
    }

    // Handle special cases
    if (!p && !n)
    {
      if ((plane.Normal () * splitPlane.Normal ()))
      {
        p = true;
      }
      else
      {
        n = true;
      }
    }

    if (!n)
    {
      //No negative, back is empty
      back.vertices.DeleteAll ();
      back.lightmapUVs.DeleteAll ();
      back.extraData.DeleteAll ();
      return true;
    }
    else if (!p)
    {
      //No positive, current is back
      back.vertices = vertices;
      back.lightmapUVs = lightmapUVs;
      back.extraData = extraData;

      vertices.DeleteAll ();
      lightmapUVs.DeleteAll ();
      extraData.DeleteAll ();
    }
    else
    {
      // Split-time! :)
      back.plane = plane;

      Vector3DArray &negVert = back.vertices;
      Vector2DArray &negUV = back.lightmapUVs;
      IntArray &negExtra = back.extraData;
      negVert.Empty (); negVert.SetSize (vertices.GetSize ());
      negUV.Empty (); negUV.SetSize (vertices.GetSize ());
      negExtra.Empty (); negExtra.SetSize (vertices.GetSize ());

      Vector3DArray posVert;
      Vector2DArray posUV;
      IntDArray posExtra;

      posVert.SetSize (vertices.GetSize ());
      posUV.SetSize (vertices.GetSize ());
      posExtra.SetSize (vertices.GetSize ());

      // Visit all edges, add vertices to back/front as needed. Add intersection-points
      // if needed
      size_t v0 = vertices.GetSize () - 1;
      for (size_t v1 = 0; v1 < vertices.GetSize (); v1++)
      {
        const csVector3 & p0 = vertices[v0];
        const csVector3 & p1 = vertices[v1];
        const csVector2 & t0 = lightmapUVs[v0];
        const csVector2 & t1 = lightmapUVs[v1];
        float d0 = classification[v0];
        float d1 = classification[v1];

        if (d0 < 0 && d1 < 0)
        {
          // neg-neg
          negVert.Push (p1);
          negUV.Push (t1);
          negExtra.Push (extraData[v1]);
        }
        else if (d0 < 0 && d1 == 0)
        {
          // neg-zero
          negVert.Push (p1);
          negUV.Push (t1);
          negExtra.Push (extraData[v1]);

          posVert.Push (p1);
          posUV.Push (t1);
          posExtra.Push (extraData[v1]);
        }
        else if (d0 < 0 && d1 > 0)
        {
          // neg-pos xing
          float D = d0 / (d1 - d0);
          csVector3 isecVert = p0 - (p1 - p0) * D;
          negVert.Push (isecVert);
          posVert.Push (isecVert);
          posVert.Push (p1);

          csVector2 isecUV = t0 - (t1 - t0) * D;
          negUV.Push (isecUV);
          posUV.Push (isecUV);
          posUV.Push (t1);

          posExtra.Push (-1);
        }
        else if (d0 == 0 && d1 > 0)
        {
          // zero-pos
          posVert.Push (p1);
          posUV.Push (t1);
          posExtra.Push (extraData[v1]);
        }
        else if (d0 == 0 && d1 < 0)
        {
          // zero-neg
          negVert.Push (p1);
          negUV.Push (t1);
          negExtra.Push (extraData[v1]);
        }
        else if (d0 > 0 && d1 > 0)
        {
          // pos-pos
          posVert.Push (p1);
          posUV.Push (t1);
          posExtra.Push (extraData[v1]);
        }
        else if (d0 > 0 && d1 < 0)
        {
          // pos-neg
          float D = d1 / (d0 - d1);
          csVector3 isecVert = p1 - (p0 - p1) * D;
          posVert.Push (isecVert);
          negVert.Push (isecVert);
          negVert.Push (p1);
         
          csVector2 isecUV = t1 - (t0 - t1) * D;
          posUV.Push (isecUV);
          negUV.Push (isecUV);
          negUV.Push (t1);

          negExtra.Push (-1);
        }
        else if (d0 > 0 && d1 == 0)
        {
          //pos-zero
          posVert.Push (p1);
          negVert.Push (p1);

          posUV.Push (t1);
          negUV.Push (t1);

          posExtra.Push (extraData[v1]);
          negExtra.Push (extraData[v1]);
        }
        else if (d0 == 0 && d1 == 0)
        {
          //zero-zero.. bad case.. shouldn't really happen
          //do nothing..
        }
        //next
        v0 = v1;
      }
      // set ourself as front
      vertices = posVert;
      lightmapUVs = posUV;
      extraData = posExtra;
    }


    // Make sure back gets any extra data
    if (back.vertices.GetSize ())
    {
      back.plane = plane;
      back.uFormVector = uFormVector;
      back.vFormVector = vFormVector;
      back.illuminationColor = illuminationColor;
      back.reflectanceColor = reflectanceColor;
      back.originalPrim = originalPrim;
      back.lightmapID = lightmapID;
    }

    return true;
  }

  void RadPrimitive::SetLightmapMapping (float uScale /* = 1.0f */, 
                                         float vScale /* = 1.0f */)
  {
    int polyAxis = csPoly3D::ComputeMainNormalAxis ();
    
    // Make sure we have room
    lightmapUVs.SetSize (vertices.GetSize ());

    if (polyAxis == CS_AXIS_X)
    {
      // use YZ-plane
      for (uint i = 0; i < vertices.GetSize (); i++)
      {
        lightmapUVs[i].x = vertices[i].z * uScale;
        lightmapUVs[i].y = -vertices[i].y * vScale;
      }
    }
    else if (polyAxis == CS_AXIS_Y)
    {
      // use XZ-plane
      for (uint i = 0; i < vertices.GetSize (); i++)
      {
        lightmapUVs[i].x = vertices[i].x * uScale;
        lightmapUVs[i].y = -vertices[i].z * vScale;
      }
    }
    else
    {
      // use XY-plane
      for (uint i = 0; i < vertices.GetSize (); i++)
      {
        lightmapUVs[i].x = vertices[i].x * uScale;
        lightmapUVs[i].y = -vertices[i].y * vScale;
      }
    }
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

  void RadPrimitive::ComputeUVTransform ()
  {
    uFormVector.Set (0.0f);
    vFormVector.Set (0.0f);

    // Some temporary holders
    csVector3 u0_3d(0.0f), u1_3d(0.0f), v0_3d(0.0f), v1_3d(0.0f);
    csVector2 u0_2d(0.0f), u1_2d(0.0f), v0_2d(0.0f), v1_2d(0.0f);

    float maxuDist = -1, maxvDist = -1;

    //visit all vertices
    for (uint i = 0; i < vertices.GetSize (); i++)
    {
      csVector3& c3D = vertices[i];
      csVector2& c2D = lightmapUVs[i];
      size_t v0 = vertices.GetSize () - 1;

      // Visit all edges
      for (size_t v1 = 0; v1 < vertices.GetSize (); v1++)
      {
        // Skip those that includes current vertex
        if (v0 != i && v1 != i)
        {
          csVector3& v03D = vertices[v0];
          csVector3& v13D = vertices[v1];

          csVector2& v02D = lightmapUVs[v0];
          csVector2& v12D = lightmapUVs[v1];

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

  void RadPrimitive::Prepare (uint uResolution, uint vResolution)
  {
    // Reset current data
    elementAreas.DeleteAll ();
    patches.DeleteAll ();

    // Compute min/max uv
    uint uc, vc;
    int maxu, minu, maxv, minv;
    ComputeMinMaxUV (minu, maxu, minv, maxv);

    uc = maxu - minu + 1;
    vc = maxv - minv + 1;

    minUV.x = minu; minUV.y = minv;
    maxUV.x = maxu; maxUV.y = maxv;

    // Min xyz
    csVector2 d = minUV - lightmapUVs[0];
    minCoord = vertices[0] + uFormVector * d.x + vFormVector * d.y;
    
    // Number of patches in u/v direction
    uPatches = uc / uResolution;
    vPatches = vc / vResolution;
    if (uc % uResolution) uPatches++;
    if (vc % vResolution) vPatches++;

    csArray<int> patchECount;

    // Set some default info
    RadPatch dpatch;
    dpatch.area = 0.0f;
    dpatch.energy = illuminationColor;
    dpatch.plane = plane;
    dpatch.center = csVector3 (0.0f);
    patches.SetSize (uPatches * vPatches, dpatch);
    patchECount.SetSize (uPatches * vPatches, 0);
    elementAreas.SetCapacity (uc * vc);

    // Create our splitplanes
    csPlane3 uCut (plane.Normal () % uFormVector);
    csVector3 uCutOrigin = minCoord;
    uCut.SetOrigin (uCutOrigin);
    
    csPlane3 vCut (plane.Normal () % vFormVector);
    csVector3 vCutOrigin = minCoord;
    vCut.SetOrigin (vCutOrigin);

    // Make sure they face correct way
    csVector3 primCenter = GetCenter ();
    if (uCut.Classify (primCenter) < 0) uCut.Normal () = -uCut.Normal ();
    if (vCut.Classify (primCenter) < 0) vCut.Normal () = -vCut.Normal ();

    // Start slicing
    csPoly3D poly = *this;

    csPlane3 evCut = vCut;
    for (uint v = 0; v  < vc; v++)
    {
      vCutOrigin += vFormVector;
      evCut.SetOrigin (vCutOrigin);
      
      // Cut of a row
      csPoly3D elRow, rest;
      if (v < (vc-1)) 
      {
        poly.SplitWithPlane (elRow, rest, evCut);
        poly = rest;
      }
      else
      {
        elRow = rest;
      }

      // Cut into elements
      csPlane3 euCut = uCut;
      csVector3 euOrigin = uCutOrigin;
      for (uint u = 0; u < uc; u++)
      {
        euOrigin += uFormVector;
        euCut.SetOrigin (euOrigin);

        csPoly3D el, restRow;
        if (u < (uc-1))
        {
          elRow.SplitWithPlane (el, restRow, euCut);
          elRow = restRow;
        }
        else
        {
          el = elRow;
        }

        float elArea = el.GetArea ();
        elementAreas.Push (elArea);

        if (elArea > 0 && patches.GetSize () > 0)
        {
          //Fix patch too
          uint patchIdx = v/vResolution * uPatches + u/uResolution;
          
          RadPatch &patch = patches[patchIdx];
          patch.area += elArea;
          patch.center += el.GetCenter ();

          patchECount[patchIdx]++;
        }
      }
    }

    // Fix patches
    if (patches.GetSize () > 0)
    {
      float primArea = GetArea ();

      for (uint i = 0; i < patches.GetSize (); i++)
      {
        RadPatch &patch = patches[i];
        patch.center /= patchECount[i];
        patch.energy *= patch.area / primArea;
      }
    }
  }

  void RadPrimitive::PrepareNoPatches ()
  {
    // Reset current data
    elementAreas.Empty ();

    // Compute min/max uv
    uint uc, vc;
    int maxu, minu, maxv, minv;
    ComputeMinMaxUV (minu, maxu, minv, maxv);

    uc = maxu - minu + 1;
    vc = maxv - minv + 1;

    minUV.x = minu; minUV.y = minv;
    maxUV.x = maxu; maxUV.y = maxv;

    // Min xyz
    csVector2 d = minUV - lightmapUVs[0];
    minCoord = vertices[0] + uFormVector * d.x + vFormVector * d.y;

    elementAreas.SetCapacity (uc * vc);

    // Create our splitplanes
    csPlane3 uCut (plane.Normal () % uFormVector);
    csVector3 uCutOrigin = minCoord;
    uCut.SetOrigin (uCutOrigin);

    csPlane3 vCut (plane.Normal () % vFormVector);
    csVector3 vCutOrigin = minCoord;
    vCut.SetOrigin (vCutOrigin);

    // Make sure they face correct way
    csVector3 primCenter = GetCenter ();
    if (uCut.Classify (primCenter) < 0) uCut.Normal () = -uCut.Normal ();
    if (vCut.Classify (primCenter) < 0) vCut.Normal () = -vCut.Normal ();

    // Start slicing
    csPoly3D poly = *this;

    csPlane3 evCut = vCut;
    for (uint v = 0; v  < vc; v++)
    {
      vCutOrigin += vFormVector;
      evCut.SetOrigin (vCutOrigin);

      // Cut of a row
      csPoly3D elRow, rest;
      if (v < (vc-1)) 
      {
        poly.SplitWithPlane (elRow, rest, evCut);
        poly = rest;
      }
      else
      {
        elRow = rest;
      }

      // Cut into elements
      csPlane3 euCut = uCut;
      csVector3 euOrigin = uCutOrigin;
      for (uint u = 0; u < uc; u++)
      {
        euOrigin += uFormVector;
        euCut.SetOrigin (euOrigin);

        csPoly3D el, restRow;
        if (u < (uc-1))
        {
          elRow.SplitWithPlane (el, restRow, euCut);
          elRow = restRow;
        }
        else
        {
          el = elRow;
        }

        float elArea = el.GetArea ();
        elementAreas.Push (elArea);
      }
    }
  }

}
