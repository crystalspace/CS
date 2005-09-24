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

#ifndef __CS_POLYEDGE_H__
#define __CS_POLYEDGE_H__

/**\file 
 * General 2D polygon, represented by edges.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/math2d.h"
#include "csgeom/segment.h"

/**
 * The following class represents a general 2D polygon represented
 * with edges instead of vertices.
 */
class CS_CRYSTALSPACE_EXPORT csPoly2DEdges
{
protected:
  /// The edges.
  csSegment2* edges;
  ///
  int num_edges;
  ///
  int max_edges;

public:
  /**
   * Make a new empty polygon.
   */
  csPoly2DEdges (int start_size = 10);

  /// Copy constructor.
  csPoly2DEdges (csPoly2DEdges& copy);

  /// Destructor.
  virtual ~csPoly2DEdges ();

  /**
   * Initialize the polygon to empty.
   */
  void MakeEmpty ();

  /**
   * Get the number of edges.
   */
  int GetEdgeCount () { return num_edges; }

  /**
   * Get the array with all edges.
   */
  csSegment2* GetEdges () { return edges; }

  /**
   * Get the specified edge.
   */
  csSegment2* GetEdge (int i)
  {
    if (i<0 || i>=num_edges) return 0;
    return &edges[i];
  }

  /**
   * Get the specified edge.
   */
  csSegment2& operator[] (int i)
  {
    CS_ASSERT (i >= 0 && i < num_edges);
    return edges[i];
  }

  /**
   * Get the first edge.
   */
  csSegment2* GetFirst ()
  { if (num_edges<=0) return 0;  else return edges; }

  /**
   * Get the last edge.
   */
  csSegment2* GetLast ()
  { if (num_edges<=0) return 0;  else return &edges[num_edges-1]; }

  /**
   * Test if this vector is inside the polygon.
   */
  bool In (const csVector2& v);

  /**
   * Test if a vector is inside the given polygon.
   */
  static bool In (csSegment2* poly, int num_edge, const csVector2& v);

  /**
   * Make room for at least the specified number of edges.
   */
  void MakeRoom (int new_max);

  /**
   * Set the number of edges.
   */
  void SetEdgeCount (int n) { MakeRoom (n); num_edges = n; }

  /**
   * Add a edge (2D) to the polygon.
   * Return index of added edge.
   */
  int AddEdge (const csSegment2& e) { return AddEdge (e.Start (), e.End ()); }

  /**
   * Add a edge (2D) to the polygon.
   * Return index of added edge.
   */
  int AddEdge (const csVector2& v1, const csVector2& v2);

  /**
   * Intersect this polygon with a given plane and return the
   * two resulting polygons in left and right. This version is
   * robust. If one of the edges of this polygon happens to be
   * on the same plane as 'plane' then this edge will be ignored
   * and 'onplane' will be set to true.<p>
   *
   * Note that this routine may result in unconnected polygons.
   * i.e. edges that are not adjacent.
   */
  void Intersect (const csPlane2& plane,
  	csPoly2DEdges& left, csPoly2DEdges& right, bool& onplane) const;
};

/**
 * This is an object pool which holds objects of type
 * csPoly2DEdges. You can ask new instances from this pool.
 * If needed it will allocate one for you but ideally it can
 * give you one which was allocated earlier.
 */
class csPoly2DEdgesPool
{
private:
  struct PoolObj
  {
    PoolObj* next;
    csPoly2DEdges* pol2d;
  };
  /// List of allocated polygons.
  PoolObj* alloced;
  /// List of previously allocated, but now unused polygons.
  PoolObj* freed;

public:
  /// Create an empty pool.
  csPoly2DEdgesPool () : alloced (0), freed (0) { }

  /// Destroy pool and all objects in the pool.
  ~csPoly2DEdgesPool ()
  {
    while (alloced)
    {
      PoolObj* n = alloced->next;
      //delete alloced->pol2d; @@@ This free is not valid!
      // We should use a ref count on the pool itself so that we
      // now when all objects in the pool are freed and the
      // 'alloced' list will be empty.
      delete alloced;
      alloced = n;
    }
    while (freed)
    {
      PoolObj* n = freed->next;
      delete freed->pol2d;
      delete freed;
      freed = n;
    }
  }

  /// Allocate a new object in the pool.
  csPoly2DEdges* Alloc ()
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
      pnew->pol2d = new csPoly2DEdges ();
    }
    pnew->next = alloced;
    alloced = pnew;
    return pnew->pol2d;
  }

  /**
   * Free an object and put it back in the pool.
   * Note that it is only legal to free objects which were allocated
   * from the pool.
   */
  void Free (csPoly2DEdges* pol)
  {
    if (alloced)
    {
      PoolObj* po = alloced;
      alloced = alloced->next;
      po->pol2d = pol;
      po->next = freed;
      freed = po;
    }
    else
    {
      // Cannot happen!
      CS_ASSERT (false);
    }
  }
};

/** @} */

#endif // __CS_POLYEDGE_H__
