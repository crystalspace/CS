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

#ifndef QUADTREE_H
#define QUADTREE_H

#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

// A quadtree node can be in three states: empty, full, or partial.
// If empty or full the state of the children does not matter.
#define CS_QUAD_EMPTY 0
#define CS_QUAD_FULL 1
#define CS_QUAD_PARTIAL 2

class csVector3;
class csQuadtree;

/**
 * A quadtree node.
 */
class csQuadtreeNode
{
  friend class csQuadtree;

private:
  /**
   * Children.
   * 0 = topleft, 1=topright, 2=bottomleft, 3=bottomright.
   */
  csQuadtreeNode* children[4];

  /**
   * Corners of this node.
   * These are 3D coordinates on the quadtree plane.
   */
  csVector3 corners[4];

  /// Center point for this node.
  csVector3 center;
  /// Contents state of this node.
  int state;

private:
  /// Make an empty quadtree node.
  csQuadtreeNode ();

  /**
   * Destroy this quadtree node.
   */
  ~csQuadtreeNode ();

  /// Set box.
  void SetBox (const csVector3& corner00, const csVector3& corner01,
  	const csVector3& corner10, const csVector3& corner11);

public:
  /// Get center.
  const csVector3& GetCenter () { return center; }

  /// Get corner.
  const csVector3& GetCorner (int idx) { return corners[idx]; }

  /// Get contents state.
  int GetState () { return state; }

  /// Set state.
  void SetState (int st) { state = st; }
};

/**
 * The quadtree.
 */
class csQuadtree
{
private:
  /// The root of the tree.
  csQuadtreeNode* root;

private:
  /// Build the tree with a given depth.
  void Build (csQuadtreeNode* node, const csVector3& corner00,
  	const csVector3& corner01, const csVector3& corner10,
	const csVector3& corner11, int depth);

  /// Insert a polygon in the node.
  bool InsertPolygon (csQuadtreeNode* node,
	csVector3* verts, int num_verts,
	bool i00, bool i01, bool i10, bool i11);
public:
  /**
   * Create an empty tree. Although a quadtree is a 2D structure
   * the corners are still given in 3D coordinates. The quadtree
   * generates a 2D tree on the plane defined by those corners.
   * 'corners' is an array of the four corners of the 2D quadtree
   * in 3D space.
   */
  csQuadtree (csVector3* corners, int depth);

  /**
   * Destroy the whole quadtree.
   */
  virtual ~csQuadtree ();

  /**
   * Make tree empty.
   */
  void MakeEmpty () { root->SetState (CS_QUAD_EMPTY); }

  /**
   * Is the tree full?
   */
  bool IsFull () { return root->GetState () == CS_QUAD_FULL; }

  /**
   * Insert a polygon into the quad-tree.
   * Return true if the tree was modified (i.e. if parts of the
   * polygon were visible.<p>
   * The polygon does not actually need to be a polygon. It can
   * be a general frustrum. Note that the frustrum is assumed
   * to start at (0,0,0).
   */
  bool InsertPolygon (csVector3* verts, int num_verts);
};

#endif /*QUADTREE_H*/

