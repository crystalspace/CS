/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
  
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
#include "csengine/polyint.h"
#include "csengine/treeobj.h"
#include "csengine/bsp.h"
#include "csengine/thing.h"
#include "csengine/engine.h"
#include "csengine/polygon.h"
#include "isystem.h"

//---------------------------------------------------------------------------

csBspNode::csBspNode ()
{
  front = back = NULL;
  polygons_on_splitter = true;
}

csBspNode::~csBspNode ()
{
  delete front;
  delete back;
}

void csBspNode::AddPolygon (csPolygonInt* poly)
{
  polygons.AddPolygon (poly);
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

int csBspNode::CountPolygons ()
{
  int count = polygons.GetNumPolygons ();
  if (front) count += ((csBspNode*)front)->CountPolygons ();
  if (back) count += ((csBspNode*)back)->CountPolygons ();
  return count;
}

//---------------------------------------------------------------------------

csBspTree::csBspTree (csThing* thing, int mode) : csPolygonTree (thing)
{
  csBspTree::mode = mode;
}

csBspTree::~csBspTree ()
{
  Clear ();
}

void csBspTree::Build (csPolygonInt** polygons, int num)
{
  root = new csBspNode;

  if (num)
  {
    csPolygonInt** new_polygons = new csPolygonInt* [num];
    int i;
    for (i = 0 ; i < num ; i++)
      new_polygons[i] = polygons[i];
    Build ((csBspNode*)root, new_polygons, num);
    delete [] new_polygons;
  }
}

int csBspTree::SelectSplitter (csPolygonInt** polygons, int num)
{
  int i, j, poly_idx;

  poly_idx = -1;

  // Several modes in the end come down to BSP_BALANCE_AND_SPLITS
  // with a different balance_factor and split_factor. Those two
  // factors determine how important the balance or split quality
  // is for the total quality factor.
  float balance_factor, split_factor;
  int cur_mode = mode;
  if (cur_mode == BSP_BALANCED)
  {
    cur_mode = BSP_BALANCE_AND_SPLITS;
    balance_factor = 1;
    split_factor = .1;	// Don't completely ignore splitting.
  }
  else if (cur_mode == BSP_ALMOST_BALANCED)
  {
    cur_mode = BSP_ALMOST_BALANCE_AND_SPLITS;
    balance_factor = 1;
    split_factor = .1;
  }
  else if (cur_mode == BSP_MINIMIZE_SPLITS)
  {
    cur_mode = BSP_BALANCE_AND_SPLITS;
    balance_factor = 0;
    split_factor = 1;
  }
  else if (cur_mode == BSP_ALMOST_MINIMIZE_SPLITS)
  {
    cur_mode = BSP_ALMOST_BALANCE_AND_SPLITS;
    balance_factor = 0;
    split_factor = 1;
  }
  else
  {
    balance_factor = 1;
    split_factor = 4;
  }

  if (cur_mode == BSP_RANDOM)
  {
    poly_idx = rand () % num;
  }
  else if (cur_mode == BSP_MOST_ON_SPLITTER)
  {
    // First choose a polygon which shares its plane with the highest
    // number of other polygons.
    int same_poly = 0;
    for (i = 0 ; i < num ; i++)
    {
      csPlane3* plane_i = polygons[i]->GetPolyPlane ();
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
  else if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS || cur_mode == BSP_BALANCE_AND_SPLITS)
  {
    // Combine balancing and least nr of splits.
    float least_penalty = 1000000;
    int n = num;
    if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS && n > 20) n = 20;
    int ii, jj;
    for (i = 0 ; i < n ; i++)
    {
      if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS)
        ii = rand () % num;
      else
        ii = i;
      const csPlane3& poly_plane = *polygons[ii]->GetPolyPlane ();
      int front = 0, back = 0;
      int splits = 0;
      for (j = 0 ; j < n ; j++)
      {
        if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS)
          jj = rand () % num;
	else
	  jj = j;
        int c = polygons[jj]->Classify (poly_plane);
	if (c == POL_FRONT) front++;
	else if (c == POL_BACK) back++;
	else if (c == POL_SPLIT_NEEDED) splits++;
      }
      // balance_penalty is 0 for a very good balanced tree and
      // 1 for a very bad one.
      float balance_penalty = ((float)ABS (front+splits-back))/(float)num;
      // split_penalty is 0 for a very good tree with regards to splitting
      // and 1 for a very bad one.
      float split_penalty = (float)splits/(float)num;
      // Total penalty is a combination of both penalties. 0 is very good,
      float penalty = balance_factor * balance_penalty + split_factor * split_penalty;

      // Add EPSILON to penalty before comparing to avoid system dependent
      // results because a cost result is ALMOST the same.
      // This is to try to get the same octree on all platforms.
      if ((penalty+EPSILON) < least_penalty)
      {
        least_penalty = penalty;
	poly_idx = ii;
      }
    }
  }
  return poly_idx;
}

void csBspTree::Build (csBspNode* node, csPolygonInt** polygons,
	int num)
{
  int i;
  if (!Overlaps (polygons, num))
  {
    // We have a convex set.
    // First we test if all polygons are coplanar. In that case
    // we still set polygons_on_splitter to true.
    // Some code optimizations may depend on this.
    node->polygons_on_splitter = true;
    node->splitter = *(polygons[0]->GetPolyPlane ());
    for (i = 1 ; i < num ; i++)
      if (polygons[i]->Classify (node->splitter) != POL_SAME_PLANE)
      {
        node->polygons_on_splitter = false;
	break;
      }
    for (i = 0 ; i < num ; i++)
      node->AddPolygon (polygons[i]);
    return;
  }

  int poly_idx = SelectSplitter (polygons, num);
  csPolygonInt* split_poly = polygons[poly_idx];
  node->splitter = *(split_poly->GetPolyPlane ());

  // Now we split the node according to the plane of that polygon.
  csPolygonInt** front_poly = new csPolygonInt* [num];
  csPolygonInt** back_poly = new csPolygonInt* [num];
  int front_idx = 0, back_idx = 0;

  for (i = 0 ; i < num ; i++)
  {
    int c = polygons[i]->Classify (node->splitter);
    switch (c)
    {
      case POL_SAME_PLANE:
      	node->AddPolygon (polygons[i]);
	break;
      case POL_FRONT:
        front_poly[front_idx++] = polygons[i];
	break;
      case POL_BACK:
        back_poly[back_idx++] = polygons[i];
	break;
      case POL_SPLIT_NEEDED:
	{
	  csPolygonInt* np1, * np2;
	  polygons[i]->SplitWithPlane (&np1, &np2, node->splitter);
	  front_poly[front_idx++] = np1;
	  back_poly[back_idx++] = np2;
	}
	break;
    }
  }

  if (front_idx)
  {
    node->front = new csBspNode;
    Build (node->front, front_poly, front_idx);
  }
  if (back_idx)
  {
    node->back = new csBspNode;
    Build (node->back, back_poly, back_idx);
  }

  delete [] front_poly;
  delete [] back_poly;
}

void csBspTree::ProcessTodo (csBspNode* node)
{
  csObjectStub* stub;
  if (!node->front && !node->back)
  {
    // This node has no children. Currently we just add
    // this stub to the stub of this node then. This is not entirely
    // correct because the current stub may not be convex. However,
    // since the stub system is going to be used for visibility testing
    // and not for perfect rendering I don't think this is a major
    // problem. @@@
    while (node->todo_stubs)
    {
      stub = node->todo_stubs;
      node->UnlinkStub (stub);	// Unlink from todo list.
      node->LinkStub (stub);	// Relink to normal list.
    }
    return;
  }

  while (node->todo_stubs)
  {
    stub = node->todo_stubs;
    node->UnlinkStub (stub);	// Unlink from todo list.
    csObjectStub* stub_on, * stub_front, * stub_back;
    stub->GetObject ()->SplitWithPlane (stub, &stub_on, &stub_front, &stub_back,
    	node->splitter);
    // Link the stub with the polygons on this splitter plane to the current node.
    if (stub_on) node->LinkStub (stub_on);	// Relink to normal list.
    // Link the stub with polygons in front to the todo list of the front node.
    if (stub_front)
      if (node->front)
        node->front->LinkStubTodo (stub_front);
      else
        node->LinkStub (stub_front);	// @@@ We should create node->front!
    // Link the stub with back polygons to the todo list of the back node.
    if (stub_back)
      if (node->back)
        node->back->LinkStubTodo (stub_back);
      else
        node->LinkStub (stub_back);	// @@@ We should create node->back!
  }
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

  ProcessTodo (node);

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if (csMath3::Visible (pos, node->splitter))
  {
    // Front.
    rc = Back2Front (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
    rc = func (thing, node->polygons.GetPolygons (),
    	node->polygons.GetNumPolygons (), node->polygons_on_splitter, data);
    if (rc) return rc;
    rc = node->TraverseObjects (thing, pos, func, data);
    if (rc) return rc;
    rc = Back2Front (node->front, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Back2Front (node->front, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
    rc = func (thing, node->polygons.GetPolygons (),
    	node->polygons.GetNumPolygons (), node->polygons_on_splitter, data);
    if (rc) return rc;
    rc = node->TraverseObjects (thing, pos, func, data);
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

  ProcessTodo (node);

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if (csMath3::Visible (pos, node->splitter))
  {
    // Front.
    rc = Front2Back (node->front, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
    rc = func (thing, node->polygons.GetPolygons (),
    	node->polygons.GetNumPolygons (), node->polygons_on_splitter, data);
    if (rc) return rc;
    rc = node->TraverseObjects (thing, pos, func, data);
    if (rc) return rc;
    rc = Front2Back (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Front2Back (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
    rc = func (thing, node->polygons.GetPolygons (),
    	node->polygons.GetNumPolygons (), node->polygons_on_splitter, data);
    if (rc) return rc;
    rc = node->TraverseObjects (thing, pos, func, data);
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

void csBspTree::Statistics ()
{
  int num_nodes;
  int num_leaves;
  int max_depth;
  int tot_polygons;
  int max_poly_in_node;
  int min_poly_in_node;
  Statistics (&num_nodes, &num_leaves, &max_depth, &tot_polygons, &max_poly_in_node,
  	&min_poly_in_node);
  CsPrintf (MSG_INITIALIZATION, "  nodes=%d leaves=%d max_depth=%d poly:tot=%d,max=%d,min=%d)\n",
  	num_nodes, num_leaves, max_depth, tot_polygons,
	max_poly_in_node, min_poly_in_node);
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
  if (!root) { count = 0; return NULL; }
  int cnt = ((csBspNode*)root)->CountVertices ();
  if (cnt == 0) { count = 0; return NULL; }
  int* idx = new int [cnt];
  int* idx2 = new int [cnt];
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
  int* indices = new int [j];
  memcpy (indices, idx2, j*sizeof (int));
  count = j;

  delete [] idx;
  delete [] idx2;
  return indices;
}

void csBspTree::AddToPVS (csBspNode* node, csPolygonArrayNoFree* polygons)
{
  if (!node) return;
  int i;
  for (i = 0 ; i < node->polygons.GetNumPolygons () ; i++)
  {
    if (node->polygons.GetPolygon (i)->GetType () != 1) continue;
    csPolygon3D* p = (csPolygon3D*)(node->polygons.GetPolygon (i));
    if (p->IsVisible ()) polygons->Push ((csPolygonInt*)p);
  }
  AddToPVS (node->front, polygons);
  AddToPVS (node->back, polygons);
}

void csBspTree::Cache (csBspNode* node, iFile* cf)
{
  if (!node) return;
  WriteLong (cf, node->polygons.GetNumPolygons ());	// Consistency check
  WriteBool (cf, node->polygons_on_splitter);
  if (!node->polygons_on_splitter) return;
  WritePlane3 (cf, node->splitter);
  if (node->front)
    Cache ((csBspNode*)node->front, cf);
  if (node->back)
    Cache ((csBspNode*)node->back, cf);
}

void csBspTree::Cache (iFile* cf)
{
  Cache ((csBspNode*)root, cf);
}

bool csBspTree::ReadFromCache (iFile* cf, csBspNode* node,
	csPolygonInt** polygons, int num)
{
  int i;
  int check_num_polygons = ReadLong (cf);

  node->polygons_on_splitter = ReadBool (cf);
  if (!node->polygons_on_splitter)
  {
    // We have a convex set.
    for (i = 0 ; i < num ; i++)
      node->AddPolygon (polygons[i]);
    if (check_num_polygons != num)
    {
      CsPrintf (MSG_WARNING, "Bsp does not match with loaded level (1)!\n");
      return false;
    }
    return true;
  }

  ReadPlane3 (cf, node->splitter);

  // Now we split the node according to the plane of that polygon.
  csPolygonInt** front_poly = new csPolygonInt* [num];
  csPolygonInt** back_poly = new csPolygonInt* [num];
  int front_idx = 0, back_idx = 0;

  for (i = 0 ; i < num ; i++)
  {
    int c = polygons[i]->Classify (node->splitter);
    switch (c)
    {
      case POL_SAME_PLANE:
      	node->AddPolygon (polygons[i]);
	break;
      case POL_FRONT:
        front_poly[front_idx++] = polygons[i];
	break;
      case POL_BACK:
        back_poly[back_idx++] = polygons[i];
	break;
      case POL_SPLIT_NEEDED:
	{
	  csPolygonInt* np1, * np2;
	  polygons[i]->SplitWithPlane (&np1, &np2, node->splitter);
	  front_poly[front_idx++] = np1;
	  back_poly[back_idx++] = np2;
	}
	break;
    }
  }

  if (check_num_polygons != node->polygons.GetNumPolygons ())
  {
    CsPrintf (MSG_WARNING, "Bsp does not match with loaded level (2)!\n");
    return false;
  }

  if (front_idx)
  {
    node->front = new csBspNode;
    bool rc = ReadFromCache (cf, node->front, front_poly, front_idx);
    if (!rc) return false;
  }
  if (back_idx)
  {
    node->back = new csBspNode;
    bool rc = ReadFromCache (cf, node->back, back_poly, back_idx);
    if (!rc) return false;
  }

  delete [] front_poly;
  delete [] back_poly;
  return true;
}

bool csBspTree::ReadFromCache (iFile* cf,
	csPolygonInt** polygons, int num)
{
  root = new csBspNode;

  csPolygonInt** new_polygons = new csPolygonInt* [num];
  int i;
  for (i = 0 ; i < num ; i++)
    new_polygons[i] = polygons[i];
  bool rc = ReadFromCache (cf, (csBspNode*)root, new_polygons, num);
  delete [] new_polygons;
  return rc;
}

int csBspTree::ClassifyPolygon (csBspNode* node, const csPoly3D& poly)
{
  if (!node)
  {
    if (ClassifyPoint (poly.GetCenter ()))
      return 1;
    else
      return 0;
  }

  if (!node->front && !node->back)
  {
    // Leaf.
    // If we come here then we know that the entire polygon is in
    // this node. In that case we test one of the vertices to see if
    // it is solid or not.
    if (ClassifyPoint (poly.GetCenter ()))
      return 1;
    else
      return 0;
  }
  int c = poly.Classify (node->splitter);
  switch (c)
  {
    case POL_SAME_PLANE:
      if (ClassifyPoint (poly.GetCenter ()))
        return 1;
      else
        return 0;
      break;
    case POL_FRONT:
      return ClassifyPolygon (node->front, poly);
    case POL_BACK:
      return ClassifyPolygon (node->back, poly);
    case POL_SPLIT_NEEDED:
      {
        csPoly3D front, back;
	poly.SplitWithPlane (front, back, node->splitter);
        int rc1 = ClassifyPolygon (node->front, front);
	if (rc1 == -1) return -1;
        int rc2 = ClassifyPolygon (node->back, back);
	if (rc2 == -1) return -1;
	if (rc1 != rc2) return -1;
	return rc1;
      }
      return -1;
  }
  return -1;
}
