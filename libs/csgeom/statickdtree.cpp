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

#define REALLY_BAD_SCORE 1000000

static void debugScore (csArray<csStaticKDTreeObject*> &items,
    float splitLocation, int axis)
{
  unsigned int leftCount = 0;
  unsigned int rightCount = 0;
  int bothCount = 0;

  csArray<csStaticKDTreeObject*>::Iterator it = items.GetIterator ();
  while (it.HasNext ())
  {
    csStaticKDTreeObject *obj = it.Next ();
    const csBox3& box = obj->GetBBox ();

    float min = box.Min (axis), max = box.Max (axis);
    if (max <= splitLocation)
      leftCount++;
    else if (min > splitLocation)
      rightCount++;
    else
      bothCount++;
  }

  leftCount += bothCount;
  rightCount += bothCount;

  printf ("analysis of split %f on %d\n", splitLocation, axis);
  printf ("  total: %d | l/r: %d/%d | both:  %d | preliminary score: %d\n",
      items.Length (), leftCount, rightCount, bothCount,
      abs (leftCount - rightCount) + bothCount);
  if (leftCount == items.Length () || rightCount == items.Length ())
    printf("  BAD SPLIT.\n");
  else
    printf("  Good split.\n");
}

// Evaluate a choice of splitting plane.
static int findScore (csArray<csStaticKDTreeObject*> &items,
    float splitLocation, int axis)
{
  unsigned int leftCount = 0;
  unsigned int rightCount = 0;
  int bothCount = 0;

  csArray<csStaticKDTreeObject*>::Iterator it = items.GetIterator ();
  while (it.HasNext ())
  {
    csStaticKDTreeObject *obj = it.Next ();
    const csBox3& box = obj->GetBBox ();

    float min = box.Min (axis), max = box.Max (axis);
    if (max <= splitLocation)
      leftCount++;
    else if (min > splitLocation)
      rightCount++;
    else
      bothCount++;
  }

  leftCount += bothCount;
  rightCount += bothCount;

  if (leftCount == items.Length () || rightCount == items.Length ())
    return REALLY_BAD_SCORE;
  else
    return abs (leftCount - rightCount) + bothCount;
}

// Tries to find a split location that works.  If nothing good is found, return
// false.
static bool findBestSplit (csArray<csStaticKDTreeObject*> &items,
    float& bestSplitLocation, int& bestAxis)
{
  // Pick an axis with the longest distance between the minimum coordinate and
  // maximum coordinate.  Assuming on average all objects have a similar
  // variance, this picks the axis that has the most space for good split
  // locations.

  csArray<csStaticKDTreeObject*>::Iterator it = items.GetIterator ();
  float xmin = 0, ymin = 0, zmin = 0;
  float xmax = 0, ymax = 0, zmax = 0;
  while (it.HasNext ()) 
  {
    csStaticKDTreeObject* obj = it.Next ();
    const csBox3& box = obj->GetBBox ();
    xmin = fmin (box.MinX (), xmin);
    ymin = fmin (box.MinY (), ymin);
    zmin = fmin (box.MinZ (), zmin);
    xmax = fmin (box.MaxX (), xmax);
    ymax = fmin (box.MaxY (), ymax);
    zmax = fmin (box.MaxZ (), zmax);
  }

  float xdist = xmax - xmin;
  float ydist = ymax - ymin;
  float zdist = zmax - zmin;
  float maxdist = fmax (xdist, fmax (ydist, zdist));

  if (xdist == maxdist) {
    bestAxis = CS_XAXIS;
  } else if (ydist == maxdist) {
    bestAxis = CS_YAXIS;
  } else {
    bestAxis = CS_ZAXIS;
  }

  // Look at min and max of every box.
  int bestScore = REALLY_BAD_SCORE;
  it = items.GetIterator ();
  while (it.HasNext ()) 
  {
    csStaticKDTreeObject *obj = it.Next ();
    const csBox3& box = obj->GetBBox ();
    float min = box.Min(bestAxis), max = box.Max(bestAxis);

    int score;
    
    score = findScore (items, min, bestAxis);  // min
    if (score < bestScore) 
    {
      bestSplitLocation = min;
      bestScore = score;
    }

    score = findScore (items, max, bestAxis);  // max
    if (score < bestScore) 
    {
      bestSplitLocation = max;
      bestScore = score;
    }
  }

  if (bestScore == REALLY_BAD_SCORE)
  {
    printf("\nWARNING:  Bad score.\n");
    debugScore (items, bestSplitLocation, bestAxis);
    return false;
  }
  else
    return true;
}


csStaticKDTree::csStaticKDTree (csArray<csStaticKDTreeObject*> &items)
{
  nodeData = NULL;

  // Determine if we need to keep recursing.
  bool recurse = false;
  if (items.Length () >= 10)
    recurse = findBestSplit (items, splitLocation, axis);
  else
    recurse = false;

  // Base case:  all items will go into leaf node.
  if (!recurse)
  {
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
  // Otherwise find a split location and axis, classify objects according to
  // them, and recurse.
  else {
    findBestSplit (items, splitLocation, axis);

    csArray<csStaticKDTreeObject*>::Iterator it = items.GetIterator ();
    csArray<csStaticKDTreeObject*> left;
    csArray<csStaticKDTreeObject*> right;

    while (it.HasNext ()) {
      csStaticKDTreeObject *obj = it.Next ();

      float min = getMin (axis, obj->box), max = getMax (axis, obj->box);
      if (max <= splitLocation)
        left.Push (obj);
      else if (min > splitLocation)
        right.Push (obj);
      else {
        left.Push (obj);
        right.Push (obj);
      }
    }

    // Ensure we aren't infinitely recursing.
    if (left.Length () == items.Length () ||
        right.Length () == items.Length ())
    {
      debugScore (items, splitLocation, axis);
      CS_ASSERT (false);
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
