/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef BSPPOL_H
#define BSPPOL_H

#include "cscom/com.h"
#include "csgeom/math3d.h"
#include "csgeom/polyint.h"
#include "csengine/polygon.h"

class csPolygonSet;

/**
 * This special class of polygon is only used by the BSP tree. In order to
 * avoid having to split csPolygon3D instances (which caused a lot of troubles
 * in the past) we build a BSP tree based on this structure which only contains
 * an array of vertices and a pointer back to the original polygon.
 */
class csPolygonBsp : public csPolygonInt
{
private:
  /// A table of vertices.
  csVector3* vertices;
  /// Number of vertices.
  int num_vertices;
  /// Maximum number of vertices.
  int max_vertices;
  /// Original polygon.
  csPolygon3D* poly3d;

public:
  /// Construct an empty polygon.
  csPolygonBsp ();
  /// Construct a polygon based on a csPolygon3D.
  csPolygonBsp (csPolygon3D* orig_poly3d);
  /// Destroy this polygon.
  virtual ~csPolygonBsp ();

  /// Set original polygon.
  void SetPolygon3D (csPolygon3D* orig_poly3d) { poly3d = orig_poly3d; }
  /// Get original polygon.
  csPolygon3D* GetPolygon3D () { return poly3d; }

  /// Add a vertex.
  void AddVertex (const csVector3& v);

  /// Get the plane of this polygon.
  virtual csPlane* GetPolyPlane () { return poly3d->GetPolyPlane (); }

  /**
   * Classify a polygon with regards to this one. If the poly is on same
   * plane as this one it returns POL_SAME_PLANE. If this poly is
   * completely in front of the given poly it returnes POL_FRONT. If this poly
   * is completely back of the given poly it returnes POL_BACK. Otherwise it
   * returns POL_SPLIT_NEEDED.
   */
  virtual int Classify (csPolygonInt* poly);

  /**
   * Split this polygon with the given plane (A,B,C,D) and return the
   * two resulting new polygons in 'front' and 'back'. The new polygons will
   * mimic the behaviour of the parent polygon as good as possible.
   * This function is mainly used by the BSP splitter.
   */
  virtual void SplitWithPlane (csPolygonInt** front, csPolygonInt** back, csPlane& plane);

  /**
   * Return true if this polygon and the given polygon are on the same
   * plane. If their planes are shared this is automatically the case.
   * Otherwise this function will check their respective plane equations
   * to test for equality.
   */
  virtual bool SamePlane (csPolygonInt* p);
};

/**
 * This is the container for the above BSP polygons.
 */
class csPolygonBspContainer : public csPolygonParentInt
{
private:
  /// Table of ptr to polygons for this container.
  csPolygonInt** polygons;
  /// Number of polygons used.
  int num_polygon;
  /// Maximum number of polygons in 'polygons' array.
  int max_polygon;

public:
  /// Construct an empty container.
  csPolygonBspContainer ();

  /// Destruct.
  virtual ~csPolygonBspContainer ();

  /// Add a polygon.
  virtual void AddPolygon (csPolygonInt* p);

  /// Get the number of polygons.
  virtual int GetNumPolygons () { return num_polygon; }

  /// Get a polygon with the index.
  virtual csPolygonInt* GetPolygon (int num) { return polygons[num]; }
};


#endif /*BSPPOL_H*/
