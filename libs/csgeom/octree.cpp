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

csOctreeNode::csOctreeNode ()
{
  int i;
  for (i = 0 ; i < 8 ; i++)
    children[i] = NULL;
  minibsp = NULL;
}

csOctreeNode::~csOctreeNode ()
{
  int i;
  for (i = 0 ; i < 8 ; i++)
  {
    CHK (delete children[i]);
  }
  CHK (delete minibsp);
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

void csOctree::Build (csOctreeNode* node, const csVector3& bmin, const csVector3& bmax,
	csPolygonInt** polygons, int num)
{
  if (num <= bsp_num)
  {
    csBspTree* bsp;
    CHK (bsp = new csBspTree (pset, mode));
    node->SetMiniBsp (bsp);
    bsp->Build (polygons, num);
    return;
  }

  csVector3 center = (bmin + bmax) / 2;
  node->SetCenter (center);

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
      Build ((csOctreeNode*)node->children[i], new_bmin, new_bmax, polys[i], idx[i]);
    }
    CHK (delete [] polys[i]);
  }
}

void csOctree::BuildDynamic (csOctreeNode* node, const csVector3& bmin, const csVector3& bmax,
	csPolygonInt** polygons, int num)
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

void* csOctree::Back2Front (const csVector3& pos, csTreeVisitFunc* func, void* data)
{
  return Back2Front ((csOctreeNode*)root, pos, func, data);
}

void* csOctree::Front2Back (const csVector3& pos, csTreeVisitFunc* func, void* data)
{
  return Front2Back ((csOctreeNode*)root, pos, func, data);
}

void* csOctree::Back2Front (csOctreeNode* node, const csVector3& pos,
	csTreeVisitFunc* func, void* data)
{
  if (!node) return NULL;

  if (node->GetMiniBsp ())
    return node->GetMiniBsp ()->Back2Front (pos, func, data);

  void* rc;
  const csVector3& center = node->GetCenter ();
  int cur_idx;
  if (pos.x <= center.x) cur_idx = 0;
  else cur_idx = 4;
  if (pos.y > center.y) cur_idx |= 2;
  if (pos.z > center.z) cur_idx |= 1;

  rc = Back2Front ((csOctreeNode*)node->children[7-cur_idx], pos, func, data); if (rc) return rc;
  rc = Back2Front ((csOctreeNode*)node->children[(7-cur_idx) ^ 1], pos, func, data); if (rc) return rc;
  rc = Back2Front ((csOctreeNode*)node->children[(7-cur_idx) ^ 2], pos, func, data); if (rc) return rc;
  rc = Back2Front ((csOctreeNode*)node->children[(7-cur_idx) ^ 4], pos, func, data); if (rc) return rc;
  rc = Back2Front ((csOctreeNode*)node->children[cur_idx ^ 1], pos, func, data); if (rc) return rc;
  rc = Back2Front ((csOctreeNode*)node->children[cur_idx ^ 2], pos, func, data); if (rc) return rc;
  rc = Back2Front ((csOctreeNode*)node->children[cur_idx ^ 4], pos, func, data); if (rc) return rc;
  rc = Back2Front ((csOctreeNode*)node->children[cur_idx], pos, func, data);
  return rc;
}

void* csOctree::Front2Back (csOctreeNode* node, const csVector3& pos,
	csTreeVisitFunc* func, void* data)
{
  if (!node) return NULL;

  if (node->GetMiniBsp ())
    return node->GetMiniBsp ()->Front2Back (pos, func, data);

  void* rc;
  const csVector3& center = node->GetCenter ();
  int cur_idx;
  if (pos.x <= center.x) cur_idx = 0;
  else cur_idx = 4;
  if (pos.y > center.y) cur_idx |= 2;
  if (pos.z > center.z) cur_idx |= 1;

  rc = Front2Back ((csOctreeNode*)node->children[cur_idx], pos, func, data); if (rc) return rc;
  rc = Front2Back ((csOctreeNode*)node->children[cur_idx ^ 1], pos, func, data); if (rc) return rc;
  rc = Front2Back ((csOctreeNode*)node->children[cur_idx ^ 2], pos, func, data); if (rc) return rc;
  rc = Front2Back ((csOctreeNode*)node->children[cur_idx ^ 4], pos, func, data); if (rc) return rc;
  rc = Front2Back ((csOctreeNode*)node->children[(7-cur_idx) ^ 1], pos, func, data); if (rc) return rc;
  rc = Front2Back ((csOctreeNode*)node->children[(7-cur_idx) ^ 2], pos, func, data); if (rc) return rc;
  rc = Front2Back ((csOctreeNode*)node->children[(7-cur_idx) ^ 4], pos, func, data); if (rc) return rc;
  rc = Front2Back ((csOctreeNode*)node->children[7-cur_idx], pos, func, data);
  return rc;
}

//---------------------------------------------------------------------------
