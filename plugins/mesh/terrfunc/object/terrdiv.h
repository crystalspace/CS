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
  /// children (all 0 - no children)
  csTerrainQuadDiv *children[4];
  /// direct neighbors (some can be 0 if you like)
  csTerrainQuadDiv *neighbors[4];

  /// is this quaddiv subdivided into smaller pieces? (true if equal to num)
  int subdivided;
  /// max height change of this quad
  float dmax;
  /// also min / max height of this quad
  float min_height, max_height;

  /// visibility check quad to use (if any);
  csTerrainQuad *visquad;

  /// cached heights for corners and middle
  float corner_height[4], middle_height;

  /// cached texture coordinates for corners and middle
  csVector2 corner_texuv[4], middle_texuv;

  /// cached lighting colors for corners and middle
  csColor corner_color[4], middle_color;

  /// cached normal directions for corners and middle
  csVector3 corner_normal[4], middle_normal;

public:
  /// create tree of certain depth (0 = create leaf node).
  csTerrainQuadDiv(int depth);
  /// and destroy subtree  (please remove it from neighbors, set 0)
  ~csTerrainQuadDiv();

  /// Add/Remove a neighbor from the tree - give direction and ptr(can be 0)
  void SetNeighbor(int dir, csTerrainQuadDiv *neigh);
  /// Set a neighbor to 0 (it will be looked up next frame)
  void RemoveNeighbor(int dir);
  /** 
   * Get neighbor for a direction (will look it up if 0 is cached)
   * 0 means no neighbor (of same size) in that direction.
  */
  csTerrainQuadDiv* GetNeighbor(int dir);

  /** Get terrainquad for visibility for this node */
  csTerrainQuad *GetVisQuad();
  /** Set visibility node to use */
  void SetVisQuad(csTerrainQuad *vis) {visquad = vis;}

  /// is this quad a leaf node? (nodes always have four or zero children)
  bool IsLeaf() const {return children[0] == 0;}

  /** Compute dmaxes from scratch.
   * Pass height func, and borders of this quad in flat space
   * Also pass a function that compute texture coords. and its userdata.
   * And pass a normal computation func.
   */
  void ComputeDmax(iTerrainHeightFunction* height_func,
    void (*texuv_func)(void*, csVector2&, float, float), void *texdata,
    iTerrainNormalFunction *normal_func,
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
   *  Also pass a function to (re)compute lighting colors, and its data.
   */
  void ComputeLOD(int framenum, const csVector3& campos,
    void (*light_func)(void*, csColor&, float, float, const csVector3&), 
    void *lightdata, float minx, float miny, float maxx, float maxy);

  /// estimate nr of triangles needed (rough estimate, will be higher)
  int EstimateTris(int framenum);

  /** Triangulate this piece of terrain.
   *  Calls back with triangles, cb(userdata, t1, t2, t3, uv1, uv2, uv3,
   *    col1, col2, col3).
   *  Pass frame number (number unique to frame so that all nodes
   *  are cleared at start)
   */
  void Triangulate(void (*cb)(void *, const csVector3&, const csVector3&,
    const csVector3&, const csVector2&, const csVector2&, const csVector2&,
    const csColor&, const csColor&, const csColor&), void *userdata, 
    int framenum, float minx, float miny, float maxx, float maxy);

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
    const csVector3&, const csVector2&, const csVector2&, const csVector2&,
    const csColor&, const csColor&, const csColor&), void *userdata, 
    int framenum, const csVector3& center, csVector3& oldv, 
    const csVector3& nextv, const csVector2& center_uv, 
    csVector2& old_uv, const csVector2& next_uv, 
    const csColor& center_col, csColor& old_col, const csColor& next_col,
    float minx, float miny, float maxx, float maxy);

};

