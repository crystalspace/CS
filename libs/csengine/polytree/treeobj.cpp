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
#include "csengine/engine.h"
#include "csengine/treeobj.h"
#include "csengine/bspbbox.h"
#include "csengine/polytree.h"

//------------------------------------------------------------------------
void csPolygonStub::RemoveData ()
{
  int i;
  for (i = 0; i < GetPolygonCount (); i++)
    poly_pool->Free (GetPolygons ()[i]);
}

void csPolygonStub::RemoveStub ()
{
  if (object)
  {
    object->UnlinkStub (this);
    object = NULL;
  }

  if (node)
  {
    node->UnlinkStub (this);
    node = NULL;
  }
}

//------------------------------------------------------------------------
csPolygonStubPool::~csPolygonStubPool ()
{
  while (alloced)
  {
    PoolObj *n = alloced->next;

    //delete alloced->ps; @@@ This free is not valid!

    // We should use a ref count on the pool itself so that we

    // now when all objects in the pool are freed and the

    // 'alloced' list will be empty.
    delete alloced;
    alloced = n;
  }

  while (freed)
  {
    PoolObj *n = freed->next;
    delete freed->ps;
    delete freed;
    freed = n;
  }
}

csPolygonStub *csPolygonStubPool::Alloc (csPolygonStubFactory *factory)
{
  PoolObj *pnew;
  if (freed)
  {
    pnew = freed;
    freed = freed->next;
  }
  else
  {
    pnew = new PoolObj ();
    pnew->ps = factory->Create ();
  }

  pnew->next = alloced;
  alloced = pnew;
  pnew->ps->Initialize ();
  pnew->ps->ref_count = 1;
  pnew->ps->object = NULL;
  pnew->ps->node = NULL;
  return pnew->ps;
}

void csPolygonStubPool::Free (csPolygonStub *ps)
{
  ps->RemoveStub ();
  ps->DecRef ();
  if (ps->ref_count > 0) return ;
  ps->RemoveData ();
  if (alloced)
  {
    PoolObj *po = alloced;
    alloced = alloced->next;
    po->ps = ps;
    po->next = freed;
    freed = po;
  }
  else
  {
    // Cannot happen!
  }
}

void csPolygonStubPool::Dump ()
{
  int cnt, freecnt;
  cnt = 0;

  PoolObj *po = alloced;
  while (po)
  {
    cnt++;
    po = po->next;
  }

  freecnt = 0;
  po = freed;
  while (po)
  {
    freecnt++;
    po = po->next;
  }

  csEngine::current_engine->Report ("ObjStub pool: %d allocated, %d freed.", cnt, freecnt);
}

//------------------------------------------------------------------------
