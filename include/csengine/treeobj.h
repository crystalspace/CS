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

#ifndef __CS_TREEOBJ_H__
#define __CS_TREEOBJ_H__

#include "csutil/csvector.h"
#include "csgeom/math3d.h"
#include "csgeom/box.h"
#include "csengine/polyint.h"
#include "csengine/polytree.h"

class csObject;
class csPolygonStubPool;
class csPolygonInt;
class csPolygonIntPool;
class csPolygonTreeNode;
class csPolyTreeBBox;
class csThing;

/**
 * An object stub. For a given tree object there can be
 * many object stubs. Every stub represents some object (or
 * part of that object) that belongs to some node.
 */
class csPolygonStub
{
  friend class csPolyTreeBBox;
  friend class csPolygonTreeNode;
  friend class csPolygonStubPool;

private:
  // Linked list in every tree node.
  csPolygonStub* next_tree, * prev_tree;
  // Linked list in every csPolyTreeBBox.
  csPolygonStub* next_obj, * prev_obj;
  // Object where this stub lives.
  csPolyTreeBBox* object;
  // Tree node where this stub lives.
  csPolygonTreeNode* node;
  // Reference counter. This ref counter is managed by the
  // csPolygonStubPool.
  int ref_count;

  // List of polygons.
  csPolygonIntArray polygons;
  // The pool that is used to create polygons.
  csPolygonIntPool* poly_pool;

public:
  /// Initialize this stub with a polygon pool.
  csPolygonStub (csPolygonIntPool* pool) : next_tree (NULL),
  	prev_tree (NULL), next_obj (NULL), prev_obj (NULL),
	object (NULL), node (NULL), ref_count (1), poly_pool (pool) { }
  ~csPolygonStub ()
  {
    RemoveStub ();
  }

  // Increment the ref counter.
  void IncRef () { ref_count++; }
  // Decrement the ref counter.
  void DecRef () { if (ref_count <= 0) DEBUG_BREAK; ref_count--; }

  /// Unlink this stub from all lists.
  void RemoveStub ();

  /// Get parent object for this stub.
  csPolyTreeBBox* GetObject () { return object; }

  /// Get access to the list of polygons in this stub.
  csPolygonIntArray& GetPolygonArray () { return polygons; }
  /// Get list of polygons.
  csPolygonInt** GetPolygons () { return polygons.GetPolygons (); }
  /// Get number of polygons.
  int GetPolygonCount () { return polygons.GetPolygonCount (); }

  /// Visit this stub while traversing the tree (octree or BSP tree).
  void* Visit (csThing* thing, csTreeVisitFunc* func, void* data)
  {
    return func (thing, polygons.GetPolygons (), polygons.GetPolygonCount (),
    	false, data);
  }

  /// Initialize this stub.
  void Initialize ()
  {
    polygons.Reset ();
  }

  /**
   * Clean up data from this stub.
   */
  void RemoveData ();
};

/**
 * A csPolygonStub factory. This factory will create objects
 * of type csPolygonStub.
 */
class csPolygonStubFactory
{
private:
  // The pool that is used to create polygons.
  csPolygonIntPool* poly_pool;

public:
  /// Initialize this stub factory with a polygon pool.
  csPolygonStubFactory (csPolygonIntPool* pool) : poly_pool (pool) { }

  /// Create a new stub.
  csPolygonStub* Create ()
  {
    return new csPolygonStub (poly_pool);
  }
};

/**
 * This is an object pool which holds objects of type
 * csPolygonStub. You control which type of csPolygonStub you need
 * by the factory that you give with Alloc().
 * You can ask new instances from this pool.
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
  /// List of previously allocated, but now unused stubs.
  PoolObj* freed;

public:
  /// Create an empty pool.
  csPolygonStubPool () : alloced (NULL), freed (NULL) { }

  /// Destroy pool and all objects in the pool.
  ~csPolygonStubPool ();

  /**
   * Allocate a new object in the pool.
   * The object is initialized to empty.
   */
  csPolygonStub* Alloc (csPolygonStubFactory* factory);

  /**
   * Free an object and put it back in the pool. This function will
   * call DecRef on the stub and only free it for real if the ref
   * counter reaches zero.
   * Note that it is only legal to free objects which were allocated
   * from the pool.
   */
  void Free (csPolygonStub* ps);

  /// Dump some information about this pool.
  void Dump ();
};

#endif // __CS_TREEOBJ_H__
