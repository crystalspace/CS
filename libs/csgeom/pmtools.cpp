/*
  Copyright (C) 2002 by Jorrit Tyberghein

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
#include "qsqrt.h"
#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/pmtools.h"
#include "igeom/polymesh.h"

void csPolygonMeshTools::CalculateNormals (iPolygonMesh* mesh,
  csVector3* normals)
{
  int p;
  csVector3* verts = mesh->GetVertices ();
  int num_poly = mesh->GetPolygonCount ();
  csMeshedPolygon* poly = mesh->GetPolygons ();
  for (p = 0 ; p < num_poly ; p++)
  {
    float ayz = 0;
    float azx = 0;
    float axy = 0;
    int i, i1;
    float x1, y1, z1, x, y, z;

    int* vi = poly->vertices;
    i1 = poly->num_vertices - 1;
    x1 = verts[vi[i1]].x;
    y1 = verts[vi[i1]].y;
    z1 = verts[vi[i1]].z;
    for (i = 0 ; i < poly->num_vertices ; i++)
    {
      x = verts[vi[i]].x;
      y = verts[vi[i]].y;
      z = verts[vi[i]].z;
      ayz += (z1 + z) * (y - y1);
      azx += (x1 + x) * (z - z1);
      axy += (y1 + y) * (x - x1);
      x1 = x;
      y1 = y;
      z1 = z;
    }

    float sqd = ayz * ayz + azx * azx + axy * axy;
    float invd;
    if (sqd < SMALL_EPSILON)
      invd = 1.0f / SMALL_EPSILON;
    else
      invd = qisqrt (sqd);
    normals[p].Set (ayz * invd, azx * invd, axy * invd);

    poly++;
  }
}

void csPolygonMeshTools::CalculatePlanes (iPolygonMesh* mesh,
  csPlane3* planes)
{
  int p;
  csVector3* verts = mesh->GetVertices ();
  int num_poly = mesh->GetPolygonCount ();
  csMeshedPolygon* poly = mesh->GetPolygons ();
  for (p = 0 ; p < num_poly ; p++)
  {
    float ayz = 0;
    float azx = 0;
    float axy = 0;
    int i, i1;
    float x1, y1, z1, x, y, z;

    int* vi = poly->vertices;
    i1 = poly->num_vertices - 1;
    x1 = verts[vi[i1]].x;
    y1 = verts[vi[i1]].y;
    z1 = verts[vi[i1]].z;
    for (i = 0 ; i < poly->num_vertices ; i++)
    {
      x = verts[vi[i]].x;
      y = verts[vi[i]].y;
      z = verts[vi[i]].z;
      ayz += (z1 + z) * (y - y1);
      azx += (x1 + x) * (z - z1);
      axy += (y1 + y) * (x - x1);
      x1 = x;
      y1 = y;
      z1 = z;
    }

    float sqd = ayz * ayz + azx * azx + axy * axy;
    float invd;
    if (sqd < SMALL_EPSILON)
      invd = 1.0f / SMALL_EPSILON;
    else
      invd = qisqrt (sqd);
    planes[p].norm.Set (ayz * invd, azx * invd, axy * invd);
    planes[p].DD = - planes[p].norm * verts[vi[0]];

    poly++;
  }
}

csPolygonMeshEdge* csPolygonMeshTools::CalculateEdges (iPolygonMesh* mesh,
	int& num_edges)
{
  struct LinkedEdge : public csPolygonMeshEdge
  {
    LinkedEdge* next;
  };
  // @@@ NOT CLEANED UP
  static LinkedEdge* free_edges = NULL;

  int num_vertices = mesh->GetVertexCount ();
  int num_polygons = mesh->GetPolygonCount ();

  // First we create a table indexed by the first vertex index of every
  // edge. Every entry of this table will then contain a linked list of
  // edges which all start with that vertex.
  LinkedEdge** edge_table = new LinkedEdge* [num_vertices];
  memset (edge_table, 0, sizeof (LinkedEdge*) * num_vertices);

  // Loop over every polygon and add the edges.
  num_edges = 0;
  LinkedEdge* edge_collector = NULL;	// Here we collect edges.
  int i, j;
  csMeshedPolygon* poly = mesh->GetPolygons ();
  for (i = 0 ; i < num_polygons ; i++, poly++)
  {
    int* vi = poly->vertices;
    int vt2 = vi[poly->num_vertices-1];
    for (j = 0 ; j < poly->num_vertices ; j++)
    {
      int vt1 = vi[j];

      // Handle an edge from vt1 to vt2.
      CS_ASSERT (vt1 >= 0 && vt1 < num_vertices);
      CS_ASSERT (vt2 >= 0 && vt2 < num_vertices);
      int vt1s, vt2s;
      if (vt1 < vt2) { vt1s = vt1; vt2s = vt2; }
      else { vt1s = vt2; vt2s = vt1; }
      LinkedEdge* le = edge_table[vt1s];

      // There are two cases here. Either we find the edge
      // in the edge_table. In that case we previously encountered
      // another polygon that shares that edge. In that case we will
      // add the edge to the linked list of real edges and remove
      // it from the edge table. If we later discover another polygon
      // with that edge then it will become a new edge.
      // If the edge is not in the table yet we will add it.
      LinkedEdge* prev = NULL;
      while (le)
      {
        CS_ASSERT (le->vt1 == vt1s);
        if (le->vt2 == vt2s)
	{
	  // Found!
	  le->poly2 = i;
	  if (prev) prev->next = le->next;
	  else edge_table[vt1s] = le->next;
	  le->next = edge_collector;
	  edge_collector = le;
	  break;
	}
	prev = le;
	le = le->next;
      }
      if (!le)
      {
        // Not found!
	num_edges++;
	// Take a free edge from free_edges if any.
	if (free_edges)
	{
	  le = free_edges;
	  free_edges = free_edges->next;
	}
	else
	{
	  le = new LinkedEdge ();
	}
	le->vt1 = vt1s;
	le->vt2 = vt2s;
	le->poly1 = i;
	le->poly2 = -1;
	le->next = edge_table[vt1s];
	edge_table[vt1s] = le;
      }

      vt2 = vt1;
    }
  }

  // After the previous loop we basically have two sets of edges.
  // First there are the edges we are fully processed (have two attached
  // polygons). These were added to edge_collector. Secondly we have
  // the edges that have no second polygon. These will still be in
  // the edge_table. Here we add them together.
  // 'num_edges' will contain the correct number of total edges already.
  csPolygonMeshEdge* edges = new csPolygonMeshEdge [num_edges];
  csPolygonMeshEdge* e = edges;
  i = 0;
  while (edge_collector)
  {
    e->vt1 = edge_collector->vt1;
    e->vt2 = edge_collector->vt2;
    e->poly1 = edge_collector->poly1;
    e->poly2 = edge_collector->poly2;
    e++;
    i++;
    LinkedEdge* el = edge_collector;
    edge_collector = edge_collector->next;
    el->next = free_edges;
    free_edges = el;
  }
  for (j = 0 ; j < num_vertices ; j++)
  {
    LinkedEdge* et = edge_table[j];
    while (et)
    {
      e->vt1 = et->vt1;
      e->vt2 = et->vt2;
      e->poly1 = et->poly1;
      e->poly2 = et->poly2;
      e++;
      i++;
      LinkedEdge* el = et;
      et = et->next;
      el->next = free_edges;
      free_edges = el;
    }
  }
  delete[] edge_table;
  CS_ASSERT (i == num_edges);
  return edges;
}

int csPolygonMeshTools::CheckActiveEdges (
	csPolygonMeshEdge* edges, int num_edges,
  	csPlane3* planes)
{
  int i;
  csPolygonMeshEdge* e = edges;
  int active = 0;
  for (i = 0 ; i < num_edges ; i++, e++)
  {
    if (e->poly2 == -1)
    {
      e->active = true;
      active++;
    }
    else
    {
      // Since we know the two planes connect we only have to check
      // the normals.
      if ((planes[e->poly1].norm - planes[e->poly2].norm) < EPSILON)
        e->active = false;
      else
      {
        e->active = true;
        active++;
      }
    }
  }
  return active;
}

void csPolygonMeshTools::CalculateOutline (
	csPolygonMeshEdge* edges, int num_edges,
  	csPlane3* planes, int num_vertices,
	const csVector3& pos,
	int* outline_edges, int& num_outline_edges,
	int* outline_verts, int& num_outline_verts,
	float& valid_radius)
{
  int i;
  csPolygonMeshEdge* e = edges;
  num_outline_edges = 0;
  valid_radius = 10000000.0;
  for (i = 0 ; i < num_vertices ; i++)
  {
    outline_verts[i] = 0;
  }
  for (i = 0 ; i < num_edges ; i++, e++)
  {
    if (!e->active) continue;

    if (e->poly2 == -1)
    {
      // If the edge is attached to only one edge it is an outline
      // edge automatically.
      *outline_edges++ = e->vt1;
      *outline_edges++ = e->vt2;
      num_outline_edges++;
      CS_ASSERT (e->vt1 >= 0 && e->vt1 < num_vertices);
      CS_ASSERT (e->vt2 >= 0 && e->vt2 < num_vertices);
      outline_verts[e->vt1] = 1;	// Mark vertices as in use.
      outline_verts[e->vt2] = 1;
    }
    else
    {
      // Otherwise we have to compare the plane equations of the two
      // adjacent planes.
      const csPlane3& pl1 = planes[e->poly1];
      const csPlane3& pl2 = planes[e->poly2];
      float cl1 = pl1.Classify (pos);
      float cl2 = pl2.Classify (pos);
      if ((cl1 < 0 && cl2 > 0) || (cl1 > 0 && cl2 < 0))
      {
        // Sign is different. So this is an outline.
        *outline_edges++ = e->vt1;
        *outline_edges++ = e->vt2;
        num_outline_edges++;
        CS_ASSERT (e->vt1 >= 0 && e->vt1 < num_vertices);
        CS_ASSERT (e->vt2 >= 0 && e->vt2 < num_vertices);
        outline_verts[e->vt1] = 1;	// Mark vertices as in use.
        outline_verts[e->vt2] = 1;
      }
      // Calculate minimum distance at which this edge changes status
      // (from in to out or vice versa).
      float cl = MIN (ABS (cl1), ABS (cl2));
      if (cl < valid_radius) valid_radius = cl;
    }
  }

  // Now check which vertices are used by the outline edges.
  // These are the vertices to transform.
  num_outline_verts = 0;
  for (i = 0 ; i < num_vertices ; i++)
  {
    CS_ASSERT (outline_verts[i] == 0 || outline_verts[i] == 1);
    if (outline_verts[i]) outline_verts[num_outline_verts++] = i;
  }
}

