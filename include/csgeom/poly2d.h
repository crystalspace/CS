/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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

#ifndef __CS_POLY2D_H__
#define __CS_POLY2D_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/math2d.h"
#include "csgeom/box.h"

struct iClipper2D;

/**
 * The following class represents a general 2D polygon.
 */
class CS_CRYSTALSPACE_EXPORT csPoly2D
{
protected:
  /// The 2D vertices.
  csVector2* vertices;
  ///
  size_t num_vertices;
  ///
  size_t max_vertices;
public:
  /**
   * Make a new empty polygon.
   */
  csPoly2D (size_t start_size = 10);

  /// Copy constructor.
  csPoly2D (const csPoly2D& copy);

  /// Destructor.
  ~csPoly2D ();

  /// Assignment operator.
  csPoly2D& operator= (const csPoly2D& other);

  /**
   * Initialize the polygon to empty.
   */
  void MakeEmpty ();

  /**
   * Get the number of vertices.
   */
  size_t GetVertexCount () const { return num_vertices; }

  /**
   * Get the array with all vertices.
   */
  csVector2* GetVertices () { return vertices; }

  /**
   * Get the array with all vertices.
   */
  const csVector2* GetVertices () const { return vertices; }

  /**
   * Get the specified vertex.
   */
  csVector2* GetVertex (size_t i)
  {
    if (i<0 || i>=num_vertices) return 0;
    return &vertices[i];
  }

  /**
   * Get the specified vertex.
   */
  csVector2& operator[] (size_t i)
  {
    CS_ASSERT (i >= 0 && i < num_vertices);
    return vertices[i];
  }

  /**
   * Get the specified vertex.
   */
  const csVector2& operator[] (size_t i) const
  {
    CS_ASSERT (i >= 0 && i < num_vertices);
    return vertices[i];
  }

  /**
   * Get the first vertex.
   */
  csVector2* GetFirst ()
  { if (num_vertices<=0) return 0;  else return vertices; }

  /**
   * Get the last vertex.
   */
  csVector2* GetLast ()
  { if (num_vertices<=0) return 0;  else return &vertices[num_vertices-1]; }

  /**
   * Test if this vector is inside the polygon.
   */
  bool In (const csVector2& v);

  /**
   * Test if a vector is inside the given polygon.
   */
  static bool In (csVector2* poly, size_t num_poly, const csVector2& v);

  /**
   * Make room for at least the specified number of vertices.
   */
  void MakeRoom (size_t new_max);

  /**
   * Set the number of vertices.
   */
  void SetVertexCount (size_t n)
  {
    MakeRoom (n);
    num_vertices = n;
  }

  /**
   * Add a vertex (2D) to the polygon.
   * Return index of added vertex.
   */
  size_t AddVertex (const csVector2& v) { return AddVertex (v.x, v.y); }

  /**
   * Add a vertex (2D) to the polygon.
   * Return index of added vertex.
   */
  size_t AddVertex (float x, float y);

  /**
   * Set all polygon vertices at once.  Copies the array.
   * Note! This doesn't update the bounding box!
   */
  void SetVertices (csVector2 const* v, size_t num)
  {
    MakeRoom (num);
    memcpy (vertices, v, (num_vertices = num) * sizeof (csVector2));
  }

  /**
   * Clipping routines. They return false if the resulting polygon is not
   * visible for some reason.
   * Note that these routines must not be called if the polygon is not visible.
   * These routines will not check that.
   * Note that these routines will put the resulting clipped 2D polygon
   * in place of the original 2D polygon.
   */
  bool ClipAgainst (iClipper2D* view);

  /**
   * Intersect this polygon with a given plane and return the
   * two resulting polygons in left and right. This version is
   * robust. If one of the edges of this polygon happens to be
   * on the same plane as 'plane' then the edge will go to the
   * polygon which already has most edges. i.e. you will not
   * get degenerate polygons.
   */
  void Intersect (const csPlane2& plane, csPoly2D& left,
  	csPoly2D& right) const;

  /**
   * This routine is similar to Intersect but it only returns the
   * polygon on the 'right' (positive) side of the plane.
   */
  void ClipPlane (const csPlane2& plane, csPoly2D& right) const;

  /**
   * Extend this polygon with another polygon so that the resulting
   * polygon is: (a) still convex, (b) fully contains this polygon,
   * and (c) contains as much as possible of the other polgon.
   * 'this_edge' is the index of the common edge for this polygon.
   * Edges are indexed with 0 being the edge from 0 to 1 and n-1 being
   * the edge from n-1 to 0.
   */
  void ExtendConvex (const csPoly2D& other, size_t this_edge);

  /**
   * Calculate the signed area of this polygon.
   */
  float GetSignedArea();

  /**
   * Generate a random convex polygon with the specified number
   * of vertices. The polygon will be inside the given bounding box.
   * @@@ Currently only triangles are supported.
   */
  void Random (size_t num, const csBox2& max_bbox);
};

/**
 * This factory is responsible for creating csPoly2D objects or subclasses
 * of csPoly2D. To create a new factory which can create subclasses of csPoly2D
 * you should create a subclass of this factory.
 */
class csPoly2DFactory
{
public:
  // Needed for GCC4. Otherwise emits a flood of "virtual functions but
  // non-virtual destructor" warnings.
  virtual ~csPoly2DFactory() {};

  /// A shared factory that you can use.
   CS_DECLARE_STATIC_CLASSVAR(sharedFactory,SharedFactory,csPoly2DFactory)

  /// Create a poly2d.
  virtual csPoly2D* Create ()
  {
    csPoly2D* p = new csPoly2D ();
    return p;
  }
};

/** @} */

#endif // __CS_POLY2D_H__
