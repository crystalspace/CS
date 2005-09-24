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

#include "csextern.h"

#include "csgeom/box.h"

#include "csutil/blockallocator.h"
#include "csutil/ref.h"
#include "csutil/scfstr.h"
#include "csutil/scf_implementation.h"

#include "iutil/dbghelp.h"

/**\file
 * KD-tree implementation.
 */
/**\addtogroup geom_utils
 * @{ */

struct iGraphics3D;
struct iString;
class csKDTree;
class csKDTreeChild;

/**
 * If you implement this interface then you can give that to the
 * KDtree. The KDtree can then use this to find the description of an object.
 * This can be used for debugging as the KDtree will print out that description
 * if it finds something is wrong.
 */
struct iKDTreeObjectDescriptor : public virtual iBase
{
  SCF_INTERFACE (iKDTreeObjectDescriptor, 0, 0, 1);

  virtual csPtr<iString> DescribeObject (csKDTreeChild* child) = 0;
};

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
 * <p>
 * 'frustum_mask' can be modified by this function to reduce the number
 * of plane tests (for frustum culling) that have to occur for children
 * of this node.
 */
typedef bool (csKDTreeVisitFunc)(csKDTree* treenode, void* userdata,
	uint32 timestamp, uint32& frustum_mask);

/**
 * A child in the KD-tree (usually some object).
 */
class CS_CRYSTALSPACE_EXPORT csKDTreeChild
{
private:
  friend class csKDTree;

  csBox3 bbox;
  void* object;			// Pointer back to the original object.
  csKDTree** leafs;		// Leafs that contain this object.
  int num_leafs;
  int max_leafs;

public:
  uint32 timestamp;		// Timestamp of last visit to this child.

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

  /**
   * Find leaf.
   */
  int FindLeaf (csKDTree* leaf);

  /**
   * Get the bounding box of this object.
   */
  inline const csBox3& GetBBox () const { return bbox; }

  /**
   * Get the pointer to the black box object.
   */
  inline void* GetObject () const { return object; }
};

enum
{
  CS_KDTREE_AXISINVALID = -1,
  CS_KDTREE_AXISX = 0,
  CS_KDTREE_AXISY = 1,
  CS_KDTREE_AXISZ = 2
};

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
class CS_CRYSTALSPACE_EXPORT csKDTree :
  public scfImplementation1<csKDTree, iDebugHelper>
{
public:
  // This is used for debugging.
  csRef<iKDTreeObjectDescriptor> descriptor;
  void DumpObject (csKDTreeChild* object, const char* msg);
  void DumpNode ();
  void DumpNode (const char* msg);
  void DebugExit ();

private:
  static csBlockAllocator<csKDTree> tree_nodes;
  static csBlockAllocator<csKDTreeChild> tree_children;

  csKDTree* child1;		// If child1 is not 0 then child2 will
  csKDTree* child2;		// also be not 0.
  csKDTree* parent;		// 0 if this is the root.

  csRef<iBase> userobject;	// An optional user object for this node.

  csBox3 node_bbox;		// Bbox of the node itself.

  int split_axis;		// One of CS_KDTREE_AXIS?
  float split_location;		// Where is the split?

  // Objects in this node. If this node also has children (child1
  // and child2) then the objects here have to be moved to these
  // children. The 'Distribute()' function will do that.
  csKDTreeChild** objects;
  int num_objects;
  int max_objects;

  // Estimate of the total number of objects in this tree including children.
  int estimate_total_objects;

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
  void AddObject (csKDTreeChild* obj);
  /// Physically remove a child from this tree node.
  void RemoveObject (int idx);
  /// Find an object. Returns -1 if not found.
  int FindObject (csKDTreeChild* obj);

  /**
   * Add an object to this kd-tree node.
   */
  void AddObject (const csBox3& bbox, csKDTreeChild* obj);

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
  float FindBestSplitLocation (int axis, float& split_loc);

  /**
   * If this node is a leaf then we will split the objects currently
   * in this leaf according to the pre-filled in split_axis
   * and split_location.
   */
  void DistributeLeafObjects ();

  /**
   * Traverse the tree from front to back. Every node of the
   * tree will be encountered. Returns false if traversal should stop.
   * The mask parameter is optionally used for frustum checking.
   * Front2Back will pass it to the tree nodes.
   */
  void Front2Back (const csVector3& pos, csKDTreeVisitFunc* func,
  	void* userdata, uint32 cur_timestamp, uint32 frustum_mask);

  /**
   * Traverse the tree in undefined order. Every node of the
   * tree will be encountered. Returns false if traversal should stop.
   * The mask parameter is optionally used for frustum checking.
   * Front2Back will pass it to the tree nodes.
   */
  void TraverseRandom (csKDTreeVisitFunc* func,
  	void* userdata, uint32 cur_timestamp, uint32 frustum_mask);

  /**
   * Reset timestamps of all objects in this treenode.
   */
  void ResetTimestamps ();

  /**
   * Flatten the children of this node to the given node.
   */
  void FlattenTo (csKDTree* node);

public:
  /// Create a new empty KD-tree.
  csKDTree ();
  /// Destroy the KD-tree.
  virtual ~csKDTree ();
  /// Set the parent.
  void SetParent (csKDTree* p) { parent = p; }

  /// For debugging: set the object descriptor.
  void SetObjectDescriptor (iKDTreeObjectDescriptor* descriptor)
  {
    csKDTree::descriptor = descriptor;
  }

  /// Make the tree empty.
  void Clear ();

  /// Get the user object attached to this node.
  inline iBase* GetUserObject () const { return userobject; }

  /**
   * Set the user object for this node. Can be 0 to clear
   * it. The old user object will be DecRef'ed and the (optional)
   * new one will be IncRef'ed.
   */
  void SetUserObject (iBase* userobj);

  /**
   * Add an object to this kd-tree node.
   * Returns a csKDTreeChild pointer which represents the object
   * inside the kd-tree. Object addition is delayed. This function
   * will not yet alter the structure of the kd-tree. Distribute()
   * will do that.
   */
  csKDTreeChild* AddObject (const csBox3& bbox, void* object);

  /**
   * Unlink an object from the kd-tree. The 'csKDTreeChild' instance
   * will NOT be deleted.
   */
  void UnlinkObject (csKDTreeChild* object);

  /**
   * Remove an object from the kd-tree. The 'csKDTreeChild' instance
   * will be deleted.
   */
  void RemoveObject (csKDTreeChild* object);

  /**
   * Move an object (give it a new bounding box).
   */
  void MoveObject (csKDTreeChild* object, const csBox3& new_bbox);

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
   * Traverse the tree in random order.
   * The mask parameter is optionally used for frustum checking.
   * TraverseRandom will pass it to the tree nodes.
   */
  void TraverseRandom (csKDTreeVisitFunc* func,
  	void* userdata, uint32 frustum_mask);

  /**
   * Traverse the tree from front to back. Every node of the
   * tree will be encountered.
   * The mask parameter is optionally used for frustum checking.
   * Front2Back will pass it to the tree nodes.
   */
  void Front2Back (const csVector3& pos, csKDTreeVisitFunc* func,
  	void* userdata, uint32 frustum_mask);

  /**
   * Start a new traversal. This will basically make a new
   * timestamp and return it. You can then use that timestamp
   * to check if objects have been visited already. This function
   * is automatically called by Front2Back() but it can be useful
   * to call this if you plan to do a manual traversal of the tree.
   */
  uint32 NewTraversal ();

  /**
   * Get child one.
   */
  inline csKDTree* GetChild1 () const { return child1; }

  /**
   * Get child two.
   */
  inline csKDTree* GetChild2 () const { return child2; }

  /**
   * Return the number of objects in this node.
   */
  inline int GetObjectCount () const { return num_objects; }

  /**
   * Get the estimated total number of objects in this node and
   * all children. This is only an estimate as it isn't kept up-to-date
   * constantly but it should give a rough idea about the complexity
   * of this node.
   */
  inline int GetEstimatedObjectCount () { return estimate_total_objects; }

  /**
   * Return the array of objects in this node.
   */
  inline csKDTreeChild** GetObjects () const { return objects; }

  /**
   * Return the bounding box of the node itself (does not always contain
   * all children since children are not split by the tree).
   */
  inline const csBox3& GetNodeBBox () const { return node_bbox; }

  // Debugging functions.
  bool Debug_CheckTree (csString& str);
  csPtr<iString> Debug_UnitTest ();
  void Debug_Dump (csString& str, int indent);
  void Debug_Statistics (int& tot_objects,
	int& tot_nodes, int& tot_leaves, int depth, int& max_depth,
	float& balance_quality);
  csPtr<iString> Debug_Statistics ();
  csTicks Debug_Benchmark (int num_iterations);

  virtual int GetSupportedTests () const
  {
    return CS_DBGHELP_UNITTEST | CS_DBGHELP_STATETEST |
      CS_DBGHELP_TXTDUMP | CS_DBGHELP_BENCHMARK;
  }

  virtual csPtr<iString> UnitTest ()
  {
    return Debug_UnitTest ();
  }

  virtual csPtr<iString> StateTest ()
  {
    scfString* rc = new scfString ();
    if (!Debug_CheckTree (rc->GetCsString ()))
      return csPtr<iString> (rc);
    delete rc;
    return 0;
  }

  virtual csTicks Benchmark (int num_iterations)
  {
    return Debug_Benchmark (num_iterations);
  }

  virtual csPtr<iString> Dump ()
  {
    scfString* rc = new scfString ();
    Debug_Dump (rc->GetCsString (), 0);
    return csPtr<iString> (rc);
  }

  virtual void Dump (iGraphics3D* /*g3d*/) { }
  virtual bool DebugCommand (const char*) { return false; }
};

/** @} */

#endif // __CS_KDTREE_H__

