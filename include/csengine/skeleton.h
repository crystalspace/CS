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

#ifndef SKELETON_H
#define SKELETON_H

#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/transfrm.h"
#include "csobject/csobj.h"

class csFrame;
class csSkeletonLimbState;

/**
 * A limb in a skeletal system. The Crystal Space skeletal system
 * works by controlling the vertices in a frame. So we combine
 * frame based sprites and skeletal sprites.
 */
class csSkeletonLimb
{
private:
  /// Next in the list.
  csSkeletonLimb* next;

  /// The vertices from the parent sprite that it controls.
  int* vertices;

  /// The number of vertices to control.
  int num_vertices;

  /// Children of this limb.
  csSkeletonLimb* children;

  /// Bounding box in object space for this limb.
  csVector3 box_min, box_max;

protected:
  /// Update state information.
  void UpdateState (csSkeletonLimbState* limb);

public:
  /// Create an empty limb.
  csSkeletonLimb () : next (NULL), vertices (NULL), num_vertices (0), children (NULL) { }

  /// Destructor.
  virtual ~csSkeletonLimb ();

  /// Add a vertex index.
  void AddVertex (int v);

  /// Get the vertex index array.
  int* GetVertices () { return vertices; }

  /// Get the number of vertices.
  int GetNumVertices () { return num_vertices; }

  /// Get the bounding box.
  void GetBoundingBox (csVector3& b_min, csVector3& b_max) { b_min = box_min; b_max = box_max; }

  /// Add a child limb.
  void AddChild (csSkeletonLimb* child);

  /// Linked list.
  void SetNext (csSkeletonLimb* n) { next = n; }
  /// Linked list.
  csSkeletonLimb* GetNext () { return next; }

  /// Create a csSkeletonState from this csSkeleton.
  virtual csSkeletonLimbState* CreateState ();

  /// For LOD. Remap vertices.
  void RemapVertices (int* mapping);

  /**
   * Compute the object space bounding box for this limb.
   */
  void ComputeBoundingBox (csFrame* source);
};

/**
 * A connection. This is basicly a transformation with a limb that
 * is controlled by the transformation.
 */
class csSkeletonConnection : public csSkeletonLimb
{
private:
  /// A transformation with respect to the parent limb.
  csTransform trans;

public:
  /// Create an empty limb with an identity transformation.
  csSkeletonConnection () { }

  /// Create an empty limb with the given transformation.
  csSkeletonConnection (csTransform& tr) : trans (tr) { }

  /// Destructor.
  virtual ~csSkeletonConnection () { }

  /// Set the transformation.
  void SetTransformation (csTransform& tr) { trans = tr; }

  /// Get the transformation.
  csTransform& GetTransformation () { return trans; }

  /// Create a csSkeletonState from this csSkeleton.
  virtual csSkeletonLimbState* CreateState ();
};

/**
 * The base skeleton class (or the 'body').
 */
class csSkeleton : public csSkeletonLimb
{
public:
  /// Create an empty skeleton.
  csSkeleton () { }

  /// Destructor.
  virtual ~csSkeleton () { }

  /// Create a csSkeletonState from this csSkeleton.
  virtual csSkeletonLimbState* CreateState ();
};

/**
 * A limb in a skeletal system (state version, see csSkeletonState for
 * more information).
 */
class csSkeletonLimbState : public csObject
{
  friend class csSkeletonLimb;

private:
  /// Next in the list.
  csSkeletonLimbState* next;

  /// Pointer to original skeleton node.
  csSkeletonLimb* tmpl;

  /// The vertices from the parent sprite that it controls.
  int* vertices;

  /// The number of vertices to control.
  int num_vertices;

  /// Children of this limb.
  csSkeletonLimbState* children;

  /// Add a child limb.
  void AddChild (csSkeletonLimbState* child);

  /// Linked list.
  void SetNext (csSkeletonLimbState* n) { next = n; }

protected:
  /// Create an empty limb (protected constructor).
  csSkeletonLimbState () : next (NULL), vertices (NULL), num_vertices (0), children (NULL) { }

public:
  /// Destructor.
  virtual ~csSkeletonLimbState ();

  /**
   * Transform the vertices in the given frame to the destination frame.
   * This is a recursive function which traverses all limbs and creates
   * a combined transformation along the way.
   */
  virtual void Transform (const csTransform& tr, csFrame* source, csVector3* dest);

  /**
   * Calculate the real bounding box for the given state.
   */
  virtual void ComputeBoundingBox (const csTransform& tr, csVector3& bbox_min,
  	csVector3& bbox_max);

  /// Get first child.
  csSkeletonLimbState* GetChildren () { return children; }

  /// Get next sibling in list.
  csSkeletonLimbState* GetNext () { return next; }

  CSOBJTYPE;
};

/**
 * A connection (state version, see csSkeletonState for more information).
 * This is basicly a transformation with a limb that
 * is controlled by the transformation.
 */
class csSkeletonConnectionState : public csSkeletonLimbState
{
  friend class csSkeletonConnection;

private:
  /// A transformation with respect to the parent limb.
  csTransform trans;

protected:
  /// Create an empty limb with an identity transformation (protected constructor).
  csSkeletonConnectionState () { }

public:
  /// Destructor.
  virtual ~csSkeletonConnectionState () { }

  /// Transform the vertices in the given frame to the destination frame.
  virtual void Transform (const csTransform& tr, csFrame* source, csVector3* dest);

  /// Calculate the real bounding box for the given state.
  virtual void ComputeBoundingBox (const csTransform& tr, csVector3& bbox_min,
  	csVector3& bbox_max);

  /// Set the transformation.
  void SetTransformation (csTransform& tr) { trans = tr; }

  /// Get the transformation.
  csTransform& GetTransformation () { return trans; }

  CSOBJTYPE;
};

/**
 * Skeleton state. This class is the runtime version of a skeleton.
 * The csSkeleton classes are used in a template. They contain a
 * static description of the skeleton code. There is at most one csSkeleton
 * for every csSpriteTemplate. This structure is the run-time
 * version. There is one csSkeletonState for every csSprite3D which
 * uses a skeleton.
 */
class csSkeletonState : public csSkeletonLimbState
{
  friend class csSkeleton;

protected:
  /**
   * Create an empty skeleton.
   * The constructor is protected because it is csSkeleton which
   * creates instances of this class.
   */
  csSkeletonState () { }

public:
  /// Destructor.
  virtual ~csSkeletonState () { }

  /// Calculate the real bounding box for the given state.
  virtual void ComputeBoundingBox (const csTransform& tr, csVector3& bbox_min,
  	csVector3& bbox_max);

  CSOBJTYPE;
};

#endif /*SKELETON_H*/
