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

class csStaticKDTreeObject 
{
public:
  friend class csStaticKDTree;

  csStaticKDTreeObject (const csBox3& box, void* object) 
  {
    csStaticKDTreeObject::box = box;
    csStaticKDTreeObject::object = object;
  }

  /// Get the bounding box of the object.
  const csBox3& GetBBox () const { return box; }
  /// Get the user data.
  void* GetObject () { return object; }

  uint32 timestamp;

private:
  csBox3 box;
  void* object;
  csArray<class csStaticKDTree*> leafs;
};

// TODO:  use a block allocator for storing leaves and nodes?
class csStaticKDTree 
{
  static uint32 globalTimestamp;
  static csArray<csStaticKDTreeObject*> emptyList;

public:
  csStaticKDTree (csStaticKDTree *parent, bool isChild1, int axis, 
        float splitLocation);
  csStaticKDTree (csStaticKDTree *parent, bool isChild1,
      csArray<csStaticKDTreeObject*> &items);
  csStaticKDTree (csArray<csStaticKDTreeObject*> &items);
  ~csStaticKDTree ();
  csStaticKDTreeObject* AddObject (const csBox3& bbox, void* userdata);
  void AddObject (csStaticKDTreeObject* object);
  void UnlinkObject (csStaticKDTreeObject* object);
  void RemoveObject (csStaticKDTreeObject* object);
  void MoveObject (csStaticKDTreeObject* object, const csBox3& bbox_new);
  const csStaticKDTree* GetChild1 () const { return child1; }
  const csStaticKDTree* GetChild2 () const { return child2; }
  csStaticKDTree* GetChild1 () { return child1; }
  csStaticKDTree* GetChild2 () { return child2; }
  void TraverseRandom (csStaticKDTreeVisitFunc* func, 
      void* userdata, uint32 frustum_mask) {
    TraverseRandom(func, userdata, NewTraversal(), frustum_mask);
  }
  void Front2Back (const csVector3& pos, csStaticKDTreeVisitFunc* func, 
      void* userdata, uint32 frustum_mask) {
    Front2Back(pos, func, userdata, NewTraversal(), frustum_mask);
  }
  void* GetNodeData () { return nodeData; }
  const void* GetNodeData () const { return nodeData; }
  void SetNodeData (void *ptr) { nodeData = ptr; }
  uint32 NewTraversal() { return globalTimestamp++; }
  int GetObjectCount() const
  { 
    return (IsLeafNode()) ? objects->Length() : 0;
  }
  csArray<csStaticKDTreeObject*>& GetObjects()
  {
    return (IsLeafNode()) ? *objects : emptyList;
  }
  const csBox3& GetNodeBBox() const {
    return nodeBBox;
  }
  csBox3& GetNodeBBox() {
    return nodeBBox;
  }
  float GetSplitLocation() const { return splitLocation; }
  int GetAxis() const { return axis; }

  bool IsLeafNode () const { return objects; }

  // WARNING:  not safe to call if not a leaf node.
//  csStaticKDTreeObject* operator[](int index) 
//  { 
//      return (*objects[i]);
//  }


protected:
  csStaticKDTree* child1;
  csStaticKDTree* child2;
  csBox3 nodeBBox;  // TODO:  calculate node bounding box
  void* nodeData;

  int axis;
  float splitLocation;

  // If no children, this node contains objects
  csArray<csStaticKDTreeObject*>* objects;

  void TraverseRandom (csStaticKDTreeVisitFunc* func, 
      void* userdata, uint32 cur_timestamp, uint32 frustum_mask);
  void Front2Back (const csVector3& pos, csStaticKDTreeVisitFunc* func, 
      void* userdata, uint32 cur_timestamp, uint32 frustum_mask);

  static float getMin (int axis, const csBox3& box) 
  {
    if (axis == CS_XAXIS) return box.MinX ();
    else if (axis == CS_YAXIS) return box.MinY ();
    else if (axis == CS_ZAXIS) return box.MinZ ();
    CS_ASSERT (false);
    return 0;
  }

  static float getMax(int axis, const csBox3& box) 
  {
    if (axis == CS_XAXIS) return box.MaxX ();
    else if (axis == CS_YAXIS) return box.MaxY ();
    else if (axis == CS_ZAXIS) return box.MaxZ ();
    CS_ASSERT (false);
    return 0;
  }

  void SanityCheck() 
  {
    if (IsLeafNode()) 
    {
      CS_ASSERT(!child1);
      CS_ASSERT(!child2);
      CS_ASSERT(objects);
    }
    else 
    {
      CS_ASSERT(child1);
      CS_ASSERT(child2);
      CS_ASSERT(!objects);
    }
  }

  void CalculateBBox();

};

#endif
