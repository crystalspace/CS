/*
    Copyright (C) 2002 by W.C.A. Wijngaards

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

#include "terrfunc.h"
#include "terrvis.h"

/// for children (like in terrvis)
//#ifndef CS_QUAD_TOPLEFT
//#define CS_QUAD_TOPLEFT 0
//#define CS_QUAD_TOPRIGHT 1
//#define CS_QUAD_BOTLEFT 2
//#define CS_QUAD_BOTRIGHT 3
//#endif

/// for neighbors
#define CS_QUAD_TOP 0
#define CS_QUAD_RIGHT 1
#define CS_QUAD_BOT 2
#define CS_QUAD_LEFT 3

/**
 * Class that subdivides a piece of terrain into quads,
 * and triangulates it.
*/
class csTerrainQuadDiv
{
  /// parent node? (if any)
  csTerrainQuadDiv *parent;
  /// place in parent node (cs_quad_topleft, etc..)
  int parentplace;
  /// children (all NULL - no children)
  csTerrainQuadDiv *children[4];
  /// direct neighbors (some can be NULL if you like)
  csTerrainQuadDiv *neighbors[4];

  /// is this quaddiv subdivided into smaller pieces? (true if equal to num)
  int subdivided;
  /// max height change of this quad
  float dmax;
  /// also min / max height of this quad
  float min_height, max_height;


public:
  /// create tree of certain depth (0 = create leaf node).
  csTerrainQuadDiv(int depth);
  /// and destroy subtree  (please remove it from neighbors, set NULL)
  ~csTerrainQuadDiv();

  /// Add/Remove a neighbor from the tree - give direction and ptr(can be NULL)
  void SetNeighbor(int dir, csTerrainQuadDiv *neigh);
  /// Set a neighbor to NULL (it will be looked up next frame)
  void RemoveNeighbor(int dir);
  /** 
   * Get neighbor for a direction (will look it up if NULL is cached)
   * NULL means no neighbor (of same size) in that direction.
  */
  csTerrainQuadDiv* GetNeighbor(int dir);

  /// is this quad a leaf node? (nodes always have four or zero children)
  bool IsLeaf() const {return children[0] == NULL;}

  /** Compute dmaxes from scratch.
   * Pass height func, and borders of this quad in flat space
   */
  void ComputeDmax(iTerrainHeightFunction* height_func,
    float minx, float miny, float maxx, float maxy);
  /// get Dmax
  float GetDmax() const {return dmax;}
  /// get min height of quad
  float GetMinHeight() const {return min_height;}
  /// get max height of quad
  float GetMaxHeight() const {return max_height;}


  /** Compute LOD levels - which nodes are subdivided and which are not.
   *  Pass frame number (number unique to frame so that all nodes
   *  are cleared at start), and camera position.
   */
  void ComputeLOD(int framenum, const csVector3& campos,
    float minx, float miny, float maxx, float maxy);

  /// estimate nr of triangles needed (rough estimate, will be higher)
  int EstimateTris(int framenum);

  /** Triangulate this piece of terrain.
   *  Calls back with triangles, cb(userdata, t1, t2, t3).
   *  Pass frame number (number unique to frame so that all nodes
   *  are cleared at start)
   */
  void Triangulate(void (*cb)(void *, const csVector3&, const csVector3&,
    const csVector3&), void *userdata, int framenum,
    float minx, float miny, float maxx, float maxy);

  /**
   * returns true if this node has a neighbor whose LOD > this LOD.
   * false means all neighbors have less or equal detail.
   * pass framenum so it can assess subdivide of neighbors.
   */
  bool HaveMoreDetailedNeighbor(int framenum);

  /**
   *  triangulate along an edge, edge in direction given.
   *  center is not modified, oldv is the prev vertice to in the triangle fan.
   *  nextv is the last vertice on the edge.
   *  Note: call this on your neighbor
   */
  void TriEdge(int dir, void (*cb)(void *, const csVector3&, const csVector3&,
    const csVector3&), void *userdata, int framenum, 
    const csVector3& center, csVector3& oldv, const csVector3& nextv,
    float minx, float miny, float maxx, float maxy);

};

