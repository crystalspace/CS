/*
    iPolygonMesh tool functions
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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
#include "cssys/csendian.h"
#include "csutil/hash.h"
#include "cstool/polymeshtools.h"

struct PolyEdge
{
  int v1, v2;
  bool flipped;

  PolyEdge (int v1, int v2)
  {
    if (v1 > v2)
    {
      PolyEdge::v1 = v2;
      PolyEdge::v2 = v1;
      flipped = true;
    }
    else
    {
      PolyEdge::v1 = v1;
      PolyEdge::v2 = v2;
      flipped = false;
    }
  }
};

struct AdjacencyCounter
{
  int adjNormal, adjFlipped;

  AdjacencyCounter (int x = 0)
  {
    adjNormal = adjFlipped = 0;
  }
};

class PolyEdgeHashKeyHandler
{
public:
  static uint32 ComputeHash (const PolyEdge& key)
  {
    uint32 key2 = 
      (key.v2 >> 24) | ((key.v2 >> 8) & 0xff00) | 
      ((key.v2 << 8) & 0xff0000) | (key.v2 << 24);
    return (((uint32)key.v1) ^ key2);
  }

  static bool CompareKeys (const PolyEdge& key1, const PolyEdge& key2)
  {
    return ((key1.v1 == key2.v1) && (key1.v2 == key2.v2));
  }

};

/*
  The closedness test works by counting the faces to adjacent edges.
  An even number of faces facing into the same direction must be adjacent
  to every edge.

  Given two vertices A and B, we count the number of faces that are
  adjacent to A->B and B->A. (If face 1 has the A->B edge, in an adjacent
  face that shares that edge and and faces into the same direction as
  face 1 this edge will appear as B->A. This is due that vertex order
  defines the direction.) The number of faces that have A->B in them must
  be equal to the number of faces that contain B->A. Otherwise, the mesh
  isn't closed.
 */
bool csPolyMeshTools::IsMeshClosed (iPolygonMesh* polyMesh)
{
  csHash<AdjacencyCounter, PolyEdge, PolyEdgeHashKeyHandler>
    adjacency;

  int numIncorrect = 0;

  int pc = polyMesh->GetPolygonCount ();
  csMeshedPolygon* polys = polyMesh->GetPolygons ();
  int p;
  for (p = 0; p < pc; p++)
  {
    const csMeshedPolygon& poly = polys[p];
    int v1 = poly.vertices[poly.num_vertices - 1];
    int v;
    for (v = 0; v < poly.num_vertices; v++)
    {
      int v2 = poly.vertices[v];

      PolyEdge edge (v1, v2);
      AdjacencyCounter counter (adjacency.Get (edge));

      if (counter.adjFlipped != counter.adjNormal)
      {
	numIncorrect--;
      }

      if (edge.flipped)
      {
	counter.adjFlipped++;
      }
      else
      {
	counter.adjNormal++;
      }

      if (counter.adjFlipped != counter.adjNormal)
      {
	numIncorrect++;
      }

      adjacency.PutFirst (edge, counter);

      v1 = v2;
    }
  }

  return (numIncorrect == 0);
}
  
void csPolyMeshTools::CloseMesh (iPolygonMesh* polyMesh, 
				 csArray<csMeshedPolygon>& newPolys)
{
  int pc = polyMesh->GetPolygonCount ();
  csMeshedPolygon* polys = polyMesh->GetPolygons ();
  int p;
  for (p = 0; p < pc; p++)
  {
    const csMeshedPolygon& poly = polys[p];
    csMeshedPolygon newPoly;
    newPoly.num_vertices = poly.num_vertices;
    newPoly.vertices = new int[poly.num_vertices];
    int v;
    for (v = 0; v < poly.num_vertices; v++)
    {
      newPoly.vertices[v] = poly.vertices[poly.num_vertices - 1 - v];
    }
    newPolys.Push (newPoly);
  }
}

