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

#include "cssysdef.h"
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
#include "ivfs.h"

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
    delete visible;
    visible = n;
  }
}

csOctreeVisible* csPVS::Add ()
{
  csOctreeVisible* ovis = new csOctreeVisible ();
  ovis->next = visible;
  ovis->prev = NULL;
  if (visible) visible->prev = ovis;
  visible = ovis;
  return ovis;
}

csOctreeVisible* csPVS::FindNode (csOctreeNode* onode)
{
  csOctreeVisible* ovis = visible;
  while (ovis)
  {
    if (ovis->node == onode) return ovis;
    ovis = ovis->next;
  }
  return NULL;
}

void csPVS::Delete (csOctreeVisible* ovis)
{
  if (ovis->prev)
    ovis->prev->next = ovis->next;
  else
    visible = ovis->next;
  if (ovis->next)
    ovis->next->prev = ovis->prev;
  delete ovis;
}

//---------------------------------------------------------------------------

#define PLANE_X 0
#define PLANE_Y 1
#define PLANE_Z 2

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

bool csOctreeNode::PVSCanSee (const csVector3& v)
{
  if (!pvs.GetFirst ()) return true; // PVS not yet computed.
  csOctreeVisible* vis;
  vis = pvs.GetFirst ();
  while (vis)
  {
    csOctreeNode* node = vis->GetOctreeNode ();
    if (node->IsLeaf () && node->GetBox ().In (v)) return true;
    vis = pvs.GetNext (vis);
  }
  return false;
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

static csVector3 GetVector3 (int plane_nr, float plane_pos,
	const csVector2& p)
{
  csVector3 v(0);
  switch (plane_nr)
  {
    case PLANE_X: v.Set (plane_pos, p.x, p.y); break;
    case PLANE_Y: v.Set (p.x, plane_pos, p.y); break;
    case PLANE_Z: v.Set (p.x, p.y, plane_pos); break;
  }
  return v;
}

struct SPIt
{
  int plane_nr;
  float plane_pos;
  int side_nr;
  int size;
  int x, y;
  UShort cur_mask;
  UShort next_mask;
  csBox2 box;
};

void* csOctreeNode::InitSolidPolygonIterator (int plane_nr, float plane_pos)
{
  SPIt* spit = new SPIt;
  spit->plane_nr = plane_nr;
  spit->plane_pos = plane_pos;
  spit->size = 4;
  spit->x = 0;
  spit->y = 0;
  if (ABS (bbox.Min (plane_nr) - plane_pos) < EPSILON) spit->side_nr = plane_nr*2;
  else spit->side_nr = plane_nr*2+1;
  spit->cur_mask = solid_masks[spit->side_nr];
  spit->next_mask = solid_masks[spit->side_nr];
  spit->box = bbox.GetSide (spit->side_nr);
  return (void*)spit;
}

bool csOctreeNode::NextSolidPolygon (void* vspit, csPoly3D& poly)
{
  SPIt* spit = (SPIt*)vspit;
  if (spit->size < 1) return false;
  poly.MakeEmpty ();
  csVector2 cor_xy = spit->box.GetCorner (BOX_CORNER_xy);
  csVector2 cor_Xy = spit->box.GetCorner (BOX_CORNER_Xy);
  csVector2 cor_xY = spit->box.GetCorner (BOX_CORNER_xY);
  csVector2 cor_XY = spit->box.GetCorner (BOX_CORNER_XY);
  int plane_nr = spit->plane_nr;
  float plane_pos = spit->plane_pos;
  UShort mask;
  csVector2 v;

  //-----
  // 4x4 sub-masks.
  //-----
  if (spit->cur_mask == 0) return false;
  if (spit->size == 4)
  {
    if (spit->cur_mask == (UShort)~0)
    {
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, cor_xy));
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, cor_Xy));
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, cor_XY));
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, cor_xY));
      spit->size = 0;
      return true;
    }
    // Initialize for first 3x3 test.
    spit->size = 3;
    spit->x = 0;
    spit->y = 0;
    spit->next_mask = spit->cur_mask;
  }
  //-----
  // 3x3 sub-masks.
  //-----
  if (spit->cur_mask == 0) return false;
  while (spit->size == 3)
  {
    // Get current top-left corner of 3x3 submask.
    int x = spit->x;
    int y = spit->y;

    // Prepare for next top-left corner of 3x3 submask or
    // prepare to go to 2x2 submasks.
    spit->x++;
    if (spit->x > 1)
    {
      spit->x = 0;
      spit->y++;
      if (spit->y > 1)
      {
	spit->y = 0;
	spit->size = 2;
	spit->cur_mask = spit->next_mask;
      }
    }

    // Fetch the 3x3 mask.
    int bitnr = y*4+x;
    mask  = (1<<bitnr) | (1<<(bitnr+1)) | (1<<(bitnr+2));
    bitnr += 4;
    mask |= (1<<bitnr) | (1<<(bitnr+1)) | (1<<(bitnr+2));
    bitnr += 4;
    mask |= (1<<bitnr) | (1<<(bitnr+1)) | (1<<(bitnr+2));
    // Test if mask is completely solid.
    if ((spit->cur_mask & mask) == mask)
    {
      // It is solid. First clear this sub-mask from the next mask
      // so that we will not process it for 2x2 or 1x1 sub-masks.
      spit->next_mask &= ~mask;
      v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + (x+3)*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + (x+3)*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + (y+3)*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + (y+3)*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      return true;
    }
  }
  //-----
  // 2x2 sub-masks.
  //-----
  if (spit->cur_mask == 0) return false;
  while (spit->size == 2)
  {
    // Get current top-left corner of 2x2 submask.
    int x = spit->x;
    int y = spit->y;

    // Prepare for next top-left corner of 2x2 submask or
    // prepare to go to 1x1 submasks.
    spit->x++;
    if (spit->x > 2)
    {
      spit->x = 0;
      spit->y++;
      if (spit->y > 2)
      {
	spit->y = 0;
	spit->size = 1;
	spit->cur_mask = spit->next_mask;
      }
    }

    // Fetch the 2x2 mask.
    int bitnr = y*4+x;
    mask  = (1<<bitnr) | (1<<(bitnr+1));
    bitnr += 4;
    mask |= (1<<bitnr) | (1<<(bitnr+1));
    // Test if mask is completely solid.
    if ((spit->cur_mask & mask) == mask)
    {
      // It is solid. First clear this sub-mask from the next mask
      // so that we will not process it for 1x1 sub-masks.
      spit->next_mask &= ~mask;
      v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + (x+2)*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + (x+2)*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + (y+2)*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + (y+2)*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      return true;
    }
  }
  //-----
  // 1x1 sub-masks.
  //-----
  if (spit->cur_mask == 0) return false;
  while (spit->size == 1)
  {
    // Get current top-left corner of 2x2 submask.
    int x = spit->x;
    int y = spit->y;

    // Prepare for next top-left corner of 1x1 submask.
    spit->x++;
    if (spit->x > 3)
    {
      spit->x = 0;
      spit->y++;
      if (spit->y > 3)
      {
	spit->y = 0;
	spit->size = 0;
      }
    }

    // Fetch the 1x1 mask.
    int bitnr = y*4+x;
    mask  = (1<<bitnr);
    // Test if mask is completely solid.
    if ((spit->cur_mask & mask) == mask)
    {
      v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + (x+1)*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + (x+1)*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + (y+1)*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + (y+1)*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      return true;
    }
  }
  return false;
}

void csOctreeNode::CleanupSolidPolygonIterator (void* vspit)
{
  SPIt* spit = (SPIt*)vspit;
  delete spit;
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
  csPolygonInt** polygons = new csPolygonInt* [num];
  for (i = 0 ; i < num ; i++) polygons[i] = sector->GetPolygonInt (i);

  root = new csOctreeNode;

  Build ((csOctreeNode*)root, bbox.Min (), bbox.Max (), polygons, num);

  delete [] polygons;

  CalculateSolidMasks ((csOctreeNode*)root);
}

void csOctree::Build (csPolygonInt** polygons, int num)
{
  root = new csOctreeNode;

  csPolygonInt** new_polygons = new csPolygonInt* [num];
  int i;
  for (i = 0 ; i < num ; i++) new_polygons[i] = polygons[i];
  Build ((csOctreeNode*)root, bbox.Min (), bbox.Max (), new_polygons, num);
  delete [] new_polygons;

  CalculateSolidMasks ((csOctreeNode*)root);
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

#if 0
static void AddPolygonTo2DBSP (const csPlane3& plane, csBspTree2D* bsp2d,
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
      seg2 = new csSegment2 (s1, s2);
    }
    else
    {
      seg2 = new csSegment2 (s2, s1);
    }
    bsp2d->Add (seg2);
  }
}
#endif

#if 0
struct TestSolidData
{
  csVector2 pos;
  bool is_solid;
};

static void* TestSolid (csSegment2** segments, int num, void* data)
{
  if (num == 0) return NULL; // Continue.
  TestSolidData* d = (TestSolidData*)data;
  csPlane2 plane (*segments[0]);
  if (plane.Classify (d->pos) > 0) d->is_solid = true;
  else d->is_solid = false;
  return (void*)1;	// Stop recursion.
}
#endif

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
    csBspTree2D* bsp2d = new csBspTree2D ();
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

    delete bsp2d;
  }

  // Try y planes.
  max_solid = -1000;
  for (i = 0 ; i < num_y ; i++)
  {
    y = yarray[i]+.1;
    csPlane3 plane (0, 1, 0, -y);
    csBspTree2D* bsp2d = new csBspTree2D ();
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
    delete bsp2d;
  }

  // Try z planes.
  max_solid = -1000;
  for (i = 0 ; i < num_z ; i++)
  {
    z = zarray[i]+.1;
    csPlane3 plane (0, 0, 1, -z);
    csBspTree2D* bsp2d = new csBspTree2D ();
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
    delete bsp2d;
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
    node->leaf = true;
    return;
  }

  int i;
  for (i = 0 ; i < num ; i++)
    node->unsplit_polygons.AddPolygon (polygons[i]);

  if (num <= bsp_num)
  {
    csBspTree* bsp;
    bsp = new csBspTree (sector, mode);
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
    int cur_idx;
    if (pos.x <= center.x) cur_idx = 0;
    else cur_idx = 4;
    if (pos.y > center.y) cur_idx |= 2;
    if (pos.z > center.z) cur_idx |= 1;
    node = (csOctreeNode*)node->children[cur_idx];
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

bool csOctree::CalculatePolygonShadow (
	const csVector3& corner,
	csPoly3D& cur_poly,
	csPoly2D& result_poly, bool first_time,
	int plane_nr, float plane_pos)
{
  csPoly2D proj_poly;

  // If ProjectAxisPlane returns false then the projection is not possible
  // without clipping the polygon to a plane near the plane going through
  // 'corner'. We just ignore this polygon to avoid clipping and make
  // things simple.
  if (!cur_poly.ProjectAxisPlane (corner, plane_nr, plane_pos, &proj_poly))
    return false;
  if (first_time)
    result_poly = proj_poly;
  else
  {
    float ar = csMath2::Area2 (proj_poly[0], proj_poly[1], proj_poly[2]);
    csClipper* clipper = new csPolygonClipper (&proj_poly, ar > 0);
    result_poly.MakeRoom (MAX_OUTPUT_VERTICES);
    int num_verts = result_poly.GetNumVertices ();
    UByte rc = clipper->Clip (result_poly.GetVertices (), num_verts,
	  result_poly.GetBoundingBox ());
    delete clipper;
    if (rc == CS_CLIP_OUTSIDE)
      num_verts = 0;
    result_poly.SetNumVertices (num_verts);
    if (num_verts == 0) return true;
  }
  return true;
}

// @@@ Put all these fields in a structure and pass them along?
// Clipper to use before sending polygons/shadows to the c-buffer.
static csClipper* box_clipper;
// Max size used for this pass of PVS building.
static csVector3 max_pvs_size;
// For statistics, number of times solid node opt is used.
static int total_solid_opt;
static int total_total_solid_opt;
// For statistics, number of times a node is culled.
static int total_cull_node;
static int total_total_cull_node;
static int count_leaves;

void csOctree::InsertShadowIntoCBuffer (csPoly2D& result_poly,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift)
{
  int j;
  // First scale the polygon to cbuffer dimensions.
  for (j = 0 ; j < result_poly.GetNumVertices () ; j++)
  {
    csVector2& v = result_poly[j];
    v = v-shift;
    v.x *= scale.x;
    v.y *= scale.y;
  }
  result_poly.UpdateBoundingBox ();
  // Then clip to cbuffer dimensions.
  //@@@ SCALE IS MORE EFFICIENT?
  result_poly.MakeRoom (MAX_OUTPUT_VERTICES);
  int num_verts = result_poly.GetNumVertices ();
  if (box_clipper->Clip (result_poly.GetVertices (), num_verts,
	  result_poly.GetBoundingBox ()))
  {
    result_poly.SetNumVertices (num_verts);
    cbuffer->InsertPolygon (result_poly.GetVertices (),
  	  result_poly.GetNumVertices ());
  }
}

void csOctree::CalculatePolygonShadowArea (
	const csBox3& occludee_box,
	csPoly3D& poly, const csPlane3& poly_plane,
	csPoly2D& result_poly,
	int plane_nr, float plane_pos)
{
  int j;
  bool first_time = true;
  for (j = 0 ; j < 8 ; j++)
  {
    const csVector3& corner = occludee_box.GetCorner (j);

    // Backface culling: if plane is totally on edge with corner or
    // if corner can see polygon then we ignore it.
    if (csMath3::Visible (corner, poly_plane) ||
	ABS (poly_plane.Classify (corner)) < SMALL_EPSILON)
    {
      result_poly.SetNumVertices (0);
      break;
    }

    if (!CalculatePolygonShadow (corner, poly, result_poly,
	first_time, plane_nr, plane_pos))
    {
      result_poly.SetNumVertices (0);
      break;
    }
    first_time = false;
    if (result_poly.GetNumVertices () == 0) break;
  }
}

void csOctree::BoxOccludeeShadowPolygons (const csBox3& /*box*/,
	const csBox3& occludee_box,
	csPolygonInt** polygons, int num_polygons,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos)
{
  int i, j;
  csPolygon3D* p;
  csPoly3D cur_poly;
  csPoly2D result_poly;
  for (i = 0 ; i < num_polygons ; i++)
    if (polygons[i]->GetType () == 1)
    {
      p = (csPolygon3D*)polygons[i];
      cur_poly.SetNumVertices (0);
      for (j = 0 ; j < p->GetNumVertices () ; j++)
        cur_poly.AddVertex (p->Vwor (j));
      csPlane3* wplane = p->GetPolyPlane ();
      CalculatePolygonShadowArea (occludee_box, cur_poly, *wplane, result_poly,
        plane_nr, plane_pos);
      if (result_poly.GetNumVertices () != 0)
      {
        InsertShadowIntoCBuffer (result_poly, cbuffer, scale, shift);
	if (cbuffer->IsFull ()) return;
      }
    }
}

void csOctree::BoxOccludeeShadowSolidBoundaries (csOctreeNode* occluder,
	const csBox3& occludee_box,
    	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos)
{
  void* spit = occluder->InitSolidPolygonIterator (plane_nr, plane_pos);
  csPoly3D cur_poly;
  csPoly2D result_poly;
  csPlane3 poly_plane;
  while (occluder->NextSolidPolygon (spit, cur_poly))
  {
    poly_plane = cur_poly.ComputePlane ();
    CalculatePolygonShadowArea (occludee_box, cur_poly, poly_plane, result_poly,
        plane_nr, plane_pos);
    if (result_poly.GetNumVertices () != 0)
    {
      InsertShadowIntoCBuffer (result_poly, cbuffer, scale, shift);
      if (cbuffer->IsFull ())
      {
        occluder->CleanupSolidPolygonIterator (spit);
        return;
      }
    }
  }
  occluder->CleanupSolidPolygonIterator (spit);
}

bool csOctree::BoxOccludeeShadowOutline (const csBox3& occluder_box,
	const csBox3& occludee,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos)
{
  int j;
  csPoly3D cur_poly;
  csPoly2D result_poly;
  bool first_time = true;

  for (j = 0 ; j < 8 ; j++)
  {
    const csVector3& corner = occludee.GetCorner (j);
    cur_poly.MakeRoom (6);
    int num_verts;
    occluder_box.GetConvexOutline (corner, cur_poly.GetVertices (),
    	num_verts);
    cur_poly.SetNumVertices (num_verts);

    if (!CalculatePolygonShadow (corner, cur_poly, result_poly,
	first_time, plane_nr, plane_pos))
      return false;
    first_time = false;
    if (result_poly.GetNumVertices () == 0) break;
  }
  if (result_poly.GetNumVertices () != 0)
  {
    InsertShadowIntoCBuffer (result_poly, cbuffer, scale, shift);
    if (cbuffer->IsFull ()) return true;
  }
  return true;
}

void csOctree::BoxOccludeeAddShadows (csOctreeNode* occluder,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos,
	const csBox3& box, const csBox3& occludee_box,
	csVector3& box_center, csVector3& occludee_center,
	bool do_polygons)
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
	box, occludee_box, box_center, occludee_center, do_polygons);
  }
  else if (occluder_box.Between (box, occludee_box))
  {
    // First see if this occluder can see occludee. If not then
    // we take the outline of this node and insert that instead
    // of the polygons in the node.
    if (!occluder->PVSCanSee (occludee_center))
    {
      total_solid_opt++;
      total_total_solid_opt++;
      if (BoxOccludeeShadowOutline (occluder_box, occludee_box,
	cbuffer, scale, shift, plane_nr, plane_pos))
      return;
    }

    // Here we first take the solid space outlines of the occluder
    // and insert them in the c-buffer as well.
    BoxOccludeeShadowSolidBoundaries (occluder, occludee_box,
    	cbuffer, scale, shift, plane_nr, plane_pos);
    if (cbuffer->IsFull ()) return;

    // Even though this box is between we will continue recursing
    // to the children to test for their outline (if an entire
    // node cannot see an occludee then we can take the outline of
    // that node to insert in the c-buffer).
    for (i = 0 ; i < 8 ; i++)
      BoxOccludeeAddShadows ((csOctreeNode*)occluder->children[i],
      	cbuffer, scale, shift, plane_nr, plane_pos,
	box, occludee_box, box_center, occludee_center, false);
    if (cbuffer->IsFull ()) return;

    if (do_polygons)
      BoxOccludeeShadowPolygons (box, occludee_box,
	occluder->unsplit_polygons.GetPolygons (),
	occluder->unsplit_polygons.GetNumPolygons (),
	cbuffer, scale, shift, plane_nr, plane_pos);
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
      if (planes2d[j].Classify (points[i]) < -SMALL_EPSILON)
      {
        in = false;
	break;
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
      csVector2 v2d(0,0);
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

bool csOctree::BoxCanSeeOccludee (const csBox3& box, const csBox3& occludee_box)
{
  // This routine works as follows: first we find a suitable plane
  // between 'box' and 'occludee_box' on which we're going to project all
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

  // First find a suitable plane between box and occludee. We take a plane
  // as close to box as possible (i.e. it will be a plane containing one
  // of the sides of 'box').
  csVector3 box_center = box.GetCenter ();
  csVector3 occludee_center = occludee_box.GetCenter ();
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
  csBox2 plane_area;
  CalcBBoxFromBoxes (box, occludee_box, plane_nr, plane_pos, plane_area);

  // From the calculated plane area we can now calculate a scale to get
  // to a c-buffer size of 1024x1024. We also allocate this c-buffer here.
  csCBuffer* cbuffer = new csCBuffer (0, 1023, 1024);
  csVector2 scale = (plane_area.Max () - plane_area.Min ()) / 1024.;
  scale.x = 1./scale.x;
  scale.y = 1./scale.y;
  csVector2 shift = plane_area.Min ();

  cbuffer->Initialize ();
  BoxOccludeeAddShadows ((csOctreeNode*)root, cbuffer, scale, shift,
  	plane_nr, plane_pos,
  	box, occludee_box, box_center, occludee_center, true);

  // If the c-buffer is full then the occludee will not be visible.
  bool full = cbuffer->IsFull ();
  delete cbuffer;
  return !full;
}

void csOctree::DeleteNodeAndChildrenFromPVS (csPVS& pvs, csOctreeNode* occludee)
{
  if (!occludee) return;
  pvs.Delete (occludee);
  int i;
  for (i = 0 ; i < 8 ; i++)
    DeleteNodeAndChildrenFromPVS (pvs, (csOctreeNode*)occludee->children[i]);
}

void csOctree::BuildPVSForLeaf (csOctreeNode* occludee, csThing* thing,
	csOctreeNode* leaf)
{
  if (!occludee) return;
  int i;
  bool visible = false;
  csVector3 dist;
  int adjacent_side;
  leaf->GetBox ().ManhattanDistance (occludee->GetBox (), dist);
  if (dist.x > max_pvs_size.x || dist.y  > max_pvs_size.y || dist.z > max_pvs_size.z)
  {
    // Node is too far away (max_pvs_size) so we don't test it
    // in this pass.
    return;
  }
  else if (!leaf->GetPVS ().FindNode (occludee))
  {
    // Node is not in PVS so it isn't visible.
    return;
  }
  else if (occludee->GetBox ().In (leaf->GetCenter ()))
    visible = true;
  else if ((adjacent_side = leaf->GetBox ().Adjacent (occludee->GetBox ())) != -1)
  {
    // Two sides are adjacent.
    csBox2 leaf_side = leaf->GetBox ().GetSide (adjacent_side);
    csBox2 occludee_side = occludee->GetBox ().GetSide (
    	csBox3::OtherSide (adjacent_side));
    // If the leaf box completely overlaps with the occludee box
    // (i.e. the occludee is smaller) then we need to test if
    // that leaf side is solid to have no visibility.
    if (leaf->GetSolidMask (adjacent_side) == (UShort)~0 &&
    	leaf_side.Overlap (occludee_side))
    {
      printf ("!");
      visible = false;
    }
    else if (occludee->GetSolidMask (csBox3::OtherSide (adjacent_side)) == (UShort)~0 &&
    	occludee_side.Overlap (leaf_side))
    {
      printf ("?");
      visible = false;
    }
    else visible = true;
  }
  else if (BoxCanSeeOccludee (leaf->GetBox (), occludee->GetBox ()))
    visible = true;

  // If visible then we add to the PVS and build the PVS for
  // the polygons in the node as well.
  // Also traverse to the children.
  if (visible)
    for (i = 0 ; i < 8 ; i++)
      BuildPVSForLeaf ((csOctreeNode*)occludee->children[i],
      	thing, leaf);
  else
  {
    total_cull_node += 1+occludee->CountChildren ();
    total_total_cull_node += 1+occludee->CountChildren ();
    csPVS& pvs = leaf->GetPVS ();
    DeleteNodeAndChildrenFromPVS (pvs, occludee);
  }
}

void csOctree::AddDummyPVSNodes (csOctreeNode* leaf, csOctreeNode* node)
{
  if (!node) return;
  csPVS& pvs = leaf->GetPVS ();
  csOctreeVisible* ovis = pvs.Add ();
  ovis->SetOctreeNode (node);
  // Traverse to the children.
  int i;
  for (i = 0 ; i < 8 ; i++)
    AddDummyPVSNodes (leaf, (csOctreeNode*)node->children[i]);
}

void csOctree::SetupDummyPVS (csOctreeNode* node)
{
  if (!node) return;
  if (node->IsLeaf ())
  {
    csPVS& pvs = node->GetPVS ();
    pvs.Clear ();
    AddDummyPVSNodes (node, (csOctreeNode*)root);
  }
  else
  {
    // Traverse to the children.
    int i;
    for (i = 0 ; i < 8 ; i++)
      SetupDummyPVS ((csOctreeNode*)node->children[i]);
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
    printf ("%d:", count_leaves); fflush (stdout);
    count_leaves++;
    total_cull_node = 0;
    total_solid_opt = 0;
    BuildPVSForLeaf ((csOctreeNode*)root, thing, node);
    if (total_cull_node)
      printf ("-%d,%d ", total_cull_node, total_solid_opt);
    else
      printf ("%d ", total_solid_opt);
    fflush (stdout);
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
  // First setup a dummy PVS which will cause all the nodes in the
  // world to be added as visible to every other node.
  SetupDummyPVS ();

  // First setup a clipper used for building the PVS.
  // This clipper will be used to clip polygons/shadows before they are
  // sent to the c-buffer.
  csBox2 b (0, 0, 1024, 1024);
  box_clipper = new csBoxClipper (b);

  // Calculate a good initial size for the first pass of PVS building.
  // In this first pass we only calculate PVS for leafs and occludee's
  // that are close to each other. The reason is that we can then
  // use that visibility information to optimize further passes of
  // PVS building -> if B cannot see A then B can be considered completely
  // solid when testing if C can see A.
  max_pvs_size.Set (
  	(bbox.MaxX ()-bbox.MinX ())/10.,
  	(bbox.MaxY ()-bbox.MinY ())/10.,
  	(bbox.MaxZ ()-bbox.MinZ ())/10.);

  // Then build the PVS for real which will remove nodes from the
  // PVS (hopefully).
  CsPrintf (MSG_INITIALIZATION, "  Pass 1...\n");
  total_total_cull_node = 0;
  total_total_solid_opt = 0;
  count_leaves = 0;
  BuildPVS (thing, (csOctreeNode*)root);
  printf ("\nTotal culled nodes=%d, total opt=%d\n", total_total_cull_node,
  	total_total_solid_opt);

  // Now we do the second pass. Currently the second pass does the full
  // PVS.
  max_pvs_size.Set (
  	(bbox.MaxX ()-bbox.MinX ())*2.,
  	(bbox.MaxY ()-bbox.MinY ())*2.,
  	(bbox.MaxZ ()-bbox.MinZ ())*2.);
  CsPrintf (MSG_INITIALIZATION, "  Pass 2...\n");
  total_total_cull_node = 0;
  total_total_solid_opt = 0;
  count_leaves = 0;
  BuildPVS (thing, (csOctreeNode*)root);
  printf ("\nTotal culled nodes=%d, total opt=%d\n", total_total_cull_node,
  	total_total_solid_opt);

  delete box_clipper;
}

static void SplitOptPlane2 (const csPoly3D* np, csPoly3D& inputF, const csPoly3D** npF,
	csPoly3D& inputB, const csPoly3D** npB,
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
    switch (xyz)
    {
      case 0: np->SplitWithPlaneX (inputF, inputB, xyz_val); break;
      case 1: np->SplitWithPlaneY (inputF, inputB, xyz_val); break;
      case 2: np->SplitWithPlaneZ (inputF, inputB, xyz_val); break;
    }
    *npF = &inputF;
    *npB = &inputB;
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

int csOctree::ClassifyPolygon (csOctreeNode* node, const csPoly3D& poly)
{
  if (node->GetMiniBsp ())
  {
    csBspTree* bsp = node->GetMiniBsp ();
    int rc = bsp->ClassifyPolygon (poly);
    if (rc == 1)
    {
      // @@@ Should we test all points of the polygon here?
      if (!ClassifyPoint (poly[0])) rc = 0;
    }
    return rc;
  }
  if (node->IsLeaf ())
  {
    // @@@ Should we test all points of the polygon here?
    if (ClassifyPoint (poly[0])) return 1;
    else return 0;
  }

  const csVector3& center = node->GetCenter ();

  csPoly3D inputF, inputB, inputFF, inputFB, inputBF, inputBB;
  const csPoly3D* npF, * npB, * npFF, * npFB, * npBF, * npBB;
  csPoly3D inputFFF, inputFFB, inputFBF, inputFBB,
  	   inputBFF, inputBFB, inputBBF, inputBBB;
  const csPoly3D* nps[8];
  SplitOptPlane2 (&poly, inputF, &npF, inputB, &npB, 0, center.x);
  SplitOptPlane2 (npF, inputFF, &npFF, inputFB, &npFB, 1, center.y);
  SplitOptPlane2 (npB, inputBF, &npBF, inputBB, &npBB, 1, center.y);
  SplitOptPlane2 (npFF, inputFFF, &nps[OCTREE_FFF],
  	inputFFB, &nps[OCTREE_FFB], 2, center.z);
  SplitOptPlane2 (npFB, inputFBF, &nps[OCTREE_FBF],
  	inputFBB, &nps[OCTREE_FBB], 2, center.z);
  SplitOptPlane2 (npBF, inputBFF, &nps[OCTREE_BFF],
  	inputBFB, &nps[OCTREE_BFB], 2, center.z);
  SplitOptPlane2 (npBB, inputBBF, &nps[OCTREE_BBF],
  	inputBBB, &nps[OCTREE_BBB], 2, center.z);
  int i;
  bool found_open = false, found_solid = false;
  for (i = 0 ; i < 8 ; i++)
    if (node->children[i] && nps[i])
    {
      int rc = ClassifyPolygon ((csOctreeNode*)node->children[i], *nps[i]);
      if (rc == -1) return rc;
      if (rc == 0) found_open = true;
      if (rc == 1) found_solid = true;
      if (found_solid && found_open) return -1;
    }
  if (found_solid) return 1;
  return 0;
}

/*
 *  0123
 *  4567
 *  89AB  -> UShort: FEDCBA9876543210
 *  CDEF
 */
UShort csOctree::ClassifyRectangle (int plane_nr, float plane_pos,
  	const csBox2& box)
{
  csVector2 cor_xy = box.GetCorner (BOX_CORNER_xy);
  csVector2 cor_Xy = box.GetCorner (BOX_CORNER_Xy);
  csVector2 cor_xY = box.GetCorner (BOX_CORNER_xY);
  csVector2 cor_XY = box.GetCorner (BOX_CORNER_XY);
  csPoly3D poly;
  poly.AddVertex (GetVector3 (plane_nr, plane_pos, cor_xy));
  poly.AddVertex (GetVector3 (plane_nr, plane_pos, cor_Xy));
  poly.AddVertex (GetVector3 (plane_nr, plane_pos, cor_XY));
  poly.AddVertex (GetVector3 (plane_nr, plane_pos, cor_xY));
  int rc = ClassifyPolygon (poly);
  if (rc == 0) return 0;
  if (rc == 1) return (UShort)~0;
  // Most general case. Here we will test every individual bit.
  int x, y;
  int bitnr = 0;
  csVector2 v;
  UShort result = 0;
  for (y = 0 ; y < 4 ; y++)
    for (x = 0 ; x < 4 ; x++)
    {
      poly.MakeEmpty ();
      v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + (x+1)*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + (x+1)*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + (y+1)*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4;
      v.y = cor_xy.y + (y+1)*(cor_XY.y-cor_xy.y)/4;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      rc = ClassifyPolygon (poly);
      if (rc == 1) result |= 1<<bitnr;
      bitnr++;
    }
  return result;
}

static csBox2 GetSideBox (int octree_side, const csBox3& box)
{
  csBox2 box2;
  switch (octree_side)
  {
    case BOX_SIDE_x:
    case BOX_SIDE_X: 
      box2.Set (box.MinY (), box.MinZ (), box.MaxY (), box.MaxZ ());
      break;
    case BOX_SIDE_y:
    case BOX_SIDE_Y: 
      box2.Set (box.MinX (), box.MinZ (), box.MaxX (), box.MaxZ ());
      break;
    case BOX_SIDE_z:
    case BOX_SIDE_Z: 
      box2.Set (box.MinX (), box.MinY (), box.MaxX (), box.MaxY ());
      break;
  }
  return box2;
}

void csOctree::CalculateSolidMasks (csOctreeNode* node)
{
  if (!node) return;

  node->solid_masks[BOX_SIDE_x] = ClassifyRectangle (PLANE_X,
  	node->GetBox ().MinX (), GetSideBox (BOX_SIDE_x, node->GetBox ()));
  node->solid_masks[BOX_SIDE_X] = ClassifyRectangle (PLANE_X,
  	node->GetBox ().MaxX (), GetSideBox (BOX_SIDE_X, node->GetBox ()));
  node->solid_masks[BOX_SIDE_y] = ClassifyRectangle (PLANE_Y,
  	node->GetBox ().MinY (), GetSideBox (BOX_SIDE_y, node->GetBox ()));
  node->solid_masks[BOX_SIDE_Y] = ClassifyRectangle (PLANE_Y,
  	node->GetBox ().MaxY (), GetSideBox (BOX_SIDE_Y, node->GetBox ()));
  node->solid_masks[BOX_SIDE_z] = ClassifyRectangle (PLANE_Z,
  	node->GetBox ().MinZ (), GetSideBox (BOX_SIDE_z, node->GetBox ()));
  node->solid_masks[BOX_SIDE_Z] = ClassifyRectangle (PLANE_Z,
  	node->GetBox ().MaxZ (), GetSideBox (BOX_SIDE_Z, node->GetBox ()));
  int i;

  for (i = 0 ; i < 8 ; i++)
    CalculateSolidMasks ((csOctreeNode*)node->children[i]);
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

  // Gather statistics about PVS.
  int avg_pvs_vis_nodes = num_pvs_leaves ? tot_pvs_vis_nodes / num_pvs_leaves : 0;
  int avg_pvs_vis_poly = num_pvs_leaves ? tot_pvs_vis_poly / num_pvs_leaves : 0;
  int tot_polygons = ((csOctreeNode*)root)->CountPolygons ();
  CsPrintf (MSG_INITIALIZATION, "  pvs vis nodes: tot=%d avg=%d min=%d max=%d\n",
  	tot_pvs_vis_nodes, avg_pvs_vis_nodes, min_pvs_vis_nodes, max_pvs_vis_nodes);
  CsPrintf (MSG_INITIALIZATION, "  pvs vis poly: avg=%d%% min=%d%% max=%d%%\n",
  	avg_pvs_vis_poly * 100 / tot_polygons,
  	min_pvs_vis_poly * 100 / tot_polygons,
  	max_pvs_vis_poly * 100 / tot_polygons);
  if (best_pvs_node)
  {
    const csVector3 center = best_pvs_node->GetCenter ();
    CsPrintf (MSG_INITIALIZATION, "  best pvs node at %f,%f,%f\n",
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
    csPVS& pvs = node->GetPVS ();
    csOctreeVisible* ovis = pvs.GetFirst ();
    while (ovis)
    {
      num_vis_nodes++;
      csOctreeNode* vis_node = ovis->GetOctreeNode ();
      if (vis_node->IsLeaf ()) num_vis_poly += vis_node->CountPolygons ();
      ovis = pvs.GetNext (ovis);
    }
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
  // Consistency check
  WriteLong (cf, node->unsplit_polygons.GetNumPolygons ());
  WriteVector3 (cf, node->GetCenter ());
  WriteUShort (cf, node->solid_masks[0]);
  WriteUShort (cf, node->solid_masks[1]);
  WriteUShort (cf, node->solid_masks[2]);
  WriteUShort (cf, node->solid_masks[3]);
  WriteUShort (cf, node->solid_masks[4]);
  WriteUShort (cf, node->solid_masks[5]);
  WriteBool (cf, node->leaf);
  WriteBool (cf, node->minibsp != NULL);
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
  iFile* cf = vfs->Open (name, VFS_FILE_WRITE);
  WriteString (cf, "OCTR", 4);
  // Version number.
  WriteLong (cf, 100002);
  WriteBox3 (cf, bbox);
  WriteLong (cf, (long)bsp_num);
  WriteLong (cf, (long)mode);
  Cache ((csOctreeNode*)root, cf);
  cf->DecRef ();
}

bool csOctree::ReadFromCache (iFile* cf, csOctreeNode* node,
	const csVector3& bmin, const csVector3& bmax,
  	csPolygonInt** polygons, int num)
{
  if (ReadLong (cf) != num)
  {
    CsPrintf (MSG_WARNING, "Octree does not match with loaded level!\n");
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
    bsp = new csBspTree (sector, mode);
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
      CsPrintf (MSG_WARNING, "Corrupt cached octree! Wrong node number!\n");
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
    CsPrintf (MSG_WARNING, "Cached octree corrupt! Expected end marker!\n");
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
    CsPrintf (MSG_WARNING, "Cached octree '%s' not valid! Will be ignored.\n",
    	name);
    cf->DecRef ();
    return false;	// Bad format!
  }
  long format_version = ReadLong (cf);
  if (format_version != 100002)
  {
    CsPrintf (MSG_WARNING, "Mismatched format version. Expected %ld, got %ld!\n",
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

void csOctree::GetNodePath (csOctreeNode* node, csOctreeNode* child,
	unsigned char* path, int& path_len)
{
  if (node == child) return;

  int i;
  for (i = 0 ; i < 8 ; i++)
    if (node->children[i])
    {
      csOctreeNode* onode = (csOctreeNode*)node->children[i];
      if (onode == child)
      {
        path[path_len++] = i+1;
	return;
      }
      if (onode->bbox.In (child->GetCenter ()))
      {
        path[path_len++] = i+1;
	GetNodePath (onode, child, path, path_len);
	return;
      }
    }
}

csOctreeNode* csOctree::GetNodeFromPath (csOctreeNode* node,
	unsigned char* path, int path_len)
{
  int i;
  for (i = 0 ; i < path_len ; i++)
  {
    if (!node) return NULL;
    node = (csOctreeNode*)node->children[path[i]-1];
  }
  return node;
}

bool csOctree::ReadFromCachePVS (iFile* cf, csOctreeNode* node)
{
  if (!node) return true;
  unsigned char buf[255];
  csPVS& pvs = node->GetPVS ();
  pvs.Clear ();
  unsigned char b = ReadByte (cf);
  while (b)
  {
    b--;
    if (b) ReadString (cf, (char*)buf, b);
    csOctreeVisible* ovis = pvs.Add ();
    csOctreeNode* occludee = GetNodeFromPath ((csOctreeNode*)root,
    	buf, b);
    if (!occludee)
    {
      CsPrintf (MSG_WARNING, "Cached PVS does not match this world!\n");
      return false;
    }
    ovis->SetOctreeNode (occludee);
    b = ReadByte (cf);
  }
  if (ReadByte (cf) != 'X')
  {
    CsPrintf (MSG_WARNING, "Cached PVS is not valid!\n");
    return false;
  }
  int i;
  for (i = 0 ; i < 8 ; i++)
    if (!ReadFromCachePVS (cf, (csOctreeNode*)node->children[i]))
      return false;
  return true;
}

bool csOctree::ReadFromCachePVS (iVFS* vfs, const char* name)
{
  iFile* cf = vfs->Open (name, VFS_FILE_READ);
  if (!cf) return false;		// File doesn't exist
  char buf[10];
  ReadString (cf, buf, 4);
  if (strncmp (buf, "OPVS", 4))
  {
    CsPrintf (MSG_WARNING, "Cached PVS '%s' not valid! Will be ignored.\n",
    	name);
    cf->DecRef ();
    return false;	// Bad format!
  }
  long format_version = ReadLong (cf);
  if (format_version != 100001)
  {
    CsPrintf (MSG_WARNING, "Unknown format version (%ld)!\n", format_version);
    cf->DecRef ();
    return false;
  }

  bool rc = ReadFromCachePVS (cf, (csOctreeNode*)root);
  cf->DecRef ();
  return rc;
}

void csOctree::CachePVS (csOctreeNode* node, iFile* cf)
{
  if (!node) return;
  csPVS& pvs = node->GetPVS ();
  csOctreeVisible* ovis = pvs.GetFirst ();
  unsigned char path[255];
  int path_len;
  while (ovis)
  {
    path_len = 0;
    GetNodePath ((csOctreeNode*)root, ovis->GetOctreeNode (), path, path_len);
    WriteByte (cf, path_len+1);		// First write length of path + 1
    if (path_len) WriteString (cf, (char*)path, path_len);
    ovis = pvs.GetNext (ovis);
  }
  WriteByte (cf, 0);	// End marker
  WriteByte (cf, 'X');	// Just a small check character to see if we're ok.
  int i;
  for (i = 0 ; i < 8 ; i++)
    CachePVS ((csOctreeNode*)node->children[i], cf);
}

void csOctree::CachePVS (iVFS* vfs, const char* name)
{
  iFile* cf = vfs->Open (name, VFS_FILE_WRITE);
  WriteString (cf, "OPVS", 4);
  // Version number.
  WriteLong (cf, 100001);
  CachePVS ((csOctreeNode*)root, cf);
  cf->DecRef ();
}

//---------------------------------------------------------------------------
