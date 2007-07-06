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

	bool Triangulate3D::Process(csContour3& polygon, csTriangleMesh& result, csContour3& result_vertices, csContour3* holes)
	{
		size_t n = polygon.GetSize();

		if (n < 3)
		{
			return false;
		}

		// we first need to find which of the vertices are in the convex and
		// reflex vertex sets
		csContour3 reflexVerts, convexVerts;
		csArray<size_t> potentialEarIndices;

		FindVertexGroups(polygon, reflexVerts, convexVerts, potentialEarIndices);
		
		while (!potentialEarIndices.IsEmpty())
		{
			// remove the next ear
			size_t nextEar = potentialEarIndices.Pop();

			// cut out the triangle around it
			Snip(polygon, nextEar, result);

			result_vertices.Push(polygon[nextEar]);
			result_vertices.Push(polygon[(nextEar+1)%n]);
			result_vertices.Push(polygon[(nextEar-1)%n]);

			size_t m = -1;
			
			// now, for each of the neighboring vertices
			while (m < 2)
			{
				csVector3 currentVertex = polygon.Get((nextEar+m)%n);
				csVector3 triA, triB, triC;
				triA = polygon.Get(nextEar);
				triB = polygon.Get((nextEar+1)%m);
				triC = polygon.Get((nextEar-1)%m);

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
				reflex.Push(poly[x]);
				ears.Push(x);
			}
			else
			{
				// add to convex set
				convex.Push(poly[x]);
			}
		}

		return true;
	}

	bool Triangulate3D::Snip(csContour3& polygon, const size_t earPoint, csTriangleMesh& addTo)
	{
		size_t vCount = addTo.GetVertexCount();
		size_t polySize = polygon.GetSize();

		addTo.AddVertex(polygon[earPoint]);
		addTo.AddVertex(polygon[(earPoint+1)%polySize]);
		addTo.AddVertex(polygon[(earPoint-1)%polySize]);

		addTo.AddTriangle((int)vCount, (int)vCount+1, (int)vCount+2);
		polygon.DeleteIndex(earPoint);

		return true;
	}

	bool Triangulate3D::IsConvex(const csContour3& polygon, const size_t index)
	{
			size_t length = polygon.GetSize();

			// determine the angle between the consecutive vectors
			csVector3 temp1(polygon[index].x - polygon[(index+1)%length].x, polygon[index].y - polygon[(index+1)%length].y, polygon[index].z - polygon[(index+1)%length].z);
			csVector3 temp2(polygon[index].x - polygon[(index-1)%length].x, polygon[index].y - polygon[(index-1)%length].y, polygon[index].z - polygon[(index-1)%length].z);

			float dotProd = temp1*temp2;
			float length1 = csVector3::Norm(temp1);
			float length2 = csVector3::Norm(temp2);
			float angle = dotProd/length1;
			angle = angle/length2;
			angle = acos(angle);
			angle = angle*(180.0/PI);

			if (angle < 180.0)
			{
				return false;
			}
			else
			{
				return true;
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

} // namespace Geom
} // namespace CS