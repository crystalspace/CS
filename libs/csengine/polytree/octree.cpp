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
#include "csengine/bsp2d.h"
#include "csengine/treeobj.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/covcube.h"
#include "csengine/cbuffer.h"
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
  for (i = 0 ; i < num ; i++) polygons[i] = sector->GetPolygonInt (i);

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

void AddPolygonTo2DBSP (const csPlane3& plane, csBspTree2D* bsp2d,
	csPolygon3D* p)
{
  // We know the octree can currently only contain csPolygon3D
  // because it's the static octree.
  csPoly3D poly;
  int i;
  for (i = 0 ; i < p->GetNumVertices () ; i++)
    poly.AddVertex (p->Vwor (i));
  csSegment3 segment;
  if (csIntersect3::IntersectPolygon (plane, &poly, segment))
  {
    const csVector3& v1 = segment.Start ();
    const csVector3& v2 = segment.End ();
    csVector2 s1 (v1.y, v1.z);
    csVector2 s2 (v2.y, v2.z);
    csSegment2* seg2;
    // @@@ This test is probably very naive. The problem
    // is that IntersectPolygon returns a un-ordered segment.
    // i.e. start or end have no real meaning. For the 2D bsp
    // tree we need an ordered segment. So we classify (0,0,0)
    // in 3D to the polygon and (0,0) in 2D to the segment and
    // see if they have the same direction. If not we swap
    // the segment.
    csPlane2 pl (s1, s2);
    float cl3d = p->GetPolyPlane ()->Classify (csVector3 (0));
    float cl2d = pl.Classify (csVector2 (0, 0));
    if ((cl3d < 0 && cl2d < 0) || (cl3d > 0 && cl2d > 0))
    {
      CHK (seg2 = new csSegment2 (s1, s2));
    }
    else
    {
      CHK (seg2 = new csSegment2 (s2, s1));
    }
    bsp2d->Add (seg2);
  }
}

struct TestSolidData
{
  csVector2 pos;
  bool is_solid;
};

void* TestSolid (csSegment2** segments, int num, void* data)
{
  if (num == 0) return NULL; // Continue.
  TestSolidData* d = (TestSolidData*)data;
  csPlane2 plane (*segments[0]);
  if (plane.Classify (d->pos) > 0) d->is_solid = true;
  else d->is_solid = false;
  return (void*)1;	// Stop recursion.
}

// Given an array of csPolygonInt (which we know to be csPolygon3D
// in this case) fill three other arrays with x, y, and z values
// for all the vertices of those polygons that are in the given box.
// @@@@ UGLY CODE!!! EXPERIMENTAL ONLY!
// If this works good it should be cleaned up a lot.
void GetVertexComponents (csPolygonInt** polygons, int num, const csBox3& box,
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
    for (j = 0 ; j < p->GetNumVertices () ; j++)
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

int compare_float (const void* v1, const void* v2)
{
  float f1 = *(float*)v1;
  float f2 = *(float*)v2;
  if (f1 < f2) return -1;
  else if (f1 > f2) return 1;
  else return 0;
}

int RemoveDoubles (float* array, int num_ar, float* new_array)
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
  CHK (xarray_all = new float [num*10]);
  CHK (yarray_all = new float [num*10]);
  CHK (zarray_all = new float [num*10]);
  GetVertexComponents (polygons, num, tbox,
	xarray_all, num_xar, yarray_all, num_yar, zarray_all, num_zar);
  // Make sure the center is always included.
  xarray_all[num_xar++] = orig.x;
  yarray_all[num_yar++] = orig.y;
  zarray_all[num_zar++] = orig.z;
  qsort (xarray_all, num_xar, sizeof (float), compare_float);
  qsort (yarray_all, num_yar, sizeof (float), compare_float);
  qsort (zarray_all, num_zar, sizeof (float), compare_float);
  CHK (xarray = new float [num_xar]);
  CHK (yarray = new float [num_yar]);
  CHK (zarray = new float [num_zar]);
  int num_x, num_y, num_z;
  num_x = RemoveDoubles (xarray_all, num_xar, xarray);
  num_y = RemoveDoubles (yarray_all, num_yar, yarray);
  num_z = RemoveDoubles (zarray_all, num_zar, zarray);
  CHK (delete [] xarray_all);
  CHK (delete [] yarray_all);
  CHK (delete [] zarray_all);

  int i, j;
  csVector3 best_center = orig;
#if 0
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

# define DTRIES 10
  float dx = tbox.MaxX () - tbox.MinX ();
  float dy = tbox.MaxY () - tbox.MinY ();
  float dz = tbox.MaxZ () - tbox.MinZ ();

  // Try a few x-planes first.
  float x, y, z;
  float tx, ty;
  TestSolidData tdata;
  int count_solid;
  int max_solid = -1000;
  for (i = 0 ; i < num_x ; i++)
  {
    x = xarray[i]+.1;
    csPlane3 plane (1, 0, 0, -x);
    // First create a 2D bsp tree for all intersections of the
    // polygons in the node with the chosen plane.
    CHK (csBspTree2D* bsp2d = new csBspTree2D ());
    for (j = 0 ; j < num ; j++)
      if (polygons[j]->ClassifyX (x) == POL_SPLIT_NEEDED)
	AddPolygonTo2DBSP (plane, bsp2d, (csPolygon3D*)polygons[j]);
    // Given the calculated 2D bsp tree we will now try to see
    // how much space is solid and how much space is open. We use
    // a simple (but not very accurate) heuristic for this.
    // We just take a number of samples and find the first segment
    // for every of those samples. We then see if we are in front
    // or behind the sample. If behind then we are in solid space.
    // Otherwise we are in open space.
    count_solid = 0;
    for (tx = tbox.MinY ()+dy/(DTRIES*2) ; tx < tbox.MaxY () ; tx += dy/DTRIES)
      for (ty = tbox.MinZ ()+dz/(DTRIES*2) ; ty < tbox.MaxZ () ; ty += dz/DTRIES)
      {
        tdata.pos.Set (tx, ty);
        bsp2d->Front2Back (tdata.pos, TestSolid, (void*)&tdata);
	if (tdata.is_solid) count_solid++;
      }
    if (count_solid > max_solid)
    {
      max_solid = count_solid;
      best_center.x = x;
    }

    CHK (delete bsp2d);
  }

  // Try y planes.
  max_solid = -1000;
  for (i = 0 ; i < num_y ; i++)
  {
    y = yarray[i]+.1;
    csPlane3 plane (0, 1, 0, -y);
    CHK (csBspTree2D* bsp2d = new csBspTree2D ());
    for (j = 0 ; j < num ; j++)
      if (polygons[j]->ClassifyY (y) == POL_SPLIT_NEEDED)
	AddPolygonTo2DBSP (plane, bsp2d, (csPolygon3D*)polygons[j]);
    count_solid = 0;
    for (tx = tbox.MinX ()+dx/(DTRIES*2) ; tx < tbox.MaxX () ; tx += dx/DTRIES)
      for (ty = tbox.MinZ ()+dz/(DTRIES*2) ; ty < tbox.MaxZ () ; ty += dz/DTRIES)
      {
        tdata.pos.Set (tx, ty);
        bsp2d->Front2Back (tdata.pos, TestSolid, (void*)&tdata);
	if (tdata.is_solid) count_solid++;
      }
    if (count_solid > max_solid)
    {
      max_solid = count_solid;
      best_center.y = y;
    }
    CHK (delete bsp2d);
  }

  // Try z planes.
  max_solid = -1000;
  for (i = 0 ; i < num_z ; i++)
  {
    z = zarray[i]+.1;
    csPlane3 plane (0, 0, 1, -z);
    CHK (csBspTree2D* bsp2d = new csBspTree2D ());
    for (j = 0 ; j < num ; j++)
      if (polygons[j]->ClassifyZ (z) == POL_SPLIT_NEEDED)
	AddPolygonTo2DBSP (plane, bsp2d, (csPolygon3D*)polygons[j]);
    count_solid = 0;
    for (tx = tbox.MinX ()+dx/(DTRIES*2) ; tx < tbox.MaxX () ; tx += dx/DTRIES)
      for (ty = tbox.MinY ()+dy/(DTRIES*2) ; ty < tbox.MaxY () ; ty += dy/DTRIES)
      {
        tdata.pos.Set (tx, ty);
        bsp2d->Front2Back (tdata.pos, TestSolid, (void*)&tdata);
	if (tdata.is_solid) count_solid++;
      }
    if (count_solid > max_solid)
    {
      max_solid = count_solid;
      best_center.z = z;
    }
    CHK (delete bsp2d);
  }
#else
  // Try a few x-planes first.
  float x, y, z;
  int splits, best_splits = 2000000000;
  for (i = 0 ; i < num_x ; i++)
  {
    x = xarray[i]+.1;
    splits = 0;
    for (j = 0 ; j < num ; j++)
      if (polygons[j]->ClassifyX (x) == POL_SPLIT_NEEDED) splits++;
    if (splits < best_splits)
    {
      best_center.x = x;
      best_splits = splits;
    }
  }
  best_splits = 2000000000;
  for (i = 0 ; i < num_y ; i++)
  {
    y = yarray[i]+.1;
    splits = 0;
    for (j = 0 ; j < num ; j++)
      if (polygons[j]->ClassifyY (y) == POL_SPLIT_NEEDED) splits++;
    if (splits < best_splits)
    {
      best_center.y = y;
      best_splits = splits;
    }
  }
  best_splits = 2000000000;
  for (i = 0 ; i < num_z ; i++)
  {
    z = zarray[i]+.1;
    splits = 0;
    for (j = 0 ; j < num ; j++)
      if (polygons[j]->ClassifyZ (z) == POL_SPLIT_NEEDED) splits++;
    if (splits < best_splits)
    {
      best_center.z = z;
      best_splits = splits;
    }
  }
#endif
  node->center = best_center;
  CHK (delete [] xarray);
  CHK (delete [] yarray);
  CHK (delete [] zarray);
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

#define PLANE_X 0
#define PLANE_Y 1
#define PLANE_Z 2

void csOctree::BoxOccludeeShadowPolygons (const csBox3& box,
	const csBox3& /*occludee*/,
	csPolygonInt** polygons, int num_polygons,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos)
{
  int i, j;
  csPolygon3D* p;
  csPoly3D cur_poly;
  csPoly2D proj_poly;
  csPoly2D result_poly;
  bool first_time = true;
  for (i = 0 ; i < num_polygons ; i++)
    if (polygons[i]->GetType () == 1)
    {
      p = (csPolygon3D*)polygons[i];
      cur_poly.SetNumVertices (0);
      for (j = 0 ; j < p->GetNumVertices () ; j++)
        cur_poly.AddVertex (p->Vwor (j));
      for (j = 0 ; j < 8 ; j++)
      {
	const csVector3& corner = box.GetCorner (j);
	cur_poly.ProjectAxisPlane (corner, plane_nr, plane_pos, &proj_poly);
	if (first_time)
	{
	  result_poly = proj_poly;
	  first_time = false;
	}
	else
	{
	  CHK (csClipper* clipper = new csPolygonClipper (&proj_poly));
	  result_poly.MakeRoom (MAX_OUTPUT_VERTICES);
	  int num_verts = result_poly.GetNumVertices ();
	  csBox2 bbox;
	  UByte rc = clipper->Clip (result_poly.GetVertices (), num_verts, bbox);
	  CHK (delete clipper);
	  if (rc == CS_CLIP_OUTSIDE)
	    num_verts = 0;
	  result_poly.SetNumVertices (num_verts);
	  if (num_verts == 0) break;
	}
      }
      if (result_poly.GetNumVertices () != 0)
      {
	// First scale the polygon to cbuffer dimensions.
	for (j = 0 ; j < result_poly.GetNumVertices () ; j++)
	{
	  csVector2& v = result_poly[j];
	  v = v-shift;
	  v.x *= scale.x;
	  v.y *= scale.y;
	}
	// Then clip to cbuffer dimensions.
	// @@@ WE NEED TO MAKE THIS CLIPPER EARLIER AND
	// GIVE IT TO THIS ROUTINE!
	csBox2 b (0, 0, 1024, 1024);
	CHK (csClipper* clipper = new csBoxClipper (b));
	result_poly.MakeRoom (MAX_OUTPUT_VERTICES);
	int num_verts = result_poly.GetNumVertices ();
	csBox2 bbox;
	if (clipper->Clip (result_poly.GetVertices (), num_verts, bbox))
	{
	printf ("cbuffer INSERT\n");
	  result_poly.SetNumVertices (num_verts);
	  cbuffer->InsertPolygon (result_poly.GetVertices (),
	  	result_poly.GetNumVertices ());
	  if (cbuffer->IsFull ()) return;
	}
	CHK (delete clipper);
      }
    }
}

void csOctree::BoxOccludeeAddShadows (csOctreeNode* occluder,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos,
	const csBox3& box, const csBox3& occludee,
	csVector3& box_center, csVector3& occludee_center)
{
  if (!occluder) return;
  if (cbuffer->IsFull ()) return;
  int i;
  const csBox3& occluder_box = occluder->GetBox ();
  if (occluder_box.In (box_center) || occluder_box.In (occludee_center))
  {
    for (i = 0 ; i < 8 ; i++)
      BoxOccludeeAddShadows ((csOctreeNode*)occluder->children[i],
      	cbuffer, scale, shift, plane_nr, plane_pos,
	box, occludee, box_center, occludee_center);
  }
  else if (occluder_box.Between (box, occludee))
  {
    if (occluder->IsLeaf ())
      BoxOccludeeShadowPolygons (box, occludee,
	occluder->unsplit_polygons.GetPolygons (),
	occluder->unsplit_polygons.GetNumPolygons (),
	cbuffer, scale, shift, plane_nr, plane_pos);
    else
      for (i = 0 ; i < 8 ; i++)
        BoxOccludeeAddShadows ((csOctreeNode*)occluder->children[i],
      	  cbuffer, scale, shift, plane_nr, plane_pos,
	  box, occludee, box_center, occludee_center);
  }
}

// This routine will take a set of 2D planes (i.e. lines) and calculate
// all the intersections. Then it will only keep those intersections that
// are in or on the smallest convex polygons formed by the intersection
// of those planes. From those vertices it will calculate a bounding box
// in 2D.
bool CalcBBoxFromLines (csPlane2* planes2d, int num_planes2d, csBox2& bbox)
{
  int i, j, k;
  csVector2 isect;
  csVector2 points[64];
  k = 0;
  // Find all intersection points between the planes.
  for (i = 0 ; i < num_planes2d ; i++)
    for (j = i+1 ; j < num_planes2d ; j++)
      if (csIntersect2::Planes (planes2d[i], planes2d[j], isect))
	points[k++] = isect;
  // Calculate the bounding box for all intersection points that
  // are in or on the smallest polygon.
  if (k == 0) return false;
  bbox.StartBoundingBox ();
  bool rc = false;
  for (i = 0 ; i < k ; i++)
  {
    bool in = true;
    for (j = 0 ; j < num_planes2d ; j++)
    {
      //printf ("i(%d)=%f,%f j(%d)=%f\n", i, points[i].x, points[i].y, j, planes2d[j].Classify (points[i]));
      if (planes2d[j].Classify (points[i]) < -SMALL_EPSILON)
      {
        in = false;
	break;
      }
    }
    if (in)
    {
      bbox.AddBoundingVertex (points[i]);
      rc = true;
    }
  }
  return rc;
}

// This routine traces all lines between the vertices of the two boxes
// and intersects those lines with an axis aligned plane. Then a bounding
// box on that plane is calculated for those intersections.
void CalcBBoxFromBoxes (const csBox3& box1, const csBox3& box2,
	int plane_nr, float plane_pos, csBox2& plane_area)
{
  int i, j;
  plane_area.StartBoundingBox ();
  for (i = 0 ; i < 8 ; i++)
  {
    csVector3 v1 = box1.GetCorner (i);
    for (j = 0 ; j < 8 ; j++)
    {
      csVector3 v2 = box2.GetCorner (j);
      csVector3 v = v2-v1;
      float dist = -(v1[plane_nr] - plane_pos) / v[plane_nr];
      csVector3 isect = v1 + dist*v;
      csVector2 v2d;
      switch (plane_nr)
      {
        case PLANE_X: v2d.Set (isect.y, isect.z); break;
        case PLANE_Y: v2d.Set (isect.x, isect.z); break;
        case PLANE_Z: v2d.Set (isect.x, isect.y); break;
      }
      plane_area.AddBoundingVertex (v2d);
    }
  }
}

bool csOctree::BoxCanSeeOccludee (const csBox3& box, const csBox3& occludee)
{
  // This routine works as follows: first we find a suitable plane
  // between 'box' and 'occludee' on which we're going to project all
  // the potentially occluding polygons (which are between box and occludee).
  // The shape of the occludee itself projected on that plane 'lights up'
  // some polygon shape on the plane. For this light polygon we create a
  // c-buffer on which we'll insert the projected occluder polygons one
  // by one (more on this projection later). As soon as the c-buffer is
  // completely full we know that all the polygons shadow the occludee
  // for the box so we can stop and return 'false'.
  //
  // To project a polygon on the plane we consider the occludee is an
  // area light source. We take every vertex of the occludee (8 total)
  // and project that to a 2D polygon on the plane. Then we intersect
  // all those polygons. The resulting intersection is then added to the
  // c-buffer.

  int i;

  // First find a suitable plane between box and occludee. We take a plane
  // as close to box as possible (i.e. it will be a plane containing one
  // of the sides of 'box').
  csVector3 box_center = box.GetCenter ();
  csVector3 occludee_center = occludee.GetCenter ();
  float dx = ABS (box_center.x - occludee_center.x);
  float dy = ABS (box_center.y - occludee_center.y);
  float dz = ABS (box_center.z - occludee_center.z);
  int plane_nr;
  float plane_pos;
  if (dx > dy && dx > dz) plane_nr = PLANE_X;
  else if (dy > dz) plane_nr = PLANE_Y;
  else plane_nr = PLANE_Z;
  if (box_center[plane_nr] > occludee_center[plane_nr])
    plane_pos = box.Min (plane_nr);
  else
    plane_pos = box.Max (plane_nr);

  // On this plane we now find the largest possible area that will
  // be used. This corresponds with the projection on the plane of
  // the outer set of planes between the occludee and the box.
#if 1
  csBox2 plane_area;
printf ("\n--------\n");
printf ("box=(%f,%f,%f)-(%f,%f,%f)\n",
box.MinX (), box.MinY (), box.MinZ (),
box.MaxX (), box.MaxY (), box.MaxZ ());
printf ("occludee=(%f,%f,%f)-(%f,%f,%f)\n",
occludee.MinX (), occludee.MinY (), occludee.MinZ (),
occludee.MaxX (), occludee.MaxY (), occludee.MaxZ ());
printf ("box_center=(%f,%f,%f) occludee_center=(%f,%f,%f)\n",
box_center.x, box_center.y, box_center.z,
occludee_center.x, occludee_center.y, occludee_center.z);
printf ("plane_nr=%d plane_pos=%f\n", plane_nr, plane_pos);
  CalcBBoxFromBoxes (box, occludee, plane_nr, plane_pos, plane_area);
#else
  csPlane3 planes[8];
  int num_planes;
  num_planes = csMath3::OuterPlanes (occludee, box, planes);
  csPlane2 planes2d[8]; // The lines making up the 2D outline.
  for (i = 0 ; i < num_planes ; i++)
    csIntersect3::PlaneAxisPlane (planes[i], plane_nr, plane_pos, planes2d[i]);
  // @@@ This routine is not ideal. We would like to be able to have
  // a convex 2D polygon for the projection of the outer planes on the
  // axis aligned plane but I'm not sure how to do this efficiently. For
  // now we just calculate a bounding box in 2D which may in some cases
  // slightly overestimate the lit area (but not much).
  csBox2 plane_area;
  if (!CalcBBoxFromLines (planes2d, num_planes, plane_area)) return false;
#endif
printf ("plane_area=%f,%f %f,%f\n", plane_area.MinX (), plane_area.MinY (),
	plane_area.MaxX (), plane_area.MaxY ());

  // From the calculated plane area we can now calculate a scale to get
  // to a c-buffer size of 1024x1024. We also allocate this c-buffer here.
  CHK (csCBuffer* cbuffer = new csCBuffer (0, 1023, 1024));
  csVector2 scale = (plane_area.Max () - plane_area.Min ()) / 1024.;
  scale.x = 1./scale.x;
  scale.y = 1./scale.y;
  csVector2 shift = plane_area.Min ();

  cbuffer->Initialize ();
  BoxOccludeeAddShadows ((csOctreeNode*)root, cbuffer, scale, shift,
  	plane_nr, plane_pos,
  	box, occludee, box_center, occludee_center);

  // If the c-buffer is full then the occludee will not be visible.
  bool full = cbuffer->IsFull ();
  CHK (delete cbuffer);
  return !full;
}

void csOctree::BuildPVSForLeaf (csOctreeNode* occludee, csThing* thing,
	csOctreeNode* leaf)
{
  if (!occludee) return;
  int i;
  bool visible = false;
  if (occludee->GetBox ().In (leaf->GetCenter ()))
    visible = true;
  else if (leaf->GetBox ().Adjacent (occludee->GetBox ()))
    // @@@ It would be nice if we could also include adjacent
    // nodes in the PVS by testing if the shared plane between
    // the two nodes is completely solid.
    visible = true;
  else
  {
    bool rc = BoxCanSeeOccludee (leaf->GetBox (), occludee->GetBox ());
    printf ("[%d]", rc); fflush (stdout);
    if (rc) visible = true;
  }

  // If visible then we add to the PVS and build the PVS for
  // the polygons in the node as well.
  // Also traverse to the children.
  if (visible)
  {
    csPVS& pvs = leaf->GetPVS ();
    csOctreeVisible* ovis = pvs.Add ();
    ovis->SetOctreeNode (occludee);

    //@@@ PVS NOT YET IMPLEMENTED FOR POLYGONS
    //if (occludee->IsLeaf ())
      //BuildPVSForLeafPolygons (occludee, ovis, ...);
    for (i = 0 ; i < 8 ; i++)
      BuildPVSForLeaf ((csOctreeNode*)occludee->children[i],
      	thing, leaf);
  }
}

void csOctree::BuildPVS (csThing* thing,
	csOctreeNode* node)
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
    csPVS& pvs = node->GetPVS ();
    pvs.Clear ();
printf ("*"); fflush (stdout);
    BuildPVSForLeaf ((csOctreeNode*)root, thing, node);
  }
  else
  {
    // Traverse to the children.
    int i;
    for (i = 0 ; i < 8 ; i++)
      BuildPVS (thing, (csOctreeNode*)node->children[i]);
  }
}

void csOctree::BuildPVS (csThing* thing)
{
  BuildPVS (thing, (csOctreeNode*)root);
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
  int avg_bsp_nodes = num_bsp_trees ? tot_bsp_nodes / num_bsp_trees : 0;
  int avg_bsp_leaves = num_bsp_trees ? tot_bsp_leaves / num_bsp_trees : 0;
  int avg_max_depth = num_bsp_trees ? tot_max_depth / num_bsp_trees : 0;
  int avg_tot_poly = num_bsp_trees ? tot_tot_poly / num_bsp_trees : 0;
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
