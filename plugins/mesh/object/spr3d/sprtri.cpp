/*
    Copyright (C) 1998,2001 by Jorrit Tyberghein

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
#include "plugins/mesh/object/spr3d/sprtri.h"

//---------------------------------------------------------------------------

csTriangleMesh::csTriangleMesh (const csTriangleMesh& mesh)
{
  max_triangles = mesh.max_triangles;
  num_triangles = mesh.num_triangles;
  triangles = new csTriangle [max_triangles];
  memcpy (triangles, mesh.triangles, sizeof (csTriangle)*max_triangles);
}

csTriangleMesh::~csTriangleMesh ()
{
  Clear ();
}

void csTriangleMesh::Clear ()
{
  delete [] triangles;
  num_triangles = max_triangles = 0;
}

void csTriangleMesh::Reset ()
{
  num_triangles = 0;
}

void csTriangleMesh::AddTriangle (int a, int b, int c)
{
  if (num_triangles >= max_triangles)
  {
    csTriangle* new_triangles = new csTriangle [max_triangles+8];
    if (triangles)
    {
      memcpy (new_triangles, triangles, sizeof (csTriangle)*max_triangles);
      delete [] triangles;
    }
    triangles = new_triangles;
    max_triangles += 8;
  }
  triangles[num_triangles].a = a;
  triangles[num_triangles].b = b;
  triangles[num_triangles].c = c;
  num_triangles++;
}

//---------------------------------------------------------------------------

void csTriangleVertex::AddTriangle (int idx)
{
  int i;
  for (i = 0 ; i < num_con_triangles ; i++)
    if (con_triangles[i] == idx) return;

  if (num_con_triangles >= max_con_triangles)
  {
    int* new_con_triangles = new int [max_con_triangles+4];
    if (con_triangles)
    {
      memcpy (new_con_triangles, con_triangles, sizeof (int)*max_con_triangles);
      delete [] con_triangles;
    }
    con_triangles = new_con_triangles;
    max_con_triangles += 4;
  }
  con_triangles[num_con_triangles] = idx;
  num_con_triangles++;
}

void csTriangleVertex::AddVertex (int idx)
{
  int i;
  for (i = 0 ; i < num_con_vertices ; i++)
    if (con_vertices[i] == idx) return;

  if (num_con_vertices >= max_con_vertices)
  {
    int* new_con_vertices = new int [max_con_vertices+4];
    if (con_vertices)
    {
      memcpy (new_con_vertices, con_vertices, sizeof (int)*max_con_vertices);
      delete [] con_vertices;
    }
    con_vertices = new_con_vertices;
    max_con_vertices += 4;
  }
  con_vertices[num_con_vertices] = idx;
  num_con_vertices++;
}

bool csTriangleVertex::DelVertex (int idx)
{
  int i;
  for (i = 0 ; i < num_con_vertices ; i++)
    if (con_vertices[i] == idx)
    {
      if (i != num_con_vertices-1) memmove (con_vertices+i, con_vertices+i+1, sizeof (int)*(num_con_vertices-i-1));
      num_con_vertices--;
      return true;
    }
  return false;
}

void csTriangleVertex::ReplaceVertex (int old, int replace)
{
  if (DelVertex (old)) AddVertex (replace);
}

void csTriangleVertex::CalculateCost (csTriangleVertices* vertices)
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
  for (i = 0 ; i < num_con_vertices ; i++)
  {
    float sq_dist = csSquaredDist::PointPoint (vertices->GetVertex (idx).pos, vertices->GetVertex (con_vertices[i]).pos);
    if (sq_dist < min_sq_dist)
    {
      min_sq_dist = sq_dist;
      to_vertex = con_vertices[i];
    }
  }
  cost = min_sq_dist;
}

csTriangleVertices::csTriangleVertices (csTriangleMesh* mesh, csVector3* verts, int num_verts)
{
  vertices = new csTriangleVertex [num_verts];
  num_vertices = num_verts;

  // Build connectivity information for all vertices in this mesh.
  csTriangle* triangles = mesh->GetTriangles ();
  int i, j;
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i].pos = verts[i];
    vertices[i].idx = i;
    for (j = 0 ; j < mesh->GetNumTriangles () ; j++)
      if (triangles[j].a == i || triangles[j].b == i || triangles[j].c == i)
      {
        vertices[i].AddTriangle (j);
	if (triangles[j].a != i) vertices[i].AddVertex (triangles[j].a);
	if (triangles[j].b != i) vertices[i].AddVertex (triangles[j].b);
	if (triangles[j].c != i) vertices[i].AddVertex (triangles[j].c);
      }
  }
}

csTriangleVertices::~csTriangleVertices ()
{
  delete [] vertices;
}

void csTriangleVertices::UpdateVertices (csVector3* verts)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i].pos = verts[i];
}

int csTriangleVertices::GetMinimalCostVertex ()
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

void csTriangleVertices::CalculateCost ()
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i].CalculateCost (this);
}

void csTriangleVertices::Dump ()
{
  printf ("=== Dump ===\n");
  int i, j;
  for (i = 0 ; i < num_vertices ; i++)
  {
    printf ("  %d idx=%d del=%d cost=%f to=%d tri=[ ", i, vertices[i].idx, (int)vertices[i].deleted,
      vertices[i].cost, vertices[i].to_vertex);
    for (j = 0 ; j < vertices[i].num_con_triangles ; j++)
      printf ("%d ", vertices[i].con_triangles[j]);
    printf ("] vt=[ ");
    for (j = 0 ; j < vertices[i].num_con_vertices ; j++)
      printf ("%d ", vertices[i].con_vertices[j]);
    printf ("]\n");
    if (!vertices[i].deleted)
      for (j = 0 ; j < vertices[i].num_con_vertices ; j++)
        if (vertices[vertices[i].con_vertices[j]].deleted)
          printf ("ERROR refering deleted vertex %d!\n", vertices[i].con_vertices[j]);
  }
}

//---------------------------------------------------------------------------

void csLOD::CalculateLOD (csTriangleMesh* mesh, csTriangleVertices* verts,
	int* translate, int* emerge_from)
{
  int i;
  // Calculate the cost for all vertices for the first time.
  // This information will change locally whenever vertices are collapsed.
  verts->CalculateCost ();

  // Collapse vertices, one by one until only one remains.
  int num = verts->GetNumVertices ();
  int from, to, col_idx;
  int *from_vertices, *to_vertices;
  from_vertices = new int [num];
  to_vertices = new int [num];
  col_idx = 0;
  while (num > 1)
  {
    from = verts->GetMinimalCostVertex ();
    from_vertices[col_idx] = from;
    csTriangleVertex* vt_from = &verts->GetVertex (from);

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
    csTriangleVertex* vt_to = &verts->GetVertex (to);
    col_idx++;

    // Fix connectivity information after moving the 'from' vertex to 'to'.
    for (i = 0 ; i < vt_from->num_con_triangles ; i++)
    {
      int id = vt_from->con_triangles[i];
      csTriangle& tr = mesh->GetTriangles ()[id];
      if (tr.a == from) { tr.a = to; vt_to->AddTriangle (id); }
      if (tr.b == from) { tr.b = to; vt_to->AddTriangle (id); }
      if (tr.c == from) { tr.c = to; vt_to->AddTriangle (id); }
    }
    for (i = 0 ; i < vt_from->num_con_vertices ; i++)
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
    for (i = 0 ; i < vt_to->num_con_vertices ; i++)
    {
      int id = vt_to->con_vertices[i];
      verts->GetVertex (id).CalculateCost (verts);
    }
  }
  // Last index gets the only remaining vertex which should now have minimal cost.
  from_vertices[col_idx] = verts->GetMinimalCostVertex ();
  to_vertices[col_idx] = -1;

  // Fill the output arrays.
  for (i = 0 ; i < verts->GetNumVertices () ; i++)
  {
    translate[from_vertices[col_idx]] = i;
    emerge_from[i] = translate[to_vertices[col_idx]];
    col_idx--;
  }

  delete [] from_vertices;
  delete [] to_vertices;
}

//---------------------------------------------------------------------------
