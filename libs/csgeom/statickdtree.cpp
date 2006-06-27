/*
    Copyright (C) 2006 by Benjamin Stover

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

#include "cssysdef.h"
#include "csgeom/statickdtree.h"

uint32 csStaticKDTree::globalTimestamp = 0;
csArray<csStaticKDTreeObject*> csStaticKDTree::emptyList;

csStaticKDTree::csStaticKDTree (csArray<csStaticKDTreeObject*> &items)
{
  nodeData = NULL;
  // Base case:  if there are less than a certain threshold, stop recursing.
  if (items.Length () < 10) {
    nodeBBox.StartBoundingBox();
    objects = new csArray<csStaticKDTreeObject*>(items);
    csArray<csStaticKDTreeObject*>::Iterator it = items.GetIterator ();
    while (it.HasNext ())
    {
      csStaticKDTreeObject* obj = it.Next ();
      nodeBBox += obj->box;
      obj->leafs.Push (this);
    }
    child1 = child2 = NULL;
  }
  // Otherwise find the axis with the longest min-max distance and use the 
  // midpoint as a splitting axis.
  else {
    csArray<csStaticKDTreeObject*>::Iterator it = items.GetIterator ();
    float xmin = 0, ymin = 0, zmin = 0;
    float xmax = 0, ymax = 0, zmax = 0;
    while (it.HasNext ()) {
      csStaticKDTreeObject & obj = *it.Next ();
      xmin = fmin (obj.box.MinX (), xmin);
      ymin = fmin (obj.box.MinY (), ymin);
      zmin = fmin (obj.box.MinZ (), zmin);
      xmax = fmin (obj.box.MaxX (), xmax);
      ymax = fmin (obj.box.MaxY (), ymax);
      zmax = fmin (obj.box.MaxZ (), zmax);
    }

    float xdist = xmax - xmin;
    float ydist = ymax - ymin;
    float zdist = zmax - zmin;
    float maxdist = fmax (xdist, fmax (ydist, zdist));

    if (xdist == maxdist) {
      splitLocation = (xmax + xmin) / 2;
      axis = CS_XAXIS;
    } else if (ydist == maxdist) {
      splitLocation = (ymax + ymin) / 2;
      axis = CS_YAXIS;
    } else {
      splitLocation = (zmax + zmin) / 2;
      axis = CS_ZAXIS;
    }

    it = items.GetIterator ();
    csArray < csStaticKDTreeObject * >left;
    csArray < csStaticKDTreeObject * >right;

    while (it.HasNext ()) {
      csStaticKDTreeObject *obj = it.Next ();

      float min = getMin (axis, obj->box), max = getMax (axis, obj->box);
      if (splitLocation < min)
        left.Push (obj);
      else if (splitLocation > max)
        right.Push (obj);
      else {
        left.Push (obj);
        right.Push (obj);
      }
    }

    objects = NULL;
    child1 = new csStaticKDTree (left);
    child2 = new csStaticKDTree (right);
    nodeBBox.StartBoundingBox();
    nodeBBox += child1->nodeBBox;
    nodeBBox += child2->nodeBBox;
  }
}

csStaticKDTree::csStaticKDTree (csStaticKDTree *parent, bool isChild1, 
    int axis, float splitLocation)
{
  if (parent != NULL) {
    CS_ASSERT(parent->objects == NULL);
    if (isChild1) {
      CS_ASSERT(parent->child1 == NULL);
      parent->child1 = this;
    }
    else {
      CS_ASSERT(parent->child2 == NULL);
      parent->child2 = this;
    }
  }

  objects = NULL;
  csStaticKDTree::axis = axis;
  csStaticKDTree::splitLocation = splitLocation;
  child1 = child2 = NULL;
  nodeData = NULL;
}

csStaticKDTree::csStaticKDTree (csStaticKDTree *parent, bool isChild1,
    csArray<csStaticKDTreeObject*> &items)
{
  if (parent != NULL) {
    CS_ASSERT(parent->objects == NULL);
    if (isChild1) {
      CS_ASSERT(parent->child1 == NULL);
      parent->child1 = this;
    }
    else {
      CS_ASSERT(parent->child2 == NULL);
      parent->child2 = this;
    }
  }

  objects = new csArray<csStaticKDTreeObject*> (items);
  csArray<csStaticKDTreeObject*>::Iterator it = items.GetIterator ();
  while (it.HasNext ())
      it.Next ()->leafs.Push (this);
  child1 = child2 = NULL;
  nodeData = NULL;
}

csStaticKDTree::~csStaticKDTree ()
{
  if (IsLeafNode ()) {
    CS_ASSERT (!child2);
    CS_ASSERT (objects);

    csArray<csStaticKDTreeObject*>::Iterator it = objects->GetIterator ();
    while (it.HasNext ()) {
      csStaticKDTreeObject* obj = it.Next ();
      obj->leafs.DeleteFast (this);
      if (obj->leafs.IsEmpty ())
        delete obj;
    }
    delete objects;
  }
  else {
    CS_ASSERT (child2);
    CS_ASSERT (!objects);
    delete child1;
    delete child2;
  }
//  delete nodeData;
}

csStaticKDTreeObject *csStaticKDTree::AddObject (const csBox3 &bbox, 
    void *userdata)
{
  csStaticKDTreeObject *ref = new csStaticKDTreeObject (bbox, userdata);
  AddObject (ref);
  return ref;
}

void csStaticKDTree::AddObject (csStaticKDTreeObject *object)
{
  nodeBBox += object->box;
  if (IsLeafNode ()) 
  {
    objects->Push (object);
    object->leafs.Push (this);
  } 
  else 
  {
    CS_ASSERT(child1);
    CS_ASSERT(child2);
    float min = getMin (axis, object->box), max = getMax (axis, object->box);
    if (max <= splitLocation) {
      child1->AddObject (object);
      child1->nodeBBox.SetMax(axis, splitLocation);
    }
    else if (min > splitLocation) {
      child2->AddObject (object);
      child2->nodeBBox.SetMin(axis, splitLocation);
    }
    else {
      child1->AddObject (object);
      child2->AddObject (object);
      child1->nodeBBox.SetMax(axis, splitLocation);
      child2->nodeBBox.SetMin(axis, splitLocation);
    }
  }
}

void csStaticKDTree::UnlinkObject (csStaticKDTreeObject * object)
{
  csArray < csStaticKDTree * >::Iterator it = object->leafs.GetIterator ();
  while (it.HasNext ()) {
    csStaticKDTree & t = *it.Next ();
    t.objects->DeleteFast (object);
  }
  object->leafs.DeleteAll ();
}

void csStaticKDTree::RemoveObject (csStaticKDTreeObject *object) {
  UnlinkObject(object);
  delete object;
}

void csStaticKDTree::TraverseRandom (csStaticKDTreeVisitFunc* func,
        void* userdata, uint32 cur_timestamp, uint32 frustum_mask)
{
  if (!func (this, userdata, cur_timestamp, frustum_mask))
    return;
  if (child1)  // Interior node
  {
    CS_ASSERT (child2 != 0);
    child1->TraverseRandom (func, userdata, cur_timestamp, frustum_mask);
    child2->TraverseRandom (func, userdata, cur_timestamp, frustum_mask);
  }
}

void csStaticKDTree::Front2Back (const csVector3& pos, 
    csStaticKDTreeVisitFunc* func, void* userdata, uint32 cur_timestamp, 
    uint32 frustum_mask)
{
  if (!func (this, userdata, cur_timestamp, frustum_mask))
    return;
  if (child1) // Interior node
  {
    if (pos[axis] <= splitLocation)
    {
      CS_ASSERT (child2 != 0);
      child1->Front2Back (pos, func, userdata, cur_timestamp, frustum_mask);
      child2->Front2Back (pos, func, userdata, cur_timestamp, frustum_mask);
    }
    else
    {
      CS_ASSERT (child1 != 0);
      child2->Front2Back (pos, func, userdata, cur_timestamp, frustum_mask);
      child1->Front2Back (pos, func, userdata, cur_timestamp, frustum_mask);
    }
  }
}

void csStaticKDTree::MoveObject (csStaticKDTreeObject *object,
                                 const csBox3 &bbox_new)
{
  // TODO:  optimization would be to check if new box is in same node
  UnlinkObject (object);
  object->box = bbox_new;
  AddObject (object);
}

void csStaticKDTree::CalculateBBox () 
{
  // TODO:  code is wrong--needs to reduce bounding box by axis every time
  if (IsLeafNode()) 
  {
    nodeBBox.StartBoundingBox();
    csArray<csStaticKDTreeObject*>::Iterator it = objects->GetIterator ();
    while (it.HasNext())
    {
      nodeBBox += it.Next()->box;
    }
  }
  else
  {
    child1->CalculateBBox();
    child2->CalculateBBox();
    nodeBBox.StartBoundingBox();
    nodeBBox += child1->nodeBBox;
    nodeBBox += child2->nodeBBox;
  }
}
