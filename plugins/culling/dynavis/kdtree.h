/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_KDTREE_H__
#define __CS_KDTREE_H__

#include "csgeom/box.h"
#include "csgeom/vector2.h"
#include "csgeom/math2d.h"
#include "csutil/scfstr.h"
#include "iutil/dbghelp.h"

struct iGraphics3D;
class csKDTree;

/**
 * A child in the KD-tree (usually some object).
 */
class csKDTreeChild
{
private:
  friend class csKDTree;

  csBox3 bbox;
  void* object;			// Pointer back to the original object.
  csKDTree** leafs;		// Leafs that contain this object.
  int num_leafs;
  int max_leafs;

public:
  csKDTreeChild ();
  ~csKDTreeChild ();

  /// Physically add a leaf to this child.
  void AddLeaf (csKDTree* leaf);
  /// Physically remove a leaf from this child.
  void RemoveLeaf (int idx);
  /// Physically remove a leaf from this child.
  void RemoveLeaf (csKDTree* leaf);
  /**
   * Replace a leaf with another one. This is more
   * efficient than doing RemoveLeaf/AddLeaf and it is
   * useful in many cases where you want to move a child
   * in the tree.
   */
  void ReplaceLeaf (csKDTree* old_leaf, csKDTree* new_leaf);
};

#define CS_KDTREE_AXISX 0
#define CS_KDTREE_AXISY 1
#define CS_KDTREE_AXISZ 2

/**
 * A KD-tree.
 * A KD-tree is a binary tree that organizes 3D space.
 * This implementation is dynamic. It allows moving, adding, and
 * removing of objects which will alter the tree dynamically.
 * The main purpose of this tree is to provide for an approximate
 * front to back ordering.
 */
class csKDTree : public iBase
{
private:
  csKDTree* child1;		// If child1 is not NULL then child2 will
  csKDTree* child2;		// also be not NULL.

  csBox3 tree_bbox;		// Bbox of all objects in this tree.

  int split_axis;		// One of CS_KDTREE_AXIS?
  float split_location;		// Where is the split?

  csKDTreeChild** objects;	// Objects in this node.
  int num_objects;
  int max_objects;

  /// Physically add a child to this tree node.
  void AddObject (csKDTreeChild* obj);
  /// Physically remove a child from this tree node.
  void RemoveObject (int idx);

  /**
   * Add an object to this kd-tree node.
   */
  void AddObject (const csBox3& bbox, csKDTreeChild* obj);

  /**
   * Add an object to this leaf assuming there is only
   * one object currently in this leaf and no children.
   * 'child_new' is the reference to either 'child1' or
   * 'child2' and will be the child containing the new
   * object. 'child_old' is the reference to the other one
   * and will be the child to which the current original
   * object is moved. The current object in this leaf
   * will be removed and this leaf will become a node.
   */
  void AddObjectToSingleChildLeaf (
	csKDTree*& child_new, csKDTree*& child_old,
	csKDTreeChild* obj_new);

  /**
   * Find the best split position for a given axis. This will
   * return a good position depending on tree balancing (i.e. try
   * to have as many objects left as right) and also minimizing the
   * number of objects that are cut. It will return a quality
   * value which is 0 for very bad and 1 for very good. It will
   * also return the location to split in the 'split_loc' value.
   */
  float FindBestSplitLocation (int axis, float& split_loc);

  /**
   * If this node is a leaf then we will split the objects currently
   * in this leaf according to the pre-filled in split_axis
   * and split_location.
   */
  void DistributeLeafObjects ();

public:
  /// Create a new empty KD-tree.
  csKDTree ();
  /// Destroy the KD-tree.
  virtual ~csKDTree ();

  /// Make the tree empty.
  void Clear ();

  SCF_DECLARE_IBASE;

  /**
   * Add an object to this kd-tree node.
   * Returns a csKDTreeChild pointer which represents the object
   * inside the kd-tree.
   */
  csKDTreeChild* AddObject (const csBox3& bbox, void* object);

  // Debugging functions.
  bool Debug_CheckTree (csString& str);
  iString* Debug_UnitTest ();
  void Debug_Dump (csString& str, int indent);

  struct DebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csKDTree);
    virtual int GetSupportedTests () const
    {
      return CS_DBGHELP_UNITTEST |
      	CS_DBGHELP_STATETEST | CS_DBGHELP_TXTDUMP;
    }
    virtual iString* UnitTest ()
    {
      return scfParent->Debug_UnitTest ();
    }
    virtual iString* StateTest ()
    {
      scfString* rc = new scfString ();
      if (!scfParent->Debug_CheckTree (rc->GetCsString ()))
        return rc;
      delete rc;
      return NULL;
    }
    virtual csTicks Benchmark (int /*num_iterations*/)
    {
      return 0;
    }
    virtual iString* Dump ()
    {
      scfString* rc = new scfString ();
      scfParent->Debug_Dump (rc->GetCsString (), 0);
      return rc;
    }
    virtual void Dump (iGraphics3D* /*g3d*/)
    {
    }
    virtual bool DebugCommand (const char*)
    {
      return false;
    }
  } scfiDebugHelper;
};

#endif // __CS_KDTREE_H__

