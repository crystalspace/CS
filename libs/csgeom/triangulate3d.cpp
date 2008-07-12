/*
    Copyright (C) 2007 by Scott Johnson

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

namespace CS
{
namespace Geometry
{

	iReporter* report;

	bool Triangulate3D::Process(csContour3& polygon, csTriangleMesh& result, iReporter* report2, csContour3* holes)
	{
		report = report2;
		int n = (int)polygon.GetSize();

		if (n < 3)
		{
			return false;
		}
		
		// first, let's establish a mapping from 3D -> 2D planar polygon
		csContour3 planarPolygon = Triangulate3D::MapToPlanar(polygon);

		// rotate the planar polygon so that it's axis-aligned
		// triangulate the 2D planar polygon
		// finally, reverse the mapping

		// @@@FIXME: Finish implementing.
		return true;
	}

	csContour3 Triangulate3D::MapToPlanar(const csContour3& poly)
	{
		// we'll accomplish this by marching along the vertices of the 
		// 3D polygon
	
		// this algorithm was developed with assistance from Martin Held, 
		// the creator of Fast Industrial Strength Triangulation (FIST).
		// for more information, take a look at:
		// Held, M. FIST: Fast Industrial Strength Triangulation of Polygons.
		//    Algorithmica. 30(4): 563-596, 2001.

		csVector3 accumulatorNormal;
		
		int n = (int) poly.GetSize();
		for (int i = 0; i < n; i++)
		{
			csVector3 first, second, third;
			first = poly[(((i-1)+n)%n)];
			second = poly[i];
			third = poly[((i+1)%n)];

			// output first second and third to verify nothing crazy is happening
			report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.triangulate3d", "First point: %f, %f, %f", first.x, first.y, first.z);
			report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.triangulate3d", "Second point: %f, %f, %f", second.x, second.y, second.z);	
			report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.triangulate3d", "Third point: %f, %f, %f", third.x, third.y, third.z);


			// compute the normals to cross product
			//csVector3 toCross1 = first - second;
			csVector3 toCross1 = second - first;
			csVector3 toCross2 = third - second;
			report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.triangulate3d", "toCross1: %f, %f, %f", toCross1.x, toCross1.y, toCross1.z);	
			report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.triangulate3d", "toCross2: %f, %f, %f", toCross2.x, toCross2.y, toCross2.z);

			csVector3 result = toCross2 % toCross1;
			report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.triangulate3d", "Cross Product: %f, %f, %f", result.x, result.y, result.z);	

			result = csVector3::Unit(result);
			report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.triangulate3d", "Result (after unit): %f, %f, %f", result.x, result.y, result.z);	

			accumulatorNormal.x += result.x;
			accumulatorNormal.y += result.y;
			accumulatorNormal.z += result.z;

		}
		
		report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.triangulate3d", "accumulator normal (before unit): %f, %f, %f", accumulatorNormal.x, accumulatorNormal.y, accumulatorNormal.z);

		accumulatorNormal = accumulatorNormal.Unit();
		
		// so now, accumulatorNormal contains the normal vector
		// for an approximate polygon which is planar

		// create a csPlane which represents this plane
		// output the accumulator normal
		// these also report -1#.IND - why?
		report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.triangulate3d", "accumulator normal: %f, %f, %f", accumulatorNormal.x, accumulatorNormal.y, accumulatorNormal.z);

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
		// this returns -1.#IND ... Why?
		float distToOrig = ourPlane.Distance(csVector3(0, 0, 0));
		report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.Triangulate3D", "distance to origin before: %f", distToOrig);
		if (distToOrig > 0)
		{
			float transX = -(ourPlane.Normal() * distToOrig).x;
			float transY = -(ourPlane.Normal() * distToOrig).y;
			float transZ = -(ourPlane.Normal() * distToOrig).z;
			
			transformation.SetO2TTranslation(csVector3(transX, transY, transZ));
			
			// print out the matrix to be sure
			csMatrix3 mat2 = transformation.GetO2T();
			report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.Triangulate3D", "%f %f %f", mat2.Row1()[0], mat2.Row1()[1], mat2.Row1()[2]);
			report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.Triangulate3D", "%f %f %f", mat2.Row2()[0], mat2.Row2()[1], mat2.Row2()[2]);
			report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.Triangulate3D", "%f %f %f", mat2.Row3()[0], mat2.Row3()[1], mat2.Row3()[2]);
			ourPlane = transformation.Other2This(ourPlane);
		}

		distToOrig = ourPlane.Distance(csVector3(0, 0, 0));
		report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.Triangulate3D", "distance to origin after: %f", distToOrig);
		return poly;
	}

	/*
	bool Triangulate3D::FindVertexGroups(csContour3& poly, csContour3& reflex, csContour3& convex, csArray<size_t>& ears)
	{
		int length = (int)poly.GetSize();

		for (int x = 0; x < length; x++)
		{
			if (!IsConvex(poly, x))
			{
				// add to reflex set
				report->ReportWarning("crystalspace.Triangulate3D", "Vertex %d is reflex", x);
				reflex.Push(poly[x]);
			}
			else
			{
				// add to convex set
				report->ReportWarning("crystalspace.Triangulate3D", "Vertex %d is convex", x);
				convex.Push(poly[x]);
				ears.Push(x);
			}
		}

		return true;
	}

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

	bool Triangulate3D::IsConvex(const csContour3& polygon, const int index)
	{
		int polyLength = (int)polygon.GetSize();
		int nextIndex = (index+1)%polyLength;
		int prevIndex = (index-1);
		
		while (prevIndex < 0)
		{
			prevIndex += polyLength;
		}

		prevIndex = prevIndex%polyLength;
		
		report->Report(CS_REPORTER_SEVERITY_WARNING, "crystalspace.Triangulate3D", "polyLength: %d, index: %d, index+1mod: %d, index-1mod: %d", polyLength, index, nextIndex, prevIndex);

		csPlane3 plane(polygon[index], polygon[nextIndex], polygon[prevIndex]);

		// detect clockwise movement, and invert plane in that case
		

		csVector3 temp1 = polygon[index] - polygon[nextIndex];
		csVector3 temp2 = polygon[index] - polygon[prevIndex];

		csVector3 crossProd = temp1%temp2;
		csVector3 crossAtVert = crossProd + polygon[index];
		
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
*/

} // namespace Geometry
} // namespace CS

