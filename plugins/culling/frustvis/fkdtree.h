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

#ifndef __CS_SIMPKDTREE_H__
#define __CS_SIMPKDTREE_H__

#include "csgeom/box.h"
#include "csgeom/vector2.h"
#include "csgeom/math2d.h"
#include "csutil/scfstr.h"

struct iGraphics3D;
class csSimpleKDTree;

/**
 * A callback function for visiting a KD-tree node. If this function
 * returns true the traversal will continue. Otherwise Front2Back()
 * will stop.
 * <p>
 * This function is itself responsible for calling Distribute() on
 * the given treenode to ensure that the objects in this node
 * are properly distributed to the children. If the function doesn't
 * want or need this functionality it doesn't have to do Distribute().
 * <p>
 * If this function decides to process the given node then it is
 * also responsible for checking the timestamp of every child in this
 * node with the timestamp given to this function. If this timestamp
 * is different the child has not been processed before. This function
 * should then update the timestamp of the child. If this is not done
 * then some objects will be encountered multiple times. In some
 * cases this may not be a problem or even desired.
 */
typedef bool (csSimpleKDTreeVisitFunc)(csSimpleKDTree* treenode,
	void* userdata, uint32 timestamp);

/**
 * A child in the KD-tree (usually some object).
 */
class csSimpleKDTreeChild
{
private:
  friend class csSimpleKDTree;

  csBox3 bbox;
  void* object;			// Pointer back to the original object.
  csSimpleKDTree** leafs;	// Leafs that contain this object.
  int num_leafs;
  int max_leafs;

public:
  uint32 timestamp;		// Timestamp of last visit to this child.

public:
  csSimpleKDTreeChild ();
  ~csSimpleKDTreeChild ();

  /// Physically add a leaf to this child.
  void AddLeaf (csSimpleKDTree* leaf);
  /// Physically remove a leaf from this child.
  void RemoveLeaf (int idx);
  /// Physically remove a leaf from this child.
  void RemoveLeaf (csSimpleKDTree* leaf);
  /**
   * Replace a leaf with another one. This is more
   * efficient than doing RemoveLeaf/AddLeaf and it is
   * useful in many cases where you want to move a child
   * in the tree.
   */
  void ReplaceLeaf (csSimpleKDTree* old_leaf, csSimpleKDTree* new_leaf);

  /**
   * Find leaf.
   */
  int FindLeaf (csSimpleKDTree* leaf);

  /**
   * Get the bounding box of this object.
   */
  const csBox3& GetBBox () const { return bbox; }

  /**
   * Get the pointer to the black box object.
   */
  void* GetObject () const { return object; }
};

#define CS_KDTREE_AXISINVALID -1
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
 * <p>
 * The KD-tree supports delayed insertion. When objects are inserted
 * in the tree they are not immediatelly distributed over the
 * nodes but instead they remain in the main node until it is really
 * needed to distribute them. The advantage of this is that you can
 * insert/remove a lot of objects in the tree and then do the distribution
 * calculation only once. This is more efficient and it also generates
 * a better tree as more information is available then.
 */
class csSimpleKDTree
{
private:
  csSimpleKDTree* child1;	// If child1 isn't NULL then child2 will
  csSimpleKDTree* child2;	// also be not NULL.
  csSimpleKDTree* parent;	// NULL if this is the root.

  bool obj_bbox_valid;		// If false obj_bbox is not valid.
  csBox3 obj_bbox;		// Bbox of all objects in this node.
  csBox3 node_bbox;		// Bbox of the node itself.

  int split_axis;		// One of CS_SIMPKDTREE_AXIS?
  float split_location;		// Where is the split?

  // Objects in this node. If this node also has children (child1
  // and child2) then the objects here have to be moved to these
  // children. The 'Distribute()' function will do that.
  csSimpleKDTreeChild** objects;
  int num_objects;
  int max_objects;

  // Disallow Distribute().
  // If this flag is true it means that we cannot find a good split
  // location for the current list of objects. So in that case we don't
  // split at all and set this flag to true so that we will no longer
  // attempt to distribute. Whenever objects are added or removed to this
  // node this flag will be set to false again so that a new Distribute()
  // attempt can be made. This situation should be rare though.
  bool disallow_distribute;

  // Current timestamp we are using for Front2Back(). Objects that
  // have the same timestamp are already visited during Front2Back().
  static uint32 global_timestamp;

  /// Physically add a child to this tree node.
  void AddObject (csSimpleKDTreeChild* obj);
  /// Physically remove a child from this tree node.
  void RemoveObject (int idx);
  /// Find an object. Returns -1 if not found.
  int FindObject (csSimpleKDTreeChild* obj);

  /**
   * Add an object to this kd-tree node.
   */
  void AddObject (const csBox3& bbox, csSimpleKDTreeChild* obj);

  /**
   * Find the best split position for a given axis. This will
   * return a good position depending on tree balancing (i.e. try
   * to have as many objects left as right) and also minimizing the
   * number of objects that are cut. It will return a quality
   * value which is 0 for very bad and positive for good. It will
   * also return the location to split in the 'split_loc' value.
   * If this function returns a negative quality this means the
   * split should not be performed at all.
   */
  int FindBestSplitLocation (int axis, float& split_loc);

  /**
   * If this node is a leaf then we will split the objects currently
   * in this leaf according to the pre-filled in split_axis
   * and split_location.
   */
  void DistributeLeafObjects ();

  /**
   * This function is called immediatelly after adding
   * one object with the given bbox. It will update the
   * bbox of the node correctly.
   */
  void UpdateBBox (const csBox3& bbox);

  /**
   * Traverse the tree from front to back. Every node of the
   * tree will be encountered. Returns false if traversal should stop.
   */
  bool Front2Back (const csVector3& pos, csSimpleKDTreeVisitFunc* func,
  	void* userdata, uint32 cur_timestamp);

  /**
   * Reset timestamps of all objects in this treenode.
   */
  void ResetTimestamps ();

public:
  /// Create a new empty KD-tree.
  csSimpleKDTree (csSimpleKDTree* parent);
  /// Destroy the KD-tree.
  virtual ~csSimpleKDTree ();

  /// Make the tree empty.
  void Clear ();

  /**
   * Add an object to this kd-tree node.
   * Returns a csSimpleKDTreeChild pointer which represents the object
   * inside the kd-tree. Object addition is delayed. This function
   * will not yet alter the structure of the kd-tree. Distribute()
   * will do that.
   */
  csSimpleKDTreeChild* AddObject (const csBox3& bbox, void* object);

  /**
   * Unlink an object from the kd-tree. The 'csSimpleKDTreeChild' instance
   * will NOT be deleted.
   */
  void UnlinkObject (csSimpleKDTreeChild* object);

  /**
   * Remove an object from the kd-tree. The 'csSimpleKDTreeChild' instance
   * will be deleted.
   */
  void RemoveObject (csSimpleKDTreeChild* object);

  /**
   * Move an object (give it a new bounding box).
   */
  void MoveObject (csSimpleKDTreeChild* object, const csBox3& new_bbox);

  /**
   * Distribute all objects in this node to its children.
   * This may also create new children if needed. Note that this
   * will only distribute one level (this node) and will not
   * recurse into the children.
   */
  void Distribute ();

  /**
   * Do a full distribution of this node and all children.
   */
  void FullDistribute ();

  /**
   * Do a full flatten of this node. This means that all
   * objects are put back in the object list of this node and
   * the KD-tree children are removed.
   */
  void Flatten ();

  /**
   * Traverse the tree from front to back. Every node of the
   * tree will be encountered.
   */
  void Front2Back (const csVector3& pos, csSimpleKDTreeVisitFunc* func,
  	void* userdata);

  /**
   * Return the number of objects in this node.
   */
  int GetObjectCount () const { return num_objects; }

  /**
   * Return the array of objects in this node.
   */
  csSimpleKDTreeChild** GetObjects () const { return objects; }

  /**
   * Return the bounding box of all objects in this node.
   */
  const csBox3& GetObjectBBox ();

  /**
   * Return the bounding box of the node itself (does not always contain
   * all children since children are not split by the tree).
   */
  const csBox3& GetNodeBBox () const { return node_bbox; }
};

#endif // __CS_SIMPKDTREE_H__

