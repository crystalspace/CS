/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __CS_IMESH_HAZE_H__
#define __CS_IMESH_HAZE_H__

#include "csutil/scf.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/cscolor.h"
#include "csgeom/vector2.h"

struct iMaterialWrapper;

SCF_VERSION (iHazeHull, 0, 0, 1);

/**
 * A mesh specially meant for use by the haze.
 * This mesh must be a convex hull.
 * for example: cubes, boxes, spheres, cones, cylinders, pyramids.
 * A convex hull object can be defined as follows:
 *   from any point inside the object, all of the polygons are completely
 *   visible (none are obscured by other polygons from the convex hull).
 * The ordering of the vertices (vertice numbering) is important when
 * multiple hulls are used.
 * Also edges must be numbered, with a number for each undirected edge.
 * (i.e. a->b and b->a have the same edge number).
 * Polygons thus have N vertices and N edges.
 * For a particular polygon, the edges return their points in clockwise
 * ordering.
 */
struct iHazeHull : public iBase
{
  /// get the number of polygons
  virtual int GetPolygonCount() const = 0;
  /// get the total number of vertices
  virtual int GetVerticeCount() const = 0;
  /// get the total number of edges
  virtual int GetEdgeCount() const = 0;

  /// get a vertex by index
  virtual void GetVertex(csVector3& res, int vertex_idx) const = 0;
  /// get the two (unordered) points of an edge (by index)
  virtual void GetEdge(int edge_num, int& vertex_idx_1, int& vertex_idx_2)
    const = 0;

  /// get the number of vertices in a polygon
  virtual int GetPolVerticeCount(int polygon_num) const = 0;
  /// get vertexindex in a polygon (given vertex number in polygon)
  virtual int GetPolVertex(int polygon_num, int vertex_num) const = 0;
  /**
   * Get the edge index that starts at given vertex number in polygon
   * Also returns the start and end vertex-idx of the edge (in clockwise
   * order)
   */
  virtual int GetPolEdge(int polygon_num, int vertex_num, int& start_idx,
    int& end_idx) const = 0;
};

SCF_VERSION (iHazeHullBox, 0, 0, 1);

/**
 * A predefined hull.
 */
struct iHazeHullBox : public iBase
{
  /// get box settings, min and max
  virtual void GetSettings(csVector3& min, csVector3& max) = 0;
};

SCF_VERSION (iHazeHullCone, 0, 0, 1);

/**
 * A predefined hull.
 */
struct iHazeHullCone : public iBase
{
  /// get Cone settings, nr_sides, start, end and radii of those
  virtual void GetSettings(int &nr, csVector3& a, csVector3& b, float &ra,
        float &rb) = 0;
};

SCF_VERSION (iHazeHullCreation, 0, 0, 2);

/**
 * This interface is implemented by the haze factory in order to be able to
 * create the predefined haze hulls.
 */
struct iHazeHullCreation : public iBase
{
  /// create a predefined hull: a box given min and max.
  virtual csRef<iHazeHullBox> CreateBox(const csVector3& min,
    const csVector3& max) const = 0;
  /// create a predefined hull: a cone
  virtual csRef<iHazeHullCone> CreateCone(int nr_sides, const csVector3& start,
    const csVector3& end, float srad, float erad) const = 0;
};

SCF_VERSION (iHazeFactoryState, 0, 0, 1);

/**
 * This interface describes the API for the sprite factory mesh object.
 * When multiple hulls are used, they must have the same number of
 * vertices, vertices are taken to be numbered in the same ordering.
 * <p>
 * The factory also implements the iHazeHullCreation interface
 */
struct iHazeFactoryState : public iBase
{
  /// Set material of sprite.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of sprite.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;
  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;

  /// Set the point of origin, the center of the texture.
  virtual void SetOrigin(const csVector3& pos) = 0;
  /// Get the point of origin
  virtual const csVector3& GetOrigin() const = 0;

  /// Set the topmiddle point of the texture
  virtual void SetDirectional(const csVector3& pos) = 0;
  /// Get the topmiddle point of the texture
  virtual const csVector3& GetDirectional() const = 0;

  /// Get the number of layers of hulls.
  virtual int GetLayerCount() const = 0;
  /// add a new layer - increasing the layer count
  virtual void AddLayer(iHazeHull *hull, float scale) = 0;
  /// Set the convex hull to be used as layer. Increfs the hull.
  virtual void SetLayerHull(int layer, iHazeHull* hull) = 0;
  /// Get the convex hull used for layer.
  virtual iHazeHull* GetLayerHull(int layer) const = 0;
  /// Set the texture percentage used by a layer (total of 1.0 is max)
  virtual void SetLayerScale(int layer, float scale) = 0;
  /// Get the layer scale
  virtual float GetLayerScale(int layer) const = 0;
};

SCF_VERSION (iHazeState, 0, 0, 1);

/**
 * This interface describes the API for the sprite factory mesh object.
 * iHazeState inherits from iHazeFactoryState.
 */
struct iHazeState : public iHazeFactoryState
{
};

#endif // __CS_IMESH_HAZE_H__

