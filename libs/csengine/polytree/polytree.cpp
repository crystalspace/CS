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
  if (ps->next_bsp) ps->next_bsp->prev_bsp = ps->prev_bsp;
  if (ps->prev_bsp) ps->prev_bsp->next_bsp = ps->next_bsp;
  else
  {
    if (first_stub == ps) first_stub = ps->next_bsp;
    else if (todo_stubs == ps) todo_stubs = ps->next_bsp;
    else
      printf ("INTERNAL ERROR! Stub not in todo or stub list!\n");
  }
  ps->prev_bsp = ps->next_bsp = NULL;
  ps->node = NULL;
}

void csPolygonTreeNode::LinkStubTodo (csPolygonStub* ps)
{
  ps->next_bsp = todo_stubs;
  ps->prev_bsp = NULL;
  if (todo_stubs) todo_stubs->prev_bsp = ps;
  todo_stubs = ps;
}

void csPolygonTreeNode::LinkStub (csPolygonStub* ps)
{
  ps->next_bsp = first_stub;
  ps->prev_bsp = NULL;
  if (first_stub) first_stub->prev_bsp = ps;
  first_stub = ps;
}

void csPolygonTree::AddObject (csPolyTreeObject* obj)
{
  csPolygonStub* stub = obj->GetBaseStub ();
  root->LinkStubTodo (stub);
}

