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
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/covcube.h"
#include "csengine/polygon.h"
#include "csengine/thing.h"
#include "isystem.h"

//---------------------------------------------------------------------------

csPVS::~csPVS ()
{
  Clear ();
}

void csPVS::Clear ()
{
  while (visible)
  {
    csOctreeVisible* n = visible->next;
    CHK (delete visible);
    visible = n;
  }
}

csOctreeVisible* csPVS::Add ()
{
  CHK (csOctreeVisible* ovis = new csOctreeVisible ());
  ovis->next = visible;
  visible = ovis;
  return ovis;
}

//---------------------------------------------------------------------------

ULong csOctreeNode::pvs_cur_vis_nr = 1;

csOctreeNode::csOctreeNode ()
{
  int i;
  for (i = 0 ; i < 8 ; i++)
    children[i] = NULL;
  minibsp = NULL;
  minibsp_verts = NULL;
  leaf = false;
  pvs_vis_nr = 0;
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

csOctree::csOctree (csSector* sect, const csVector3& imin_bbox,
	const csVector3& imax_bbox, int ibsp_num, int imode)
	: csPolygonTree (sect)
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
  int num = sector->GetNumPolygons ();
  CHK (csPolygonInt** polygons = new csPolygonInt* [num]);
  for (i = 0 ; i < num ; i++) polygons[i] = sector->GetPolygon (i);

  CHK (root = new csOctreeNode);

  // Initialize the random generator to a fixed value to make
  // sure we get the same octree everytime (easier for debugging).
  srand (12345);
  Build ((csOctreeNode*)root, bbox.Min (), bbox.Max (), polygons, num);

  CHK (delete [] polygons);
}

void csOctree::Build (csPolygonInt** polygons, int num)
{
  CHK (root = new csOctreeNode);

  CHK (csPolygonInt** new_polygons = new csPolygonInt* [num]);
  int i;
  for (i = 0 ; i < num ; i++) new_polygons[i] = polygons[i];
  // Initialize the random generator to a fixed value to make
  // sure we get the same octree everytime (easier for debugging).
  srand (12345);
  Build ((csOctreeNode*)root, bbox.Min (), bbox.Max (), new_polygons, num);
  CHK (delete [] new_polygons);
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
    csPolyTreeObject* pto = stub->GetObject ();
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

float randflt ()
{
  return ((float)rand ()) / (float)RAND_MAX;
}

void csOctree::ChooseBestCenter (csOctreeNode* node,
	csPolygonInt** polygons, int num)
{
  const csVector3& bmin = node->GetMinCorner ();
  const csVector3& bmax = node->GetMaxCorner ();
  const csVector3& orig = node->GetCenter ();
  float xsize = (bmax.x - bmin.x)/10;
  float ysize = (bmax.y - bmin.y)/10;
  float zsize = (bmax.z - bmin.z)/10;

  int i, j;
  csVector3 best_center = orig;
  int best_splits = 2000000000;
  csVector3 tryit;
#if 0
  for (tryit.x = orig.x-xsize/2 ; tryit.x < orig.x+xsize/2 ; tryit.x += xsize/10)
    for (tryit.y = orig.y-ysize/2 ; tryit.y < orig.y+ysize/2 ; tryit.y += ysize/10)
      for (tryit.z = orig.z-zsize/2 ; tryit.z < orig.z+zsize/2 ; tryit.z += zsize/10)
      {
        int splits = 0;
        for (j = 0 ; j < num ; j++)
        {
          if (polygons[j]->ClassifyX (tryit.x) == POL_SPLIT_NEEDED) splits++;
          if (polygons[j]->ClassifyY (tryit.y) == POL_SPLIT_NEEDED) splits++;
          if (polygons[j]->ClassifyZ (tryit.z) == POL_SPLIT_NEEDED) splits++;
        }
        if (splits < best_splits)
        {
          best_center = tryit;
          best_splits = splits;
        }
      }
#elif 0
  // @@@ Choose based on vertices in polygons!
#elif 0
  // @@@ Choose one axis at a time instead of all three together!
#elif 0
  // @@@ Choose while trying to maximize the area of solid space
  // This will improve occlusion!
  // One way to do this:
  //    - Consider each plane (x,y,z) seperatelly.
  //    - Try several values for each plane.
  //    - Intersect plane with all polygons: resulting in set of lines.
  //    - Possibly organize lines in 2D BSP or beam tree.
  //    - Take a few samples on the plane and for every sample
  //      we draw four lines (to above, down, right, and left). We
  //      intersect every line with the nearest line from the intersections
  //      and calculate the distance. Also by checking the normal of the
  //      line we intersect with we can see if we are in solid or open
  //      space. The four distances are used to calculate an approx
  //      area of solid and open space. We add all solid space areas and
  //      subtract all open space areas and so we have a total solid space
  //      approx.
  //    - Take the plane with most solid space.
#else
  for (i = 0 ; i < 500 ; i++)
  {
    tryit.x = orig.x + xsize * (randflt ()-.5);
    tryit.y = orig.y + ysize * (randflt ()-.5);
    tryit.z = orig.z + zsize * (randflt ()-.5);
    int splits = 0;
    for (j = 0 ; j < num ; j++)
    {
      if (polygons[j]->ClassifyX (tryit.x) == POL_SPLIT_NEEDED) splits++;
      if (polygons[j]->ClassifyY (tryit.y) == POL_SPLIT_NEEDED) splits++;
      if (polygons[j]->ClassifyZ (tryit.z) == POL_SPLIT_NEEDED) splits++;
    }
    if (splits < best_splits)
    {
      best_center = tryit;
      best_splits = splits;
    }
  }
#endif
  node->center = best_center;
}

void csOctree::Build (csOctreeNode* node, const csVector3& bmin,
	const csVector3& bmax, csPolygonInt** polygons, int num)
{
  node->SetBox (bmin, bmax);

  if (num == 0)
  {
    node->leaf = true;
    return;
  }

  int i;
  for (i = 0 ; i < num ; i++)
    node->unsplit_polygons.AddPolygon (polygons[i]);

  if (num <= bsp_num)
  {
    csBspTree* bsp;
    CHK (bsp = new csBspTree (sector, mode));
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
    // Even if there are no polygons in the node we create
    // a child octree node because some of the visibility stuff
    // depends on that (i.e. adding dynamic objects).
    CHK (node->children[i] = new csOctreeNode);
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
  __TRAVERSE__ ((7-cur_idx) ^ 1);
  __TRAVERSE__ ((7-cur_idx) ^ 2);
  __TRAVERSE__ ((7-cur_idx) ^ 4);
  __TRAVERSE__ (7-cur_idx);
  return rc;
}

void csOctree::MarkVisibleFromPVS (const csVector3& pos)
{
  // First locate the leaf this position is in.
  csOctreeNode* node = (csOctreeNode*)root;
  while (node)
  {
    if (node->IsLeaf ()) break;
    const csVector3& center = node->GetCenter ();
    if (pos.x < center.x)
      if (pos.y < center.y)
        if (pos.z < center.z)
	  node = (csOctreeNode*)node->children[OCTREE_BBB];
	else
	  node = (csOctreeNode*)node->children[OCTREE_BBF];
      else
        if (pos.z < center.z)
	  node = (csOctreeNode*)node->children[OCTREE_BFB];
	else
	  node = (csOctreeNode*)node->children[OCTREE_BFF];
    else
      if (pos.y < center.y)
        if (pos.z < center.z)
	  node = (csOctreeNode*)node->children[OCTREE_FBB];
	else
	  node = (csOctreeNode*)node->children[OCTREE_FBF];
      else
        if (pos.z < center.z)
	  node = (csOctreeNode*)node->children[OCTREE_FFB];
	else
	  node = (csOctreeNode*)node->children[OCTREE_FFF];
  }

  // Mark all objects in the world as invisible.
  csOctreeNode::pvs_cur_vis_nr++;

  csPVS& pvs = node->GetPVS ();
  int j;
  // Here we mark all octree nodes as visible.
  // The polygons from the pvs are only marked visible
  // when we actually traverse to an octree node.
  csOctreeVisible* ovis = pvs.GetFirst ();
  while (ovis)
  {
    ovis->GetOctreeNode ()->MarkVisible ();
    csPolygonArrayNoFree& pol = ovis->GetPolygons ();
    for (j = 0 ; j < pol.Length () ; j++)
      pol.Get (j)->MarkVisible ();
    ovis = pvs.GetNext (ovis);
  }
}

struct PVSBuildData
{
  csCovcube* cube;
  csVector3 center;
};

void* MarkPolygonsVisiblePVS (csSector*,
	csPolygonInt** polygon, int num, void* data)
{
  PVSBuildData* pvsdata = (PVSBuildData*)data;
  csCovcube* cube = pvsdata->cube;
  const csVector3& center = pvsdata->center;
  int i, j;
  for (i = 0 ; i < num ; i++)
  {
    if (polygon[i]->GetType () != 1) continue;
    csPolygon3D* p = (csPolygon3D*)polygon[i];

    csVector3 poly[50];	// @@@ HARDCODED! BAD!
    for (j = 0 ; j < p->GetNumVertices () ; j++)
      poly[j] = p->Vwor (j)-center;

    bool vis = cube->InsertPolygon (poly, p->GetNumVertices ());
    if (vis) p->MarkVisible (); //@@@ Backface culling?
  }
  return NULL;
}

bool CullOctreeNodePVS (csPolygonTree* tree, csPolygonTreeNode* node,
	const csVector3& /*pos*/, void* data)
{
  if (!node) return false;
  if (node->Type () != NODE_OCTREE) return true;

  csOctree* otree = (csOctree*)tree;
  csOctreeNode* onode = (csOctreeNode*)node;

  PVSBuildData* pvsdata = (PVSBuildData*)data;
  csCovcube* cube = pvsdata->cube;
  const csVector3& center = pvsdata->center;

  // Test node against coverage mask cube.
  csVector3 outline[6];
  int num_outline;
  otree->GetConvexOutline (onode, center, outline, num_outline);
  if (num_outline > 0)
  {
    int i;
    for (i = 0 ; i < num_outline ; i++)
      outline[i] -= center;
    if (!cube->TestPolygon (outline, num_outline)) return false;
  }
  onode->MarkVisible ();
  return true;
}

struct PVSBuildData2
{
  // All collected leafs that can potentially block sight
  // between the original leaf and the current node.
  csVector leaf_queue;
  // The box for the original leaf.
  csBox3 leaf_box;
};

bool BuildPVSOctreeNode (csPolygonTree* /*tree*/, csPolygonTreeNode* node,
	const csVector3& /*pos*/, void* data)
{
  if (!node) return false;
  if (node->Type () != NODE_OCTREE) return true;

//   csOctree* otree = (csOctree*)tree;
  csOctreeNode* onode = (csOctreeNode*)node;

  PVSBuildData2* pvsdata = (PVSBuildData2*)data;
  csVector& leaf_queue = pvsdata->leaf_queue;

  // Test this node against all polygons in all leafs in the
  // queue already. If there is a polygon in there that completely
  // shadows the original leaf then this node is not visible.
  int leaf_num, poly_num, cor_num, i = 0, i1;
//   bool invisible = false;
  for (leaf_num = 0 ; i < leaf_queue.Length () ; i++)
  {
    csOctreeNode* leaf = (csOctreeNode*)leaf_queue[leaf_num];
    csPolygonIntArray& polygons = leaf->GetUnsplitPolygons ();
    for (poly_num = 0 ; poly_num < polygons.GetNumPolygons () ; poly_num++)
    {
      csPolygonInt* polygon = polygons.GetPolygon (poly_num);
      if (polygon->GetType () == 1)
      {
        // csPolygon3D.
	csPolygon3D* p = (csPolygon3D*)polygon;
	csPlane3* plane = p->GetPolyPlane ();
	for (cor_num = 0 ; cor_num < 8 ; cor_num++)
	{
	  csVector3 corner = onode->GetBox ().GetCorner (cor_num);
	  // Do backface culling first.
	  if (csMath3::Visible (corner, *plane))
	  {
	    i1 = p->GetNumVertices ()-1;
	    for (i = 0 ; i < p->GetNumVertices () ; i++)
	    {
// 	      csVector3& v1 = p->Vwor (i1);
// 	      csVector3& v2 = p->Vwor (i);

	      i1 = i;
	    }
	  }
	  else
	  {
	    // If backface culling fails then we assume
	    // that this polygon can never cull the node.
	    break;
	  }
	}
      }
    }
  }

  onode->MarkVisible ();
  return true;
}

void csOctree::AddToPVS (csPVS& pvs, csOctreeNode* node)
{
  if (!node) return;
  if (!node->IsVisible ()) return;

  csOctreeVisible* ovis = pvs.Add ();
  ovis->SetOctreeNode (node);

  if (node->GetMiniBsp ())
  {
    csBspTree* bsp = node->GetMiniBsp ();
    bsp->AddToPVS (&ovis->GetPolygons ());
  }

  int i;
  for (i = 0 ; i < 8 ; i++)
    AddToPVS (pvs, (csOctreeNode*)node->children[i]);
}

#if 1
void csOctree::BuildPVSForLeaf (csThing* thing,
	csOctreeNode* leaf, csCovcube* /*cube*/)
{
printf ("*"); fflush (stdout);
  // Here we traverse the octree starting from the leaf node.
  // Every other leaf we encounter is first checked against
  // the contents of the queue and if visible marked as visible
  // and then added to the queue as well.

  // Mark all objects in the world as invisible.
  csOctreeNode::pvs_cur_vis_nr++;

  // The root is always visible.
  ((csOctreeNode*)root)->MarkVisible ();

  const csVector3& center = leaf->GetCenter ();
  thing->UpdateTransformation (center);
  PVSBuildData2 pvsdata;
  pvsdata.leaf_box = leaf->bbox;
  Front2Back (center, NULL, NULL, BuildPVSOctreeNode, (void*)&pvsdata);

  // Now that we have traversed the tree for several positions we have
  // found a superset of all visible nodes and polygons. Here
  // we will fetch them all and put them in the PVS for this leaf.
  csPVS& pvs = leaf->GetPVS ();
  pvs.Clear ();
  AddToPVS (pvs, (csOctreeNode*)root);
}
#else
void csOctree::BuildPVSForLeaf (csThing* thing,
	csOctreeNode* leaf, csCovcube* cube)
{
printf ("*"); fflush (stdout);
  // Here we simulate a limited rendering on a 2D culler
  // cube. This is used to test for visibility so that we
  // can mark everything we encounter (nodes and polygons)
  // as visible.

  // Mark all objects in the world as invisible.
  csOctreeNode::pvs_cur_vis_nr++;

  // The root is always visible.
  ((csOctreeNode*)root)->MarkVisible ();

  // @@@ This algorithm is not good. It just samples a few
  // points in the cube and gathers all visible objects seen
  // from that point. It has two problems:
  //    1. Not accurate: it is possible to miss polygons or
  //       entire nodes that are visible from some point we
  //       didn't test.
  //    2. Not fast.
  // But this is easy to code and a temporary algorithm to
  // test the PVS in principle.
  const csVector3& bmin = leaf->GetMinCorner ();
  const csVector3& bmax = leaf->GetMaxCorner ();
  csVector3 steps = (bmax-bmin)/2.;
  //csVector3 steps = (bmax-bmin);//@@@
  csVector3 pos;
  PVSBuildData pvsdata;
  pvsdata.center = pos;
  pvsdata.cube = cube;
  for (pos.x = bmin.x+steps.x/2. ; pos.x < bmax.x ; pos.x += steps.x)
    for (pos.y = bmin.y+steps.y/2. ; pos.y < bmax.y ; pos.y += steps.y)
      for (pos.z = bmin.z+steps.z/2. ; pos.z < bmax.z ; pos.z += steps.z)
      {
        // Traverse the tree as seen from this position and mark every
	// node and polygon we see as visible.
        cube->MakeEmpty ();
        thing->UpdateTransformation (pos);
	Front2Back (pos, MarkPolygonsVisiblePVS, (void*)&pvsdata,
		CullOctreeNodePVS, (void*)&pvsdata);
      }

  // Now that we have traversed the tree for several positions we have
  // found all (not really, see above) visible nodes and polygons. Here
  // we will fetch them all and put them in the PVS for this leaf.
  csPVS& pvs = leaf->GetPVS ();
  pvs.Clear ();
  AddToPVS (pvs, (csOctreeNode*)root);
}
#endif

void csOctree::BuildPVS (csThing* thing,
	csOctreeNode* node, csCovcube* cube)
{
  if (!node) return;

  if (node->IsLeaf ())
  {
    // We have a leaf, here we start our pvs building.

    // @@@ Optimization note: it might be a good idea to
    // add a few deeper levels in the octree for the purpose
    // of the PVS only. i.e. our octree with mini-bsp tree
    // may remain exactly the same as it is now but we add
    // extra smaller octree leafs so that our granularity
    // for the PVS is better. Those octree leafs are ignored
    // by the normal polygon/node traversal process.
    BuildPVSForLeaf (thing, node, cube);
  }
  else
  {
    // Traverse to the children.
    int i;
    for (i = 0 ; i < 8 ; i++)
      BuildPVS (thing, (csOctreeNode*)node->children[i], cube);
  }
}

void csOctree::BuildPVS (csThing* thing)
{
  csCovcube* cube;
  CHK (cube = new csCovcube (csWorld::current_world->GetCovMaskLUT ()));
  BuildPVS (thing, (csOctreeNode*)root, cube);
  CHK (delete cube);
}

void csOctree::Statistics ()
{
  int num_oct_nodes = 0, max_oct_depth = 0, num_bsp_trees = 0;
  int tot_bsp_nodes = 0, min_bsp_nodes = 1000000000, max_bsp_nodes = 0;
  int tot_bsp_leaves = 0, min_bsp_leaves = 1000000000, max_bsp_leaves = 0;
  int tot_max_depth = 0, min_max_depth = 1000000000, max_max_depth = 0;
  int tot_tot_poly = 0, min_tot_poly = 1000000000, max_tot_poly = 0;
  Statistics ((csOctreeNode*)root, 0,
  	&num_oct_nodes, &max_oct_depth, &num_bsp_trees,
  	&tot_bsp_nodes, &min_bsp_nodes, &max_bsp_nodes,
	&tot_bsp_leaves, &min_bsp_leaves, &max_bsp_leaves,
	&tot_max_depth, &min_max_depth, &max_max_depth,
	&tot_tot_poly, &min_tot_poly, &max_tot_poly);
  int avg_bsp_nodes = tot_bsp_nodes / num_bsp_trees;
  int avg_bsp_leaves = tot_bsp_leaves / num_bsp_trees;
  int avg_max_depth = tot_max_depth / num_bsp_trees;
  int avg_tot_poly = tot_tot_poly / num_bsp_trees;
  CsPrintf (MSG_INITIALIZATION, "  oct_nodes=%d max_oct_depth=%d num_bsp_trees=%d\n",
  	num_oct_nodes, max_oct_depth, num_bsp_trees);
  CsPrintf (MSG_INITIALIZATION, "  bsp nodes: tot=%d avg=%d min=%d max=%d\n",
  	tot_bsp_nodes, avg_bsp_nodes, min_bsp_nodes, max_bsp_nodes);
  CsPrintf (MSG_INITIALIZATION, "  bsp leaves: tot=%d avg=%d min=%d max=%d\n",
  	tot_bsp_leaves, avg_bsp_leaves, min_bsp_leaves, max_bsp_leaves);
  CsPrintf (MSG_INITIALIZATION, "  bsp max depth: tot=%d avg=%d min=%d max=%d\n",
  	tot_max_depth, avg_max_depth, min_max_depth, max_max_depth);
  CsPrintf (MSG_INITIALIZATION, "  bsp tot poly: tot=%d avg=%d min=%d max=%d\n",
  	tot_tot_poly, avg_tot_poly, min_tot_poly, max_tot_poly);
  	
}

void csOctree::Statistics (csOctreeNode* node, int depth,
  	int* num_oct_nodes, int* max_oct_depth, int* num_bsp_trees,
  	int* tot_bsp_nodes, int* min_bsp_nodes, int* max_bsp_nodes,
	int* tot_bsp_leaves, int* min_bsp_leaves, int* max_bsp_leaves,
	int* tot_max_depth, int* min_max_depth, int* max_max_depth,
	int* tot_tot_poly, int* min_tot_poly, int* max_tot_poly)
{
  if (!node) return;
  depth++;
  if (depth > *max_oct_depth) *max_oct_depth = depth;
  (*num_oct_nodes)++;
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
	tot_tot_poly, min_tot_poly, max_tot_poly);
      }
  }
  depth--;
}

//---------------------------------------------------------------------------
