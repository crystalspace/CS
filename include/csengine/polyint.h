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

#ifndef POLYINT_H
#define POLYINT_H

#include "csgeom/math3d.h"

// Values returned by classify.
#define POL_SAME_PLANE 0
#define POL_FRONT 1
#define POL_BACK 2
#define POL_SPLIT_NEEDED 3

class csPolygonIntPool;

/**
 * This class indicates what methods a class should use in order
 * to be a 'polygon'. It acts as an 'interface' in JAVA terminology.
 * There is no data in this class and no method implementations.<p>
 *
 * The BSP tree implementation is an example of a class that
 * uses this csPolygonInt interface. The consequence of this is that
 * the BSP tree can be used for several sorts of polygons (even
 * 3D or 2D ones). The csOctree class also uses csPolygonInt.
 */
class csPolygonInt
{
  friend class csPolygonIntPool;

private:
  // Reference counter.
  int ref_count;

public:
  /// Constructor. Init ref counter to 1.
  csPolygonInt () { ref_count = 1; }

  /**
   * Increase the reference counter in this csPolygonInt.
   */
  void IncRefCount () { ref_count++; }

  /**
   * Decrease the reference counter. This function returns
   * false as soon as the reference counter reaches zero.
   * The object is NOT deleted automatically in that case.
   */
  bool DecRefCount () { ref_count--; return ref_count > 0; }

  /**
   * Return false if object is not referenced.
   */
  bool IsReferenced () { return ref_count > 0; }

  /**
   * Return the plane of this polygon.
   */
  virtual csPlane* GetPolyPlane () = 0;

  /**
   * Classify this polygon with regards to a plane (in world space). If this poly
   * is on same plane it returns POL_SAME_PLANE. If this poly is
   * completely in front of the given plane it returnes POL_FRONT. If this poly
   * is completely back of the given plane it returnes POL_BACK. Otherwise it
   * returns POL_SPLIT_NEEDED.
   */
  virtual int Classify (const csPlane& pl) = 0;

  /**
   * Classify to X plane. The default implementation just calls Classify()
   * above with a constructed plane but this function can be overridden
   * for more efficiency.
   */
  virtual int ClassifyX (float x) { return Classify (csPlane (1, 0, 0, -x)); }

  /**
   * Classify to Y plane. The default implementation just calls Classify()
   * above with a constructed plane but this function can be overridden
   * for more efficiency.
   */
  virtual int ClassifyY (float y) { return Classify (csPlane (0, 1, 0, -y)); }

  /**
   * Classify to Z plane. The default implementation just calls Classify()
   * above with a constructed plane but this function can be overridden
   * for more efficiency.
   */
  virtual int ClassifyZ (float z) { return Classify (csPlane (0, 0, 1, -z)); }

  /**
   * Split this polygon with the given plane (A,B,C,D) and return the
   * two resulting new polygons in 'front' and 'back'. The new polygons will
   * mimic the behaviour of the parent polygon as good as possible.
   * Note that the 'front' should be the negative side of the plane
   * and 'back' the positive side.
   */
  virtual void SplitWithPlane (csPolygonInt** front, csPolygonInt** back,
  	const csPlane& plane) = 0;

  /**
   * Split this polygon with the X plane. Default implementation just
   * calls SplitWithPlane() with a constructed plane but this function
   * can be overridden for more efficiency.
   */
  virtual void SplitWithPlaneX (csPolygonInt** front, csPolygonInt** back,
  	float x)
  {
    SplitWithPlane (front, back, csPlane (1, 0, 0, -x));
  }

  /**
   * Split this polygon with the Y plane. Default implementation just
   * calls SplitWithPlane() with a constructed plane but this function
   * can be overridden for more efficiency.
   */
  virtual void SplitWithPlaneY (csPolygonInt** front, csPolygonInt** back,
  	float y)
  {
    SplitWithPlane (front, back, csPlane (0, 1, 0, -y));
  }

  /**
   * Split this polygon with the Z plane. Default implementation just
   * calls SplitWithPlane() with a constructed plane but this function
   * can be overridden for more efficiency.
   */
  virtual void SplitWithPlaneZ (csPolygonInt** front, csPolygonInt** back,
  	float z)
  {
    SplitWithPlane (front, back, csPlane (0, 0, 1, -z));
  }

  /**
   * Return some type-id which BSP visitors can use for their
   * own purpose. The purpose of this is to allow several different
   * types of polygons to be added to the same tree. With this number
   * you can recognize them.
   */
  virtual int GetType () = 0;

  /**
   * Get number of vertices for this polygon.
   */
  virtual int GetNumVertices () = 0;

  /**
   * Get pointer to array of vertex indices.
   */
  virtual int* GetVertexIndices () = 0;

  /**
   * If known and if there is any, return the pointer to
   * the original unsplit polygon.
   */
  virtual csPolygonInt* GetUnsplitPolygon () = 0;

  /**
   * Compare this polygon to the other one. If there exists
   * a position in space where this polygon would cover the other
   * then return true. The other polygon is not guaranteed to
   * have the same type but it is guaranteed to be part of the
   * same set of added polygon then this one. So the type should
   * always be known.
   */
  virtual bool Covers (csPolygonInt* covered) = 0;
};

/**
 * An array of csPolygonInt pointers.
 */
class csPolygonIntArray
{
private:
  /// Array.
  csPolygonInt** polygons;
  ///
  int num;
  ///
  int max;

public:
  /// Constructor. Initialize empty array.
  csPolygonIntArray ();

  /**
   * Destructor. The list of polygons is removed
   * but NOT the polygons themselves.
   */
  ~csPolygonIntArray ();

  /// Reset the array to empty.
  void Reset () { num = 0; }

  /// Add a polygon.
  void AddPolygon (csPolygonInt* polygon);

  /// Get polygon.
  csPolygonInt* GetPolygon (int idx) { return polygons[idx]; }

  /// Get polygons.
  csPolygonInt** GetPolygons () { return polygons; }

  /// Get number of polygons.
  int GetNumPolygons () { return num; }

  /**
   * Set number of polygons (i.e. truncate list).
   * The new number of polygons cannot be greater than the
   * current number of polygons.
   */
  void SetNumPolygons (int n) { num = n; }
};

/**
 * An abstract factory class to create specific instances of csPolygonInt.
 * To create a real factory you need to subclass this factory.
 * In addition to creation this factory also manages the reference
 * counter.
 */
class csPolygonIntFactory
{
public:
  /// Create a csPolygonInt.
  virtual csPolygonInt* Create () = 0;
};

/**
 * This is an object pool which holds objects of type
 * csPolygonInt. You can ask new instances from this pool.
 * If needed it will allocate one for you but ideally it can
 * give you one which was allocated earlier.
 */
class csPolygonIntPool
{
private:
  struct PoolObj
  {
    PoolObj* next;
    csPolygonInt* pi;
  };
  /// List of allocated polygons.
  PoolObj* alloced;
  /// List of previously allocated, but now unused polygons.
  PoolObj* freed;
  /// Factory to create new polygons.
  csPolygonIntFactory* poly_fact;

public:
  /// Create an empty pool.
  csPolygonIntPool (csPolygonIntFactory* fact) : alloced (NULL),
  	freed (NULL), poly_fact (fact) { }

  /// Destroy pool and all objects in the pool.
  ~csPolygonIntPool ()
  {
    while (alloced)
    {
      PoolObj* n = alloced->next;
      //CHK (delete alloced->pi); @@@ This free is not valid!
      // We should use a ref count on the pool itself so that we
      // now when all objects in the pool are freed and the
      // 'alloced' list will be empty.
      CHK (delete alloced);
      alloced = n;
    }
    while (freed)
    {
      PoolObj* n = freed->next;
      if (!freed->pi->DecRefCount ()) CHKB (delete freed->pi);
      CHK (delete freed);
      freed = n;
    }
  }

  /**
   * Allocate a new object in the pool.
   */
  csPolygonInt* Alloc ()
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
      pnew->pi = poly_fact->Create ();
    }
    pnew->next = alloced;
    alloced = pnew;
    pnew->pi->ref_count = 1;
    return pnew->pi;
  }

  /**
   * Free an object and put it back in the pool.
   * Note that it is only legal to free objects which were allocated
   * from the pool.
   */
  void Free (csPolygonInt* pi)
  {
    if (pi->DecRefCount ()) return;
    if (alloced)
    {
      PoolObj* po = alloced;
      alloced = alloced->next;
      po->pi = pi;
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
    printf ("PolyInt pool: %d allocated, ", cnt);
    cnt = 0;
    po = freed;
    while (po) { cnt++; po = po->next; }
    printf ("%d freed.\n", cnt);
  }
};

#endif /*POLYINT_H*/
