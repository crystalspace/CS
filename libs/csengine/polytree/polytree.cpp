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
#include "csengine/polytree.h"
#include "csengine/polygon.h"
#include "csengine/treeobj.h"
#include "csengine/engine.h"
#include "cssys/csendian.h"
#include "ivfs.h"
#include "isystem.h"

csPolygonTreeNode::~csPolygonTreeNode ()
{
  while (first_stub || todo_stubs)
    if (first_stub)
      csPolyTreeObject::stub_pool.Free (first_stub);
    else
      csPolyTreeObject::stub_pool.Free (todo_stubs);
}

void csPolygonTreeNode::UnlinkStub (csObjectStub* ps)
{
  if (!ps->node) return;
  if (ps->next_tree) ps->next_tree->prev_tree = ps->prev_tree;
  if (ps->prev_tree) ps->prev_tree->next_tree = ps->next_tree;
  else
  {
    if (first_stub == ps) first_stub = ps->next_tree;
    else if (todo_stubs == ps) todo_stubs = ps->next_tree;
    else
      printf ("INTERNAL ERROR! Stub not in todo or stub list!\n");
  }
  ps->prev_tree = ps->next_tree = NULL;
  ps->node = NULL;
}

void csPolygonTreeNode::LinkStubTodo (csObjectStub* ps)
{
  if (ps->node) return;
  ps->next_tree = todo_stubs;
  ps->prev_tree = NULL;
  if (todo_stubs) todo_stubs->prev_tree = ps;
  todo_stubs = ps;
  ps->node = this;
}

void csPolygonTreeNode::LinkStub (csObjectStub* ps)
{
  if (ps->node) return;
  ps->next_tree = first_stub;
  ps->prev_tree = NULL;
  if (first_stub) first_stub->prev_tree = ps;
  first_stub = ps;
  ps->node = this;
}

void* csPolygonTreeNode::TraverseObjects (csSector* sector, 
	const csVector3& /*pos*/, csTreeVisitFunc* func, void* data)
{
  csObjectStub* stub;
  void* rc;
  stub = first_stub;
  while (stub)
  {
    rc = stub->Visit (sector, func, data);
    if (rc) return rc;
    stub = stub->next_tree;
  }
  return NULL;
}

void csPolygonTree::AddObject (csPolyTreeObject* obj)
{
  csObjectStub* stub = obj->GetBaseStub ();
  root->LinkStubTodo (stub);
  obj->LinkStub (stub);
  stub->IncRef ();
}

bool csPolygonTree::Overlaps (csPolygonInt** polygons, int num)
{
  // Don't compute this if more than six vertices (overhead).
  // Return true in this case so that the tree build routine assumes
  // that we'll have non-convex polygons.
  if (num > 20) return true;
  int i, j;
  for (i = 0 ; i < num ; i++)
    for (j = 0 ; j < num ; j++)
      if (i != j)
      {
        if (polygons[i]->Overlaps (polygons[j])) return true;
      }

  // None of the polygons covers the other.
  return false;
}

void csPolygonTree::WriteString (iFile* cf, char* str, int len)
{
  cf->Write (str, len);
}

void csPolygonTree::WriteBox3 (iFile* cf, const csBox3& box)
{
  float f;
  f = convert_endian (box.MinX ()); cf->Write ((char*)&f, 4);
  f = convert_endian (box.MinY ()); cf->Write ((char*)&f, 4);
  f = convert_endian (box.MinZ ()); cf->Write ((char*)&f, 4);
  f = convert_endian (box.MaxX ()); cf->Write ((char*)&f, 4);
  f = convert_endian (box.MaxY ()); cf->Write ((char*)&f, 4);
  f = convert_endian (box.MaxZ ()); cf->Write ((char*)&f, 4);
}

void csPolygonTree::WriteVector3 (iFile* cf, const csVector3& v)
{
  float f;
  f = convert_endian (v.x); cf->Write ((char*)&f, 4);
  f = convert_endian (v.y); cf->Write ((char*)&f, 4);
  f = convert_endian (v.z); cf->Write ((char*)&f, 4);
}

void csPolygonTree::WritePlane3 (iFile* cf, const csPlane3& v)
{
  float f;
  f = convert_endian (v.A ()); cf->Write ((char*)&f, 4);
  f = convert_endian (v.B ()); cf->Write ((char*)&f, 4);
  f = convert_endian (v.C ()); cf->Write ((char*)&f, 4);
  f = convert_endian (v.D ()); cf->Write ((char*)&f, 4);
}

void csPolygonTree::WriteLong (iFile* cf, long l)
{
  l = convert_endian (l); cf->Write ((char*)&l, 4);
}

void csPolygonTree::WriteUShort (iFile* cf, UShort l)
{
  l = convert_endian (l); cf->Write ((char*)&l, 2);
}

void csPolygonTree::WriteByte (iFile* cf, unsigned char b)
{
  cf->Write ((char*)&b, 1);
}

void csPolygonTree::WriteBool (iFile* cf, bool b)
{
  char c = (char)b;
  cf->Write (&c, 1);
}

void csPolygonTree::ReadString (iFile* cf, char* str, int len)
{
  str[0] = '\0';
  cf->Read (str, len);
}

void csPolygonTree::ReadBox3 (iFile* cf, csBox3& box)
{
  float f = 0;
  csVector3 bmin, bmax;
  if (cf->Read ((char*)&f, 4)) bmin.x = convert_endian (f);
  if (cf->Read ((char*)&f, 4)) bmin.y = convert_endian (f);
  if (cf->Read ((char*)&f, 4)) bmin.z = convert_endian (f);
  if (cf->Read ((char*)&f, 4)) bmax.x = convert_endian (f);
  if (cf->Read ((char*)&f, 4)) bmax.y = convert_endian (f);
  if (cf->Read ((char*)&f, 4)) bmax.z = convert_endian (f);
  box.Set (bmin, bmax);
}

void csPolygonTree::ReadVector3 (iFile* cf, csVector3& v)
{
  float f = 0;
  if (cf->Read ((char*)&f, 4)) v.x = convert_endian (f);
  if (cf->Read ((char*)&f, 4)) v.y = convert_endian (f);
  if (cf->Read ((char*)&f, 4)) v.z = convert_endian (f);
}

void csPolygonTree::ReadPlane3 (iFile* cf, csPlane3& v)
{
  float f = 0;
  if (cf->Read ((char*)&f, 4)) v.A () = convert_endian (f);
  if (cf->Read ((char*)&f, 4)) v.B () = convert_endian (f);
  if (cf->Read ((char*)&f, 4)) v.C () = convert_endian (f);
  if (cf->Read ((char*)&f, 4)) v.D () = convert_endian (f);
}

long csPolygonTree::ReadLong (iFile* cf)
{
  long l = 0;
  cf->Read ((char*)&l, 4);
  return convert_endian (l);
}

UShort csPolygonTree::ReadUShort (iFile* cf)
{
  UShort l = 0;
  cf->Read ((char*)&l, 2);
  return convert_endian (l);
}

unsigned char csPolygonTree::ReadByte (iFile* cf)
{
  unsigned char b = 0;
  cf->Read ((char*)&b, 1);
  return b;
}

bool csPolygonTree::ReadBool (iFile* cf)
{
  char c = 0;
  cf->Read ((char*)&c, 1);
  return (bool)c;
}

struct CPTraverseData
{
  int cnt_solid;
  int wanted_solid;
  csVector3 pos;
  csVector3 test_points[6];
  bool polygon_was_hit[6];
  int num_polygon_was_hit;
  csPolygon3D* touched_polygon;
};

struct CPSortPolDist
{
  csPolygon3D* pol;
  float sqdist;
};

static int CPSortDistance (const void* el1, const void* el2)
{
  CPSortPolDist* po1 = (CPSortPolDist*)el1;
  CPSortPolDist* po2 = (CPSortPolDist*)el2;
  if (po1->sqdist < po2->sqdist) return -1;
  else if (po1->sqdist > po2->sqdist) return 1;
  else return 0;
}

static void* ClassifyPointTraverse (csSector*, csPolygonInt** polygons,
	int num, bool same_plane, void* vdata)
{
  int i, j;

  // Only for csPolygon3D.
  if (polygons[0]->GetType () != 1) return NULL;

  CPTraverseData* data = (CPTraverseData*)vdata;

  // First put all polygons in a list which will be sorted
  // by distance (closest polygon first). If 'same_plane'
  // is true then this sorting is not needed.
  CPSortPolDist* sorted_polygons = new CPSortPolDist[num];
  for (i = 0 ; i < num ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)polygons[i];
    // If our point to test solid space is on the polygon then we
    // can stop and return solid already.
    if (p->PointOnPolygon (data->pos))
    {
      //@@@ THINK ABOUT THIS SITUATION!
      // For dmburg this seems to make no difference at all.
      //data->cnt_solid = 6;		// End this now.
      //data->touched_polygon = p;	// Remember this polygon.
      //return (void*)1;
      sorted_polygons[i].pol = NULL;
    }
    else sorted_polygons[i].pol = p;
  }

  // Now try all six directions that haven't been tried earlier.
  for (j = 0 ; j < 6 ; j++)
    if (!data->polygon_was_hit[j])
    {
      if (!same_plane)
      {
        /* initialize this vector to something to avoid compiler warnings */
        csVector3 isect(0);
        for (i = 0 ; i < num ; i++)
	{
          csPolygon3D* p = sorted_polygons[i].pol;
	  bool is = p ? p->IntersectRayPlane (data->pos, data->test_points[j],
	  	isect) : false;
	  if (is) sorted_polygons[i].sqdist =
	    	csSquaredDist::PointPoint (data->pos, isect);
	  else sorted_polygons[i].sqdist = 1000000000.;
	}
	qsort ((void*)sorted_polygons, num, sizeof (CPSortPolDist),
		CPSortDistance);
      }

      for (i = 0 ; i < num ; i++)
      {
        csPolygon3D* p = sorted_polygons[i].pol;
        bool is = p ? p->IntersectRayNoBackFace (data->pos, data->test_points[j]) : false;
        if (is)
        {
          data->polygon_was_hit[j] = true;
	  data->num_polygon_was_hit++;
	  // @@@ This is not efficient. We should only do BF culling here.
          if (!p->IntersectRay (data->pos, data->test_points[j]))
	  {
	    // We can not see the polygon from 'pos'. So we could be in solid
	    // space.
	    data->cnt_solid++;
	    if (data->cnt_solid >= data->wanted_solid)
	    {
	      delete [] sorted_polygons;
	      return (void*)1;
	    }
	  }
	  // We tested all points.
	  if (data->num_polygon_was_hit >= 6)
	  {
	    delete [] sorted_polygons;
	    return (void*)1;
	  }
	  // Otherwise don't test any other polygons from this node.
	  break;
        }
      }
    }
  delete [] sorted_polygons;
  return NULL;
}

bool csPolygonTree::ClassifyPoint (const csVector3& p)
{
  CPTraverseData data;
  data.cnt_solid = 0;
  // This is a configuration parameter, currently set to 2. This
  // means that we want at least two directions around the point
  // to be returned as solid before we consider the point solid.
  data.wanted_solid = 2;
  data.pos = p;
  data.test_points[0] = p+csVector3 (1, 0, 0);
  data.test_points[1] = p+csVector3 (-1, 0, 0);
  data.test_points[2] = p+csVector3 (0, 1, 0);
  data.test_points[3] = p+csVector3 (0, -1, 0);
  data.test_points[4] = p+csVector3 (0, 0, 1);
  data.test_points[5] = p+csVector3 (0, 0, -1);
  data.touched_polygon = NULL;
  int i;
  for (i = 0 ; i < 6 ; i++) data.polygon_was_hit[i] = false;
  data.num_polygon_was_hit = 0;
  Front2Back (p, ClassifyPointTraverse, (void*)&data, NULL, NULL);
  return data.cnt_solid >= data.wanted_solid;
}
