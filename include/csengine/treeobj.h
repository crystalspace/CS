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

#ifndef TREEOBJ_H
#define TREEOBJ_H

#include "csutil/csvector.h"
#include "csgeom/math3d.h"

class csObject;
class csPolygonStub;
class csPolygonInt;
class csPolyTreeObject;
class csPolygonTreeNode;

/**
 * A polygon stub. For a given tree object there can be
 * many polygon stubs. Every stub represents a list of polygons
 * that belongs to some node.
 */
class csPolygonStub
{
  friend class csPolyTreeObject;
  friend class csPolygonTreeNode;

private:
  // Linked list in every tree node.
  csPolygonStub* next_bsp, * prev_bsp;
  // Linked list in every csPolyTreeObject.
  csPolygonStub* next_obj, * prev_obj;
  // Object where this stub lives.
  csPolyTreeObject* object;
  // Tree node where this stub lives.
  csPolygonTreeNode* node;
  // List of polygons.
  csPolygonInt** polygons;

public:
  ///
  csPolygonStub () : object (NULL), node (NULL), polygons (NULL) { }
  ///
  ~csPolygonStub ();
  /**
   * Set list of polygons. This stub will delete this
   * list (but not the individual polygons) at destruction time.
   */
  void SetPolygons (csPolygonInt** pols) { polygons = pols; }
  /// Get list of polygons.
  csPolygonInt** GetPolygons () { return polygons; }
  /// Unlink this stub from all lists.
  void RemoveStub ();
};

/**
 * This is an object pool which holds objects of type
 * csPolygonStub. You can ask new instances from this pool.
 * If needed it will allocate one for you but ideally it can
 * give you one which was allocated earlier.
 */
class csPolygonStubPool
{
private:
  struct PoolObj
  {
    PoolObj* next;
    csPolygonStub* ps;
  };
  /// List of allocated polygon stubs.
  PoolObj* alloced;
  /// List of previously allocated, but now unused polygon stubs.
  PoolObj* freed;

public:
  /// Create an empty pool.
  csPolygonStubPool () : alloced (NULL), freed (NULL) { }

  /// Destroy pool and all objects in the pool.
  ~csPolygonStubPool ()
  {
    while (alloced)
    {
      PoolObj* n = alloced->next;
      CHK (delete alloced->ps);
      CHK (delete alloced);
      alloced = n;
    }
    while (freed)
    {
      PoolObj* n = freed->next;
      CHK (delete freed->ps);
      CHK (delete freed);
      freed = n;
    }
  }

  /// Allocate a new object in the pool.
  csPolygonStub* Alloc ()
  {
    PoolObj* pnew;
    if (freed)
    {
      pnew = freed;
      freed = freed->next;
    }
    else
    {
      CHK (pnew = new PoolObj ());
      CHK (pnew->ps = new csPolygonStub ());
    }
    pnew->next = alloced;
    alloced = pnew;
    return pnew->ps;
  }

  /**
   * Free an object and put it back in the pool.
   * Note that it is only legal to free objects which were allocated
   * from the pool.
   */
  void Free (csPolygonStub* ps)
  {
    ps->RemoveStub ();
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

  /// Dump some information about this pool.
  void Dump ()
  {
    int cnt;
    cnt = 0;
    PoolObj* po = alloced;
    while (po) { cnt++; po = po->next; }
    printf ("PolyStub pool: %d allocated, ", cnt);
    cnt = 0;
    po = freed;
    while (po) { cnt++; po = po->next; }
    printf ("%d freed.\n", cnt);
  }
};

/**
 * This abstract class represents a (dynamic) object that can be placed
 * in a polygon tree (BSP, octree, ...). Every engine entity that
 * is interested in adding itself to the tree should either inherit
 * from this class or else let another class inherit from this one
 * and use composition (preferable).
 */
class csPolyTreeObject
{
private:
  /// Owner (original dynamic object).
  csObject* owner;
  /**
   * A linked list for all polygons stubs that are added
   * to the tree. These stubs represents sets of polygons
   * from this object that belong to the same plane as the
   * plane of the splitter in the tree node.
   */
  csPolygonStub* first_stub;

public:
  /// A pool of polygon stubs.
  static csPolygonStubPool stub_pool;

public:
  /// Constructor.
  csPolyTreeObject (csObject* owner);
  /// Destructor.
  virtual ~csPolyTreeObject ();

  /// Get owner.
  csObject* GetOwner () const { return owner; }
  /// Set owner.
  void SetOwner (csObject* newOwner) { owner = newOwner; }

  /**
   * Split the given stub with a plane and return
   * two new stubs. This is a pure virtual function that will
   * have to be implemented by a subclass.
   */
  virtual void SplitWithPlane (csPolygonStub* stub,
  	csPolygonStub** stub_front, csPolygonStub** stub_back,
	const csPlane& plane) = 0;

  /**
   * Split the given stub with an X plane and return
   * two new stubs. The default implementation will just call
   * SplitWithPlane() but it is recommended to implement a more
   * efficient version here.
   */
  virtual void SplitWithPlaneX (csPolygonStub* stub,
  	csPolygonStub** stub_front, csPolygonStub** stub_back,
	float x)
  {
    SplitWithPlane (stub, stub_front, stub_back, csPlane (1, 0, 0, -x));
  }

  /**
   * Split the given stub with an Y plane and return
   * two new stubs. The default implementation will just call
   * SplitWithPlane() but it is recommended to implement a more
   * efficient version here.
   */
  virtual void SplitWithPlaneY (csPolygonStub* stub,
  	csPolygonStub** stub_front, csPolygonStub** stub_back,
	float y)
  {
    SplitWithPlane (stub, stub_front, stub_back, csPlane (0, 1, 0, -y));
  }

  /**
   * Split the given stub with an Z plane and return
   * two new stubs. The default implementation will just call
   * SplitWithPlane() but it is recommended to implement a more
   * efficient version here.
   */
  virtual void SplitWithPlaneZ (csPolygonStub* stub,
  	csPolygonStub** stub_front, csPolygonStub** stub_back,
	float z)
  {
    SplitWithPlane (stub, stub_front, stub_back, csPlane (0, 0, 1, -z));
  }

  /**
   * Unlink a stub from the stub list.
   * Warning! This function does not test if the stub
   * is really on the list!
   */
  void UnlinkStub (csPolygonStub* ps);
};

#endif /*TREEOBJ_H*/

