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
#include "csengine/bspbbox.h"
#include "csengine/engine.h"
#include "cssys/csendian.h"
#include "iutil/vfs.h"

csPolygonTreeNode::~csPolygonTreeNode ()
{
  while (first_stub || todo_stubs)
  {
    if (first_stub)
      csPolyTreeBBox::GetPolyStubPool()->Free (first_stub);
    else
      csPolyTreeBBox::GetPolyStubPool()->Free (todo_stubs);
  }
}

void csPolygonTreeNode::UnlinkStub (csPolygonStub *ps)
{
  if (!ps->node) return ;
  if (ps->next_tree) ps->next_tree->prev_tree = ps->prev_tree;
  if (ps->prev_tree)
    ps->prev_tree->next_tree = ps->next_tree;
  else
  {
    if (first_stub == ps)
      first_stub = ps->next_tree;
    else if (todo_stubs == ps)
      todo_stubs = ps->next_tree;
    else
      csEngine::current_engine->Warn ("INTERNAL ERROR! Stub not in todo or stub list!");
  }

  ps->prev_tree = ps->next_tree = NULL;
  ps->node = NULL;
}

void csPolygonTreeNode::LinkStubTodo (csPolygonStub *ps)
{
  if (ps->node) return ;
  ps->next_tree = todo_stubs;
  ps->prev_tree = NULL;
  if (todo_stubs) todo_stubs->prev_tree = ps;
  todo_stubs = ps;
  ps->node = this;
}

void csPolygonTreeNode::LinkStub (csPolygonStub *ps)
{
  if (ps->node) return ;
  ps->next_tree = first_stub;
  ps->prev_tree = NULL;
  if (first_stub) first_stub->prev_tree = ps;
  first_stub = ps;
  ps->node = this;
}

void *csPolygonTreeNode::TraverseObjects (
  csThing *thing,
  const csVector3 &

  /*pos*/,
  csTreeVisitFunc *func,
  void *data)
{
  csPolygonStub *stub;
  void *rc;
  stub = first_stub;
  while (stub)
  {
    rc = stub->Visit (thing, func, data);
    if (rc) return rc;
    stub = stub->next_tree;
  }

  return NULL;
}

void csPolygonTree::AddObject (csPolyTreeBBox *obj)
{
  csPolygonStub *stub = obj->GetBaseStub ();
  root->LinkStubTodo (stub);
  obj->LinkStub (stub);
  stub->IncRef ();
}

bool csPolygonTree::Overlaps (csPolygonInt **polygons, int num)
{
  // Don't compute this if more than six vertices (overhead).
  // Return true in this case so that the tree build routine assumes
  // that we'll have non-convex polygons.
  if (num > 20) return true;

  int i, j;
  for (i = 0; i < num; i++)
    for (j = 0; j < num; j++)
      if (i != j)
      {
        if (polygons[i]->Overlaps (polygons[j])) return true;
      }

  // None of the polygons covers the other.
  return false;
}

void csPolygonTree::WriteString (iFile *cf, char *str, int len)
{
  cf->Write (str, len);
}

void csPolygonTree::WriteBox3 (iFile *cf, const csBox3 &box)
{
  float f;
  f = convert_endian (box.MinX ());
  cf->Write ((char *) &f, 4);
  f = convert_endian (box.MinY ());
  cf->Write ((char *) &f, 4);
  f = convert_endian (box.MinZ ());
  cf->Write ((char *) &f, 4);
  f = convert_endian (box.MaxX ());
  cf->Write ((char *) &f, 4);
  f = convert_endian (box.MaxY ());
  cf->Write ((char *) &f, 4);
  f = convert_endian (box.MaxZ ());
  cf->Write ((char *) &f, 4);
}

void csPolygonTree::WriteVector3 (iFile *cf, const csVector3 &v)
{
  float f;
  f = convert_endian (v.x);
  cf->Write ((char *) &f, 4);
  f = convert_endian (v.y);
  cf->Write ((char *) &f, 4);
  f = convert_endian (v.z);
  cf->Write ((char *) &f, 4);
}

void csPolygonTree::WritePlane3 (iFile *cf, const csPlane3 &v)
{
  float f;
  f = convert_endian (v.A ());
  cf->Write ((char *) &f, 4);
  f = convert_endian (v.B ());
  cf->Write ((char *) &f, 4);
  f = convert_endian (v.C ());
  cf->Write ((char *) &f, 4);
  f = convert_endian (v.D ());
  cf->Write ((char *) &f, 4);
}

void csPolygonTree::WriteLong (iFile *cf, long l)
{
  l = convert_endian (l);
  cf->Write ((char *) &l, 4);
}

void csPolygonTree::WriteUShort (iFile *cf, uint16 l)
{
  l = convert_endian (l);
  cf->Write ((char *) &l, 2);
}

void csPolygonTree::WriteByte (iFile *cf, unsigned char b)
{
  cf->Write ((char *) &b, 1);
}

void csPolygonTree::WriteBool (iFile *cf, bool b)
{
  char c = (char)b;
  cf->Write (&c, 1);
}

void csPolygonTree::ReadString (iFile *cf, char *str, int len)
{
  str[0] = '\0';
  cf->Read (str, len);
}

void csPolygonTree::ReadBox3 (iFile *cf, csBox3 &box)
{
  float f = 0;
  csVector3 bmin, bmax;
  if (cf->Read ((char *) &f, 4)) bmin.x = convert_endian (f);
  if (cf->Read ((char *) &f, 4)) bmin.y = convert_endian (f);
  if (cf->Read ((char *) &f, 4)) bmin.z = convert_endian (f);
  if (cf->Read ((char *) &f, 4)) bmax.x = convert_endian (f);
  if (cf->Read ((char *) &f, 4)) bmax.y = convert_endian (f);
  if (cf->Read ((char *) &f, 4)) bmax.z = convert_endian (f);
  box.Set (bmin, bmax);
}

void csPolygonTree::ReadVector3 (iFile *cf, csVector3 &v)
{
  float f = 0;
  if (cf->Read ((char *) &f, 4)) v.x = convert_endian (f);
  if (cf->Read ((char *) &f, 4)) v.y = convert_endian (f);
  if (cf->Read ((char *) &f, 4)) v.z = convert_endian (f);
}

void csPolygonTree::ReadPlane3 (iFile *cf, csPlane3 &v)
{
  float f = 0;
  if (cf->Read ((char *) &f, 4)) v.A () = convert_endian (f);
  if (cf->Read ((char *) &f, 4)) v.B () = convert_endian (f);
  if (cf->Read ((char *) &f, 4)) v.C () = convert_endian (f);
  if (cf->Read ((char *) &f, 4)) v.D () = convert_endian (f);
}

long csPolygonTree::ReadLong (iFile *cf)
{
  long l = 0;
  cf->Read ((char *) &l, 4);
  return convert_endian (l);
}

uint16 csPolygonTree::ReadUShort (iFile *cf)
{
  uint16 l = 0;
  cf->Read ((char *) &l, 2);
  return convert_endian (l);
}

unsigned char csPolygonTree::ReadByte (iFile *cf)
{
  unsigned char b = 0;
  cf->Read ((char *) &b, 1);
  return b;
}

bool csPolygonTree::ReadBool (iFile *cf)
{
  char c = 0;
  cf->Read ((char *) &c, 1);
  return (bool) c;
}
