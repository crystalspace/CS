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
#include "csgeom/polyint.h"
#include "csgeom/octree.h"
#include "csgeom/bsp.h"

//---------------------------------------------------------------------------

// We have a coordinate system around our node which is
// divided into 27 regions. The center region at coordinate (1,1,1)
// is the node itself. Every one of the 26 remaining regions
// defines an number of vertices which are the convex outline
// as seen from a camera view point in that region.
// The numbers inside the outlines table are indices from 0 to
// 7 which describe the 8 vertices outlining the node:
//	0: left/down/front vertex
//	1: left/down/back
//	2: left/up/front
//	3: left/up/back
//	4: right/down/front
//	5: right/down/back
//	6: right/up/front
//	7: right/up/back
struct Outline
{
  int num;
  int vertices[6];
};
/// Outline lookup table.
static Outline outlines[27] =
{
  { 6, { 3, 2, 6, 4, 5, 1 } },		// 0,0,0
  { 6, { 3, 2, 0, 4, 5, 1 } },		// 0,0,1
  { 6, { 7, 3, 2, 0, 4, 5 } },		// 0,0,2
  { 6, { 3, 2, 6, 4, 0, 1 } },		// 0,1,0
  { 4, { 3, 2, 0, 1, -1, -1 } },	// 0,1,1
  { 6, { 7, 3, 2, 0, 1, 5 } },		// 0,1,2
  { 6, { 3, 7, 6, 4, 0, 1 } },		// 0,2,0
  { 6, { 3, 7, 6, 2, 0, 1 } },		// 0,2,1
  { 6, { 7, 6, 2, 0, 1, 5 } },		// 0,2,2
  { 6, { 2, 6, 4, 5, 1, 0 } },		// 1,0,0
  { 4, { 0, 4, 5, 1, -1, -1 } },	// 1,0,1
  { 6, { 3, 1, 0, 4, 5, 7 } },		// 1,0,2
  { 4, { 2, 6, 4, 0, -1, -1 } },	// 1,1,0
  { 0, { -1, -1, -1, -1, -1, -1 } },	// 1,1,1
  { 4, { 7, 3, 1, 5, -1, -1 } },	// 1,1,2
  { 6, { 3, 7, 5, 4, 0, 2 } },		// 1,2,0
  { 4, { 3, 7, 6, 2, -1, -1 } },	// 1,2,1
  { 6, { 2, 3, 1, 5, 7, 6 } },		// 1,2,2
  { 6, { 2, 6, 7, 5, 1, 0 } },		// 2,0,0
  { 6, { 6, 7, 5, 1, 0, 4 } },		// 2,0,1
  { 6, { 6, 7, 3, 1, 0, 4 } },		// 2,0,2
  { 6, { 2, 6, 7, 5, 4, 0 } },		// 2,1,0
  { 4, { 6, 7, 5, 4, -1, -1 } },	// 2,1,1
  { 6, { 6, 7, 3, 1, 5, 4 } },		// 2,1,2
  { 6, { 2, 3, 7, 5, 4, 0 } },		// 2,2,0
  { 6, { 2, 3, 7, 5, 4, 6 } },		// 2,2,1
  { 6, { 6, 2, 3, 1, 5, 4 } }		// 2,2,2
};

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
  if (minibsp) minibsp->RemoveDynamicPolygons ();
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

csOctree::csOctree (csPolygonParentInt* pset, const csVector3& min_bbox,
	const csVector3& max_bbox, int bsp_num, int mode) : csPolygonTree (pset)
{
  csOctree::min_bbox = min_bbox;
  csOctree::max_bbox = max_bbox;
  csOctree::bsp_num = bsp_num;
  csOctree::mode = mode;
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

  Build ((csOctreeNode*)root, min_bbox, max_bbox, polygons, num);

  CHK (delete [] polygons);
}

void csOctree::Build (csPolygonInt** polygons, int num)
{
  CHK (root = new csOctreeNode);

  CHK (csPolygonInt** new_polygons = new csPolygonInt* [num]);
  int i;
  for (i = 0 ; i < num ; i++) new_polygons[i] = polygons[i];
  Build ((csOctreeNode*)root, min_bbox, max_bbox, new_polygons, num);
  CHK (delete [] new_polygons);
}


void csOctree::AddDynamicPolygons (csPolygonInt** polygons, int num)
{
  // @@@ We should only do this copy if there is a split in the first level.
  // Now it is just overhead.
  CHK (csPolygonInt** new_polygons = new csPolygonInt* [num]);
  int i;
  for (i = 0 ; i < num ; i++) new_polygons[i] = polygons[i];
  BuildDynamic ((csOctreeNode*)root, min_bbox, max_bbox, new_polygons, num);
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

void csOctree::BuildDynamic (csOctreeNode* node, const csVector3& bmin,
	const csVector3& bmax, csPolygonInt** polygons, int num)
{
#if 0
  int i;
  csPolygonInt* split_poly;
  if (node->num) split_poly = node->polygons[0];
  else split_poly = polygons[SelectSplitter (polygons, num)];
  csPlane* split_plane = split_poly->GetPolyPlane ();

  // Now we split the list of polygons according to the plane of that polygon.
  CHK (csPolygonInt** front_poly = new csPolygonInt* [num]);
  CHK (csPolygonInt** back_poly = new csPolygonInt* [num]);
  int front_idx = 0, back_idx = 0;

  for (i = 0 ; i < num ; i++)
  {
    int c = polygons[i]->Classify (split_poly);
    switch (c)
    {
      case POL_SAME_PLANE: node->AddPolygon (polygons[i], true); break;
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
#endif
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

void csOctree::GetConvexOutline (csOctreeNode* node, const csVector3& pos,
	csVector3* array, int& num_array)
{
  const csVector3& bmin = node->GetMinCorner ();
  const csVector3& bmax = node->GetMaxCorner ();
  int idx;
  // First select x part of coordinate.
  if (pos.x < bmin.x)		idx = 0*9;
  else if (pos.x > bmax.x)	idx = 2*9;
  else				idx = 1*9;
  // Then y part.
  if (pos.y < bmin.y)		idx += 0*3;
  else if (pos.y > bmax.y)	idx += 2*3;
  else				idx += 1*3;
  // Then z part.
  if (pos.z < bmin.z)		idx += 0;
  else if (pos.z > bmax.z)	idx += 2;
  else				idx += 1;

  const Outline& ol = outlines[idx];
  num_array = ol.num;
  int i;
  for (i = 0 ; i < num_array ; i++)
  {
    switch (ol.vertices[i])
    {
      case 0: array[i].x = bmin.x; array[i].y = bmin.y; array[i].z = bmin.z; break;
      case 1: array[i].x = bmin.x; array[i].y = bmin.y; array[i].z = bmax.z; break;
      case 2: array[i].x = bmin.x; array[i].y = bmax.y; array[i].z = bmin.z; break;
      case 3: array[i].x = bmin.x; array[i].y = bmax.y; array[i].z = bmax.z; break;
      case 4: array[i].x = bmax.x; array[i].y = bmin.y; array[i].z = bmin.z; break;
      case 5: array[i].x = bmax.x; array[i].y = bmin.y; array[i].z = bmax.z; break;
      case 6: array[i].x = bmax.x; array[i].y = bmax.y; array[i].z = bmin.z; break;
      case 7: array[i].x = bmax.x; array[i].y = bmax.y; array[i].z = bmax.z; break;
    }
  }
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
