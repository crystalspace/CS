/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CS_PVSTREE_H__
#define __CS_PVSTREE_H__

#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "csgeom/box.h"
#include "ivaria/pvstree.h"

class csPVSVis;
class csPVSVisObjectWrapper;

class csStaticPVSNode;

typedef bool (csPVSTreeVisitFunc)(csStaticPVSNode* treenode, void* userdata,
	uint32 timestamp, uint32& frustum_mask);

/**
 * A node in the KDTree for PVS.
 */
class csStaticPVSNode
{
public:
  csBox3 node_bbox;
  int axis;
  float where;
  csStaticPVSNode* child1, * child2;
  uint32 id;

  // Invisibility number. If equal to the current traversal number
  // this this node (and children) are invisible.
  uint32 invisible_number;
  
  // Array of invisible nodes as seen from this node.
  csArray<csStaticPVSNode*> invisible_nodes;

  // Array of objects in this node. @@@ Use a hashset?
  csArray<csPVSVisObjectWrapper*> objects;

public:
  csStaticPVSNode ();
  ~csStaticPVSNode ();

  /**
   * Return the bounding box of the node itself (does not always contain
   * all children since children are not split by the tree).
   */
  const csBox3& GetNodeBBox () const { return node_bbox; }

  /**
   * Set the box of this node and propagate it further down the tree
   * by splitting the box along the split axis.
   */
  void PropagateBBox (const csBox3& box);

  /**
   * Reset timestamps of all objects in this treenode.
   */
  void ResetTimestamps ();

  /**
   * Front to back node traversal.
   */
  void Front2Back (const csVector3& pos, csPVSTreeVisitFunc* func,
  	void* userdata, uint32 cur_timestamp, uint32 frustum_mask);

  /**
   * Random traversal.
   */
  void TraverseRandom (csPVSTreeVisitFunc* func,
  	void* userdata, uint32 cur_timestamp, uint32 frustum_mask);

  /**
   * Mark nodes invisible by filling the 'invisible_number' field
   * with the current timestamp.
   */
  void MarkInvisible (const csVector3& pos, uint32 cur_timestamp);

  /**
   * Add a dynamic object to the tree.
   */
  void AddObject (const csBox3& bbox, csPVSVisObjectWrapper* object);
  /**
   * Move a dynamic object in the tree.
   */
  void MoveObject (csPVSVisObjectWrapper* object, const csBox3& bbox);
};

/**
 * A Static KDTree for PVS.
 */
class csStaticPVSTree : public iStaticPVSTree
{
private:
  csPVSVis* pvsvis;

  csStaticPVSNode* root;	// @@@ TODO: allocator for tree!
  // 'id' is the index, the pointer to the node is the contents.
  csArray<csStaticPVSNode*> nodes_by_id;

  iObjectRegistry* object_reg;

  // Current timestamp we are using for Front2Back(). Objects that
  // have the same timestamp are already visited during Front2Back().
  static uint32 global_timestamp;

  // File name to write out this tree.
  csString pvscache;

  // Bounding box of the root.
  csBox3 root_box;

  // Create a node and assign an id to it.
  csStaticPVSNode* CreateNode ();
  // Create a node with a given id. This function will return
  // the node if there is already one there.
  csStaticPVSNode* CheckOrCreateNode (uint32 id);
  // Calculate the written size of this node and children.
  size_t CalculateSize (csStaticPVSNode* node);
  // Write out data.
  void WriteOut (char*& data, csStaticPVSNode* node);
  // Read data.
  const char* ReadPVS (char*& data, csStaticPVSNode*& node);

public:
  csStaticPVSTree ();
  virtual ~csStaticPVSTree ();

  void SetPVSVis (csPVSVis* pvsvis)
  {
    csStaticPVSTree::pvsvis = pvsvis;
  }

  void SetObjectRegistry (iObjectRegistry* o)
  {
    object_reg = o;
  }

  /**
   * Add a dynamic object to the tree.
   */
  void AddObject (const csBox3& bbox, csPVSVisObjectWrapper* object);
  /**
   * Remove a dynamic object.
   */
  void RemoveObject (csPVSVisObjectWrapper* object);
  /**
   * Move a dynamic object in the tree.
   */
  void MoveObject (csPVSVisObjectWrapper* object, const csBox3& bbox);

  /**
   * Reset timestamps of all objects in this treenode.
   */
  void ResetTimestamps ();

  /**
   * Start a new traversal. This will basically make a new
   * timestamp and return it. You can then use that timestamp
   * to check if objects have been visited already. This function
   * is automatically called by Front2Back() but it can be useful
   * to call this if you plan to do a manual traversal of the tree.
   */
  uint32 NewTraversal ();

  /**
   * Traverse the tree from front to back. Every node of the
   * tree will be encountered. Returns false if traversal should stop.
   * The mask parameter is optionally used for frustum checking.
   * Front2Back will pass it to the tree nodes.
   */
  void Front2Back (const csVector3& pos, csPVSTreeVisitFunc* func,
  	void* userdata, uint32 frustum_mask);

  /**
   * Traverse the tree randomly. Every node of the
   * tree will be encountered. Returns false if traversal should stop.
   * The mask parameter is optionally used for frustum checking.
   * Front2Back will pass it to the tree nodes.
   */
  void TraverseRandom (csPVSTreeVisitFunc* func,
  	void* userdata, uint32 frustum_mask);

  /**
   * Mark nodes invisible by filling the 'invisible_number' field
   * with the current timestamp.
   */
  void MarkInvisible (const csVector3& pos, uint32 cur_timestamp);

  csStaticPVSNode* GetRealRootNode () { return root; }

  SCF_DECLARE_IBASE;

  virtual void Clear ();
  virtual void* CreateRootNode ();
  virtual void* GetRootNode () { return root; }
  virtual void SplitNode (void* parent, int axis, float where,
  	void*& child1, void*& child2);
  virtual void* GetFirstChild (void* parent) const
  {
    return ((csStaticPVSNode*)parent)->child1;
  }
  virtual void* GetSecondChild (void* parent) const
  {
    return ((csStaticPVSNode*)parent)->child2;
  }
  virtual const csBox3& GetNodeBBox (void* node) const
  {
    return ((csStaticPVSNode*)node)->GetNodeBBox ();
  }
  virtual void GetAxisAndPosition (
  	void* node, int& axis, float& where) const
  {
    axis = ((csStaticPVSNode*)node)->axis;
    where = ((csStaticPVSNode*)node)->where;
  }
  virtual void MarkInvisible (void* source, void* target);
  virtual bool WriteOut ();

  /**
   * Set the bounding box for this PVS.
   */
  virtual void SetBoundingBox (const csBox3& bbox);
  virtual const csBox3& GetBoundingBox () const
  {
    return root_box;
  }
  virtual void UpdateBoundingBoxes ();

  /**
   * Clear PVS and read it from the data buffer.
   * Return 0 on success and otherwise a description of the error.
   */
  const char* ReadPVS (iDataBuffer* buf);

  /**
   * Clear PVS and read it from the cache.
   * Return 0 on success and otherwise a description of the error.
   */
  const char* ReadPVS ();

  /**
   * Set the PVS cache name.
   */
  void SetPVSCacheName (const char* pvscache);
};


#endif // __CS_PVSTREE_H__

