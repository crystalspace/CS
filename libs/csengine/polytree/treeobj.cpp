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
#include "csengine/treeobj.h"
#include "csengine/polytree.h"

csPolygonStubPool csPolyTreeObject::stub_pool;

csPolyTreeObject::csPolyTreeObject (csObject* owner)
{
  this->owner = owner;
  first_stub = NULL;
}

csPolyTreeObject::~csPolyTreeObject ()
{
  RemoveFromTree ();
}

void csPolyTreeObject::RemoveFromTree ()
{
  while (first_stub)
    stub_pool.Free (first_stub);
}

void csPolyTreeObject::UnlinkStub (csPolygonStub* ps)
{
  if (ps->next_obj) ps->next_obj->prev_obj = ps->prev_obj;
  if (ps->prev_obj) ps->prev_obj->next_obj = ps->next_obj;
  else first_stub = ps->next_obj;
  ps->prev_obj = ps->next_obj = NULL;
  ps->object = NULL;
}

//------------------------------------------------------------------------

csPolygonStub::~csPolygonStub ()
{
  RemoveStub ();
}

void csPolygonStub::RemoveStub ()
{
  if (object) object->UnlinkStub (this);
  if (node) node->UnlinkStub (this);
}


