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
#include "csengine/octree.h"
#include "csengine/bsp.h"
#include "csengine/treeobj.h"

//---------------------------------------------------------------------------

csOctreeNode::csOctreeNode ()
{
  int i;
  for (i = 0 ; i < 8 ; i++)
    children[i] = NULL;
  minibsp = NULL;
  minibsp_verts = NULL;
}

csOctreeNode::~csOctreeNode ()
{
  int i;
  for (i = 0 ; i < 8 ; i++)
  {
    CHK (delete children[i]);
  }
  CHK (delete minibsp);
  CHK (delete [] minibsp_verts);
}

void csOctreeNode::RemoveDynamicPolygons ()
{
  int i;
  for (i = 0 ; i < 8 ; i++)
    if (children[i])
    {
      children[i]->RemoveDynamicPolygons ();
      if (children[i]->IsEmpty ())
      {
        CHK (delete children[i]);
        children[i] = NULL;
      }
    }
  if (minibsp)
  {
    minibsp->RemoveDynamicPolygons ();
    if (minibsp->IsEmpty ())
    {
      CHK (delete minibsp);
      minibsp = NULL;
    }
  }
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
  CHK (delete [] minibsp_verts);
  if (minibsp)
    minibsp_verts = minibsp->GetVertices (minibsp_numverts);
  int i;
  for (i = 0 ; i < 8 ; i++)
    if (children[i]) ((csOctreeNode*)children[i])->BuildVertexTables ();
}

//---------------------------------------------------------------------------

csOctree::csOctree (csPolygonParentInt* pset, const csVector3& imin_bbox,
	const csVector3& imax_bbox, int ibsp_num, int imode) : csPolygonTree (pset)
{
  bbox.Set (imin_bbox, imax_bbox);
  bsp_num = ibsp_num;
  mode = imode;
}

csOctree::~csOctree ()
{
  Clear ();
}

void csOctree::Build ()
{
  int i;
  int num = pset->GetNumPolygons ();
  CHK (csPolygonInt** polygons = new csPolygonInt* [num]);
  for (i = 0 ; i < num ; i++) polygons[i] = pset->GetPolygon (i);

  CHK (root = new csOctreeNode);

  Build ((csOctreeNode*)root, bbox.Min (), bbox.Max (), polygons, num);

  CHK (delete [] polygons);
}

void csOctree::Build (csPolygonInt** polygons, int num)
{
  CHK (root = new csOctreeNode);

  CHK (csPolygonInt** new_polygons = new csPolygonInt* [num]);
  int i;
  for (i = 0 ; i < num ; i++) new_polygons[i] = polygons[i];
  Build ((csOctreeNode*)root, bbox.Min (), bbox.Max (), new_polygons, num);
  CHK (delete [] new_polygons);
}


void csOctree::AddDynamicPolygons (csPolygonInt** polygons, int num)
{
  // @@@ We should only do this copy if there is a split in the first level.
  // Now it is just overhead.
  CHK (csPolygonInt** new_polygons = new csPolygonInt* [num]);
  int i;
  for (i = 0 ; i < num ; i++) new_polygons[i] = polygons[i];
  BuildDynamic ((csOctreeNode*)root, new_polygons, num);
  CHK (delete [] new_polygons);
}

void csOctree::RemoveDynamicPolygons ()
{
  if (root) root->RemoveDynamicPolygons ();
}

void SplitOptPlane (csPolygonInt* np, csPolygonInt** npF, csPolygonInt** npB,
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

void SplitOptPlaneDyn (csPolygonInt* np, csPolygonInt** npF, csPolygonInt** npB,
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
  {
    // In case of dynamic polygon adding we don't split the polygon.
    *npF = np;
    *npB = np;
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

void csOctree::Build (csOctreeNode* node, const csVector3& bmin,
	const csVector3& bmax, csPolygonInt** polygons, int num)
{
  node->SetBox (bmin, bmax);

  if (num <= bsp_num)
  {
    csBspTree* bsp;
    CHK (bsp = new csBspTree (pset, mode));
    bsp->Build (polygons, num);
    node->SetMiniBsp (bsp);
    return;
  }

  const csVector3& center = node->GetCenter ();

  int i, k;

  // Now we split the node according to the planes.
  csPolygonInt** polys[8];
  int idx[8];
  for (i = 0 ; i < 8 ; i++)
  {
    CHK (polys[i] = new csPolygonInt* [num]);
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
    if (idx[i])
    {
      csVector3 new_bmin;
      csVector3 new_bmax;
      CHK (node->children[i] = new csOctreeNode);
      if (i & 4) { new_bmin.x = center.x; new_bmax.x = bmax.x; }
      else { new_bmin.x = bmin.x; new_bmax.x = center.x; }
      if (i & 2) { new_bmin.y = center.y; new_bmax.y = bmax.y; }
      else { new_bmin.y = bmin.y; new_bmax.y = center.y; }
      if (i & 1) { new_bmin.z = center.z; new_bmax.z = bmax.z; }
      else { new_bmin.z = bmin.z; new_bmax.z = center.z; }
      Build ((csOctreeNode*)node->children[i], new_bmin, new_bmax,
      	polys[i], idx[i]);
    }
    CHK (delete [] polys[i]);
  }
}

void csOctree::BuildDynamic (csOctreeNode* node, csPolygonInt** polygons, int num)
{
  if (node->minibsp)
  {
    node->minibsp->AddDynamicPolygons (polygons, num);
    return;
  }

  const csVector3& center = node->GetCenter ();
  const csVector3& bmin = node->bbox.Min ();
  const csVector3& bmax = node->bbox.Max ();

  int i, k;

  // Now we split the node according to the planes.
  //@@@ Avoid the following allocation some way.
  csPolygonInt** polys[8];
  int idx[8];
  for (i = 0 ; i < 8 ; i++)
  {
    CHK (polys[i] = new csPolygonInt* [num]);
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
    SplitOptPlaneDyn (polygons[k], &npF, &npB, 0, center.x);
    SplitOptPlaneDyn (npF, &npFF, &npFB, 1, center.y);
    SplitOptPlaneDyn (npB, &npBF, &npBB, 1, center.y);
    SplitOptPlaneDyn (npFF, &nps[OCTREE_FFF], &nps[OCTREE_FFB], 2, center.z);
    SplitOptPlaneDyn (npFB, &nps[OCTREE_FBF], &nps[OCTREE_FBB], 2, center.z);
    SplitOptPlaneDyn (npBF, &nps[OCTREE_BFF], &nps[OCTREE_BFB], 2, center.z);
    SplitOptPlaneDyn (npBB, &nps[OCTREE_BBF], &nps[OCTREE_BBB], 2, center.z);
    for (i = 0 ; i < 8 ; i++)
      if (nps[i])
        polys[i][idx[i]++] = nps[i];
  }

  for (i = 0 ; i < 8 ; i++)
  {
    if (idx[i])
    {
      if (!node->children[i])
      {
        csVector3 new_bmin;
        csVector3 new_bmax;
        CHK (node->children[i] = new csOctreeNode);
        if (i & 4) { new_bmin.x = center.x; new_bmax.x = bmax.x; }
        else { new_bmin.x = bmin.x; new_bmax.x = center.x; }
        if (i & 2) { new_bmin.y = center.y; new_bmax.y = bmax.y; }
        else { new_bmin.y = bmin.y; new_bmax.y = center.y; }
        if (i & 1) { new_bmin.z = center.z; new_bmax.z = bmax.z; }
        else { new_bmin.z = bmin.z; new_bmax.z = center.z; }
        ((csOctreeNode*)(node->children[i]))->SetBox (bmin, bmax);
        csBspTree* bsp;
        CHK (bsp = new csBspTree (pset, mode));
        ((csOctreeNode*)(node->children[i]))->SetMiniBsp (bsp);
      }
      BuildDynamic ((csOctreeNode*)node->children[i], polys[i], idx[i]);
    }
    CHK (delete [] polys[i]);
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

  if (node->GetMiniBsp ())
    return node->GetMiniBsp ()->Back2Front (pos, func, data, cullfunc, culldata);

  void* rc = NULL;
  const csVector3& center = node->GetCenter ();
  int cur_idx;
  if (pos.x <= center.x) cur_idx = 0;
  else cur_idx = 4;
  if (pos.y > center.y) cur_idx |= 2;
  if (pos.z > center.z) cur_idx |= 1;

# undef __TRAVERSE__
# define __TRAVERSE__(x) \
  if (!cullfunc || cullfunc (this, node->children[x], pos, culldata)) \
  { \
    rc = Back2Front ((csOctreeNode*)node->children[x], pos, func, data, cullfunc, culldata); \
    if (rc) return rc; \
  }

  __TRAVERSE__ (7-cur_idx);
  __TRAVERSE__ ((7-cur_idx) ^ 1);
  __TRAVERSE__ ((7-cur_idx) ^ 2);
  __TRAVERSE__ ((7-cur_idx) ^ 4);
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

  if (node->GetMiniBsp ())
    return node->GetMiniBsp ()->Front2Back (pos, func, data, cullfunc, culldata);

  void* rc = NULL;
  const csVector3& center = node->GetCenter ();
  int cur_idx;
  if (pos.x <= center.x) cur_idx = 0;
  else cur_idx = 4;
  if (pos.y > center.y) cur_idx |= 2;
  if (pos.z > center.z) cur_idx |= 1;

# undef __TRAVERSE__
# define __TRAVERSE__(x) \
  if (!cullfunc || cullfunc (this, node->children[x], pos, culldata)) \
  { \
    rc = Front2Back ((csOctreeNode*)node->children[x], pos, func, data, cullfunc, culldata); \
    if (rc) return rc; \
  }

  __TRAVERSE__ (cur_idx);
  __TRAVERSE__ (cur_idx ^ 1);
  __TRAVERSE__ (cur_idx ^ 2);
  __TRAVERSE__ (cur_idx ^ 4);
  __TRAVERSE__ ((7-cur_idx) ^ 1);
  __TRAVERSE__ ((7-cur_idx) ^ 2);
  __TRAVERSE__ ((7-cur_idx) ^ 4);
  __TRAVERSE__ (7-cur_idx);
  return rc;
}

void csOctree::Statistics (csOctreeNode* node, int depth, int* num_nodes,
	int* num_leaves, int* max_depth,
  	int* tot_polygons, int* max_poly_in_node, int* min_poly_in_node)
{
  depth++;
  if (depth > *max_depth) *max_depth = depth;
  if (node->GetMiniBsp ())
  {
    (*num_leaves)++;
    int bsp_num_nodes;
    int bsp_num_leaves;
    int bsp_max_depth;
    int bsp_tot_polygons;
    int bsp_max_poly_in_node;
    int bsp_min_poly_in_node;
    node->GetMiniBsp ()->Statistics (&bsp_num_nodes, &bsp_num_leaves, &bsp_max_depth,
    	&bsp_tot_polygons, &bsp_max_poly_in_node, &bsp_min_poly_in_node);
    (*tot_polygons) += bsp_tot_polygons;
    if (bsp_max_poly_in_node > *max_poly_in_node) *max_poly_in_node = bsp_max_poly_in_node;
    if (bsp_min_poly_in_node < *min_poly_in_node) *min_poly_in_node = bsp_min_poly_in_node;
  }
  else
  {
    (*num_nodes)++;
    int i;
    for (i = 0 ; i < 8 ; i++)
      if (node->children[i])
      {
        Statistics ((csOctreeNode*)(node->children[i]), depth, num_nodes,
		num_leaves, max_depth, tot_polygons, max_poly_in_node, min_poly_in_node);
      }
  }
  depth--;
}

void csOctree::Statistics (int* num_nodes, int* num_leaves, int* max_depth,
  	int* tot_polygons, int* max_poly_in_node, int* min_poly_in_node)
{
  *num_nodes = 0;
  *num_leaves = 0;
  *max_depth = 0;
  *tot_polygons = 0;
  *max_poly_in_node = 0;
  *min_poly_in_node = 10000000;
  if (root) Statistics ((csOctreeNode*)root, 0, num_nodes, num_leaves,
  	max_depth, tot_polygons,
  	max_poly_in_node, min_poly_in_node);
}

//---------------------------------------------------------------------------
