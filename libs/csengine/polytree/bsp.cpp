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
#include "iutil/vfs.h"
#include "csengine/polyint.h"
#include "csengine/treeobj.h"
#include "csengine/bspbbox.h"
#include "csengine/bsp.h"
#include "csengine/thing.h"
#include "csengine/engine.h"
#include "csengine/polygon.h"

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

void csBspNode::AddPolygon (csPolygonInt *poly)
{
  polygons.AddPolygon (poly);
}

int csBspNode::CountVertices ()
{
  int num_verts = 0;
  if (front) num_verts += front->CountVertices ();
  if (back) num_verts += back->CountVertices ();

  int i;
  for (i = 0; i < polygons.GetPolygonCount (); i++)
  {
    num_verts += polygons.GetPolygon (i)->GetVertexCount ();
    if (polygons.GetPolygon (i)->GetUnsplitPolygon ())
      num_verts += polygons.GetPolygon (i)->GetUnsplitPolygon ()->GetVertexCount ();
  }

  return num_verts;
}

void csBspNode::FetchVertices (int *array, int &cur_idx)
{
  if (front) front->FetchVertices (array, cur_idx);
  if (back) back->FetchVertices (array, cur_idx);

  int i;
  for (i = 0; i < polygons.GetPolygonCount (); i++)
  {
    int *idx = polygons.GetPolygon (i)->GetVertexIndices ();
    int n = polygons.GetPolygon (i)->GetVertexCount ();
    memcpy (array + cur_idx, idx, sizeof (int) * n);
    cur_idx += n;
    if (polygons.GetPolygon (i)->GetUnsplitPolygon ())
    {
      int *idx = polygons.GetPolygon (i)->GetUnsplitPolygon ()
        ->GetVertexIndices ();
      int n = polygons.GetPolygon (i)->GetUnsplitPolygon ()->GetVertexCount ();
      memcpy (array + cur_idx, idx, sizeof (int) * n);
      cur_idx += n;
    }
  }
}

int csBspNode::CountPolygons ()
{
  int count = polygons.GetPolygonCount ();
  if (front) count += ((csBspNode *)front)->CountPolygons ();
  if (back) count += ((csBspNode *)back)->CountPolygons ();
  return count;
}

//---------------------------------------------------------------------------
csBspTree::csBspTree (csThing *thing, int mode) :
  csPolygonTree(thing)
{
  csBspTree::mode = mode;
}

csBspTree::~csBspTree ()
{
  Clear ();
}

void csBspTree::Build (csPolygonInt **polygons, int num)
{
  root = new csBspNode;

  if (num)
  {
    csPolygonInt **new_polygons = new csPolygonInt *[num];
    int i;
    for (i = 0; i < num; i++) new_polygons[i] = polygons[i];
    Build ((csBspNode *)root, new_polygons, num);
    delete[] new_polygons;
  }
}

int csBspTree::SelectSplitter (csPolygonInt **polygons, int num)
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
    split_factor = 0.1f;                    // Don't completely ignore splitting.
  }
  else if (cur_mode == BSP_ALMOST_BALANCED)
  {
    cur_mode = BSP_ALMOST_BALANCE_AND_SPLITS;
    balance_factor = 1;
    split_factor = 0.1f;
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
    for (i = 0; i < num; i++)
    {
      csPlane3 *plane_i = polygons[i]->GetPolyPlane ();
      int cnt = 1;
      for (j = i + 1; j < num; j++)
      {
        if (
          plane_i == polygons[j]->GetPolyPlane () ||
          csMath3::PlanesEqual (*plane_i, *polygons[j]->GetPolyPlane ()))
          cnt++;
      }

      if (cnt > same_poly)
      {
        same_poly = cnt;
        poly_idx = i;
      }
    }
  }
  else if (
        cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS ||
        cur_mode == BSP_BALANCE_AND_SPLITS)
  {
    // Combine balancing and least nr of splits.
    float least_penalty = 1000000;
    int n = num;
    if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS && n > 20) n = 20;

    int ii, jj;
    for (i = 0; i < n; i++)
    {
      if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS)
        ii = rand () % num;
      else
        ii = i;

      const csPlane3 &poly_plane = *polygons[ii]->GetPolyPlane ();
      int front = 0, back = 0;
      int splits = 0;
      for (j = 0; j < n; j++)
      {
        if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS)
          jj = rand () % num;
        else
          jj = j;

        int c = polygons[jj]->Classify (poly_plane);
        if (c == CS_POL_FRONT)
          front++;
        else if (c == CS_POL_BACK)
          back++;
        else if (c == CS_POL_SPLIT_NEEDED)
          splits++;
      }

      // balance_penalty is 0 for a very good balanced tree and
      // 1 for a very bad one.
      float balance_penalty = ((float)ABS (front + splits - back)) /
        (float)num;

      // split_penalty is 0 for a very good tree with regards to splitting
      // and 1 for a very bad one.
      float split_penalty = (float)splits / (float)num;

      // Total penalty is a combination of both penalties. 0 is very good,
      float penalty = balance_factor *
        balance_penalty +
        split_factor *
        split_penalty;

      // Add EPSILON to penalty before comparing to avoid system dependent
      // results because a cost result is ALMOST the same.
      // This is to try to get the same octree on all platforms.
      if ((penalty + EPSILON) < least_penalty)
      {
        least_penalty = penalty;
        poly_idx = ii;
      }
    }
  }

  return poly_idx;
}

void csBspTree::Build (csBspNode *node, csPolygonInt **polygons, int num)
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
    for (i = 1; i < num; i++)
    {
      if (polygons[i]->Classify (node->splitter) != CS_POL_SAME_PLANE)
      {
        node->polygons_on_splitter = false;
        break;
      }
    }

    for (i = 0; i < num; i++) node->AddPolygon (polygons[i]);
    return ;
  }

  int poly_idx = SelectSplitter (polygons, num);
  csPolygonInt *split_poly = polygons[poly_idx];
  node->splitter = *(split_poly->GetPolyPlane ());

  // Now we split the node according to the plane of that polygon.
  csPolygonInt **front_poly = new csPolygonInt *[num];
  csPolygonInt **back_poly = new csPolygonInt *[num];
  int front_idx = 0, back_idx = 0;

  for (i = 0; i < num; i++)
  {
    int c = polygons[i]->Classify (node->splitter);
    switch (c)
    {
      case CS_POL_SAME_PLANE:
        node->AddPolygon (polygons[i]);
        break;
      case CS_POL_FRONT:
        front_poly[front_idx++] = polygons[i];
        break;
      case CS_POL_BACK:
        back_poly[back_idx++] = polygons[i];
        break;
      case CS_POL_SPLIT_NEEDED:
        {
          csPolygonInt *np1, *np2;
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

  delete[] front_poly;
  delete[] back_poly;
}

void csBspTree::ProcessTodo (csBspNode *node)
{
  csPolygonStub *stub;
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
      node->UnlinkStub (stub);              // Unlink from todo list.
      node->LinkStub (stub);                // Relink to normal list.
    }

    return ;
  }

  while (node->todo_stubs)
  {
    stub = node->todo_stubs;
    node->UnlinkStub (stub);                // Unlink from todo list.
    csPolygonStub *stub_on, *stub_front, *stub_back;
    stub->GetObject ()->SplitWithPlane (
        stub,
        &stub_on,
        &stub_front,
        &stub_back,
        node->splitter);

    // Link the stub with the polygons on this splitter plane to the current node.
    if (stub_on) node->LinkStub (stub_on);  // Relink to normal list.

    // Link the stub with polygons in front to the todo list of the front node.
    if (stub_front)
      if (node->front)
        node->front->LinkStubTodo (stub_front);
      else
        node->LinkStub (stub_front);        // @@@ We should create node->front!

    // Link the stub with back polygons to the todo list of the back node.
    if (stub_back)
      if (node->back)
        node->back->LinkStubTodo (stub_back);
      else
        node->LinkStub (stub_back);         // @@@ We should create node->back!
  }
}

void *csBspTree::Back2Front (
  const csVector3 &pos,
  csTreeVisitFunc *func,
  void *data,
  csTreeCullFunc *cullfunc,
  void *culldata)
{
  return Back2Front ((csBspNode *)root, pos, func, data, cullfunc, culldata);
}

void *csBspTree::Front2Back (
  const csVector3 &pos,
  csTreeVisitFunc *func,
  void *data,
  csTreeCullFunc *cullfunc,
  void *culldata)
{
  return Front2Back ((csBspNode *)root, pos, func, data, cullfunc, culldata);
}

void *csBspTree::Back2Front (
  csBspNode *node,
  const csVector3 &pos,
  csTreeVisitFunc *func,
  void *data,
  csTreeCullFunc *cullfunc,
  void *culldata)
{
  if (!node) return NULL;

  void *rc;

  ProcessTodo (node);

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if (csMath3::Visible (pos, node->splitter))
  {
    // Front.
    rc = Back2Front (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
    rc = func (
        thing,
        node->polygons.GetPolygons (),
        node->polygons.GetPolygonCount (),
        node->polygons_on_splitter,
        data);
    if (rc) return rc;

    // IMPORTANT: First the real polygons have to be traversed and
    // then the bounding box polygons!!!
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
    rc = func (
        thing,
        node->polygons.GetPolygons (),
        node->polygons.GetPolygonCount (),
        node->polygons_on_splitter,
        data);
    if (rc) return rc;
    rc = node->TraverseObjects (thing, pos, func, data);
    if (rc) return rc;
    rc = Back2Front (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
  }

  return NULL;
}

void *csBspTree::Front2Back (
  csBspNode *node,
  const csVector3 &pos,
  csTreeVisitFunc *func,
  void *data,
  csTreeCullFunc *cullfunc,
  void *culldata)
{
  if (!node) return NULL;

  void *rc;

  ProcessTodo (node);

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if (csMath3::Visible (pos, node->splitter))
  {
    // Front.
    rc = Front2Back (node->front, pos, func, data, cullfunc, culldata);
    if (rc) return rc;

    // IMPORTANT: First the bounding box polygons have to be traversed
    // before the polygons of this node!!!
    rc = node->TraverseObjects (thing, pos, func, data);
    if (rc) return rc;
    rc = func (
        thing,
        node->polygons.GetPolygons (),
        node->polygons.GetPolygonCount (),
        node->polygons_on_splitter,
        data);
    if (rc) return rc;
    rc = Front2Back (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Front2Back (node->back, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
    rc = node->TraverseObjects (thing, pos, func, data);
    if (rc) return rc;
    rc = func (
        thing,
        node->polygons.GetPolygons (),
        node->polygons.GetPolygonCount (),
        node->polygons_on_splitter,
        data);
    if (rc) return rc;
    rc = Front2Back (node->front, pos, func, data, cullfunc, culldata);
    if (rc) return rc;
  }

  return NULL;
}

void csBspTree::Statistics (
  csBspNode *node,
  int depth,
  int *num_nodes,
  int *num_leaves,
  int *max_depth,
  int *tot_polygons,
  int *max_poly_in_node,
  int *min_poly_in_node)
{
  depth++;
  if (depth > *max_depth) *max_depth = depth;

  int num = node->polygons.GetPolygonCount ();
  *tot_polygons += num;
  if (num > *max_poly_in_node) *max_poly_in_node = num;
  if (num < *min_poly_in_node) *min_poly_in_node = num;
  if (node->front || node->back)
    (*num_nodes)++;
  else
    (*num_leaves)++;
  if (node->front)
  {
    Statistics (
      node->front,
      depth,
      num_nodes,
      num_leaves,
      max_depth,
      tot_polygons,
      max_poly_in_node,
      min_poly_in_node);
  }

  if (node->back)
  {
    Statistics (
      node->back,
      depth,
      num_nodes,
      num_leaves,
      max_depth,
      tot_polygons,
      max_poly_in_node,
      min_poly_in_node);
  }

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
  Statistics (
    &num_nodes,
    &num_leaves,
    &max_depth,
    &tot_polygons,
    &max_poly_in_node,
    &min_poly_in_node);
  csEngine::current_engine->Report (
      "  nodes=%d leaves=%d max_depth=%d poly:tot=%d,max=%d,min=%d).",
      num_nodes,
      num_leaves,
      max_depth,
      tot_polygons,
      max_poly_in_node,
      min_poly_in_node);
}

void csBspTree::Statistics (
  int *num_nodes,
  int *num_leaves,
  int *max_depth,
  int *tot_polygons,
  int *max_poly_in_node,
  int *min_poly_in_node)
{
  *num_nodes = 0;
  *num_leaves = 0;
  *max_depth = 0;
  *tot_polygons = 0;
  *max_poly_in_node = 0;
  *min_poly_in_node = 10000000;
  if (root)
  {
    Statistics (
      (csBspNode *)root,
      0,
      num_nodes,
      num_leaves,
      max_depth,
      tot_polygons,
      max_poly_in_node,
      min_poly_in_node);
  }
}

static int compare_int (const void *p1, const void *p2)
{
  int i1 = *(int *)p1;
  int i2 = *(int *)p2;
  if (i1 < i2)
    return -1;
  else if (i1 > i2)
    return 1;
  return 0;
}

int *csBspTree::GetVertices (int &count)
{
  if (!root)
  {
    count = 0;
    return NULL;
  }

  int cnt = ((csBspNode *)root)->CountVertices ();
  if (cnt == 0)
  {
    count = 0;
    return NULL;
  }

  int *idx = new int[cnt];
  int *idx2 = new int[cnt];
  int cur_idx = 0;
  ((csBspNode *)root)->FetchVertices (idx, cur_idx);

  // Sort first.
  qsort (idx, cur_idx, sizeof (int), compare_int);

  // Remove all doubles.
  idx2[0] = idx[0];

  int i, j;
  j = 1;
  for (i = 1; i < cur_idx; i++)
  {
    if (idx[i] != idx2[j - 1]) idx2[j++] = idx[i];
  }

  int *indices = new int[j];
  memcpy (indices, idx2, j * sizeof (int));
  count = j;

  delete[] idx;
  delete[] idx2;
  return indices;
}

void csBspTree::Cache (iFile* cf, csBspNode *node,
	csPolygonInt** polygons, int num)
{
  if (!node) return ;
  WriteLong (cf, node->polygons.GetPolygonCount ());  // Consistency check
  WriteBool (cf, node->polygons_on_splitter);
  if (!node->polygons_on_splitter) return ;
  WritePlane3 (cf, node->splitter);

  // Now we split the node according to the plane of that polygon.
  // We do this so that we can correctly cache how the splits operated.
  // Previously we only cached the splitting plane and then we let
  // the cache reader split everything again. But due to small inacuraccies
  // in floating point calculations this is not robust and sometimes a
  // cached octree is not valid for another platform.
  csPolygonInt **front_poly = new csPolygonInt *[num];
  csPolygonInt **back_poly = new csPolygonInt *[num];
  int front_idx = 0, back_idx = 0;

  // Here we calculate a bit array with two bits for every polygon.
  // The value of these two bits will be one of CS_POL results calculated
  // below.
  int size_bits = (num+3)/4;
  uint8* bits = new uint8[size_bits];

  int cnt_same = 0;
  int i;
  for (i = 0 ; i < num ; i++)
  {
    int c = polygons[i]->Classify (node->splitter);
    switch (c)
    {
      case CS_POL_SAME_PLANE:
        cnt_same++;
        break;
      case CS_POL_FRONT:
        front_poly[front_idx++] = polygons[i];
        break;
      case CS_POL_BACK:
        back_poly[back_idx++] = polygons[i];
        break;
      case CS_POL_SPLIT_NEEDED:
        {
          csPolygonInt *np1, *np2;
          polygons[i]->SplitWithPlane (&np1, &np2, node->splitter);
          front_poly[front_idx++] = np1;
          back_poly[back_idx++] = np2;
        }
        break;
    }
    int byte_idx = i/4;
    int bit_idx = (i%4)<<1;
    int bit_mask = 3 << bit_idx;
    bits[byte_idx] = (bits[byte_idx] & ~bit_mask) | (c << bit_idx);
  }

  WriteLong (cf, size_bits);
  cf->Write ((const char*)bits, size_bits);
  delete[] bits;

  if (cnt_same != node->polygons.GetPolygonCount ())
  {
    csEngine::current_engine->Warn ("INTERNAL ERROR! cnt_same != node->polygons.GetPolygonCount ()");
  }

  if (node->front) Cache (cf, (csBspNode *)node->front, front_poly,
  	front_idx);
  if (node->back) Cache (cf, (csBspNode *)node->back, back_poly,
  	back_idx);
}

void csBspTree::Cache (iFile *cf, csPolygonInt** polygons, int num)
{
  Cache (cf, (csBspNode *)root, polygons, num);
}

bool csBspTree::ReadFromCache (
  iFile *cf,
  csBspNode *node,
  csPolygonInt **polygons,
  int num)
{
  int i;
  int check_num_polygons = ReadLong (cf);

  node->polygons_on_splitter = ReadBool (cf);
  if (!node->polygons_on_splitter)
  {
    // We have a convex set.
    for (i = 0; i < num; i++) node->AddPolygon (polygons[i]);
    if (check_num_polygons != num)
    {
      csEngine::current_engine->Warn (
          "Bsp does not match with loaded level (1)!");
      return false;
    }

    return true;
  }

  ReadPlane3 (cf, node->splitter);

  int size_bits = ReadLong (cf);
  uint8* bits = new uint8[size_bits];
  cf->Read ((char*)bits, size_bits);

  // Now we split the node according to the plane of that polygon.
  csPolygonInt **front_poly = new csPolygonInt *[num];
  csPolygonInt **back_poly = new csPolygonInt *[num];
  int front_idx = 0, back_idx = 0;

  for (i = 0 ; i < num ; i++)
  {
    // Using the classify function should have been perfect but since
    // there can be floating point inaccuracies this isn't compatible
    // on all platforms. So I use the bit array here.
    //int c = polygons[i]->Classify (node->splitter);
    int byte_idx = i/4;
    int bit_idx = (i%4)<<1;
    int bit_mask = 3 << bit_idx;
    int c = ((bits[byte_idx] & bit_mask) >> bit_idx) & 3;
    switch (c)
    {
      case CS_POL_SAME_PLANE:
        node->AddPolygon (polygons[i]);
        break;
      case CS_POL_FRONT:
        front_poly[front_idx++] = polygons[i];
        break;
      case CS_POL_BACK:
        back_poly[back_idx++] = polygons[i];
        break;
      case CS_POL_SPLIT_NEEDED:
        {
          csPolygonInt *np1, *np2;
          polygons[i]->SplitWithPlane (&np1, &np2, node->splitter);
          front_poly[front_idx++] = np1;
          back_poly[back_idx++] = np2;
        }
        break;
    }
  }

  delete[] bits;

  if (check_num_polygons != node->polygons.GetPolygonCount ())
  {
    csEngine::current_engine->Warn (
        "Bsp does not match with loaded level (2)!");
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

  delete[] front_poly;
  delete[] back_poly;
  return true;
}

bool csBspTree::ReadFromCache (iFile *cf, csPolygonInt **polygons, int num)
{
  root = new csBspNode;

  csPolygonInt **new_polygons = new csPolygonInt *[num];
  int i;
  for (i = 0; i < num; i++) new_polygons[i] = polygons[i];

  bool rc = ReadFromCache (cf, (csBspNode *)root, new_polygons, num);
  delete[] new_polygons;
  return rc;
}
