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
#include "csengine/treeobj.h"
#include "csengine/polytree.h"

csObjectStubPool csPolyTreeObject::stub_pool;

csPolyTreeObject::csPolyTreeObject (csObjectStubFactory* factory)
{
  first_stub = NULL;
  stub_factory = factory;
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

void csPolyTreeObject::UnlinkStub (csObjectStub* ps)
{
  if (!ps->object) return;
  if (ps->next_obj) ps->next_obj->prev_obj = ps->prev_obj;
  if (ps->prev_obj) ps->prev_obj->next_obj = ps->next_obj;
  else first_stub = ps->next_obj;
  ps->prev_obj = ps->next_obj = NULL;
  ps->object = NULL;
}

void csPolyTreeObject::LinkStub (csObjectStub* ps)
{
  if (ps->object) return;
  ps->next_obj = first_stub;
  ps->prev_obj = NULL;
  if (first_stub) first_stub->prev_obj = ps;
  first_stub = ps;
  ps->object = this;
}

//------------------------------------------------------------------------

void csPolygonStub::RemoveData ()
{
  int i;
  for (i = 0 ; i < GetNumPolygons () ; i++)
    poly_pool->Free (GetPolygons ()[i]);
}

//------------------------------------------------------------------------

void csDetailedPolyTreeObject::SplitWithPlane (csObjectStub* stub,
  	csObjectStub** p_stub_on, csObjectStub** p_stub_front,
	csObjectStub** p_stub_back,
	const csPlane3& plane)
{
  csPolygonStub* stub_on, * stub_front, * stub_back;
  stub_front = (csPolygonStub*)stub_pool.Alloc (stub_factory);
  LinkStub (stub_front);
  stub_back = (csPolygonStub*)stub_pool.Alloc (stub_factory);
  LinkStub (stub_back);
  if (p_stub_on)
  {
    stub_on = (csPolygonStub*)stub_pool.Alloc (stub_factory);
    LinkStub (stub_on);
  }
  else stub_on = stub_front;

  // Fill the stubs with the needed polygons.
  int i;
  csPolygonInt** polygons = ((csPolygonStub*)stub)->GetPolygons ();
  for (i = 0 ; i < ((csPolygonStub*)stub)->GetNumPolygons () ; i++)
  {
    int c = polygons[i]->Classify (plane);
    switch (c)
    {
      case POL_SAME_PLANE:
        stub_on->GetPolygonArray ().AddPolygon (polygons[i]);
	polygons[i]->IncRefCount ();
	break;
      case POL_FRONT:
        stub_front->GetPolygonArray ().AddPolygon (polygons[i]);
	polygons[i]->IncRefCount ();
	break;
      case POL_BACK:
        stub_back->GetPolygonArray ().AddPolygon (polygons[i]);
	polygons[i]->IncRefCount ();
	break;
      case POL_SPLIT_NEEDED:
	{
	  csPolygonInt* np1, * np2;
	  polygons[i]->SplitWithPlane (&np1, &np2, plane);
	  stub_front->GetPolygonArray ().AddPolygon (np1);
	  stub_back->GetPolygonArray ().AddPolygon (np2);
	}
	break;
    }
  }

  // If the stubs are empty (no polygons) then free them again.
  if (p_stub_on && stub_on->GetNumPolygons () == 0)
  {
    stub_pool.Free (stub_on);
    stub_on = NULL;
  }
  if (stub_front->GetNumPolygons () == 0)
  {
    stub_pool.Free (stub_front);
    stub_front = NULL;
  }
  if (stub_back->GetNumPolygons () == 0)
  {
    stub_pool.Free (stub_back);
    stub_back = NULL;
  }

  // Fill in the return pointers.
  if (p_stub_on) *p_stub_on = stub_on;
  *p_stub_front = stub_front;
  *p_stub_back = stub_back;

  stub_pool.Free (stub);
}

void csDetailedPolyTreeObject::SplitWithPlaneX (csObjectStub* stub,
  	csObjectStub** p_stub_on, csObjectStub** p_stub_front,
	csObjectStub** p_stub_back,
	float x)
{
  csPolygonStub* stub_front, * stub_back;
  stub_front = (csPolygonStub*)stub_pool.Alloc (stub_factory);
  LinkStub (stub_front);
  stub_back = (csPolygonStub*)stub_pool.Alloc (stub_factory);
  LinkStub (stub_back);
  if (p_stub_on) *p_stub_on = NULL;

  // Fill the stubs with the needed polygons.
  int i;
  csPolygonInt** polygons = ((csPolygonStub*)stub)->GetPolygons ();
  for (i = 0 ; i < ((csPolygonStub*)stub)->GetNumPolygons () ; i++)
  {
    int c = polygons[i]->ClassifyX (x);
    switch (c)
    {
      case POL_SAME_PLANE:
      case POL_FRONT:
        stub_front->GetPolygonArray ().AddPolygon (polygons[i]);
	polygons[i]->IncRefCount ();
	break;
      case POL_BACK:
        stub_back->GetPolygonArray ().AddPolygon (polygons[i]);
	polygons[i]->IncRefCount ();
	break;
      case POL_SPLIT_NEEDED:
	{
	  csPolygonInt* np1, * np2;
	  polygons[i]->SplitWithPlaneX (&np1, &np2, x);
	  stub_front->GetPolygonArray ().AddPolygon (np1);
	  stub_back->GetPolygonArray ().AddPolygon (np2);
	}
	break;
    }
  }

  // If the stubs are empty (no polygons) then free them again.
  if (stub_front->GetNumPolygons () == 0)
  {
    stub_pool.Free (stub_front);
    stub_front = NULL;
  }
  if (stub_back->GetNumPolygons () == 0)
  {
    stub_pool.Free (stub_back);
    stub_back = NULL;
  }

  // Fill in the return pointers.
  *p_stub_front = stub_front;
  *p_stub_back = stub_back;

  stub_pool.Free (stub);
}

void csDetailedPolyTreeObject::SplitWithPlaneY (csObjectStub* stub,
  	csObjectStub** p_stub_on, csObjectStub** p_stub_front,
	csObjectStub** p_stub_back,
	float y)
{
  csPolygonStub* stub_front, * stub_back;
  stub_front = (csPolygonStub*)stub_pool.Alloc (stub_factory);
  LinkStub (stub_front);
  stub_back = (csPolygonStub*)stub_pool.Alloc (stub_factory);
  LinkStub (stub_back);
  if (p_stub_on) *p_stub_on = NULL;

  // Fill the stubs with the needed polygons.
  int i;
  csPolygonInt** polygons = ((csPolygonStub*)stub)->GetPolygons ();
  for (i = 0 ; i < ((csPolygonStub*)stub)->GetNumPolygons () ; i++)
  {
    int c = polygons[i]->ClassifyY (y);
    switch (c)
    {
      case POL_SAME_PLANE:
      case POL_FRONT:
        stub_front->GetPolygonArray ().AddPolygon (polygons[i]);
	polygons[i]->IncRefCount ();
	break;
      case POL_BACK:
        stub_back->GetPolygonArray ().AddPolygon (polygons[i]);
	polygons[i]->IncRefCount ();
	break;
      case POL_SPLIT_NEEDED:
	{
	  csPolygonInt* np1, * np2;
	  polygons[i]->SplitWithPlaneY (&np1, &np2, y);
	  stub_front->GetPolygonArray ().AddPolygon (np1);
	  stub_back->GetPolygonArray ().AddPolygon (np2);
	}
	break;
    }
  }

  // If the stubs are empty (no polygons) then free them again.
  if (stub_front->GetNumPolygons () == 0)
  {
    stub_pool.Free (stub_front);
    stub_front = NULL;
  }
  if (stub_back->GetNumPolygons () == 0)
  {
    stub_pool.Free (stub_back);
    stub_back = NULL;
  }

  // Fill in the return pointers.
  *p_stub_front = stub_front;
  *p_stub_back = stub_back;

  stub_pool.Free (stub);
}

void csDetailedPolyTreeObject::SplitWithPlaneZ (csObjectStub* stub,
  	csObjectStub** p_stub_on, csObjectStub** p_stub_front,
	csObjectStub** p_stub_back,
	float z)
{
  csPolygonStub* stub_front, * stub_back;
  stub_front = (csPolygonStub*)stub_pool.Alloc (stub_factory);
  LinkStub (stub_front);
  stub_back = (csPolygonStub*)stub_pool.Alloc (stub_factory);
  LinkStub (stub_back);
  if (p_stub_on) *p_stub_on = NULL;

  // Fill the stubs with the needed polygons.
  int i;
  csPolygonInt** polygons = ((csPolygonStub*)stub)->GetPolygons ();
  for (i = 0 ; i < ((csPolygonStub*)stub)->GetNumPolygons () ; i++)
  {
    int c = polygons[i]->ClassifyZ (z);
    switch (c)
    {
      case POL_SAME_PLANE:
      case POL_FRONT:
        stub_front->GetPolygonArray ().AddPolygon (polygons[i]);
	polygons[i]->IncRefCount ();
	break;
      case POL_BACK:
        stub_back->GetPolygonArray ().AddPolygon (polygons[i]);
	polygons[i]->IncRefCount ();
	break;
      case POL_SPLIT_NEEDED:
	{
	  csPolygonInt* np1, * np2;
	  polygons[i]->SplitWithPlaneZ (&np1, &np2, z);
	  stub_front->GetPolygonArray ().AddPolygon (np1);
	  stub_back->GetPolygonArray ().AddPolygon (np2);
	}
	break;
    }
  }

  // If the stubs are empty (no polygons) then free them again.
  if (stub_front->GetNumPolygons () == 0)
  {
    stub_pool.Free (stub_front);
    stub_front = NULL;
  }
  if (stub_back->GetNumPolygons () == 0)
  {
    stub_pool.Free (stub_back);
    stub_back = NULL;
  }

  // Fill in the return pointers.
  *p_stub_front = stub_front;
  *p_stub_back = stub_back;

  stub_pool.Free (stub);
}


//------------------------------------------------------------------------

void csObjectStub::RemoveStub ()
{
  if (object) { object->UnlinkStub (this); object = NULL; }
  if (node) { node->UnlinkStub (this); node = NULL; }
}

//------------------------------------------------------------------------

csObjectStubPool::~csObjectStubPool ()
{
  while (alloced)
  {
    PoolObj* n = alloced->next;
    //delete alloced->ps; @@@ This free is not valid!
    // We should use a ref count on the pool itself so that we
    // now when all objects in the pool are freed and the
    // 'alloced' list will be empty.
    delete alloced;
    alloced = n;
  }
  while (freed)
  {
    PoolObj* n = freed->next;
    delete freed->ps;
    delete freed;
    freed = n;
  }
}

csObjectStub* csObjectStubPool::Alloc (csObjectStubFactory* factory)
{
  PoolObj* pnew;
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

void csObjectStubPool::Free (csObjectStub* ps)
{
  ps->RemoveStub ();
  ps->DecRef ();
  if (ps->ref_count > 0) return;
  ps->RemoveData ();
  if (alloced)
  {
    PoolObj* po = alloced;
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

void csObjectStubPool::Dump ()
{
  int cnt;
  cnt = 0;
  PoolObj* po = alloced;
  while (po) { cnt++; po = po->next; }
  printf ("ObjStub pool: %d allocated, ", cnt);
  cnt = 0;
  po = freed;
  while (po) { cnt++; po = po->next; }
  printf ("%d freed.\n", cnt);
}

//------------------------------------------------------------------------
