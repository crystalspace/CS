/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
  
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

#ifndef POL2D_H
#define POL2D_H

#include "csutil/scf.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/poly2d.h"
#include "igraph3d.h"

class csPolygon3D;
class csPolyPlane;
class csClipper;
struct iRenderView;
struct iGraphics2D;
struct iGraphics3D;

/**
 * The following class represents a 2D polygon (the 2D coordinates are
 * perspective corrected coordinates).<p>
 *
 * This class is used as the main driver for the engine pipeline.
 * The source Polygon is first converted to 2D using csPolygon3D::DoPerspective.
 */
class csPolygon2D : public csPoly2D
{
  friend class Dumper;

public:
  /**
   * Compute the perspective transformation of a 3D vertex and add it to the polygon.
   */
  void AddPerspective (const csVector3& v);

  /**
   * Compute the perspective transformation of a 3D vertex and add it to the polygon.
   * This version computes unit perspective correction for which aspect ratio
   * is one and shift_x and shift_y are zero.
   */
  void AddPerspectiveUnit (const csVector3& v);

  /**
   * Compute the perspective transformation of a 3D vertex and add it to the polygon.
   * This version computes perspective correction for a given aspect
   * ratio and given shift value.
   */
  void AddPerspectiveAspect (const csVector3& v, float ratio, float shift);

  /**
   * Draw the polygon (wireframe version).
   */
  void Draw (iGraphics2D* g2d, int col);
  
  /**
   * Draw a texture mapped polygon.
   * 'plane' should be a correctly transformed plane (transformed to camera
   * space).
   * 'poly' is only used for debugging. The plane and vertices are not used.
   */
  void DrawFilled (iRenderView* rview, csPolygon3D* poly, csPolyPlane* plane,
  	bool use_z_buf = false);

  /**
   * Z fill the Z buffer for this polygon.
   * Nothing else is rendered.
   */
  void FillZBuf (iRenderView* rview, csPolygon3D* poly, csPolyPlane* plane);

  /**
   * Add this polygon as a back or front polygon of a fog object.
   * NOTE! Don't forget to open the fog object first with g3d->OpenFogObject ();
   */
  void AddFogPolygon (iGraphics3D* g3d, csPolygon3D* poly, csPolyPlane* plane,
  	bool mirror, CS_ID id, int fog_type);
};

/**
 * Factory to create csPolygon2D objects.
 */
class csPolygon2DFactory : public csPoly2DFactory
{
public:
  /// A shared factory that you can use.
  static csPolygon2DFactory* SharedFactory();

  /// Create a poly2d.
  virtual csPoly2D* Create () { csPolygon2D* p = new csPolygon2D (); return (csPoly2D*)p; }
};


struct G3DPolygonDPFX;
struct csVertexStatus;

/**
 * Prepare a filled in G3DPolygonDPFX structure for drawing via
 * g3d->DrawPolygonFX
 */
extern void PreparePolygonFX (G3DPolygonDPFX* g3dpoly, csVector2* clipped_poly,
  int num_vertices, csVector2 *orig_triangle, bool gouraud);
extern void PreparePolygonFX2 (G3DPolygonDPFX* g3dpoly, csVector2* clipped_poly,
  int num_vertices, csVertexStatus* clipped_vtstats,
  int orig_num_vertices, bool gouraud);

/*
 * An element for the queue.
 */
struct csQueueElement
{
  csPolygon3D* poly3d;
  csPolygon2D* poly2d;
};

/**
 * A queue for polygon 2D objects to render at a later time.
 * This queue is useful to remember a set of 2D polygons efficiently
 * and play them back in front to back or back to front.
 * A queue is always allocated for a given maximum size. This size
 * is known in advance because we know for which polygons the queue
 * is going to be used.<br>
 * Note! The queue will never delete polygons from memory. You are
 * responsible for cleaning up the queue first.
 */
class csPolygon2DQueue
{
private:
  // The queue.
  csQueueElement* queue;

  // Maximum number of elements in the queue.
  int max_queue;

  // Current number of elements in the queue.
  int num_queue;

public:
  /// Construct a queue for a given maximum size.
  csPolygon2DQueue (int max_size);

  /// Destruct queue.
  ~csPolygon2DQueue ();

  /// Push a 2D polygon to the queue.
  void Push (csPolygon3D* poly3d, csPolygon2D* poly2d);

  /**
   * Pop last added 2D polygon from the queue.
   * Return false if there are no more 2D polygons.
   */
  bool Pop (csPolygon3D** poly3d, csPolygon2D** poly2d);
};


#endif /*POL2D_H*/
