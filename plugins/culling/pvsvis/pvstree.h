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
#include "csgeom/box.h"
#include "ivaria/pvstree.h"

/**
 * A node in the KDTree for PVS.
 */
struct csStaticPVSNode
{
  int axis;
  float where;
  csStaticPVSNode* child1, * child2;
  uint32 id;
  // Array of invisible nodes as seen from this node.
  csArray<csStaticPVSNode*> invisible_nodes;

  csStaticPVSNode ();
  ~csStaticPVSNode ();
};

/**
 * A Static KDTree for PVS.
 */
class csStaticPVSTree : public iStaticPVSTree
{
private:
  csBox3 root_box;
  csStaticPVSNode* root;	// @@@ TODO: allocator for tree!
  // 'id' is the index, the pointer to the node is the contents.
  csArray<csStaticPVSNode*> nodes_by_id;

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

  SCF_DECLARE_IBASE;

  virtual void Clear ();
  virtual void* CreateRootNode (const csBox3& box);
  virtual void* GetRootNode () { return 0; }
  virtual const csBox3& GetRootBox () const { return root_box; }
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
  virtual void GetAxisAndPosition (
  	void* node, int& axis, float& where) const
  {
    axis = ((csStaticPVSNode*)node)->axis;
    where = ((csStaticPVSNode*)node)->where;
  }
  virtual void MarkInvisible (void* source, void* target);
  virtual csPtr<iDataBuffer> WriteOut ();

  /**
   * Clear PVS and read it from the data buffer.
   * Return 0 on success and otherwise a description of the error.
   */
  const char* ReadPVS (iDataBuffer* buf);
};


#endif // __CS_PVSTREE_H__

