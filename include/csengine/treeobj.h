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
class csObjectStubPool;
class csPolygonInt;
class csPolygonIntPool;
class csPolyTreeObject;
class csPolygonTreeNode;
class csThing;
class Dumper;

/**
 * An object stub. For a given tree object there can be
 * many object stubs. Every stub represents some object (or
 * part of that object) that belongs to some node.
 */
class csObjectStub
{
  friend class csPolyTreeObject;
  friend class csPolygonTreeNode;
  friend class csObjectStubPool;
  friend class Dumper;

private:
  // Linked list in every tree node.
  csObjectStub* next_tree, * prev_tree;
  // Linked list in every csPolyTreeObject.
  csObjectStub* next_obj, * prev_obj;
  // Object where this stub lives.
  csPolyTreeObject* object;
  // Tree node where this stub lives.
  csPolygonTreeNode* node;
  // Reference counter. This ref counter is managed by the
  // csObjectStubPool.
  int ref_count;

public:
  ///
  csObjectStub () : next_tree (NULL), prev_tree (NULL), next_obj (NULL),
  	prev_obj (NULL), object (NULL), node (NULL), ref_count (1) { }
  ///
  virtual ~csObjectStub ()
  {
    RemoveStub ();
  }

  // Increment the ref counter.
  void IncRef () { ref_count++; }
  // Decrement the ref counter.
  void DecRef () { if (ref_count <= 0) CRASH; ref_count--; }

  /// Unlink this stub from all lists.
  void RemoveStub ();

  /// Get parent object for this stub.
  csPolyTreeObject* GetObject () { return object; }

  /// Visit this stub while traversing the tree (octree or BSP tree).
  virtual void* Visit (csThing* thing, csTreeVisitFunc* func, void* data) = 0;

  /// Initialize this stub.
  virtual void Initialize () = 0;

  /**
   * Clean up data from this stub (i.e. remove polygons and so on).
   * This is a virtual function because in general the framework
   * cannot know how the polygons need to be freed (i.e. is a
   * pool used? reference counter? ...).
   */
  virtual void RemoveData () = 0;
};

/**
 * An ObjectStub factory. This factory will create objects
 * of type (or subclass of) csObjectStub.
 */
class csObjectStubFactory
{
public:
  /// Create a new stub.
  virtual csObjectStub* Create () = 0;
};

/**
 * A specific implementation of csObjectStub implementing
 * a stub containing polygons.
 */
class csPolygonStub : public csObjectStub
{
private:
  // List of polygons.
  csPolygonIntArray polygons;
  // The pool that is used to create polygons.
  csPolygonIntPool* poly_pool;

public:
  /// Initialize this stub with a polygon pool.
  csPolygonStub (csPolygonIntPool* pool) : poly_pool (pool) { }

  /// Get access to the list of polygons in this stub.
  csPolygonIntArray& GetPolygonArray () { return polygons; }
  /// Get list of polygons.
  csPolygonInt** GetPolygons () { return polygons.GetPolygons (); }
  /// Get number of polygons.
  int GetNumPolygons () { return polygons.GetNumPolygons (); }

  /// Visit this stub while traversing the tree (octree or BSP tree).
  virtual void* Visit (csThing* thing, csTreeVisitFunc* func, void* data)
  {
    return func (thing, polygons.GetPolygons (), polygons.GetNumPolygons (),
    	false, data);
  }

  /// Initialize this stub.
  virtual void Initialize ()
  {
    polygons.Reset ();
  }

  /**
   * Clean up data from this stub.
   */
  virtual void RemoveData ();
};

/**
 * Specific implementation of the factory for csPolygonStub.
 */
class csPolygonStubFactory : public csObjectStubFactory
{
private:
  // The pool that is used to create polygons.
  csPolygonIntPool* poly_pool;

public:
  /// Initialize this stub factory with a polygon pool.
  csPolygonStubFactory (csPolygonIntPool* pool) : poly_pool (pool) { }

  /// Create a new stub.
  virtual csObjectStub* Create ()
  {
    return new csPolygonStub (poly_pool);
  }
};

/**
 * This is an object pool which holds objects of type
 * csObjectStub. You control which type of csObjectStub you need
 * by the factory that you give with Alloc().
 * You can ask new instances from this pool.
 * If needed it will allocate one for you but ideally it can
 * give you one which was allocated earlier.
 */
class csObjectStubPool
{
  friend class Dumper;

private:
  struct PoolObj
  {
    PoolObj* next;
    csObjectStub* ps;
  };
  /// List of allocated polygon stubs.
  PoolObj* alloced;
  /// List of previously allocated, but now unused stubs.
  PoolObj* freed;

public:
  /// Create an empty pool.
  csObjectStubPool () : alloced (NULL), freed (NULL) { }

  /// Destroy pool and all objects in the pool.
  ~csObjectStubPool ();

  /**
   * Allocate a new object in the pool.
   * The object is initialized to empty.
   */
  csObjectStub* Alloc (csObjectStubFactory* factory);

  /**
   * Free an object and put it back in the pool. This function will
   * call DecRef on the stub and only free it for real if the ref
   * counter reaches zero.
   * Note that it is only legal to free objects which were allocated
   * from the pool.
   */
  void Free (csObjectStub* ps);

  /// Dump some information about this pool.
  void Dump ();
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
  friend class Dumper;

private:
  /**
   * A linked list for all object stubs that are added
   * to the tree. These stubs represents parts of
   * this object that belong to the tree. In case of csPolygonStub
   * every stub will represent a list of polygons that are coplanar
   * with the splitter plane at that node.
   */
  csObjectStub* first_stub;

protected:
  /// Bounding box for this object.
  csBox3 world_bbox;

public:
  /// A pool of polygon stubs.
  static csObjectStubPool stub_pool;

protected:
  /// A factory which is responsible for creating stubs for this object.
  csObjectStubFactory* stub_factory;

public:
  /// Constructor.
  csPolyTreeObject (csObjectStubFactory* factory);
  /// Destructor.
  virtual ~csPolyTreeObject ();

  /**
   * Remove this object from its tree.
   */
  void RemoveFromTree ();

  /**
   * Create the base stub. In case of csPolygonStub this corresponds to the
   * set of polygons that make up the desired object to be placed
   * in the polygon tree. In most cases this will be a bounding
   * box for the real object.
   */
  virtual csObjectStub* GetBaseStub () = 0;

  /**
   * Split the given stub with a plane and return
   * three new stubs (all on the plane, in front, or
   * back of the plane). This is a pure virtual function that will
   * have to be implemented by a subclass.<p>
   *
   * Note that this function is responsible for freeing 'stub' itself
   * if needed. Also this function can return NULL for stub_on, stub_front,
   * and stub_back in which case there simply is no stub for that
   * particular case.<p>
   *
   * Other note. This function will also correctly account for
   * the case where the given stub_on pointer is NULL. In that case
   * the tree is not interested in the polygons on the plane and those
   * polygons will be distributed to stub_front.
   */
  virtual void SplitWithPlane (csObjectStub* stub,
  	csObjectStub** stub_on, csObjectStub** stub_front,
	csObjectStub** stub_back,
	const csPlane3& plane) = 0;

  /**
   * Split the given stub with an X plane.
   */
  virtual void SplitWithPlaneX (csObjectStub* stub,
  	csObjectStub** stub_on, csObjectStub** stub_front,
	csObjectStub** stub_back,
	float x) = 0;

  /**
   * Split the given stub with an Y plane.
   */
  virtual void SplitWithPlaneY (csObjectStub* stub,
  	csObjectStub** stub_on, csObjectStub** stub_front,
	csObjectStub** stub_back,
	float y) = 0;

  /**
   * Split the given stub with an Z plane.
   */
  virtual void SplitWithPlaneZ (csObjectStub* stub,
  	csObjectStub** stub_on, csObjectStub** stub_front,
	csObjectStub** stub_back,
	float z) = 0;

  /**
   * Unlink a stub from the stub list.
   * Warning! This function does not test if the stub
   * is really on the list!
   */
  void UnlinkStub (csObjectStub* ps);

  /**
   * Link a stub to the stub list.
   */
  void LinkStub (csObjectStub* ps);

  /**
   * Get the bounding box that represents this object.
   * If the camera is inside this bbox then the object
   * is certainly visible.
   */
  const csBox3& GetWorldBoundingBox ()
  {
    return world_bbox;
  }
};

/**
 * Specific implementation of csPolyTreeObject for csPolygonStub.
 * This implementation is the most involved one where stubs represents
 * polygon lists and polygons are split to the tree.
 */
class csDetailedPolyTreeObject : public csPolyTreeObject
{
  friend class Dumper;

public:
  /// Constructor.
  csDetailedPolyTreeObject (csObjectStubFactory* stub_fact) :
  	csPolyTreeObject (stub_fact) { }
  /// Destructor.
  virtual ~csDetailedPolyTreeObject () { }

  /// Split to plane.
  virtual void SplitWithPlane (csObjectStub* stub,
  	csObjectStub** stub_on, csObjectStub** stub_front,
	csObjectStub** stub_back,
	const csPlane3& plane);

  /**
   * Split the given stub with an X plane.
   */
  virtual void SplitWithPlaneX (csObjectStub* stub,
  	csObjectStub** stub_on, csObjectStub** stub_front,
	csObjectStub** stub_back,
	float x);

  /**
   * Split the given stub with an Y plane.
   */
  virtual void SplitWithPlaneY (csObjectStub* stub,
  	csObjectStub** stub_on, csObjectStub** stub_front,
	csObjectStub** stub_back,
	float y);

  /**
   * Split the given stub with an Z plane.
   */
  virtual void SplitWithPlaneZ (csObjectStub* stub,
  	csObjectStub** stub_on, csObjectStub** stub_front,
	csObjectStub** stub_back,
	float z);
};

#endif // __CS_TREEOBJ_H__
