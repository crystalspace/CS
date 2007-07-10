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

namespace CS
{
namespace Geom
{

	iReporter* report;

	bool Triangulate3D::Process(csContour3& polygon, csTriangleMesh& result, iReporter* report2, csContour3* holes)
	{
		int n = (int)polygon.GetSize();

		if (n < 3)
		{
			return false;
		}

		report = report2;

		// we first need to find which of the vertices are in the convex and
		// reflex vertex sets
		csContour3 reflexVerts, convexVerts;
		csArray<size_t> potentialEarIndices;

		FindVertexGroups(polygon, reflexVerts, convexVerts, potentialEarIndices);
		csContour3 reflexVerts2(reflexVerts), convexVerts2(convexVerts);

		while (!reflexVerts2.IsEmpty() || !convexVerts2.IsEmpty())
		{
			if (!reflexVerts2.IsEmpty())
			{
				csVector3 popped = reflexVerts2.Pop();
				report->ReportWarning("crystalspace.Triangulate3D", "Reflex Vertex: (%f, %f, %f)", popped.x, popped.y, popped.z);
			}
			else
			{
				csVector3 popped = convexVerts2.Pop();
				report->ReportWarning("crystalspace.Triangulate3D", "Convex Vertex: (%f, %f, %f)", popped.x, popped.y, popped.z);
		
			}
		}

		report->ReportWarning("crystalspace.Triangulate3D", "Number of reflex verts: %d, Number of convex verts: %d", reflexVerts.GetSize(), convexVerts.GetSize()); 
		while (!potentialEarIndices.IsEmpty())
		{
			// remove the next ear
			size_t nextEar = potentialEarIndices.Pop();

/*  Old code to make it more like Triangulate2
			result_vertices.Push(polygon[nextEar]);
			result_vertices.Push(polygon[(nextEar+1)%n]);
			result_vertices.Push(polygon[(nextEar-1)%n]);
*/
			// cut out the triangle around it
			report->ReportWarning("crystalspace.Triangulate3D", "Snipping out point number %d", nextEar);
			report->ReportWarning("crystalspace.Triangulate3D", "Snipped point is: (%d, %d, %d)", polygon[nextEar].x, polygon[nextEar].y, polygon[nextEar].z);
			Snip(polygon, potentialEarIndices, nextEar, result);
			n--;  // otherwise we have too many vertices in the counter

			int m = -1;
			
			// now, for each of the neighboring vertices

			while (m < 2)
			{
				report->ReportWarning("crystalspace.Triangulate3D", "Checking vertex %d", (nextEar+m)%n);
				csVector3 currentVertex = polygon.Get((nextEar+m)%n);
				csVector3 triA, triB, triC;


				// I think this is the wrong triangle
				triA = currentVertex;
				triB = polygon.Get((nextEar+m+1)%n);
				triC = polygon.Get((nextEar+m-1)%n);

				// if the vertex is convex
				if (IsConvex(polygon, (nextEar+m)%n))
				{
					// test to see if it is an ear by iterating over all the 
					// reflex vertices and testing to see if they are contained within
					// the triangle of this vertex
					
					size_t numRef = reflexVerts.GetSize();

					for (size_t y = 0; y < numRef; y++)
					{
						if (IsContained(reflexVerts[y], triA, triB, triC))
						{
							report->ReportWarning("crystalspace.Triangulate3D", "Removing vertex %d", y);

							// remove it from the reflex list
							reflexVerts.DeleteIndex(y);

							// add it to the convex list (in sorted order)
							potentialEarIndices.InsertSorted(y);
						}
					}
				}

				m+=2;
			}
		}

		return true;
	}

	bool Triangulate3D::FindVertexGroups(csContour3& poly, csContour3& reflex, csContour3& convex, csArray<size_t>& ears)
	{
		size_t length = poly.GetSize();

		for (size_t x = 0; x < length; x++)
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

	bool Triangulate3D::IsConvex(const csContour3& polygon, const size_t index)
	{
			size_t polyLength = polygon.GetSize();

			// determine the angle between the consecutive vectors
			csVector3 temp1(polygon[index].x + polygon[(index+1)%polyLength].x, polygon[index].y + polygon[(index+1)%polyLength].y, polygon[index].z + polygon[(index+1)%polyLength].z);
			csVector3 temp2(polygon[index].x + polygon[(index-1)%polyLength].x, polygon[index].y + polygon[(index-1)%polyLength].y, polygon[index].z + polygon[(index-1)%polyLength].z);
//			temp2 = -temp2;

			float normalDir;
			if ((temp2%temp1).Norm() > 0)
			{
				return true;
			}

			else
			{
				return false;
			}

/*
			float dotProd = temp2*temp1;
			float length1 = csVector3::Norm(temp1);
			float length2 = csVector3::Norm(temp2);
			float angle = dotProd/length1;
			angle = angle/length2;
			angle = acos(angle);
			angle = angle*(180.0/PI);

			if (angle < 180.0)
			{
				return true;
			}
			else
			{
				return false;
			}
	*/
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

} // namespace Geom
} // namespace CS