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

#include "sysdef.h"
#include "csengine/polytree.h"
#include "csengine/treeobj.h"

csPolygonTreeNode::~csPolygonTreeNode ()
{
  while (first_stub || todo_stubs)
    if (first_stub)
      csPolyTreeObject::stub_pool.Free (first_stub);
    else
      csPolyTreeObject::stub_pool.Free (todo_stubs);
}

void csPolygonTreeNode::UnlinkStub (csPolygonStub* ps)
{
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

void csPolygonTreeNode::LinkStubTodo (csPolygonStub* ps)
{
  ps->next_tree = todo_stubs;
  ps->prev_tree = NULL;
  if (todo_stubs) todo_stubs->prev_tree = ps;
  todo_stubs = ps;
}

void csPolygonTreeNode::LinkStub (csPolygonStub* ps)
{
  ps->next_tree = first_stub;
  ps->prev_tree = NULL;
  if (first_stub) first_stub->prev_tree = ps;
  first_stub = ps;
}

void* csPolygonTreeNode::TraverseObjects (csSector* sector, 
	const csVector3& /*pos*/, csTreeVisitFunc* func, void* data)
{
  csPolygonStub* stub;
  void* rc;
  stub = first_stub;
  while (stub)
  {
    rc = func (sector, stub->GetPolygons (), stub->GetNumPolygons (), data);
    if (rc) return rc;
    stub = stub->next_tree;
  }
  return NULL;
}

void csPolygonTree::AddObject (csPolyTreeObject* obj)
{
  csPolygonStub* stub = obj->GetBaseStub ();
  root->LinkStubTodo (stub);
}

