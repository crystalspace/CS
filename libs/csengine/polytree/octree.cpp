/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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
#include "csengine/octree.h"
#include "csengine/bsp.h"
#include "csengine/bsp2d.h"
#include "csengine/treeobj.h"
#include "csengine/bspbbox.h"
#include "csengine/thing.h"
#include "csengine/engine.h"
#include "csengine/cbufcube.h"
#include "csengine/cbuffer.h"
#include "csengine/polygon.h"
#include "csengine/thing.h"
#include "csutil/memfile.h"
#include "iutil/vfs.h"

//---------------------------------------------------------------------------

#define PLANE_X 0
#define PLANE_Y 1
#define PLANE_Z 2

//---------------------------------------------------------------------------

csOctreeNode::csOctreeNode ()
{
  int i;
  for (i = 0 ; i < 8 ; i++)
    children[i] = NULL;
  minibsp = NULL;
  minibsp_verts = NULL;
  minibsp_numverts = 0;
  leaf = false;
}

csOctreeNode::~csOctreeNode ()
{
  int i;
  for (i = 0 ; i < 8 ; i++)
    delete children[i];
  delete minibsp;
  delete [] minibsp_verts;
}

bool csOctreeNode::IsEmpty ()
{
  int i;
  for (i = 0 ; i < 8 ; i++)
    if (children[i] && !children[i]->IsEmpty ()) return false;
  if (minibsp) return false;
  return true;
}

void csOctreeNode::SetMiniBsp (csBspTree* mbsp)
{
  minibsp = mbsp;
}

void csOctreeNode::BuildVertexTables ()
{
  delete [] minibsp_verts;
  if (minibsp)
    minibsp_verts = minibsp->GetVertices (minibsp_numverts);
  int i;
  for (i = 0 ; i < 8 ; i++)
    if (children[i]) ((csOctreeNode*)children[i])->BuildVertexTables ();
}

int csOctreeNode::CountChildren ()
{
  int i;
  int count = 0;
  for (i = 0 ; i < 8 ; i++)
    if (children[i])
      count += 1+((csOctreeNode*)children[i])->CountChildren ();
  return count;
}

int csOctreeNode::CountPolygons ()
{
  int i;
  int count = 0;
  if (GetMiniBsp ()) count += GetMiniBsp ()->CountPolygons ();
  for (i = 0 ; i < 8 ; i++)
    if (children[i])
      count += ((csOctreeNode*)children[i])->CountPolygons ();
  return count;
}

//---------------------------------------------------------------------------

csOctree::csOctree (csThing* thing, const csVector3& imin_bbox,
	const csVector3& imax_bbox, int ibsp_num, int imode)
	: csPolygonTree (thing)
{
  bbox.Set (imin_bbox, imax_bbox);
  bsp_num = ibsp_num;
  mode = imode;
}

csOctree::~csOctree ()
{
  Clear ();
}

void csOctree::Build (csPolygonInt** polygons, int num)
{
  root = new csOctreeNode;

  csPolygonInt** new_polygons = new csPolygonInt* [num];
  int i;
  for (i = 0 ; i < num ; i++) new_polygons[i] = polygons[i];
  Build ((csOctreeNode*)root, bbox.Min (), bbox.Max (), new_polygons, num);
  delete [] new_polygons;
  //Dumper::dump (this);
}

void csOctree::ProcessTodo (csOctreeNode* node)
{
  csPolygonStub* stub;

  if (node->GetMiniBsp ())
  {
    csBspTree* bsp = node->GetMiniBsp ();
    while (node->todo_stubs)
    {
      stub = node->todo_stubs;
      node->UnlinkStub (stub);	// Unlink from todo list.
      bsp->AddStubTodo (stub);
    }
    return;
  }

  if (node->IsLeaf ())
  {
    // A leaf but no children. @@@ We should probably create children here?
    while (node->todo_stubs)
    {
      stub = node->todo_stubs;
      node->UnlinkStub (stub);	// Unlink from todo list.
      node->LinkStub (stub);
    }
    return;
  }

  const csVector3& center = node->GetCenter ();
  while (node->todo_stubs)
  {
    stub = node->todo_stubs;
    node->UnlinkStub (stub);	// Unlink from todo list.
    csPolygonStub* xf, * xb;
    csPolyTreeBBox* pto = stub->GetObject ();
    pto->SplitWithPlaneX (stub, NULL, &xf, &xb, center.x);
    if (xf)
    {
      csPolygonStub* xfyf, * xfyb;
      pto->SplitWithPlaneY (xf, NULL, &xfyf, &xfyb, center.y);
      if (xfyf)
      {
        csPolygonStub* xfyfzf, * xfyfzb;
        pto->SplitWithPlaneZ (xfyf, NULL, &xfyfzf, &xfyfzb, center.z);
	if (xfyfzf) node->children[OCTREE_FFF]->LinkStubTodo (xfyfzf);
	if (xfyfzb) node->children[OCTREE_FFB]->LinkStubTodo (xfyfzb);
      }
      if (xfyb)
      {
        csPolygonStub* xfybzf, * xfybzb;
        pto->SplitWithPlaneZ (xfyb, NULL, &xfybzf, &xfybzb, center.z);
	if (xfybzf) node->children[OCTREE_FBF]->LinkStubTodo (xfybzf);
	if (xfybzb) node->children[OCTREE_FBB]->LinkStubTodo (xfybzb);
      }
    }
    if (xb)
    {
      csPolygonStub* xbyf, * xbyb;
      pto->SplitWithPlaneY (xb, NULL, &xbyf, &xbyb, center.y);
      if (xbyf)
      {
        csPolygonStub* xbyfzf, * xbyfzb;
        pto->SplitWithPlaneZ (xbyf, NULL, &xbyfzf, &xbyfzb, center.z);
	if (xbyfzf) node->children[OCTREE_BFF]->LinkStubTodo (xbyfzf);
	if (xbyfzb) node->children[OCTREE_BFB]->LinkStubTodo (xbyfzb);
      }
      if (xbyb)
      {
        csPolygonStub* xbybzf, * xbybzb;
        pto->SplitWithPlaneZ (xbyb, NULL, &xbybzf, &xbybzb, center.z);
	if (xbybzf) node->children[OCTREE_BBF]->LinkStubTodo (xbybzf);
	if (xbybzb) node->children[OCTREE_BBB]->LinkStubTodo (xbybzb);
      }
    }
  }
}

static void SplitOptPlane (csPolygonInt* np, csPolygonInt** npF, csPolygonInt** npB,
	int xyz, float xyz_val)
{
  if (!np)
  {
    *npF = NULL;
    *npB = NULL;
    return;
  }
  int rc = 0;
  switch (xyz)
  {
    case 0: rc = np->ClassifyX (xyz_val); break;
    case 1: rc = np->ClassifyY (xyz_val); break;
    case 2: rc = np->ClassifyZ (xyz_val); break;
  }
  if (rc == POL_SPLIT_NEEDED)
    switch (xyz)
    {
      case 0: np->SplitWithPlaneX (npF, npB, xyz_val); break;
      case 1: np->SplitWithPlaneY (npF, npB, xyz_val); break;
      case 2: np->SplitWithPlaneZ (npF, npB, xyz_val); break;
    }
  else if (rc == POL_BACK)
  {
    *npF = NULL;
    *npB = np;
  }
  else
  {
    *npF = np;
    *npB = NULL;
  }
}

// Given an array of csPolygonInt (which we know to be csPolygon3D
// in this case) fill three other arrays with x, y, and z values
// for all the vertices of those polygons that are in the given box.
// @@@@ UGLY CODE!!! EXPERIMENTAL ONLY!
// If this works good it should be cleaned up a lot.
static void GetVertexComponents (csPolygonInt** polygons, int num, const csBox3& box,
	float* xarray, int& num_xar,
	float* yarray, int& num_yar,
	float* zarray, int& num_zar)
{
  int i, j;
  num_xar = 0;
  num_yar = 0;
  num_zar = 0;
  for (i = 0 ; i < num ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)polygons[i];
    for (j = 0 ; j < p->GetVertexCount () ; j++)
    {
      const csVector3& v = p->Vwor (j);
      if (v.x >= box.MinX () && v.x <= box.MaxX ())
      {
        *xarray++ = v.x;
	num_xar++;
      }
      if (v.y >= box.MinY () && v.y <= box.MaxY ())
      {
        *yarray++ = v.y;
	num_yar++;
      }
      if (v.z >= box.MinZ () && v.z <= box.MaxZ ())
      {
        *zarray++ = v.z;
	num_zar++;
      }
    }
  }
}

static int compare_float (const void* v1, const void* v2)
{
  float f1 = *(float*)v1;
  float f2 = *(float*)v2;
  if (f1 < f2) return -1;
  else if (f1 > f2) return 1;
  else return 0;
}

static int RemoveDoubles (float* array, int num_ar, float* new_array)
{
  int i;
  int num = 0;
  *new_array++ = *array++;
  num++;
  for (i = 1 ; i < num_ar ; i++)
    if (ABS ((*array) - (*(new_array-1))) > EPSILON)
    {
      *new_array++ = *array++;
      num++;
    }
    else array++;
  return num;
}

void csOctree::ChooseBestCenter (csOctreeNode* node,
	csPolygonInt** polygons, int num)
{
  const csVector3& bmin = node->GetMinCorner ();
  const csVector3& bmax = node->GetMaxCorner ();
  const csVector3& orig = node->GetCenter ();

  // The test box for finding the center.
  csBox3 tbox (orig-(bmax-bmin)/5, orig+(bmax-bmin)/5);

  // First find all x, y, z components in the box we want to test.
  // These are the ones we're going to try.
  float* xarray_all, * yarray_all, * zarray_all;
  float* xarray, * yarray, * zarray;
  int num_xar, num_yar, num_zar;
  xarray_all = new float [num*10];
  yarray_all = new float [num*10];
  zarray_all = new float [num*10];
  GetVertexComponents (polygons, num, tbox,
	xarray_all, num_xar, yarray_all, num_yar, zarray_all, num_zar);
  // Make sure the center is always included.
  xarray_all[num_xar++] = orig.x;
  yarray_all[num_yar++] = orig.y;
  zarray_all[num_zar++] = orig.z;
  qsort (xarray_all, num_xar, sizeof (float), compare_float);
  qsort (yarray_all, num_yar, sizeof (float), compare_float);
  qsort (zarray_all, num_zar, sizeof (float), compare_float);
  xarray = new float [num_xar];
  yarray = new float [num_yar];
  zarray = new float [num_zar];
  int num_x, num_y, num_z;
  num_x = RemoveDoubles (xarray_all, num_xar, xarray);
  num_y = RemoveDoubles (yarray_all, num_yar, yarray);
  num_z = RemoveDoubles (zarray_all, num_zar, zarray);
  delete [] xarray_all;
  delete [] yarray_all;
  delete [] zarray_all;

  int i, j;
  csVector3 best_center = orig;
  // Try a few x-planes first.
  float x, y, z;
  int splits, best_splits = 2000000000;
  int rc;
  for (i = 0 ; i < num_x ; i++)
  {
    x = xarray[i];
    splits = 0;
    for (j = 0 ; j < num ; j++)
    {
      rc = polygons[j]->ClassifyX (x);
      if (rc == POL_SPLIT_NEEDED) splits++;
      //else if (rc == POL_SAME_PLANE) splits -= 2;	// Very good!
    }
    if (splits < best_splits)
    {
      best_center.x = x;
      best_splits = splits;
    }
  }
  best_splits = 2000000000;
  for (i = 0 ; i < num_y ; i++)
  {
    y = yarray[i];
    splits = 0;
    for (j = 0 ; j < num ; j++)
    {
      rc = polygons[j]->ClassifyY (y);
      if (rc == POL_SPLIT_NEEDED) splits++;
      //else if (rc == POL_SAME_PLANE) splits -= 2;	// Very good!
    }
    if (splits < best_splits)
    {
      best_center.y = y;
      best_splits = splits;
    }
  }
  best_splits = 2000000000;
  for (i = 0 ; i < num_z ; i++)
  {
    z = zarray[i];
    splits = 0;
    for (j = 0 ; j < num ; j++)
    {
      rc = polygons[j]->ClassifyZ (z);
      if (rc == POL_SPLIT_NEEDED) splits++;
      //else if (rc == POL_SAME_PLANE) splits -= 2;	// Very good!
    }
    if (splits < best_splits)
    {
      best_center.z = z;
      best_splits = splits;
    }
  }
  node->center = best_center;
  delete [] xarray;
  delete [] yarray;
  delete [] zarray;
}

void csOctree::Build (csOctreeNode* node, const csVector3& bmin,
	const csVector3& bmax, csPolygonInt** polygons, int num)
{
  node->SetBox (bmin, bmax);

  if (num == 0)
  {
    // The only reason we create a BSP tree here is to solve a
    // bug related to sprite visibility. Previously when a sprite
    // enters an octree node containing no polygons it would not
    // be marked visible because that node contained no bsp tree
    // to be used for marking the visibility stubs.
    csBspTree* bsp;
    bsp = new csBspTree (thing, mode);
    bsp->Build (polygons, num);
    node->SetMiniBsp (bsp);
    node->leaf = true;
    return;
  }

  int i;
  for (i = 0 ; i < num ; i++)
    node->unsplit_polygons.AddPolygon (polygons[i]);

  if (num <= bsp_num)
  {
    csBspTree* bsp;
    bsp = new csBspTree (thing, mode);
    bsp->Build (polygons, num);
    node->SetMiniBsp (bsp);
    node->leaf = true;
    return;
  }

  ChooseBestCenter (node, polygons, num);

  const csVector3& center = node->GetCenter ();

  int k;

  // Now we split the node according to the planes.
  csPolygonInt** polys[8];
  int idx[8];
  for (i = 0 ; i < 8 ; i++)
  {
    polys[i] = new csPolygonInt* [num];
    idx[i] = 0;
  }

  for (k = 0 ; k < num ; k++)
  {
    // The following is approach is most likely not the best way
    // to do it. We should have a routine which can split a polygon
    // immediatelly to the eight octree nodes.
    // But since polygons will not often be split that heavily
    // it probably doesn't really matter much.
    csPolygonInt* npF, * npB, * npFF, * npFB, * npBF, * npBB;
    csPolygonInt* nps[8];
    SplitOptPlane (polygons[k], &npF, &npB, 0, center.x);
    SplitOptPlane (npF, &npFF, &npFB, 1, center.y);
    SplitOptPlane (npB, &npBF, &npBB, 1, center.y);
    SplitOptPlane (npFF, &nps[OCTREE_FFF], &nps[OCTREE_FFB], 2, center.z);
    SplitOptPlane (npFB, &nps[OCTREE_FBF], &nps[OCTREE_FBB], 2, center.z);
    SplitOptPlane (npBF, &nps[OCTREE_BFF], &nps[OCTREE_BFB], 2, center.z);
    SplitOptPlane (npBB, &nps[OCTREE_BBF], &nps[OCTREE_BBB], 2, center.z);
    for (i = 0 ; i < 8 ; i++)
      if (nps[i])
        polys[i][idx[i]++] = nps[i];
  }

  for (i = 0 ; i < 8 ; i++)
  {
    // Even if there are no polygons in the node we create
    // a child octree node because some of the visibility stuff
    // depends on that (i.e. adding dynamic objects).
    node->children[i] = new csOctreeNode;
    csVector3 new_bmin;
    csVector3 new_bmax;
    if (i & 4) { new_bmin.x = center.x; new_bmax.x = bmax.x; }
    else { new_bmin.x = bmin.x; new_bmax.x = center.x; }
    if (i & 2) { new_bmin.y = center.y; new_bmax.y = bmax.y; }
    else { new_bmin.y = bmin.y; new_bmax.y = center.y; }
    if (i & 1) { new_bmin.z = center.z; new_bmax.z = bmax.z; }
    else { new_bmin.z = bmin.z; new_bmax.z = center.z; }
    Build ((csOctreeNode*)node->children[i], new_bmin, new_bmax,
    	polys[i], idx[i]);

    delete [] polys[i];
  }
}

void* csOctree::Back2Front (const csVector3& pos,
	csTreeVisitFunc* func, void* data,
	csTreeCullFunc* cullfunc, void* culldata)
{
  return Back2Front ((csOctreeNode*)root, pos, func, data, cullfunc, culldata);
}

void* csOctree::Front2Back (const csVector3& pos,
	csTreeVisitFunc* func, void* data,
	csTreeCullFunc* cullfunc, void* culldata)
{
  return Front2Back ((csOctreeNode*)root, pos, func, data, cullfunc, culldata);
}

void* csOctree::Back2Front (csOctreeNode* node, const csVector3& pos,
	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata)
{
  if (!node) return NULL;
  if (cullfunc && !cullfunc (this, node, pos, culldata)) return NULL;

  ProcessTodo (node);

  if (node->GetMiniBsp ())
    return node->GetMiniBsp ()->Back2Front (pos, func, data, cullfunc, culldata);
  if (node->IsLeaf ()) return NULL;

  void* rc = NULL;
  const csVector3& center = node->GetCenter ();
  int cur_idx;
  if (pos.x <= center.x) cur_idx = 0;
  else cur_idx = 4;
  if (pos.y > center.y) cur_idx |= 2;
  if (pos.z > center.z) cur_idx |= 1;

# undef __TRAVERSE__
# define __TRAVERSE__(x) \
  rc = Back2Front ((csOctreeNode*)node->children[x], pos, func, data, cullfunc, culldata); \
  if (rc) return rc;

  __TRAVERSE__ (cur_idx ^ 7);
  __TRAVERSE__ (cur_idx ^ 6);
  __TRAVERSE__ (cur_idx ^ 5);
  __TRAVERSE__ (cur_idx ^ 3);
  __TRAVERSE__ (cur_idx ^ 1);
  __TRAVERSE__ (cur_idx ^ 2);
  __TRAVERSE__ (cur_idx ^ 4);
  __TRAVERSE__ (cur_idx);
  return rc;
}

void* csOctree::Front2Back (csOctreeNode* node, const csVector3& pos,
	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata)
{
  if (!node) return NULL;
  if (cullfunc && !cullfunc (this, node, pos, culldata)) return NULL;

  ProcessTodo (node);

  if (node->GetMiniBsp ())
    return node->GetMiniBsp ()->Front2Back (pos, func, data, cullfunc, culldata);
  if (node->IsLeaf ()) return NULL;

  void* rc = NULL;
  const csVector3& center = node->GetCenter ();
  int cur_idx;
  if (pos.x <= center.x) cur_idx = 0;
  else cur_idx = 4;
  if (pos.y > center.y) cur_idx |= 2;
  if (pos.z > center.z) cur_idx |= 1;

# undef __TRAVERSE__
# define __TRAVERSE__(x) \
  rc = Front2Back ((csOctreeNode*)node->children[x], pos, func, data, cullfunc, culldata); \
  if (rc) return rc;

  __TRAVERSE__ (cur_idx);
  __TRAVERSE__ (cur_idx ^ 1);
  __TRAVERSE__ (cur_idx ^ 2);
  __TRAVERSE__ (cur_idx ^ 4);
  __TRAVERSE__ (cur_idx ^ 6);
  __TRAVERSE__ (cur_idx ^ 5);
  __TRAVERSE__ (cur_idx ^ 3);
  __TRAVERSE__ (cur_idx ^ 7);
  return rc;
}

csOctreeNode* csOctree::GetLeaf (const csVector3& pos)
{
  // First locate the leaf this position is in.
  csOctreeNode* node = (csOctreeNode*)root;
  while (node)
  {
    if (node->IsLeaf ()) break;
    const csVector3& center = node->GetCenter ();
    int cur_idx;
    if (pos.x <= center.x) cur_idx = 0;
    else cur_idx = 4;
    if (pos.y > center.y) cur_idx |= 2;
    if (pos.z > center.z) cur_idx |= 1;
    node = (csOctreeNode*)node->children[cur_idx];
  }
  return node;
}


static csOctreeNode* best_pvs_node;
void csOctree::Statistics ()
{
  int num_oct_nodes = 0, max_oct_depth = 0, num_bsp_trees = 0;
  int tot_bsp_nodes = 0, min_bsp_nodes = 1000000000, max_bsp_nodes = 0;
  int tot_bsp_leaves = 0, min_bsp_leaves = 1000000000, max_bsp_leaves = 0;
  int tot_max_depth = 0, min_max_depth = 1000000000, max_max_depth = 0;
  int tot_tot_poly = 0, min_tot_poly = 1000000000, max_tot_poly = 0;
  int num_pvs_leaves = 0;
  int tot_pvs_vis_nodes = 0, min_pvs_vis_nodes = 1000000000, max_pvs_vis_nodes = 0;
  int tot_pvs_vis_poly = 0, min_pvs_vis_poly = 1000000000, max_pvs_vis_poly = 0;
  best_pvs_node = NULL;
  Statistics ((csOctreeNode*)root, 0,
  	&num_oct_nodes, &max_oct_depth, &num_bsp_trees,
  	&tot_bsp_nodes, &min_bsp_nodes, &max_bsp_nodes,
	&tot_bsp_leaves, &min_bsp_leaves, &max_bsp_leaves,
	&tot_max_depth, &min_max_depth, &max_max_depth,
	&tot_tot_poly, &min_tot_poly, &max_tot_poly,
	&num_pvs_leaves,
	&tot_pvs_vis_nodes, &min_pvs_vis_nodes, &max_pvs_vis_nodes,
	&tot_pvs_vis_poly, &min_pvs_vis_poly, &max_pvs_vis_poly);
  int avg_bsp_nodes = num_bsp_trees ? tot_bsp_nodes / num_bsp_trees : 0;
  int avg_bsp_leaves = num_bsp_trees ? tot_bsp_leaves / num_bsp_trees : 0;
  int avg_max_depth = num_bsp_trees ? tot_max_depth / num_bsp_trees : 0;
  int avg_tot_poly = num_bsp_trees ? tot_tot_poly / num_bsp_trees : 0;
  csEngine* e = csEngine::current_engine;
  e->Report ("  oct_nodes=%d max_oct_depth=%d num_bsp_trees=%d",
  	num_oct_nodes, max_oct_depth, num_bsp_trees);
  e->Report ("  bsp nodes: tot=%d avg=%d min=%d max=%d",
  	tot_bsp_nodes, avg_bsp_nodes, min_bsp_nodes, max_bsp_nodes);
  e->Report ("  bsp leaves: tot=%d avg=%d min=%d max=%d",
  	tot_bsp_leaves, avg_bsp_leaves, min_bsp_leaves, max_bsp_leaves);
  e->Report ("  bsp max depth: tot=%d avg=%d min=%d max=%d",
  	tot_max_depth, avg_max_depth, min_max_depth, max_max_depth);
  e->Report ("  bsp tot poly: tot=%d avg=%d min=%d max=%d",
  	tot_tot_poly, avg_tot_poly, min_tot_poly, max_tot_poly);

  // Gather statistics about PVS.
  int avg_pvs_vis_nodes = num_pvs_leaves ? tot_pvs_vis_nodes / num_pvs_leaves : 0;
  int avg_pvs_vis_poly = num_pvs_leaves ? tot_pvs_vis_poly / num_pvs_leaves : 0;
  int tot_polygons = ((csOctreeNode*)root)->CountPolygons ();
  e->Report ("  pvs vis nodes: tot=%d avg=%d min=%d max=%d",
  	tot_pvs_vis_nodes, avg_pvs_vis_nodes, min_pvs_vis_nodes,
	max_pvs_vis_nodes);
  e->Report ("  pvs vis poly: avg=%d%% min=%d%% max=%d%%",
  	avg_pvs_vis_poly * 100 / tot_polygons,
  	min_pvs_vis_poly * 100 / tot_polygons,
  	max_pvs_vis_poly * 100 / tot_polygons);
  if (best_pvs_node)
  {
    const csVector3 center = best_pvs_node->GetCenter ();
    e->Report ("  best pvs node at %f,%f,%f",
    	center.x, center.y, center.z);
  }
}

void csOctree::Statistics (csOctreeNode* node, int depth,
  	int* num_oct_nodes, int* max_oct_depth, int* num_bsp_trees,
  	int* tot_bsp_nodes, int* min_bsp_nodes, int* max_bsp_nodes,
	int* tot_bsp_leaves, int* min_bsp_leaves, int* max_bsp_leaves,
	int* tot_max_depth, int* min_max_depth, int* max_max_depth,
	int* tot_tot_poly, int* min_tot_poly, int* max_tot_poly,
	int* num_pvs_leaves,
	int* tot_pvs_vis_nodes, int* min_pvs_vis_nodes, int* max_pvs_vis_nodes,
	int* tot_pvs_vis_poly, int* min_pvs_vis_poly, int* max_pvs_vis_poly)
{
  if (!node) return;
  depth++;
  if (depth > *max_oct_depth) *max_oct_depth = depth;
  (*num_oct_nodes)++;

  if (node->IsLeaf ())
  {
    // PVS statistics.
    (*num_pvs_leaves)++;
    int num_vis_nodes = 0;
    int num_vis_poly = 0;
    (*tot_pvs_vis_nodes) += num_vis_nodes;
    if (num_vis_nodes > *max_pvs_vis_nodes) *max_pvs_vis_nodes = num_vis_nodes;
    if (num_vis_nodes < *min_pvs_vis_nodes) *min_pvs_vis_nodes = num_vis_nodes;
    (*tot_pvs_vis_poly) += num_vis_poly;
    if (num_vis_poly > *max_pvs_vis_poly) *max_pvs_vis_poly = num_vis_poly;
    if (num_vis_poly < *min_pvs_vis_poly)
    {
      *min_pvs_vis_poly = num_vis_poly;
      best_pvs_node = node;
    }
  }

  if (node->GetMiniBsp ())
  {
    (*num_bsp_trees)++;
    int bsp_num_nodes;
    int bsp_num_leaves;
    int bsp_max_depth;
    int bsp_tot_polygons;
    int bsp_max_poly_in_node;
    int bsp_min_poly_in_node;
    node->GetMiniBsp ()->Statistics (&bsp_num_nodes, &bsp_num_leaves, &bsp_max_depth,
    	&bsp_tot_polygons, &bsp_max_poly_in_node, &bsp_min_poly_in_node);
    (*tot_tot_poly) += bsp_tot_polygons;
    if (bsp_tot_polygons > *max_tot_poly) *max_tot_poly = bsp_tot_polygons;
    if (bsp_tot_polygons < *min_tot_poly) *min_tot_poly = bsp_tot_polygons;
    (*tot_max_depth) += bsp_max_depth;
    if (bsp_max_depth > *max_max_depth) *max_max_depth = bsp_max_depth;
    if (bsp_max_depth < *min_max_depth) *min_max_depth = bsp_max_depth;
    (*tot_bsp_nodes) += bsp_num_nodes;
    if (bsp_num_nodes > *max_bsp_nodes) *max_bsp_nodes = bsp_num_nodes;
    if (bsp_num_nodes < *min_bsp_nodes) *min_bsp_nodes = bsp_num_nodes;
    (*tot_bsp_leaves) += bsp_num_leaves;
    if (bsp_num_leaves > *max_bsp_leaves) *max_bsp_leaves = bsp_num_leaves;
    if (bsp_num_leaves < *min_bsp_leaves) *min_bsp_leaves = bsp_num_leaves;
  }
  else
  {
    int i;
    for (i = 0 ; i < 8 ; i++)
      if (node->children[i])
      {
	Statistics ((csOctreeNode*)(node->children[i]), depth,
  	num_oct_nodes, max_oct_depth, num_bsp_trees,
  	tot_bsp_nodes, min_bsp_nodes, max_bsp_nodes,
	tot_bsp_leaves, min_bsp_leaves, max_bsp_leaves,
	tot_max_depth, min_max_depth, max_max_depth,
	tot_tot_poly, min_tot_poly, max_tot_poly,
	num_pvs_leaves,
	tot_pvs_vis_nodes, min_pvs_vis_nodes, max_pvs_vis_nodes,
	tot_pvs_vis_poly, min_pvs_vis_poly, max_pvs_vis_poly);
      }
  }
  depth--;
}

void csOctree::Cache (csOctreeNode* node, iFile* cf)
{
  if (!node) return;
  long const num = node->unsplit_polygons.GetPolygonCount ();
  // Consistency check
  WriteLong (cf, num);
  WriteVector3 (cf, node->GetCenter ());
  WriteUShort (cf, node->solid_masks[0]);
  WriteUShort (cf, node->solid_masks[1]);
  WriteUShort (cf, node->solid_masks[2]);
  WriteUShort (cf, node->solid_masks[3]);
  WriteUShort (cf, node->solid_masks[4]);
  WriteUShort (cf, node->solid_masks[5]);
  WriteBool (cf, node->leaf);
  WriteBool (cf, node->minibsp != NULL);
  if (num == 0) return;
  if (node->minibsp)
  {
    node->minibsp->Cache (cf);
    return;
  }
  if (node->leaf) return;
  int i;
  for (i = 0 ; i < 8 ; i++)
    if (node->children[i])
    {
      WriteByte (cf, i);	// There follows a child with this number.
      Cache ((csOctreeNode*)node->children[i], cf);
    }
  WriteByte (cf, 255);		// No more children.
}

void csOctree::Cache (iVFS* vfs, const char* name)
{
  csMemFile m;
  iFile* mf = SCF_QUERY_INTERFACE_FAST ((&m), iFile);
  WriteString (mf, "OCTR", 4);
  WriteLong (mf, 100002); // Version number.
  WriteBox3 (mf, bbox);
  WriteLong (mf, (long)bsp_num);
  WriteLong (mf, (long)mode);
  Cache ((csOctreeNode*)root, mf);
  vfs->WriteFile(name, m.GetData(), m.GetSize());
  mf->DecRef ();
}

bool csOctree::ReadFromCache (iFile* cf, csOctreeNode* node,
	const csVector3& bmin, const csVector3& bmax,
  	csPolygonInt** polygons, int num)
{
  if (ReadLong (cf) != num)
  {
    csEngine::current_engine->Warn (
      "Octree does not match with loaded level!");
    return false;
  }

  node->SetBox (bmin, bmax);
  ReadVector3 (cf, node->center);
  node->solid_masks[0] = ReadUShort (cf);
  node->solid_masks[1] = ReadUShort (cf);
  node->solid_masks[2] = ReadUShort (cf);
  node->solid_masks[3] = ReadUShort (cf);
  node->solid_masks[4] = ReadUShort (cf);
  node->solid_masks[5] = ReadUShort (cf);
  node->leaf = ReadBool (cf);
  bool do_minibsp = ReadBool (cf);

  if (num == 0) return true;

  int i;
  for (i = 0 ; i < num ; i++)
    node->unsplit_polygons.AddPolygon (polygons[i]);

  if (do_minibsp)
  {
    csBspTree* bsp;
    bsp = new csBspTree (thing, mode);
    bool rc = bsp->ReadFromCache (cf, polygons, num);
    node->SetMiniBsp (bsp);
    if (!rc) return false;
    node->leaf = true;
    return true;
  }

  const csVector3& center = node->GetCenter ();

  int k;

  // Now we split the node according to the planes.
  csPolygonInt** polys[8];
  int idx[8];
  for (i = 0 ; i < 8 ; i++)
  {
    polys[i] = new csPolygonInt* [num];
    idx[i] = 0;
  }

  for (k = 0 ; k < num ; k++)
  {
    // The following is approach is most likely not the best way
    // to do it. We should have a routine which can split a polygon
    // immediatelly to the eight octree nodes.
    // But since polygons will not often be split that heavily
    // it probably doesn't really matter much.
    csPolygonInt* npF, * npB, * npFF, * npFB, * npBF, * npBB;
    csPolygonInt* nps[8];
    SplitOptPlane (polygons[k], &npF, &npB, 0, center.x);
    SplitOptPlane (npF, &npFF, &npFB, 1, center.y);
    SplitOptPlane (npB, &npBF, &npBB, 1, center.y);
    SplitOptPlane (npFF, &nps[OCTREE_FFF], &nps[OCTREE_FFB], 2, center.z);
    SplitOptPlane (npFB, &nps[OCTREE_FBF], &nps[OCTREE_FBB], 2, center.z);
    SplitOptPlane (npBF, &nps[OCTREE_BFF], &nps[OCTREE_BFB], 2, center.z);
    SplitOptPlane (npBB, &nps[OCTREE_BBF], &nps[OCTREE_BBB], 2, center.z);
    for (i = 0 ; i < 8 ; i++)
      if (nps[i])
        polys[i][idx[i]++] = nps[i];
  }

  for (i = 0 ; i < 8 ; i++)
  {
    int nr = ReadByte (cf);
    if (nr != i)
    {
      int j;
      for (j = i ; j < 8 ; j++)
        delete [] polys[j];
      csEngine::current_engine->Warn (
      	"Corrupt cached octree! Wrong node number!");
      return false;
    }
    // Even if there are no polygons in the node we create
    // a child octree node because some of the visibility stuff
    // depends on that (i.e. adding dynamic objects).
    node->children[i] = new csOctreeNode;
    csVector3 new_bmin;
    csVector3 new_bmax;
    if (i & 4) { new_bmin.x = center.x; new_bmax.x = bmax.x; }
    else { new_bmin.x = bmin.x; new_bmax.x = center.x; }
    if (i & 2) { new_bmin.y = center.y; new_bmax.y = bmax.y; }
    else { new_bmin.y = bmin.y; new_bmax.y = center.y; }
    if (i & 1) { new_bmin.z = center.z; new_bmax.z = bmax.z; }
    else { new_bmin.z = bmin.z; new_bmax.z = center.z; }
    bool rc = ReadFromCache (cf, (csOctreeNode*)node->children[i],
    	new_bmin, new_bmax, polys[i], idx[i]);
    if (!rc)
    {
      int j;
      for (j = i ; j < 8 ; j++)
        delete [] polys[j];
      return false;
    }

    delete [] polys[i];
  }

  // Read and ignore the 255 that should be here.
  if (ReadByte (cf) != 255)
  {
    csEngine::current_engine->Warn (
      "Cached octree corrupt! Expected end marker.");
    return false;
  }
  return true;
}

bool csOctree::ReadFromCache (iVFS* vfs, const char* name,
	csPolygonInt** polygons, int num)
{
  iFile* cf = vfs->Open (name, VFS_FILE_READ);
  if (!cf) return false;		// File doesn't exist
  char buf[10];
  ReadString (cf, buf, 4);
  if (strncmp (buf, "OCTR", 4))
  {
    csEngine::current_engine->Warn (
      "Cached octree '%s' not valid! Will be ignored.",
    	name);
    cf->DecRef ();
    return false;	// Bad format!
  }
  long format_version = ReadLong (cf);
  if (format_version != 100002)
  {
    csEngine::current_engine->Warn (
      "Mismatched format version. Expected %d, got %ld!",
    	100002, format_version);
    cf->DecRef ();
    return false;
  }

  ReadBox3 (cf, bbox);
  bsp_num = ReadLong (cf);
  mode = ReadLong (cf);

  root = new csOctreeNode;
  csPolygonInt** new_polygons = new csPolygonInt* [num];
  int i;
  for (i = 0 ; i < num ; i++) new_polygons[i] = polygons[i];
  bool rc = ReadFromCache (cf, (csOctreeNode*)root, bbox.Min (), bbox.Max (),
  	new_polygons, num);
  delete [] new_polygons;
  cf->DecRef ();
  return rc;
}
