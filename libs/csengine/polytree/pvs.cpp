/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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
#include "csengine/cbufcube.h"
#include "csengine/cbuffer.h"
#include "csengine/polygon.h"
#include "csengine/thing.h"
#include "csengine/poledges.h"
#include "csengine/dumper.h"
#include "isystem.h"
#include "ivfs.h"

#define DO_PVS_PASS1 1
#define DO_PVS_SOLID_NODE_OPT 1
#define DO_PVS_SOLID_SPACE_OPT 1
#define DO_PVS_ADJACENT_NODES 1
#define DO_PVS_POLYGONS 1
#define DO_PVS_MERGE_ADJACENT_POLYGONS 0
#define DO_PVS_QAD 0

#define PLANE_X 0
#define PLANE_Y 1
#define PLANE_Z 2

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

ULong csOctreeNode::pvs_cur_vis_nr = 1;

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

static csVector3 GetVector3 (int plane_nr, float plane_pos,
	const csVector2& p)
{
  csVector3 v;
  switch (plane_nr)
  {
    case PLANE_X: v.Set (plane_pos, p.x, p.y); break;
    case PLANE_Y: v.Set (p.x, plane_pos, p.y); break;
    case PLANE_Z: v.Set (p.x, p.y, plane_pos); break;
    default: v.Set (0, 0, 0); break;
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

void* csOctreeNode::InitSolidPolygonIterator (int side_nr)
{
  SPIt* spit = new SPIt;
  spit->side_nr = side_nr;
  spit->plane_nr = side_nr/2;
  spit->plane_pos = (side_nr&1) ? bbox.Max (spit->plane_nr)
  				: bbox.Min (spit->plane_nr);
  spit->size = 4;
  spit->x = 0;
  spit->y = 0;
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

void csOctree::CalculatePolygonShadow (
	csPoly2D& proj_poly,
	csPoly2D& result_poly, bool first_time)
{
  if (first_time)
    result_poly = proj_poly;
  else
  {
    //float ar = csMath2::Area2 (proj_poly[0], proj_poly[1], proj_poly[2]);
    float ar = proj_poly.GetSignedArea ();
    csClipper* clipper = new csPolygonClipper (&proj_poly, ar > 0);
    result_poly.MakeRoom (MAX_OUTPUT_VERTICES);
    int num_verts = result_poly.GetNumVertices ();
    UByte rc = clipper->Clip (result_poly.GetVertices (), num_verts,
	  result_poly.GetBoundingBox ());
    delete clipper;
    if (rc == CS_CLIP_OUTSIDE)
      num_verts = 0;
    result_poly.SetNumVertices (num_verts);
    if (num_verts == 0) return;
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
  CalculatePolygonShadow (proj_poly, result_poly, first_time);
  return true;
}

// @@@ Put all these fields in a structure and pass them along?
// Clipper to use before sending polygons/shadows to the c-buffer.
static csClipper* box_clipper;
// In pass one we only do solid boundaries.
static bool pvs_solid_boundaries_only;
// For statistics, number of times solid node opt is used.
static int total_total_solid_opt;
// For statistics, number of times a node is culled.
static int total_cull_node;
static int total_total_cull_node;
static int count_leaves;

bool csOctree::TestShadowIntoCBuffer (const csPoly2D& result_poly,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift)
{
  int j;
  csPoly2D scaled_poly;
  // First scale the polygon to cbuffer dimensions.
  for (j = 0 ; j < result_poly.GetNumVertices () ; j++)
  {
    csVector2 v = result_poly[j];
    v = v-shift;
    v.x *= scale.x;
    v.y *= scale.y;
    scaled_poly.AddVertex (v);
  }
  // Then clip to cbuffer dimensions.
  //@@@ SCALE IS MORE EFFICIENT?
  scaled_poly.MakeRoom (MAX_OUTPUT_VERTICES);
  int num_verts = scaled_poly.GetNumVertices ();
  if (box_clipper->Clip (scaled_poly.GetVertices (), num_verts,
	  scaled_poly.GetBoundingBox ()))
  {
    scaled_poly.SetNumVertices (num_verts);
    return cbuffer->TestPolygon (scaled_poly.GetVertices (),
  	  scaled_poly.GetNumVertices ());
  }
  return false;
}

void csOctree::InsertShadowIntoCBuffer (const csPoly2D& result_poly,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift)
{
  int j;
  csPoly2D scaled_poly;
  // First scale the polygon to cbuffer dimensions.
  for (j = 0 ; j < result_poly.GetNumVertices () ; j++)
  {
    csVector2 v = result_poly[j];
    v = v-shift;
    v.x *= scale.x;
    v.y *= scale.y;
    scaled_poly.AddVertex (v);
  }
  // Then clip to cbuffer dimensions.
  //@@@ SCALE IS MORE EFFICIENT?
  scaled_poly.MakeRoom (MAX_OUTPUT_VERTICES);
  int num_verts = scaled_poly.GetNumVertices ();
  if (box_clipper->Clip (scaled_poly.GetVertices (), num_verts,
	  scaled_poly.GetBoundingBox ()))
  {
    scaled_poly.SetNumVertices (num_verts);
    cbuffer->InsertPolygon (scaled_poly.GetVertices (),
  	  scaled_poly.GetNumVertices ());
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
    if (poly_plane.Classify (corner) < SMALL_EPSILON)
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

bool csOctree::CalculatePolygonsShadowArea (
	const csBox3& occludee_box,
	csPoly3D& poly1, const csPlane3& plane1, int edge1,
	csPoly3D& poly2, const csPlane3& plane2,
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
    if (plane1.Classify (corner) < SMALL_EPSILON)
      return false;
    if (plane2.Classify (corner) < SMALL_EPSILON)
      return false;

    csPoly2D proj_poly1, proj_poly2;
    if (!poly1.ProjectAxisPlane (corner, plane_nr, plane_pos, &proj_poly1))
      return false;
    if (!poly2.ProjectAxisPlane (corner, plane_nr, plane_pos, &proj_poly2))
      return false;
    proj_poly1.ExtendConvex (proj_poly2, edge1);
    CalculatePolygonShadow (proj_poly1, result_poly, first_time);
    first_time = false;
    if (result_poly.GetNumVertices () == 0)
      return false;
  }
  return true;
}

#if DO_PVS_MERGE_ADJACENT_POLYGONS
void csOctree::BoxOccludeeShadowPolygons (const csBox3& /*box*/,
	const csBox3& occludee_box,
	csPolygonInt** polygons, int num_polygons,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos)
{
  // This version will try to merge
  // adjacent polygons into one bigger convex polygon.
  csPolygonEdges edges (polygons, num_polygons);

  int i, j, j1, k;
  csPolygon3D* p;
  csPoly3D cur_poly, other_poly;
  csPoly2D result_poly;

  // Scan every polygon in the list.
  for (i = 0 ; i < num_polygons ; i++)
    if (polygons[i]->GetType () == 1)
    {
      p = (csPolygon3D*)polygons[i];
      csPlane3* wplane = p->GetPolyPlane ();
      cur_poly.SetNumVertices (0);
      for (k = 0 ; k < p->GetNumVertices () ; k++)
        cur_poly.AddVertex (p->Vwor (k));

      // Scan every edge of this polygon and for every edge
      // we find all adjacent polygons. For every combination
      // of this polygon and an adjacent polygon we project a
      // shadow if possible.
      // The 'casted_shadow' variable will be set to true as soon
      // as a combined shadow is succesfully casted. In this case
      // we don't have to cast the shadow of this polygon anymore.
      bool casted_shadow = false;
      j1 = p->GetNumVertices ()-1;
      for (j = 0 ; j > p->GetNumVertices () ; j++)
      {
        csPolEdgeIterator* pol_it = edges.GetPolygons (j1, j);
	while (pol_it->HasNext ())
	{
	  csPolygon3D* other = pol_it->Next ();
          other_poly.SetNumVertices (0);
          for (k = 0 ; k < other->GetNumVertices () ; k++)
            other_poly.AddVertex (other->Vwor (k));
	  csPlane3* oplane = other->GetPolyPlane ();
	  if (CalculatePolygonsShadowArea (occludee_box, cur_poly, *wplane, j1,
	  	other_poly, *oplane, result_poly, plane_nr, plane_pos))
	  {
	    casted_shadow = true;
	    // We have a shadow. Insert it into the C-buffer.
            if (result_poly.GetNumVertices () != 0)
            {
              InsertShadowIntoCBuffer (result_poly, cbuffer, scale, shift);
	      if (cbuffer->IsFull ())
	      {
	      	delete pol_it;
	        return;
	      }
            }
	  }
	}
	delete pol_it;
        j1 = j;
      }

      // If we did not yet cast a shadow we have to try this polygon
      // as well.
      if (!casted_shadow)
      {
        CalculatePolygonShadowArea (occludee_box, cur_poly, *wplane, result_poly,
          plane_nr, plane_pos);
        if (result_poly.GetNumVertices () != 0)
        {
          InsertShadowIntoCBuffer (result_poly, cbuffer, scale, shift);
	  if (cbuffer->IsFull ()) return;
        }
      }
    }
}
#else
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
#endif

void csOctree::BoxOccludeeShadowSolidBoundaries (csOctreeNode* occluder,
	const csBox3& occludee_box,
    	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos)
{
  int side;
  for (side = 0 ; side < 6 ; side++)
  {
    void* spit = occluder->InitSolidPolygonIterator (side);
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
}

bool csOctree::BoxOccludeeShadowOutline (const csBox3& occluder_box,
	const csBox3& occludee,
	int plane_nr, float plane_pos, csPoly2D& result_poly)
{
  int j;
  csPoly3D cur_poly;
  result_poly.MakeEmpty ();
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
    // Calculate the shadow of the occluder node. If there is no shadow
    // then it doesn't make sense to test the contents of this node further.
    csPoly2D occluder_shadow;
    if (BoxOccludeeShadowOutline (occluder_box, occludee_box,
    	plane_nr, plane_pos, occluder_shadow))
    {
      if (occluder_shadow.GetNumVertices () == 0)
        return;
      // Then we test the shadow on the c-buffer and see if that part of
      // the c-buffer has some unshadowed parts. If not then we don't
      // have to process this occluder either.
      if (!TestShadowIntoCBuffer (occluder_shadow, cbuffer, scale, shift))
        return;
    }

    // Then we see if this occluder can see occludee. If not then
    // we take the outline of this node and insert that instead
    // of the polygons in the node.
#   if DO_PVS_SOLID_NODE_OPT
    if (!pvs_solid_boundaries_only)
      if (!occluder->PVSCanSee (occludee_center))
      {
        total_total_solid_opt++;
        InsertShadowIntoCBuffer (occluder_shadow, cbuffer, scale, shift);
	return;
      }
#   endif // DO_PVS_SOLID_NODE_OPT

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

#   if DO_PVS_POLYGONS
    if (do_polygons && !pvs_solid_boundaries_only)
      BoxOccludeeShadowPolygons (box, occludee_box,
	occluder->unsplit_polygons.GetPolygons (),
	occluder->unsplit_polygons.GetNumPolygons (),
	cbuffer, scale, shift, plane_nr, plane_pos);
#   endif // DO_PVS_POLYGONS
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
  csVector3 man_dist;
  box.ManhattanDistance (occludee_box, man_dist);
  // Take as the plane the component with the largest manhattan
  // distance value.
  int plane_nr;
  if (man_dist.x >= man_dist.y && man_dist.x >= man_dist.z)
    plane_nr = PLANE_X;
  else if (man_dist.y >= man_dist.z && man_dist.y >= man_dist.x)
    plane_nr = PLANE_Y;
  else
    plane_nr = PLANE_Z;

  csVector3 box_center = box.GetCenter ();
  csVector3 occludee_center = occludee_box.GetCenter ();

  float plane_pos;
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

static float randflt ()
{
  return ((float)rand ()) / RAND_MAX;
}

bool csOctree::BoxCanSeeOccludeeSuperSlow (const csBox3& box,
	const csBox3& occludee_box)
{
  int tries = 0;
  csVector3 box_pos, occludee_pos;
  for (;;)
  {
    box_pos.Set (
      box.MinX ()+randflt ()*(box.MaxX ()-box.MinX ()),
      box.MinY ()+randflt ()*(box.MaxY ()-box.MinY ()),
      box.MinZ ()+randflt ()*(box.MaxZ ()-box.MinZ ()));
    occludee_pos.Set (
      occludee_box.MinX ()+randflt ()*(occludee_box.MaxX ()-occludee_box.MinX ()),
      occludee_box.MinY ()+randflt ()*(occludee_box.MaxY ()-occludee_box.MinY ()),
      occludee_box.MinZ ()+randflt ()*(occludee_box.MaxZ ()-occludee_box.MinZ ()));
    csVector3 isect;
    csPolygon3D* p = sector->HitBeam (box_pos, occludee_pos, isect);
    if (!p) goto error;
    // We hit a polygon. Check if intersection is in occludee node.
    if (occludee_box.In (isect)) goto error;
    tries++;
    if (tries % 1000 == 0) printf ("Tries %d\n", tries);
    if (tries > 40000) return false;
  }
  // This function will actually never return false. It will just try finding
  // intersections forever.
  return false;

error:
printf ("(%f,%f,%f) - (%f,%f,%f)\n", box_pos.x, box_pos.y, box_pos.z,
occludee_pos.x, occludee_pos.y, occludee_pos.z);
  exit (0);
  return true;
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
  int adjacent_side;
  csPVS& pvs = leaf->GetPVS ();
  csOctreeVisible* ovis = pvs.FindNode (occludee);
  if (!ovis)
  {
    // Node is not in PVS so it isn't visible.
    return;
  }
  //else if (ovis->IsReallyVisible ())
  //{
    // Node is in PVS and is marked REALLY visible.
    // In that case we don't waste time processing it.
    //visible = true;
  //}
  else if (occludee->GetBox ().In (leaf->GetCenter ()))
    visible = true;
  else if ((adjacent_side = leaf->GetBox ().Adjacent (occludee->GetBox ())) != -1)
  {
#   if DO_PVS_ADJACENT_NODES
    // Two sides are adjacent.
    csBox2 leaf_side = leaf->GetBox ().GetSide (adjacent_side);
    csBox2 occludee_side = occludee->GetBox ().GetSide (
    	csBox3::OtherSide (adjacent_side));
    // If the leaf box completely overlaps with the occludee box
    // (i.e. the occludee is smaller) then we need to test if
    // that leaf side is solid to have no visibility.
    if (leaf->GetSolidMask (adjacent_side) == (UShort)~0 &&
    	leaf_side.Contains (occludee_side))
    {
      printf ("!");
      visible = false;
    }
    else visible = true;
#   else
    visible = true;
#   endif // DO_PVS_ADJACENT_NODES
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
#   if DO_PVS_QAD
    if (ovis->IsReallyVisible ())
    {
      // @@@ ERROR!!!
      printf ("E");
      BoxCanSeeOccludeeSuperSlow (leaf->GetBox (), occludee->GetBox ());
    }
#   endif // DO_PVS_QAD
    total_cull_node += 1+occludee->CountChildren ();
    total_total_cull_node += 1+occludee->CountChildren ();
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
    total_cull_node = 0;
    BuildPVSForLeaf ((csOctreeNode*)root, thing, node);
    if (total_cull_node)
      printf (" %d:-%d ", count_leaves, total_cull_node);
    else
      printf (".");
    fflush (stdout);
    count_leaves++;
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

# if DO_PVS_QAD
  // First build a Quick And Dirty PVS which is going to be used to
  // optimize the real PVS building.
  CsPrintf (MSG_INITIALIZATION, "  QAD Pass 0...\n");
  total_total_cull_node = 0;
  count_leaves = 0;
  BuildQADPVS ((csOctreeNode*)root);
  printf ("\nTotal culled nodes=%d\n", total_total_cull_node);
# endif // DO_PVS_QAD

# if DO_PVS_PASS1
  // Then build the PVS for real which will remove nodes from the
  // PVS (hopefully).
  CsPrintf (MSG_INITIALIZATION, "  Pass 1...\n");
  total_total_cull_node = 0;
  total_total_solid_opt = 0;
  count_leaves = 0;
  pvs_solid_boundaries_only = true;
  BuildPVS (thing, (csOctreeNode*)root);
  printf ("\nTotal culled nodes=%d, total opt=%d\n", total_total_cull_node,
  	total_total_solid_opt);
# endif // DO_PVS_PASS1

# if DO_PVS_SOLID_NODE_OPT | DO_PVS_POLYGONS
  // Now we do the second pass. The second pass does the full PVS.
  CsPrintf (MSG_INITIALIZATION, "  Pass 2...\n");
  total_total_cull_node = 0;
  total_total_solid_opt = 0;
  count_leaves = 0;
  pvs_solid_boundaries_only = false;
  BuildPVS (thing, (csOctreeNode*)root);
  printf ("\nTotal culled nodes=%d, total opt=%d\n", total_total_cull_node,
  	total_total_solid_opt);
# endif

  delete box_clipper;
}

static csOctreeNode* next_visible = NULL;

static void node_pvs_func (csOctreeNode* node, csFrustumView* /*lview*/)
{
  // When we discover a node we only mark it visible if we will
  // hit a polygon in this node.
  next_visible = node;
}

static void poly_pvs_func (csObject* obj, csFrustumView* lview)
{
  if (next_visible)
  {
    csPolygon3D* p = (csPolygon3D*)obj;
    if (next_visible->GetBox ().In (p->Vwor (0)) ||
        next_visible->GetBox ().In (p->Vwor (1)))
    {
      csOctreeNode* leaf = (csOctreeNode*)(lview->userdata);
      csPVS& pvs = leaf->GetPVS ();
      csOctreeVisible* ovis = pvs.FindNode (next_visible);
      ovis->MarkReallyVisible ();
    }
    next_visible = NULL;
  }
}

static void curve_pvs_func (csObject*, csFrustumView*)
{
}

// If a child is really visible then its parent is really visible too.
// This function makes sure that all parents with visible children are
// also marked visible.
static bool MarkParentsVisible (csPVS& pvs, csOctreeNode* node)
{
  if (!node) return false;
  csOctreeVisible* ovis = pvs.FindNode (node);
  if (ovis->IsReallyVisible ()) return true;
  int i;
  bool rc = false;
  for (i = 0 ; i < 8 ; i++)
    rc = rc || MarkParentsVisible (pvs, node->GetChild (i));
  if (rc) ovis->MarkReallyVisible ();
  return rc;
}

void csOctree::BuildQADPVS (csOctreeNode* node)
{
  if (!node) return;

  if (node->IsLeaf ())
  {
    // We have a leaf, here we start our QAD pvs building.
    csPVS& pvs = node->GetPVS ();

    csFrustumView lview;
    lview.userdata = (void*)node;
    lview.node_func = node_pvs_func;
    lview.poly_func = poly_pvs_func;
    lview.curve_func = curve_pvs_func;
    lview.radius = 1000000000.;
    lview.sq_radius = 1000000000.;
    lview.things_shadow = false;
    lview.mirror = false;
    lview.dynamic = false;
    csVector3 cen = node->GetCenter ();
    lview.light_frustum = new csFrustum (cen);
    lview.light_frustum->MakeInfinite ();
    sector->CheckFrustum (lview);

    int i;
    for (i = 0 ; i < 8 ; i++)
    {
      csVector3 cor = (node->GetBox ().GetCorner (i)+cen)/2;
      delete lview.light_frustum;
      lview.light_frustum = new csFrustum (cor);
      lview.light_frustum->MakeInfinite ();
      sector->CheckFrustum (lview);
    }

    MarkParentsVisible (pvs, (csOctreeNode*)root);

    total_cull_node = 0;
    csOctreeVisible* ovis = pvs.GetFirst ();
    while (ovis)
    {
      if (!ovis->IsReallyVisible ()) total_cull_node++;
      ovis = pvs.GetNext (ovis);
    }
    total_total_cull_node += total_cull_node;
    if (total_cull_node)
      printf (" %d:-%d ", count_leaves, total_cull_node);
    else
      printf (".");
    fflush (stdout);
    count_leaves++;
  }
  else
  {
    // Traverse to the children.
    int i;
    for (i = 0 ; i < 8 ; i++)
      BuildQADPVS ((csOctreeNode*)node->children[i]);
  }
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
    //if (rc == 1)
    //{
      //if (!ClassifyPoint (poly.GetCenter ())) rc = 0;
    //}
    return rc;
  }
  if (node->IsLeaf ())
  {
    if (ClassifyPoint (poly.GetCenter ())) return 1;
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
#if DO_PVS_SOLID_SPACE_OPT
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
      v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4.;
      v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4.;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + (x+1)*(cor_XY.x-cor_xy.x)/4.;
      v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4.;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + (x+1)*(cor_XY.x-cor_xy.x)/4.;
      v.y = cor_xy.y + (y+1)*(cor_XY.y-cor_xy.y)/4.;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4.;
      v.y = cor_xy.y + (y+1)*(cor_XY.y-cor_xy.y)/4.;
      poly.AddVertex (GetVector3 (plane_nr, plane_pos, v));
      rc = ClassifyPolygon (poly);
      if (rc == 1) result |= 1<<bitnr;
      bitnr++;
    }
  return result;
#else
  (void)plane_nr;
  (void)plane_pos;
  (void)box;
  return 0;
#endif // DO_PVS_SOLID_SPACE_OPT
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

  // When classifying the sides of a node we slightly shift the rectangle
  // that we use to test solid space with. This is to avoid hits with
  // polygons that are exactly on the node boundary but actually belong
  // to the neighbour node.
  // @@@ Maybe there are better solutions for this?

  node->solid_masks[BOX_SIDE_x] = ClassifyRectangle (PLANE_X,
  	node->GetBox ().MinX ()+.01, GetSideBox (BOX_SIDE_x, node->GetBox ()));
  node->solid_masks[BOX_SIDE_X] = ClassifyRectangle (PLANE_X,
  	node->GetBox ().MaxX ()-.01, GetSideBox (BOX_SIDE_X, node->GetBox ()));
  node->solid_masks[BOX_SIDE_y] = ClassifyRectangle (PLANE_Y,
  	node->GetBox ().MinY ()+.01, GetSideBox (BOX_SIDE_y, node->GetBox ()));
  node->solid_masks[BOX_SIDE_Y] = ClassifyRectangle (PLANE_Y,
  	node->GetBox ().MaxY ()-.01, GetSideBox (BOX_SIDE_Y, node->GetBox ()));
  node->solid_masks[BOX_SIDE_z] = ClassifyRectangle (PLANE_Z,
  	node->GetBox ().MinZ ()+.01, GetSideBox (BOX_SIDE_z, node->GetBox ()));
  node->solid_masks[BOX_SIDE_Z] = ClassifyRectangle (PLANE_Z,
  	node->GetBox ().MaxZ ()-.01, GetSideBox (BOX_SIDE_Z, node->GetBox ()));
  int i;
  for (i = 0 ; i < 8 ; i++)
    CalculateSolidMasks ((csOctreeNode*)node->children[i]);
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
