/*
    Copyright (C) 1998,2001,2003 by Jorrit Tyberghein

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
#include "csgeom/trimeshlod.h"

//---------------------------------------------------------------------------

bool csTriangleVertexCost::DelVertex (int idx)
{
  int idxidx = con_vertices.Find (idx);
  if (idxidx == -1)
    return false;
  con_vertices.DeleteIndex (idxidx);
  return true;
}

void csTriangleVertexCost::ReplaceVertex (int old, int replace)
{
  if (DelVertex (old)) AddVertex (replace);
}

void csTriangleVertexCost::CalculateCost (csTriangleVerticesCost* vertices)
{
  int i;
  to_vertex = -1;
  float min_sq_dist = 1000000.;
  if (deleted)
  {
    // If the vertex is deleted we have a very high cost.
    // The cost is higher than the maximum cost you can get for
    // a non-deleted vertex. This is to make sure that we get
    // the last non-deleted vertex at the end of the LOD algorithm.
    cost = min_sq_dist+1;
    return;
  }
  for (i = 0 ; i < con_vertices.Length () ; i++)
  {
    float sq_dist = csSquaredDist::PointPoint (vertices->GetVertex (idx).pos,
    	vertices->GetVertex (con_vertices[i]).pos);
    if (sq_dist < min_sq_dist)
    {
      min_sq_dist = sq_dist;
      to_vertex = con_vertices[i];
    }
  }
  cost = min_sq_dist;
}

csTriangleVerticesCost::csTriangleVerticesCost (csTriangleMesh* mesh,
	csVector3* verts, int num_verts)
{
  vertices = new csTriangleVertexCost [num_verts];
  num_vertices = num_verts;

  // Build connectivity information for all vertices in this mesh.
  csTriangle* triangles = mesh->GetTriangles ();
  int i, j;
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i].pos = verts[i];
    vertices[i].idx = i;
    for (j = 0 ; j < mesh->GetTriangleCount () ; j++)
      if (triangles[j].a == i || triangles[j].b == i || triangles[j].c == i)
      {
        vertices[i].AddTriangle (j);
	if (triangles[j].a != i) vertices[i].AddVertex (triangles[j].a);
	if (triangles[j].b != i) vertices[i].AddVertex (triangles[j].b);
	if (triangles[j].c != i) vertices[i].AddVertex (triangles[j].c);
      }
  }
}

csTriangleVerticesCost::~csTriangleVerticesCost ()
{
  delete [] vertices;
}

void csTriangleVerticesCost::UpdateVertices (csVector3* verts)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i].pos = verts[i];
}

int csTriangleVerticesCost::GetMinimalCostVertex ()
{
  int i;
  int min_idx = -1;
  float min_cost = 2.+1000000.;
  for (i = 0 ; i < num_vertices ; i++)
    if (!vertices[i].deleted && vertices[i].cost < min_cost)
    {
      min_idx = i;
      min_cost = vertices[i].cost;
    }
  return min_idx;
}

void csTriangleVerticesCost::CalculateCost ()
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i].CalculateCost (this);
}

void csTriangleVerticesCost::Dump ()
{
  printf ("=== Dump ===\n");
  int i, j;
  for (i = 0 ; i < num_vertices ; i++)
  {
    printf ("  %d idx=%d del=%d cost=%f to=%d tri=[ ", i,
    	vertices[i].idx, (int)vertices[i].deleted,
	vertices[i].cost, vertices[i].to_vertex);
    for (j = 0 ; j < vertices[i].con_triangles.Length () ; j++)
      printf ("%d ", vertices[i].con_triangles[j]);
    printf ("] vt=[ ");
    for (j = 0 ; j < vertices[i].con_vertices.Length () ; j++)
      printf ("%d ", vertices[i].con_vertices[j]);
    printf ("]\n");
    if (!vertices[i].deleted)
      for (j = 0 ; j < vertices[i].con_vertices.Length () ; j++)
        if (vertices[vertices[i].con_vertices[j]].deleted)
          printf ("ERROR refering deleted vertex %d!\n",
	  	vertices[i].con_vertices[j]);
  }
}

//---------------------------------------------------------------------------

void csTriangleMeshLOD::CalculateLOD (csTriangleMesh* mesh,
	csTriangleVerticesCost* verts, int* translate, int* emerge_from)
{
  int i;
  // Calculate the cost for all vertices for the first time.
  // This information will change locally whenever vertices are collapsed.
  verts->CalculateCost ();

  // Collapse vertices, one by one until only one remains.
  int num = verts->GetVertexCount ();
  int from, to, col_idx;
  int *from_vertices, *to_vertices;
  from_vertices = new int [num];
  to_vertices = new int [num];
  col_idx = 0;
  while (num > 1)
  {
    from = verts->GetMinimalCostVertex ();
    from_vertices[col_idx] = from;
    csTriangleVertexCost* vt_from = &verts->GetVertex (from);

    to = verts->GetVertex (from).to_vertex;

    // If to == -1 then it is possible that we have solitary vertices.
    // In that case it makes no sense to collapse them.
    if (to == -1)
    {
      to_vertices[col_idx] = from;
      col_idx++;
      vt_from->deleted = true;
      num--;
      continue;
    }

    to_vertices[col_idx] = to;
    csTriangleVertexCost* vt_to = &verts->GetVertex (to);
    col_idx++;

    // Fix connectivity information after moving the 'from' vertex to 'to'.
    for (i = 0 ; i < vt_from->con_triangles.Length () ; i++)
    {
      int id = vt_from->con_triangles[i];
      csTriangle& tr = mesh->GetTriangles ()[id];
      if (tr.a == from) { tr.a = to; vt_to->AddTriangle (id); }
      if (tr.b == from) { tr.b = to; vt_to->AddTriangle (id); }
      if (tr.c == from) { tr.c = to; vt_to->AddTriangle (id); }
    }
    for (i = 0 ; i < vt_from->con_vertices.Length () ; i++)
    {
      int id = vt_from->con_vertices[i];
      if (id != to)
      {
        verts->GetVertex (id).ReplaceVertex (from, to);
	vt_to->AddVertex (id);
      }
    }
    vt_to->DelVertex (from);
    vt_from->deleted = true;
    num--;

    // Recalculate cost for all involved vertices.
    vt_from->CalculateCost (verts);
    vt_to->CalculateCost (verts);
    for (i = 0 ; i < vt_to->con_vertices.Length () ; i++)
    {
      int id = vt_to->con_vertices[i];
      verts->GetVertex (id).CalculateCost (verts);
    }
  }
  // Last index gets the only remaining vertex which should now have
  // minimal cost.
  from_vertices[col_idx] = verts->GetMinimalCostVertex ();
  to_vertices[col_idx] = -1;

  // Vertex 0.
  translate[from_vertices[col_idx]] = 0;
  emerge_from[0] = -1;
  col_idx--;

  // Fill the output arrays.
  for (i = 1 ; i < verts->GetVertexCount () ; i++)
  {
    translate[from_vertices[col_idx]] = i;
    emerge_from[i] = translate[to_vertices[col_idx]];
    col_idx--;
  }

  delete [] from_vertices;
  delete [] to_vertices;
}

//--------------------------------------------------------------------------

