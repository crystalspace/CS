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

#ifndef BSPBBOX_H
#define BSPBBOX_H

#include "csgeom/math3d.h"
#include "csgeom/polyidx.h"
#include "csgeom/poly3d.h"
#include "csgeom/transfrm.h"
#include "csengine/polyint.h"
#include "csengine/pol2d.h"
#include "csobject/csobject.h"

class csBspContainer;

/**
 * This class represents a polygon which can be inserted dynamically
 * in a BSP tree. It is specifically designed to be able to add bounding
 * boxes to sprites and dynamic things and then add those bounding boxes
 * to the BSP tree.
 */
class csBspPolygon : public csPolygonInt
{
private:
  /// The 3D polygon.
  csPolyIndexed polygon;
  /// The plane.
  csPlane plane;
  /// The parent.
  csBspContainer* parent;
  /// The original object for which this polygon is a bounding box polygon.
  csObject* originator;

public:
  /// Destructor.
  virtual ~csBspPolygon () { }

  /// Get the parent container.
  csBspContainer* GetParent () { return parent; }

  /// Set the parent container.
  void SetParent (csBspContainer* par) { parent = par; }

  /**
   * Get the original object for which this polygon is a bounding box polygon.
   */
  csObject* GetOriginator () { return originator; }

  /// Set the original object.
  void SetOriginator (csObject* orig) { originator = orig; }

  /// Get the reference to the polygon.
  csPolyIndexed& GetPolygon () { return polygon; }

  /// Get number of vertices.
  virtual int GetNumVertices () { return polygon.GetNumVertices (); }

  /// Get vertex index table (required for csPolygonInt).
  virtual int* GetVertexIndices () { return polygon.GetVertexIndices (); }

  /// Get original polygon (not known for this polygon type).
  virtual csPolygonInt* GetUnsplitPolygon () { return NULL; }

  /// Set the plane for this polygon.
  void SetPolyPlane (const csPlane& pl) { plane = pl; }

  /// Return the plane of this polygon.
  csPlane* GetPolyPlane () { return &plane; }

  /// Classify a polygon with regards to this one.
  int Classify (const csPlane& pl);

  /// Same as Classify() but for X plane only.
  int ClassifyX (float x);

  /// Same as Classify() but for Y plane only.
  int ClassifyY (float y);

  /// Same as Classify() but for Z plane only.
  int ClassifyZ (float z);

  /// Split this polygon with the given plane (A,B,C,D).
  void SplitWithPlane (csPolygonInt** front, csPolygonInt** back,
  	const csPlane& split_plane);

  /// Return 2 to indicate it is a bsp polygon.
  int GetType () { return 2; }

  /// Transform the plane of this polygon.
  void Transform (const csTransform& trans);

  /**
   * Transform the plane of this polygon from world to
   * camera space.
   */
  void World2Camera (const csTransform& trans);

  /**
   * Clip this polygon to the Z plane and if portal_plane is given also
   * clip the polygon to that plane.
   * @@@ NOTE @@@ This function is almost identical to the one in
   * csPolygon3D. It should be possible to reuse that code.
   */
  bool ClipToPlane (csPlane* portal_plane, const csVector3& v_w2c,
	csVector3*& pverts, int& num_verts, bool cw = true);

  /**
   * Perspective correct a polygon. Note that this function is nearly
   * identical to a version in csPolygon3D. It should be possible to
   * reuse that code.
   */
  bool DoPerspective (const csTransform& trans,
	csVector3* source, int num_verts, csPolygon2D* dest, bool mirror);
};

/**
 * A container for a bunch of BSP polygons.
 */
class csBspContainer
{
private:
  /// Array of vertices.
  csVector3Array vertices;
  /// Array of camera space vertices.
  csVector3Array cam_vertices;

  /// All polygons.
  csPolygonIntArray polygons;

public:
  /// Constructor.
  csBspContainer () { }
  /// Destructor.
  ~csBspContainer () { }

  /// Get vector array for this container.
  csVector3Array& GetVertices () { return vertices; }
  
  /// Get camera vector array for this container.
  csVector3Array& GetCameraVertices () { return cam_vertices; }

  /// Add a polygon to this container.
  void AddPolygon (csPolygonInt* poly)
  {
    polygons.AddPolygon (poly);
    ((csBspPolygon*)poly)->SetParent (this);
  }

  /// Get the number of polygons in this polygonset.
  int GetNumPolygons () { return polygons.GetNumPolygons (); }

  /// Get the specified polygon from this set.
  csPolygonInt* GetPolygon (int idx) { return polygons.GetPolygon (idx); }

  /// Get the array of polygons.
  csPolygonInt** GetPolygons () { return polygons.GetPolygons (); }

  /// Transform the vertices of this container from world to camera.
  void World2Camera (const csTransform& trans);
};

#endif /*BSPBBOX_H*/
