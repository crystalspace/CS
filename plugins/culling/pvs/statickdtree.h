/*
    Copyright (C) 2002 by Benjamin Stover

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

#ifndef __CS_STATICKDTREE_H
#define __CS_STATICKDTREE_H

#include "csextern.h"
#include "csutil/array.h"
#include "csgeom/box.h"


// TODO:  enumify and maybe add to cstypes?
#define CS_XAXIS 0
#define CS_YAXIS 1
#define CS_ZAXIS 2

class csStaticKDTreeObject;
class csStaticKDTree;

typedef bool (csStaticKDTreeVisitFunc)(csStaticKDTree* treenode, 
    void* userdata, uint32 timestamp, uint32& frustum_mask);

class csStaticKDTreeObject {
public:
  friend class csStaticKDTree;

  csStaticKDTreeObject (const csBox3& box, void* object) {
    csStaticKDTreeObject::box = box;
    csStaticKDTreeObject::object = object;
  }

  /// Get the bounding box of the object.
  const csBox3& GetBBox () const { return box; }
  /// Get the user data.
  void* GetObject () { return object; }

private:
  csBox3 box;
  void* object;
  csArray<class csStaticKDTree*> leafs;
};

// TODO:  use a block allocator for storing leaves and nodes?
class csStaticKDTree {
  static uint32 globalTimestamp;

public:
  csStaticKDTree (csArray<csStaticKDTreeObject*>& items);
  ~csStaticKDTree ();
  csStaticKDTreeObject* AddObject (const csBox3& bbox, void* userdata);
  void UnlinkObject (csStaticKDTreeObject* object);
  void RemoveObject (csStaticKDTreeObject* object);
  void MoveObject (const csBox3& bbox_new, csStaticKDTreeObject* object);
  csStaticKDTree* getChild1 () { return child1; }
  csStaticKDTree* getChild2 () { return child2; }
  void TraverseRandom (csStaticKDTreeVisitFunc* func, 
      void* userdata, uint32 cur_timestamp, uint32 frustum_mask);
  void Front2Back (const csVector3& pos, csStaticKDTreeVisitFunc* func, 
      void* userdata, uint32 cur_timestamp, uint32 frustum_mask);
  void* GetNodeData () { return nodeData; }
  void SetNodeData (void *ptr) { nodeData = ptr; }

private:
  csStaticKDTree* child1;
  csStaticKDTree* child2;
  csBox3 nodeBBox;  // TODO:  calculate node bounding box
  void* nodeData;

  int axis;
  float splitLocation;

  // If no children, this node contains objects
  csArray<csStaticKDTreeObject*>* objects;

  bool isLeafNode () { return child1; }

  static float getMin (int axis, const csBox3& box) {
    if (axis == CS_XAXIS) return box.MinX ();
    else if (axis == CS_YAXIS) return box.MinY ();
    else if (axis == CS_ZAXIS) return box.MinZ ();
    CS_ASSERT (false);
    return 0;
  }

  static float getMax(int axis, const csBox3& box) {
    if (axis == CS_XAXIS) return box.MaxX ();
    else if (axis == CS_YAXIS) return box.MaxY ();
    else if (axis == CS_ZAXIS) return box.MaxZ ();
    CS_ASSERT (false);
    return 0;
  }

  void AddObject (csStaticKDTreeObject* object);
};

#endif
