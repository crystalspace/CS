/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include "sysdef.h"
#include "csengine/polyint.h"
#include "csengine/treeobj.h"
#include "csengine/bsp.h"

//---------------------------------------------------------------------------

csBspNode::csBspNode ()
{
  front = back = NULL;
  dynamic_idx = -1;
}

csBspNode::~csBspNode ()
{
  CHK (delete front);
  CHK (delete back);
}

void csBspNode::AddPolygon (csPolygonInt* poly, bool dynamic)
{
  polygons.AddPolygon (poly);
  if (dynamic && dynamic_idx == -1) dynamic_idx = polygons.GetNumPolygons ()-1;
}

void csBspNode::RemoveDynamicPolygons ()
{
  if (dynamic_idx != -1)
  {
    polygons.SetNumPolygons (dynamic_idx);
    dynamic_idx = -1;
  }
  if (front)
  {
    front->RemoveDynamicPolygons ();
    if (front->IsEmpty ())
    {
      CHK (delete front);
      front = NULL;
    }
  }
  if (back)
  {
    back->RemoveDynamicPolygons ();
    if (back->IsEmpty ())
    {
      CHK (delete back);
      back = NULL;
    }
  }
}

int csBspNode::CountVertices ()
{
  int num_verts = 0;
  if (front) num_verts += front->CountVertices ();
  if (back) num_verts += back->CountVertices ();
  int i;
  for (i = 0 ; i < polygons.GetNumPolygons () ; i++)
  {
    num_verts += polygons.GetPolygon (i)->GetNumVertices ();
    if (polygons.GetPolygon (i)->GetUnsplitPolygon ())
      num_verts += polygons.GetPolygon (i)->GetUnsplitPolygon ()->GetNumVertices ();
  }
  return num_verts;
}

void csBspNode::FetchVertices (int* array, int& cur_idx)
{
  if (front) front->FetchVertices (array, cur_idx);
  if (back) back->FetchVertices (array, cur_idx);
  int i;
  for (i = 0 ; i < polygons.GetNumPolygons () ; i++)
  {
    int* idx = polygons.GetPolygon (i)->GetVertexIndices ();
    int n = polygons.GetPolygon (i)->GetNumVertices ();
    memcpy (array+cur_idx, idx, sizeof (int)*n);
    cur_idx += n;
    if (polygons.GetPolygon (i)->GetUnsplitPolygon ())
    {
      int* idx = polygons.GetPolygon (i)->GetUnsplitPolygon ()->GetVertexIndices ();
      int n = polygons.GetPolygon (i)->GetUnsplitPolygon ()->GetNumVertices ();
      memcpy (array+cur_idx, idx, sizeof (int)*n);
      cur_idx += n;
    }
  }
}

//---------------------------------------------------------------------------

csBspTree::csBspTree (csPolygonParentInt* pset, int mode) : csPolygonTree (pset)
{
  csBspTree::mode = mode;
}

csBspTree::~csBspTree ()
{
  Clear ();
}

void csBspTree::Build ()
{
  int i;
  int num = pset->GetNumPolygons ();
  CHK (csPolygonInt** polygons = new csPolygonInt* [num]);
  for (i = 0 ; i < num ; i++) polygons[i] = pset->GetPolygon (i);

  CHK (root = new csBspNode);

  Build ((csBspNode*)root, polygons, num);

  CHK (delete [] polygons);
}

void csBspTree::Build (csPolygonInt** polygons, int num)
{
  CHK (root = new csBspNode);

  CHK (csPolygonInt** new_polygons = new csPolygonInt* [num]);
  int i;
  for (i = 0 ; i < num ; i++) new_polygons[i] = polygons[i];
  Build ((csBspNode*)root, new_polygons, num);
  CHK (delete [] new_polygons);
}

void csBspTree::AddDynamicPolygons (csPolygonInt** polygons, int num)
{
  // @@@ We should only do this copy if there is a split in the first level.
  // Now it is just overhead.
  if (!root) { CHK (root = new csBspNode); }
  CHK (csPolygonInt** new_polygons = new csPolygonInt* [num]);
  int i;
  for (i = 0 ; i < num ; i++) new_polygons[i] = polygons[i];
  BuildDynamic ((csBspNode*)root, new_polygons, num);
  CHK (delete [] new_polygons);
}

void csBspTree::RemoveDynamicPolygons ()
{
  if (root)
  {
    root->RemoveDynamicPolygons ();
    if (root->IsEmpty ())
    {
      CHK (delete root);
      root = NULL;
    }
  }
}

int csBspTree::SelectSplitter (csPolygonInt** polygons, int num)
{
  int i, j, poly_idx;

  poly_idx = -1;
  if (mode == BSP_RANDOM)
  {
    poly_idx = rand () % num;
  }
  else if (mode == BSP_MINIMIZE_SPLITS)
  {
    // Choose the polygon which generates the least number of splits.
    int min_splits = 32767;
    for (i = 0 ; i < num ; i++)
    {
      int cnt = 0;
      for (j = 0 ; j < num ; j++)
        if (polygons[j]->Classify (*polygons[i]->GetPolyPlane ()) == POL_SPLIT_NEEDED) cnt++;
      if (cnt < min_splits) { min_splits = cnt; poly_idx = i; }
    }
  }
  else if (mode == BSP_ALMOST_MINIMIZE_SPLITS)
  {
    // Choose the polygon which generates the least number of splits.
    int min_splits = 32767;
    int n = num;
    if (n > 20) n = 20;
    for (i = 0 ; i < n ; i++)
    {
      int ii = rand () % num;
      int cnt = 0;
      for (j = 0 ; j < n ; j++)
      {
        int jj = rand () % num;
        if (polygons[jj]->Classify (*polygons[ii]->GetPolyPlane ()) == POL_SPLIT_NEEDED) cnt++;
      }
      if (cnt < min_splits) { min_splits = cnt; poly_idx = ii; }
    }
  }
  else if (mode == BSP_BALANCED)
  {
    // Choose the polygon which generates a balanced tree.
    int min_front_back_diff = 32767;
    int min_splits = 32767;
    for (i = 0 ; i < num ; i++)
    {
      int front = 0, back = 0;
      int splits = 0;
      for (j = 0 ; j < num ; j++)
      {
        int c = polygons[j]->Classify (*polygons [i]->GetPolyPlane ());
	if (c == POL_FRONT) front++;
	else if (c == POL_BACK) back++;
	else if (c == POL_SPLIT_NEEDED) splits++;
      }
      if (ABS (front-back) < min_front_back_diff)
      {
        min_splits = 32767;
        min_front_back_diff = ABS (front-back);
	poly_idx = i;
      }
      else if (ABS (front-back) == min_front_back_diff)
      {
        if (splits < min_splits) { min_splits = splits; poly_idx = i; }
      }
    }
  }
  else if (mode == BSP_ALMOST_BALANCED)
  {
    // Choose the polygon which generates a balanced tree.
    int min_front_back_diff = 32767;
    int min_splits = 32767;
    int n = num;
    if (n > 20) n = 20;
    for (i = 0 ; i < n ; i++)
    {
      int ii = rand () % num;
      int front = 0, back = 0;
      int splits = 0;
      for (j = 0 ; j < n ; j++)
      {
        int jj = rand () % num;
        int c = polygons[jj]->Classify (*polygons [ii]->GetPolyPlane ());
	if (c == POL_FRONT) front++;
	else if (c == POL_BACK) back++;
	else if (c == POL_SPLIT_NEEDED) splits++;
      }
      if (ABS (front-back) < min_front_back_diff)
      {
        min_splits = 32767;
        min_front_back_diff = ABS (front-back);
	poly_idx = ii;
      }
      else if (ABS (front-back) == min_front_back_diff)
      {
        if (splits < min_splits) { min_splits = splits; poly_idx = ii; }
      }
    }
  }
  else if (mode == BSP_MOST_ON_SPLITTER)
  {
    // First choose a polygon which shares its plane with the highest
    // number of other polygons.
    int same_poly = 0;
    for (i = 0 ; i < num ; i++)
    {
      csPlane* plane_i = polygons[i]->GetPolyPlane ();
      int cnt = 1;
      for (j = i+1 ; j < num ; j++)
      {
        if (plane_i == polygons[j]->GetPolyPlane () ||
		csMath3::PlanesEqual (*plane_i, *polygons[j]->GetPolyPlane ()))
	  cnt++;
      }
      if (cnt > same_poly) { same_poly = cnt; poly_idx = i; }
    }
  }
  return poly_idx;
}

void csBspTree::Build (csBspNode* node, csPolygonInt** polygons, int num)
{
  int i;
  csPolygonInt* split_poly = polygons[SelectSplitter (polygons, num)];
  csPlane* split_plane = split_poly->GetPolyPlane ();

  // Now we split the node according to the plane of that polygon.
  CHK (csPolygonInt** front_poly = new csPolygonInt* [num]);
  CHK (csPolygonInt** back_poly = new csPolygonInt* [num]);
  int front_idx = 0, back_idx = 0;

  for (i = 0 ; i < num ; i++)
  {
    int c = polygons[i]->Classify (*split_plane);
    switch (c)
    {
      case POL_SAME_PLANE: node->AddPolygon (polygons[i]); break;
      case POL_FRONT: front_poly[front_idx++] = polygons[i]; break;
      case POL_BACK: back_poly[back_idx++] = polygons[i]; break;
      case POL_SPLIT_NEEDED:
	{
	  csPolygonInt* np1, * np2;
	  polygons[i]->SplitWithPlane (&np1, &np2, *split_plane);
	  front_poly[front_idx++] = np1;
	  back_poly[back_idx++] = np2;
	}
	break;

    }
  }

  if (front_idx)
  {
    CHK (node->front = new csBspNode);
    Build (node->front, front_poly, front_idx);
  }
  if (back_idx)
  {
    CHK (node->back = new csBspNode);
    Build (node->back, back_poly, back_idx);
  }

  CHK (delete [] front_poly);
  CHK (delete [] back_poly);
}

void csBspTree::BuildDynamic (csBspNode* node, csPolygonInt** polygons, int num)
{
  int i;
  csPolygonInt* split_poly;
  if (node->polygons.GetNumPolygons ()) split_poly = node->polygons.GetPolygon (0);
  else split_poly = polygons[SelectSplitter (polygons, num)];
  csPlane* split_plane = split_poly->GetPolyPlane ();

  // Now we split the list of polygons according to the plane of that polygon.
  CHK (csPolygonInt** front_poly = new csPolygonInt* [num]);
  CHK (csPolygonInt** back_poly = new csPolygonInt* [num]);
  int front_idx = 0, back_idx = 0;

  for (i = 0 ; i < num ; i++)
  {
    int c = polygons[i]->Classify (*split_plane);
    switch (c)
    {
      case POL_SAME_PLANE: node->AddPolygon (polygons[i], true); break;
      case POL_FRONT: front_poly[front_idx++] = polygons[i]; break;
      case POL_BACK: back_poly[back_idx++] = polygons[i]; break;
      case POL_SPLIT_NEEDED:
	{
    	  // In case of dynamic polygon adding we don't split the polygon.
	  //csPolygonInt* np1, * np2;
	  //polygons[i]->SplitWithPlane (&np1, &np2, *split_plane);
	  //front_poly[front_idx++] = np1;
	  //back_poly[back_idx++] = np2;
	  front_poly[front_idx++] = polygons[i];
	  back_poly[back_idx++] = polygons[i];
	}
	break;

    }
  }

  if (front_idx)
  {
    if (!node->front) CHKB (node->front = new csBspNode);
    BuildDynamic (node->front, front_poly, front_idx);
  }
  if (back_idx)
  {
    if (!node->back) CHKB (node->back = new csBspNode);
    BuildDynamic (node->back, back_poly, back_idx);
  }

  CHK (delete [] front_poly);
  CHK (delete [] back_poly);
}

void* csBspTree::HandleObjects (csBspNode* node, const csVector3& /*pos*/,
  	csTreeVisitFunc* func, void* data)
{
  csPolygonStub* stub;
  while (node->todo_stubs)
  {
    stub = node->todo_stubs;
    node->UnlinkStub (stub);	// Unlink from todo list.
    csPolygonStub* stub_on, * stub_front, * stub_back;
    stub->GetObject ()->SplitWithPlane (stub, &stub_on, &stub_front, &stub_back,
    	*(node->polygons.GetPolygon (0)->GetPolyPlane ()));
    // Link the stub with the polygons on this splitter plane to the current node.
    if (stub_on) node->LinkStub (stub_on);	// Relink to normal list.
    // Link the stub with polygons in front to the todo list of the front node.
    if (stub_front) node->front->LinkStubTodo (stub_front);
    // Link the stub with back polygons to the todo list of the back node.
    if (stub_back) node->back->LinkStubTodo (stub_back);
    // Free the old stub.
    csPolyTreeObject::stub_pool.Free (stub);
  }
  void* rc;
  stub = node->first_stub;
  while (stub)
  {
    rc = func (pset, stub->GetPolygons (), stub->GetNumPolygons (), data);
    stub = stub->GetNextTree ();
  }
  return NULL;
}

void* csBspTree::Back2Front (const csVector3& pos, csTreeVisitFunc* func,
	void* data, csTreeCullFunc* cullfunc, void* culldata)
{
  return Back2Front ((csBspNode*)root, pos, func, data, cullfunc, culldata);
}

void* csBspTree::Front2Back (const csVector3& pos, csTreeVisitFunc* func,
	void* data, csTreeCullFunc* cullfunc, void* culldata)
{
  return Front2Back ((csBspNode*)root, pos, func, data, cullfunc, culldata);
}

void* csBspTree::Back2Front (csBspNode* node, const csVector3& pos,
	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata)
{
  if (!node) return NULL;
  void* rc;

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if (csMath3::Visible (pos, *(node->polygons.GetPolygon (0)->GetPolyPlane ())))
  {
    // Front.
    rc = Back2Front (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
    rc = func (pset, node->polygons.GetPolygons (), node->polygons.GetNumPolygons (), data);
    if (rc) return rc;
    rc = HandleObjects (node, pos, func, data);
    if (rc) return rc;
    rc = Back2Front (node->front, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Back2Front (node->front, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
    rc = func (pset, node->polygons.GetPolygons (), node->polygons.GetNumPolygons (), data);
    if (rc) return rc;
    rc = HandleObjects (node, pos, func, data);
    if (rc) return rc;
    rc = Back2Front (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
  }
  return NULL;
}

void* csBspTree::Front2Back (csBspNode* node, const csVector3& pos,
	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata)
{
  if (!node) return NULL;
  void* rc;

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if (csMath3::Visible (pos, *(node->polygons.GetPolygon (0)->GetPolyPlane ())))
  {
    // Front.
    rc = Front2Back (node->front, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
    rc = func (pset, node->polygons.GetPolygons (), node->polygons.GetNumPolygons (), data);
    if (rc) return rc;
    rc = HandleObjects (node, pos, func, data);
    if (rc) return rc;
    rc = Front2Back (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Front2Back (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
    rc = func (pset, node->polygons.GetPolygons (), node->polygons.GetNumPolygons (), data);
    if (rc) return rc;
    rc = HandleObjects (node, pos, func, data);
    if (rc) return rc;
    rc = Front2Back (node->front, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
  }
  return NULL;
}

void csBspTree::Statistics (csBspNode* node, int depth, int* num_nodes,
	int* num_leaves, int* max_depth,
  	int* tot_polygons, int* max_poly_in_node, int* min_poly_in_node)
{
  depth++;
  if (depth > *max_depth) *max_depth = depth;

  int num = node->polygons.GetNumPolygons ();
  *tot_polygons += num;
  if (num > *max_poly_in_node) *max_poly_in_node = num;
  if (num < *min_poly_in_node) *min_poly_in_node = num;
  if (node->front || node->back) (*num_nodes)++;
  else (*num_leaves)++;
  if (node->front) Statistics (node->front, depth, num_nodes,
  	num_leaves, max_depth, tot_polygons, max_poly_in_node,
	min_poly_in_node);
  if (node->back) Statistics (node->back, depth, num_nodes,
  	num_leaves, max_depth, tot_polygons, max_poly_in_node,
	min_poly_in_node);
  depth--;
}

void csBspTree::Statistics (int* num_nodes, int* num_leaves, int* max_depth,
  	int* tot_polygons, int* max_poly_in_node, int* min_poly_in_node)
{
  *num_nodes = 0;
  *num_leaves = 0;
  *max_depth = 0;
  *tot_polygons = 0;
  *max_poly_in_node = 0;
  *min_poly_in_node = 10000000;
  if (root) Statistics ((csBspNode*)root, 0, num_nodes, num_leaves,
  	max_depth, tot_polygons,
  	max_poly_in_node, min_poly_in_node);
}

int compare_int (const void* p1, const void* p2)
{
  int i1 = *(int*)p1;
  int i2 = *(int*)p2;
  if (i1 < i2) return -1;
  else if (i1 > i2) return 1;
  return 0;
}

int* csBspTree::GetVertices (int& count)
{
  if (!root) return 0;
  int cnt = ((csBspNode*)root)->CountVertices ();
  CHK (int* idx = new int [cnt]);
  CHK (int* idx2 = new int [cnt]);
  int cur_idx = 0;
  ((csBspNode*)root)->FetchVertices (idx, cur_idx);

  // Sort first.
  qsort (idx, cur_idx, sizeof (int), compare_int);
  // Remove all doubles.
  idx2[0] = idx[0];
  int i, j;
  j = 1;
  for (i = 1 ; i < cur_idx ; i++)
  {
    if (idx[i] != idx2[j-1]) idx2[j++] = idx[i];
  }
  CHK (int* indices = new int [j]);
  memcpy (indices, idx2, j*sizeof (int));
  count = j;

  CHK (delete [] idx);
  CHK (delete [] idx2);
  return indices;
}

//---------------------------------------------------------------------------
