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

#ifndef POL2D_H
#define POL2D_H

#include "cscom/com.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

class csPolygon3D;
class csPolyPlane;
class csClipper;
class csRenderView;
interface IGraphics2D;
interface IGraphics3D;

/**
 * The maximum number of vertices that any Polygon2D can have. This should
 * be considerably larger than what you expect to be the real maximum
 * number of vertices in a 3D polygon since clipping may add new vertices.
 * Note that 100 is probably overkill but this is no problem since there
 * are not going to be many of these Polygon2D objects anyway.
 */
#define MAX_VERTICES 100

/**
 * The following class represents a 2D polygon (the 2D coordinates are
 * perspective corrected coordinates).<p>
 *
 * This class is used as the main driver for the engine pipeline.
 * The source Polygon is first converted to 2D using csPolygon3D::DoPerspective.
 */
class csPolygon2D
{
  friend class Dumper;

private:
  /// The 2D vertices.
  csVector2* vertices;
  ///
  int num_vertices;
  ///
  int max_vertices;

  /// A 2D bounding box that is maintained automatically.
  csBox bbox;

public:
  /**
   * Make a new polygon.
   */
  csPolygon2D ();

  /// Copy constructor.
  csPolygon2D (csPolygon2D& copy);

  /// Destructor.
  virtual ~csPolygon2D ();

  /**
   * Initialize the csPolygon2D to empty.
   */
  void MakeEmpty ();

  /**
   * Get the number of vertices.
   */
  int GetNumVertices () { return num_vertices; }

  /**
   * Get the array with all vertices.
   */
  csVector2* GetVertices () { return vertices; }

  /**
   * Get the specified vertex.
   */
  csVector2* GetVertex (int i) 
  {
    if (i<0 || i>=num_vertices) return NULL;
    return &vertices[i];
  }

  /**
   * Get the first vertex.
   */
  csVector2* GetFirst ()
  { if (num_vertices<=0) return NULL;  else return vertices; }

  /**
   * Get the last vertex.
   */
  csVector2* GetLast ()
  { if (num_vertices<=0) return NULL;  else return &vertices[num_vertices-1]; }

  /**
   * Make room for at least the specified number of vertices.
   */
  void MakeRoom (int new_max);

  /**
   * Add a vertex (2D) to the polygon.
   */
  void AddVertex (const csVector2& v) { AddVertex (v.x, v.y); }

  /**
   * Add a vertex (2D) to the polygon.
   */
  void AddVertex (float x, float y);

  /**
   * Set all polygon vertices at once.
   */
  void SetVertices (csVector2 *v, int num)
  { memcpy (vertices, v, (num_vertices = num) * sizeof (csVector2)); }

  /**
   * Compute the perspective transformation of a 3D vertex and add it to the polygon.
   */
  void AddPerspective (csVector3& v) { AddPerspective (v.x, v.y, v.z); }

  /**
   * Compute the perspective transformation of a 3D vertex and add it to the polygon.
   */
  void AddPerspective (float x, float y, float z);

  /// Get the bounding box (in 2D space) for this polygon.
  csBox& GetBoundingBox () { return bbox; }

  /**
   * Clipping routines. They return false if the resulting polygon is not
   * visible for some reason.
   * Note that these routines must not be called if the polygon is not visible.
   * These routines will not check that.
   * Note that these routines will put the resulting clipped 2D polygon
   * in place of the original 2D polygon.
   */
  bool ClipAgainst (csClipper* view);

  /**
   * Draw the polygon (wireframe version).
   */
  void Draw (IGraphics2D* g2d, int col);
  
  /**
   * Draw a texture mapped polygon.
   * 'plane' should be a correctly transformed plane (transformed to camera
   * space).
   * 'poly' is only used for debugging. The plane and vertices are not used.
   */
  void DrawFilled (csRenderView* rview, csPolygon3D* poly, csPolyPlane* plane,
  	bool use_z_buf = false);

  /**
   * Add this polygon as a back or front polygon of a fog object.
   * NOTE! Don't forget to open the fog object first with g3d->OpenFogObject ();
   */
  void AddFogPolygon (IGraphics3D* g3d, csPolygon3D* poly, csPolyPlane* plane, bool mirror, CS_ID id, int fog_type);
};

struct G3DPolygonDPFX;

/**
 * Prepare a filled in G3DPolygonDPFX structure for drawing via
 * g3d->DrawPolygonFX
 */
extern void PreparePolygonFX (G3DPolygonDPFX* g3dpoly, csVector2* clipped_poly,
  int num_vertices, csVector2 *orig_triangle, bool gouraud);

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
