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
  size_t idxidx = con_vertices.Find (idx);
  if (idxidx == csArrayItemNotFound)
    return false;
  con_vertices.DeleteIndex (idxidx);
  return true;
}

void csTriangleVertexCost::ReplaceVertex (int old, int replace)
{
  if (DelVertex (old)) AddVertex (replace);
}

//=========================================================================

void csTriangleLODAlgoEdge::CalculateCost (csTriangleVerticesCost* vertices,
	csTriangleVertexCost* vertex)
{
  size_t i;
  vertex->to_vertex = -1;
  float min_sq_dist = 1000000.;
  if (vertex->deleted)
  {
    // If the vertex is deleted we have a very high cost.
    // The cost is higher than the maximum cost you can get for
    // a non-deleted vertex. This is to make sure that we get
    // the last non-deleted vertex at the end of the LOD algorithm.
    vertex->cost = min_sq_dist+1;
    return;
  }
  const csVector3& this_pos = vertex->pos;
  for (i = 0 ; i < vertex->con_vertices.Length () ; i++)
  {
    float sq_dist = csSquaredDist::PointPoint (this_pos,
    	vertices->GetVertex (vertex->con_vertices[i]).pos);
    if (sq_dist < min_sq_dist)
    {
      min_sq_dist = sq_dist;
      vertex->to_vertex = vertex->con_vertices[i];
    }
  }
  vertex->cost = min_sq_dist;
}

//=========================================================================

csTriangleVerticesCost::csTriangleVerticesCost (csTriangleMesh* mesh,
	csVector3* verts, int num_verts)
{
  vertices = new csTriangleVertexCost[num_verts];
  num_vertices = num_verts;

  // Build connectivity information for all vertices in this mesh.
  csTriangle* triangles = mesh->GetTriangles ();
  size_t j;

  // First add the triangles that are used by every vertex.
  size_t tricount = mesh->GetTriangleCount ();
  for (j = 0 ; j < tricount ; j++)
  {
    vertices[triangles[j].a].AddTriangle (j);
    vertices[triangles[j].b].AddTriangle (j);
    vertices[triangles[j].c].AddTriangle (j);
  }
  // Now setup the vertices and add the connected vertices.
  int i;
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i].pos = verts[i];
    vertices[i].idx = i;
    for (j = 0 ; j < vertices[i].con_triangles.Length () ; j++)
    {
      size_t triidx = vertices[i].con_triangles[j];
      if (triangles[triidx].a != i) vertices[i].AddVertex (triangles[triidx].a);
      if (triangles[triidx].b != i) vertices[i].AddVertex (triangles[triidx].b);
      if (triangles[triidx].c != i) vertices[i].AddVertex (triangles[triidx].c);
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

int csTriangleVerticesCost::GetMinimalCostVertex (float& min_cost)
{
  int i;
  int min_idx = -1;
  min_cost = 2.+1000000.;
  for (i = 0 ; i < num_vertices ; i++)
    if (!vertices[i].deleted && vertices[i].cost < min_cost)
    {
      min_idx = i;
      min_cost = vertices[i].cost;
    }
  return min_idx;
}

csTriangleVerticesSorted* csTriangleVerticesCost::SortVertices ()
{
  csTriangleVerticesSorted* sorted = new csTriangleVerticesSorted (
  	this);
  return sorted;
}

void csTriangleVerticesCost::CalculateCost (csTriangleLODAlgo* lodalgo)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    lodalgo->CalculateCost (this, &vertices[i]);
}

void csTriangleVerticesCost::Dump ()
{
}

//---------------------------------------------------------------------------

static csTriangleVertexCost* sort_table;
static int compare_vt_cost (const void* p1, const void* p2)
{
  int i1 = *(int*)p1;
  int i2 = *(int*)p2;
  if (sort_table[i1].cost < sort_table[i2].cost)
    return -1;
  else if (sort_table[i1].cost > sort_table[i2].cost)
    return 1;
  else
    return 0;
}

csTriangleVerticesSorted::csTriangleVerticesSorted (
	csTriangleVerticesCost* vertices)
{
  csTriangleVerticesSorted::vertices = vertices;
  verts = vertices->GetVertices ();
  num_vertices = vertices->GetVertexCount ();

  entry_per_vertex = new csList<int>::Iterator [num_vertices];
  int* sorted_idx = new int[num_vertices];
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    sorted_idx[i] = i;
  sort_table = verts;
  qsort (sorted_idx, num_vertices, sizeof (int), compare_vt_cost);
  for (i = 0 ; i < num_vertices ; i++)
  {
    entry_per_vertex[sorted_idx[i]] = sorted_list.PushBack (sorted_idx[i]);
  }
  delete[] sorted_idx;
}

csTriangleVerticesSorted::~csTriangleVerticesSorted ()
{
  delete[] entry_per_vertex;
}

int csTriangleVerticesSorted::GetLowestCostVertex ()
{
  if (sorted_list.IsEmpty ()) return -1;
  int vt = sorted_list.Front ();
  sorted_list.PopFront ();
  return vt;
}

void csTriangleVerticesSorted::ChangeCostVertex (int vtidx)
{
  // First check if we need to resort our entry. To do that
  // we copy the iterator in entry_per_vertex and check left/right.
  csList<int>::Iterator it = entry_per_vertex[vtidx];

  // Cost of this vertex.
  float cost = verts[vtidx].cost;

  // Check if we still have a higher cost then the vertex left of us.
  if (it.HasPrevious () && cost < verts[it.FetchPrevious ()].cost)
  {
    // No, we are lower. So we need to move it down.

    // Remember our current element so that we can move
    // it away later.
    csList<int>::Iterator it_cur = it;

    it.Prev ();
    while (it.HasPrevious () && cost < verts[it.FetchPrevious ()].cost)
    {
      it.Previous ();
    }
    // 'it' now points to the element in the list just after where
    // we want to move our current vertex.
    sorted_list.MoveBefore (it, it_cur);
    return;
  }

  // Check if we still have a lower cost then the vertex right of us.
  if (it.HasNext () && cost > verts[it.FetchNext ()].cost)
  {
    // No, we are higher. So we need to move it up.

    // Remember our current element so that we can move
    // it away later.
    csList<int>::Iterator it_cur = it;

    it.Next ();
    while (it.HasNext () && cost > verts[it.FetchNext ()].cost)
    {
      it.Next ();
    }
    // 'it' now points to the element in the list just before where
    // we want to move our current vertex.
    sorted_list.MoveAfter (it, it_cur);
    return;
  }
}

//---------------------------------------------------------------------------

void csTriangleMeshLOD::CalculateLOD (csTriangleMesh* mesh,
	csTriangleVerticesCost* verts, int* translate, int* emerge_from,
	csTriangleLODAlgo* lodalgo)
{
  size_t i;
  // Calculate the cost for all vertices for the first time.
  // This information will change locally whenever vertices are collapsed.
  verts->CalculateCost (lodalgo);

  // Sort the vertices.
  csTriangleVerticesSorted* sorted = verts->SortVertices ();

  // Collapse vertices, one by one until only one remains.
  int num = verts->GetVertexCount ();
  int from, to, col_idx;
  int *from_vertices, *to_vertices;
  from_vertices = new int [num];
  to_vertices = new int [num];

  col_idx = 0;
  while (num > 1)
  {
    from = sorted->GetLowestCostVertex ();
    csTriangleVertexCost* vt_from = &verts->GetVertex (from);

    from_vertices[col_idx] = from;
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
      size_t id = vt_from->con_triangles[i];
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
    lodalgo->CalculateCost (verts, vt_to);
    sorted->ChangeCostVertex (vt_to->idx);
    for (i = 0 ; i < vt_to->con_vertices.Length () ; i++)
    {
      int id = vt_to->con_vertices[i];
      lodalgo->CalculateCost (verts, &verts->GetVertex (id));
      sorted->ChangeCostVertex (id);
    }
  }

  // Last index gets the only remaining vertex which should now have
  // minimal cost.
  from_vertices[col_idx] = sorted->GetLowestCostVertex ();
  CS_ASSERT (from_vertices[col_idx] != -1);
  to_vertices[col_idx] = -1;

  delete sorted;

  // Vertex 0.
  translate[from_vertices[col_idx]] = 0;
  emerge_from[0] = -1;
  col_idx--;

  int j;
  // Fill the output arrays.
  for (j = 1 ; j < verts->GetVertexCount () ; j++)
  {
    translate[from_vertices[col_idx]] = j;
    emerge_from[j] = translate[to_vertices[col_idx]];
    col_idx--;
  }

  delete [] from_vertices;
  delete [] to_vertices;
}

static int FindVertex (int* move_table, int idx)
{
  int newidx = move_table[idx];
  while (newidx != idx)
  {
    CS_ASSERT (newidx != -1);
    idx = newidx;
    newidx = move_table[idx];
  }
  return newidx;
}

csTriangle* csTriangleMeshLOD::CalculateLOD (csTriangleMesh* mesh,
	csTriangleVerticesCost* verts, float max_cost,
	int& num_triangles,
	csTriangleLODAlgo* lodalgo)
{
  size_t i;
  // Calculate the cost for all vertices for the first time.
  // This information will change locally whenever vertices are collapsed.
  verts->CalculateCost (lodalgo);

  int num = verts->GetVertexCount ();
  int from, to;

  // Table which keeps information on how the vertices move.
  int* move_table = new int[num];
  for (int j = 0 ; j < num ; j++)
    move_table[j] = j;

  // Sort the vertices.
  csTriangleVerticesSorted* sorted = verts->SortVertices ();

  // Collapse vertices, one by one until only one remains.
  while (num > 1)
  {
    from = sorted->GetLowestCostVertex ();
    csTriangleVertexCost* vt_from = &verts->GetVertex (from);
    float min_cost = vt_from->cost;
    if (min_cost >= max_cost) break;

    to = verts->GetVertex (from).to_vertex;

    CS_ASSERT (from != to);
    move_table[from] = to;

    // If to == -1 then it is possible that we have solitary vertices.
    // In that case it makes no sense to collapse them.
    if (to == -1)
    {
      vt_from->deleted = true;
      num--;
      continue;
    }

    csTriangleVertexCost* vt_to = &verts->GetVertex (to);

    // Fix connectivity information after moving the 'from' vertex to 'to'.
    for (i = 0 ; i < vt_from->con_triangles.Length () ; i++)
    {
      size_t id = vt_from->con_triangles[i];
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
    bool isdel = vt_to->DelVertex (from);
    (void)isdel; // Pacify compiler in non-debug mode.
    CS_ASSERT (isdel == true);
    vt_from->deleted = true;
    num--;

    // Recalculate cost for all involved vertices.
    lodalgo->CalculateCost (verts, vt_to);
    sorted->ChangeCostVertex (vt_to->idx);
    for (i = 0 ; i < vt_to->con_vertices.Length () ; i++)
    {
      int id = vt_to->con_vertices[i];
      lodalgo->CalculateCost (verts, &verts->GetVertex (id));
      sorted->ChangeCostVertex (id);
    }
  }
  delete sorted;

  // Add all triangles that are not deleted to the result array.
  csTriangle* triangles = new csTriangle[mesh->GetTriangleCount ()];
  num_triangles = 0;
  for (i = 0 ; i < mesh->GetTriangleCount () ; i++)
  {
    csTriangle& tr = mesh->GetTriangles ()[i];
    csTriangle& ntr = triangles[num_triangles];
    ntr.a = FindVertex (move_table, tr.a);
    ntr.b = FindVertex (move_table, tr.b);
    ntr.c = FindVertex (move_table, tr.c);
    if (ntr.a != ntr.b && ntr.a != ntr.c && ntr.b != ntr.c)
    { 
      num_triangles++;
    }
  }

  delete[] move_table;

  return triangles;
}

csTriangle* csTriangleMeshLOD::CalculateLODFast (csTriangleMesh* mesh,
	csTriangleVerticesCost* verts, float max_cost,
	int& num_triangles,
	csTriangleLODAlgo* lodalgo)
{
  size_t i;
  // Calculate the cost for all vertices for the first time.
  // This information will change locally whenever vertices are collapsed.
  verts->CalculateCost (lodalgo);

  int num = verts->GetVertexCount ();
  int from, to;

  // Table which keeps information on how the vertices move.
  int* move_table = new int[num];
  for (int j = 0 ; j < num ; j++)
    move_table[j] = j;

  // Sort the vertices.
  csTriangleVerticesSorted* sorted = verts->SortVertices ();

  // Collapse vertices, one by one until only one remains.
  while (num > 1)
  {
    from = sorted->GetLowestCostVertex ();
    csTriangleVertexCost* vt_from = &verts->GetVertex (from);
    float min_cost = vt_from->cost;
    // Recalculation of cost after edge collapse may cause the
    // cost of a single vertex to become very high. In the normal
    // algorithm we know we have perfect sorting when we get the
    // lowest cost vertex. Here we don't know that so we just
    // skip this vertex and continue.
    if (min_cost >= max_cost) { num--; continue; }

    to = verts->GetVertex (from).to_vertex;

    CS_ASSERT (from != to);
    move_table[from] = to;

    // If to == -1 then it is possible that we have solitary vertices.
    // In that case it makes no sense to collapse them.
    if (to == -1)
    {
      vt_from->deleted = true;
      num--;
      continue;
    }

    csTriangleVertexCost* vt_to = &verts->GetVertex (to);

    // Fix connectivity information after moving the 'from' vertex to 'to'.
    for (i = 0 ; i < vt_from->con_triangles.Length () ; i++)
    {
      size_t id = vt_from->con_triangles[i];
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
    bool isdel = vt_to->DelVertex (from);
    (void)isdel; // Pacify compiler in non-debug mode.
    CS_ASSERT (isdel);
    vt_from->deleted = true;
    num--;

    // Recalculate cost for all involved vertices.
    // In this version we will not actually do the resorting.
    // We still have to recalculate cost to make sure that internal
    // data is recalculated correctly. i.e. some edge collapses
    // may change the cost of a vertex collapse to a very high
    // value.
    lodalgo->CalculateCost (verts, vt_to);
    for (i = 0 ; i < vt_to->con_vertices.Length () ; i++)
    {
      int id = vt_to->con_vertices[i];
      lodalgo->CalculateCost (verts, &verts->GetVertex (id));
    }
  }
  delete sorted;

  // Add all triangles that are not deleted to the result array.
  csTriangle* triangles = new csTriangle[mesh->GetTriangleCount ()];
  num_triangles = 0;
  for (i = 0 ; i < mesh->GetTriangleCount () ; i++)
  {
    csTriangle& tr = mesh->GetTriangles ()[i];
    csTriangle& ntr = triangles[num_triangles];
    ntr.a = FindVertex (move_table, tr.a);
    ntr.b = FindVertex (move_table, tr.b);
    ntr.c = FindVertex (move_table, tr.c);
    if (ntr.a != ntr.b && ntr.a != ntr.c && ntr.b != ntr.c)
    { 
      num_triangles++;
    }
  }

  delete[] move_table;

  return triangles;
}

//--------------------------------------------------------------------------

