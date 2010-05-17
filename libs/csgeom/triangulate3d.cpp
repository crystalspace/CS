/*
  Copyright (C) 2007-2008 by Scott Johnson

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
#include "csgeom/triangulate3d.h"
#include "csgeom/transfrm.h"

#define DEGTORAD(x) ((x)*PI/180.0f)

namespace CS
{
  namespace Geometry
  {
    csEarClipper::csEarClipper(csContour3 polygon)
    {
      for (int i = 0; i < (int)polygon.GetSize(); i++)
      {
        clipPoly.Push(polygon[i]);
        originalIndices.Push(i);
      }

      ClassifyVertices();
    }

    bool csEarClipper::IsConvex(const int index)
    {
      int polyLength = (int)clipPoly.GetSize();
      int nextIndex = (index+1)%polyLength;
      int prevIndex = (index-1);

      if (prevIndex < 0)
      {
        prevIndex = polyLength + prevIndex;
      }

      csPlane3 plane(clipPoly[index], clipPoly[nextIndex], clipPoly[prevIndex]);

      // detect clockwise movement, and invert plane in that case
      csVector3 temp1 = clipPoly[index] - clipPoly[nextIndex];
      csVector3 temp2 = clipPoly[index] - clipPoly[prevIndex];

      csVector3 crossProd = temp1%temp2;
      csVector3 crossAtVert = crossProd + clipPoly[index];

      float indicator = plane.Classify(crossAtVert);
      if (indicator > 0)
      {
        return true;
      }

      else
      {
        return false;
      }

    }

    void csEarClipper::ClassifyVertices()
    {
      ears.DeleteAll();

      int length = (int)clipPoly.GetSize();
      isVertexReflex.SetSize(length);

      for (int x = 0; x < length; x++)
      {
        if (!IsConvex(x))
        {
          // add to reflex set
          isVertexReflex[x] = true;
        }
        else
        {
          // add to convex set
          isVertexReflex[x] = false;
          ears.Push(x);
        }
      }
    }

    bool csEarClipper::IsFinished()
    {
      if (clipPoly.GetSize() <= 3)
      {
        return true;
      }

      return false;
    }

    csVertexSet csEarClipper::ClipEar()
    {
      csVertexSet resultingSet;

      // clip the first ear in ears
      size_t earIndex = ears.Top();
      ears.Pop();
      
      // get the index in the clip poly array of the ear
      size_t indexInPoly = originalIndices[earIndex];

      // this vertex is the one we want to remove
      size_t indexToReturn = originalIndices[earIndex];

      // build the set of vertices that make up the triangle we just clipped
      size_t prevIndex;
      if (earIndex == 0)
      {
        prevIndex = originalIndices.GetSize() - 1;
      }
      else
      {
        prevIndex = earIndex - 1;
      }

      size_t nextIndex = earIndex + 1;
      while (nextIndex >= originalIndices.GetSize())
      {
        nextIndex -= originalIndices.GetSize();
      }

      resultingSet.Push(originalIndices[prevIndex]);
      resultingSet.Push(indexToReturn);
      resultingSet.Push(originalIndices[nextIndex]);

      // now, remove the ear and reclassify the vertices
      clipPoly.DeleteIndex(indexInPoly);
      originalIndices.DeleteIndex(indexInPoly);

      ClassifyVertices();

      return resultingSet;
    }

    bool Triangulate3D::Process(csContour3& polygon, csTriangleMesh& result)
    {
      int n = (int)polygon.GetSize();

      if (n < 3)
      {
        return false;
      }

      if (n == 3)
      {
        // trivial case, just return the result
        for (int i = 0; i < (int)polygon.GetSize(); i++)
        {
          result.AddVertex(polygon[i]);
        }

        result.AddTriangle(0, 1, 2);      

        return true;
      }

      // Removed for debugging algorithm    
      /*
      if (n == 4)
      {
      // special case of quadrilateral

      for (int i = 0; i < (int)polygon.GetSize(); i++)
      {
      result.AddVertex(polygon[i]);
      }

      result.AddTriangle(0, 1, 3);
      result.AddTriangle(1, 2, 3);

      return true;
      }
      */
      result.Clear();

      // add all of the vertices from the polygon into the triangle mesh
      for (size_t i = 0; i < polygon.GetSize(); i++)
      {
        result.AddVertex(polygon[i]);
      }

      // first, let's establish a mapping from 3D -> 2D planar polygon
      csVector3 normal(0.0, 0.0, 0.0);
      csContour3 planarPolygon = Triangulate3D::MapToPlanar(polygon, normal);

      // rotate the planar polygon so that it's in the XY plane
      if (ABS(normal.x) > EPSILON || ABS(normal.y) > EPSILON)
      {
        // output the normal (for testing)
        //csPrintf("Normal: (%f, %f, %f)\n", normal.x, normal.y, normal.z);

        // so, the z coordinate isn't the only coordinate in the normal
        // it means we need to rotate the plane

        // the y coordinate needs to be removed by rotating about x
        // the normal in the y direction gives the percentage of 90 we need to rotate
        float rotationAboutX = normal.y * 90.0;
        rotationAboutX = DEGTORAD(rotationAboutX);

        // the x coordinate needs to be removed by rotating about y
        float rotationAboutY = normal.x * 90.0;
        rotationAboutY = DEGTORAD(rotationAboutY);

        //csPrintf("rotation about y: %f, rotation about x: %f\n", rotationAboutY, rotationAboutX);

        // build the transformation
        csMatrix3 rotationMatrix = csXRotMatrix3(rotationAboutX) * csYRotMatrix3(rotationAboutY);

        // apply the transformation to the planar polygon
        for (size_t i = 0; i < planarPolygon.GetSize(); i++)
        {
          csVector3 newVert = rotationMatrix*planarPolygon[i];
          planarPolygon[i] = newVert;
          planarPolygon[i].z = 0.0;
        }
      }

      // triangulate the (now) 2D planar polygon in the XY plane using an 
      // ear clipping method
      // note that we will actually be triangulating the original 3D polygon
      // by using vertex indices.  this allows us to skip the reverse mapping
      // step.

      // utilize a csEarClipper data structure and remove all of the ears from it
      csEarClipper clipper(planarPolygon);

      while (!clipper.IsFinished())
      {
        // grab the next index of the vertex to be clipped
        csVertexSet clippedTri = clipper.ClipEar();

        // connect the vertices to the left and right of this index
        size_t leftIndex = clippedTri.Get(0);
        size_t earIndex = clippedTri.Get(1);
        size_t rightIndex = clippedTri.Get(2);

        // do a check, just to make sure that the polygon is sided
        // correctly
        if (leftIndex > rightIndex)
        {
          result.AddTriangle((int)leftIndex, (int)earIndex, (int)rightIndex);
        }

        else
        {
          result.AddTriangle((int)rightIndex, (int)earIndex, (int)leftIndex);
        }
      }

      // if we're done, then make sure to add the remainder to
      // the triangle mesh
      size_t firstIndex = clipper.GetOriginalIndex(0);
      size_t secondIndex = clipper.GetOriginalIndex(1);
      size_t thirdIndex = clipper.GetOriginalIndex(2);

      result.AddTriangle((int)firstIndex, (int)secondIndex, (int)thirdIndex);

      // @@@FIXME: Finish implementing.
      return true;
    }

    csContour3 Triangulate3D::MapToPlanar(const csContour3& poly, csVector3& normal)
    {
      // we'll accomplish this by marching along the vertices of the 
      // 3D polygon

      // this algorithm was developed with assistance from Martin Held, 
      // the creator of Fast Industrial Strength Triangulation (FIST).
      // for more information, take a look at:
      // Held, M. FIST: Fast Industrial Strength Triangulation of Polygons.
      //    Algorithmica. 30(4): 563-596, 2001.

      csVector3 accumulatorNormal(0.0, 0.0, 0.0);

      int n = (int) poly.GetSize();
      for (int i = 0; i < n; i++)
      {
        csVector3 first, second, third;
        first = poly[(((i-1)+n)%n)];
        second = poly[i];
        third = poly[((i+1)%n)];

        // output first second and third to verify nothing crazy is happening
        //csPrintf("crystalspace.triangulate3d: First point: %f, %f, %f\n", first.x, first.y, first.z);
        //csPrintf("crystalspace.triangulate3d: Second point: %f, %f, %f\n", second.x, second.y, second.z);	
        //csPrintf("crystalspace.triangulate3d: Third point: %f, %f, %f\n", third.x, third.y, third.z);


        // compute the normals to cross product
        //csVector3 toCross1 = first - second;
        csVector3 toCross1 = second - first;
        csVector3 toCross2 = third - second;
        //csPrintf("crystalspace.triangulate3d: toCross1: %f, %f, %f\n", toCross1.x, toCross1.y, toCross1.z);	
        //csPrintf("crystalspace.triangulate3d: toCross2: %f, %f, %f\n", toCross2.x, toCross2.y, toCross2.z);

        csVector3 result = toCross2 % toCross1;
        //csPrintf("crystalspace.triangulate3d: Cross Product: %f, %f, %f\n", result.x, result.y, result.z);	

        result = csVector3::Unit(result);
        //csPrintf("crystalspace.triangulate3d: Result (after unit): %f, %f, %f\n", result.x, result.y, result.z);	

        //if (SIGN(result.x) != SIGN(first.x))
        //{
        //  result.x *= -1.0;
        //}

        //if (SIGN(result.y) != SIGN(first.y))
        //{
        //  result.y *= -1.0;
        //}

        //if (SIGN(result.z) != SIGN(first.z))
        //{
        //  result.z *= -1.0;
        //}

        accumulatorNormal.x += result.x;
        accumulatorNormal.y += result.y;
        accumulatorNormal.z += result.z;

        //csPrintf("crystalspace.triangulate3d: accumulator normal (before unit): %f, %f, %f\n", accumulatorNormal.x, accumulatorNormal.y, accumulatorNormal.z);
      }

      accumulatorNormal = accumulatorNormal.Unit();

      // so now, accumulatorNormal contains the normal vector
      // for an approximate polygon which is planar

      // create a csPlane which represents this plane
      // output the accumulator normal
      //csPrintf("crystalspace.triangulate3d: accumulator normal: %f, %f, %f\n", accumulatorNormal.x, accumulatorNormal.y, accumulatorNormal.z);

      csPlane3 ourPlane(accumulatorNormal);

      // we now need to map from our original polygon to 
      // this new planar polygon
      csContour3 projectedPoly;
      for (int i = 0; i < n; i++)
      {
        csVector3 projected = ourPlane.ProjectOnto(poly[i]);
        projectedPoly.Push(projected);
      }

      // now, we need to rotate the plane so that it's parallel to the
      // xy-axis.
      // let's find the angles we need to rotate by
      csTransform transformation;

      // now, before rotating, we need to make sure we are at the origin
      float distToOrig = ourPlane.Distance(csVector3(0, 0, 0));
      //csPrintf("crystalspace.Triangulate3D: distance to origin before: %f\n", distToOrig);
      if (distToOrig > 0)
      {
        float transX = -(ourPlane.Normal() * distToOrig).x;
        float transY = -(ourPlane.Normal() * distToOrig).y;
        float transZ = -(ourPlane.Normal() * distToOrig).z;

        transformation.SetO2TTranslation(csVector3(transX, transY, transZ));

        // print out the matrix to be sure
        csMatrix3 mat2 = transformation.GetO2T();
        //csPrintf("crystalspace.Triangulate3D: %f %f %f\n", mat2.Row1()[0], mat2.Row1()[1], mat2.Row1()[2]);
        //csPrintf("crystalspace.Triangulate3D: %f %f %f\n", mat2.Row2()[0], mat2.Row2()[1], mat2.Row2()[2]);
        //csPrintf("crystalspace.Triangulate3D: %f %f %f\n", mat2.Row3()[0], mat2.Row3()[1], mat2.Row3()[2]);
        ourPlane = transformation.Other2This(ourPlane);
      }

      distToOrig = ourPlane.Distance(csVector3(0, 0, 0));
      //csPrintf("crystalspace.Triangulate3D: distance to origin after: %f\n", distToOrig);

      //csPrintf("New polygon:\n");
      //for (size_t i = 0; i < poly.GetSize(); i++)
      //{
      //  csPrintf("(%f, %f, %f)\n", poly[i].x, poly[i].y, poly[i].z);
      //}

      normal = accumulatorNormal;
      return poly;
    }

    /*
    bool Triangulate3D::Snip(csContour3& polygon, csArray<size_t>& ears, const size_t earPoint, csTriangleMesh& addTo)
    {
    size_t vCount = addTo.GetVertexCount();
    size_t polySize = polygon.GetSize();

    addTo.AddVertex(polygon[earPoint]);
    addTo.AddVertex(polygon[(earPoint+1)%polySize]);
    addTo.AddVertex(polygon[(earPoint-1)%polySize]);

    addTo.AddTriangle((int)vCount, (int)vCount+1, (int)vCount+2);
    polygon.DeleteIndex(earPoint);
    ears.Delete(earPoint);

    return true;
    }
    */


    bool Triangulate3D::IsContained(const csVector3& testVertex, const csVector3& a, const csVector3& b, const csVector3& c)
    {
      if (IsSameSide(testVertex, a, b, c) && IsSameSide(testVertex, b, a, c) && IsSameSide(testVertex, c, a, b))
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    bool Triangulate3D::IsSameSide(const csVector3& p1, const csVector3& p2, const csVector3& a, const csVector3& b)
    {
      csVector3 cross1 = (b-a)%(p1-a);
      csVector3 cross2 = (b-a)%(p2-a);
      if (cross1*cross2 >= 0.0)
      {
        return true;
      }
      else
      {
        return false;
      }
    }

  } // namespace Geometry
} // namespace CS


