/*
    Copyright (C) 2011 by Jorrit Tyberghein

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
#include "csutil/sysfunc.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "csqint.h"
#include "csqsqrt.h"
#include "csgeom/math3d.h"
#include "csgeom/kdtreex.h"

//---------------------------------------------------------------------------
namespace CS
{
namespace Geometry
{

struct StaticContainer
{
  csBlockAllocator<KDTree> tree_nodes;
  csBlockAllocator<KDTreeChild> tree_children;
};
CS_IMPLEMENT_STATIC_VAR (TreeAlloc, StaticContainer, ());

KDTreeChild::KDTreeChild ()
{
  object = 0;
  num_leafs = 0;
  max_leafs = 2;
  leafs = new KDTree* [max_leafs];
  timestamp = 0;
}

KDTreeChild::~KDTreeChild ()
{
  delete[] leafs;
}

void KDTreeChild::AddLeaf (KDTree* leaf)
{
  if (num_leafs >= max_leafs)
  {
    max_leafs += 3;
    KDTree** new_leafs = new KDTree* [max_leafs];
    if (leafs && num_leafs > 0)
      memcpy (new_leafs, leafs, sizeof (KDTree*) * num_leafs);
    delete[] leafs;
    leafs = new_leafs;
  }

  leafs[num_leafs++] = leaf;
}

void KDTreeChild::RemoveLeaf (int idx)
{
  CS_ASSERT (idx >= 0 && idx < num_leafs);
  if (num_leafs == 1)
  {
    // Easy case.
    num_leafs = 0;
    return;
  }

  if (idx < num_leafs-1)
    memmove (&leafs[idx], &leafs[idx+1],
        sizeof (KDTree*) * (num_leafs-idx-1));
  num_leafs--;
}

void KDTreeChild::RemoveLeaf (KDTree* leaf)
{
  int i;
  for (i = 0 ; i < num_leafs ; i++)
  {
    if (leafs[i] == leaf)
    {
      RemoveLeaf (i);
      return;
    }
  }
  // We shouldn't be able to come here.
  csPrintfErr ("Something bad happened in KDTreeChild::RemoveLeaf!\n");
  if (leaf) leaf->DumpObject (this, "  Trying to remove leaf for: %s!\n");
  KDTree::DebugExit ();
}

void KDTreeChild::ReplaceLeaf (KDTree* old_leaf, KDTree* new_leaf)
{
  int i;
  for (i = 0 ; i < num_leafs ; i++)
  {
    if (leafs[i] == old_leaf)
    {
      leafs[i] = new_leaf;
      return;
    }
  }
  // We shouldn't be able to come here.
  csPrintfErr ("Something bad happened in KDTreeChild::ReplaceLeaf!\n");
  if (old_leaf) old_leaf->DumpObject (this,
        "  Trying to replace leaf for: %s!\n");
  KDTree::DebugExit ();
}

int KDTreeChild::FindLeaf (KDTree* leaf)
{
  int i;
  for (i = 0 ; i < num_leafs ; i++)
  {
    if (leafs[i] == leaf) return i;
  }
  return -1;
}

//---------------------------------------------------------------------------

uint32 KDTree::global_timestamp = 1;

#define KDTREE_MAX 100000.

KDTree::KDTree () : scfImplementationType(this)
{
  child1 = 0;
  child2 = 0;
  objects = 0;
  parent = 0;
  num_objects = max_objects = 0;
  disallow_distribute = 0;
  split_axis = CS_KDTREE_AXISINVALID;
  min_split_objects = 1;

  node_bbox.Set (-KDTREE_MAX, -KDTREE_MAX,
        -KDTREE_MAX, KDTREE_MAX,
        KDTREE_MAX, KDTREE_MAX);

  estimate_total_objects = 0;
}

KDTree::~KDTree ()
{
  Clear ();
}

void KDTree::SetUserObject (iKDTreeUserData* userobj)
{
  userobject = userobj;
}

void KDTree::Clear ()
{
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    objects[i]->RemoveLeaf (this);
    // Remove this object if there are no more leafs refering to it.
    if (objects[i]->num_leafs == 0)
      TreeAlloc()->tree_children.Free (objects[i]);
  }
  delete[] objects;
  objects = 0;
  num_objects = 0;
  max_objects = 0;
  if (child1)
  {
    TreeAlloc()->tree_nodes.Free (child1);
    child1 = 0;
  }
  if (child2)
  {
    TreeAlloc()->tree_nodes.Free (child2);
    child2 = 0;
  }
  disallow_distribute = 0;
  SetUserObject (0);
  estimate_total_objects = 0;
}

void KDTree::AddObject (KDTreeChild* obj)
{
  if (!((max_objects == 0) == (objects == 0)))
  {
    csPrintfErr ("AddObject failed!\n");
    DumpObject (obj, "  Trying to add object: %s!\n");
    DebugExit ();
  }

  if (num_objects >= max_objects)
  {
    max_objects += MIN (max_objects+2, 80);
    KDTreeChild** new_objects = new KDTreeChild* [max_objects];
    if (objects && num_objects > 0)
      memcpy (new_objects, objects, sizeof (KDTreeChild*) * num_objects);
    delete[] objects;
    objects = new_objects;
  }

  objects[num_objects++] = obj;
  estimate_total_objects++;
}

void KDTree::DebugExit ()
{
  fflush (stdout);
  fflush (stderr);
#ifdef CS_DEBUG
  CS_ASSERT (false);
#else
  exit (-1);
#endif
}

void KDTree::DumpObject (KDTreeChild* object, const char* msg)
{
  if (descriptor)
  {
    csRef<iString> d = descriptor->DescribeObject (object);
    if (d)
      csPrintfErr (msg, d->GetData ());
  }
}

void KDTree::DumpNode ()
{
  if (descriptor)
  {
    csPrintfErr ("  This node contains the following objects:\n");
    size_t i;
    for (i = 0 ; i < size_t (num_objects) ; i++)
      if (objects[i])
      {
        csRef<iString> d = descriptor->DescribeObject (objects[i]);
        if (d)
          csPrintfErr ("    %zd: %s\n", i, d->GetData ());
      }
  }
}

void KDTree::DumpNode (const char* msg)
{
  csPrintfErr ("%s\n", msg);
  DumpNode ();
}

void KDTree::RemoveObject (int idx)
{
  if (idx < 0 && idx >= num_objects)
  {
    DumpNode ("Something bad happened in KDTree::RemoveObject!\n");
    DebugExit ();
  }
  estimate_total_objects--;
  if (num_objects == 1)
  {
    // Easy case.
    num_objects = 0;
    return;
  }

  if (idx < num_objects-1)
    memmove (&objects[idx], &objects[idx+1],
        sizeof (KDTreeChild*) * (num_objects-idx-1));
  num_objects--;
}

int KDTree::FindObject (KDTreeChild* obj)
{
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i] == obj)
      return i;
  }
  return -1;
}

float KDTree::FindBestSplitLocation (int axis, float& split_loc)
{
  int i, j;

  // If we have only two objects we use the average location between
  // the two objects.
  if (num_objects == 2)
  {
    const csSphere& bsphere0 = objects[0]->bsphere;
    const csSphere& bsphere1 = objects[1]->bsphere;
    float max0 = bsphere0.GetCenter ()[axis] + bsphere0.GetRadius ();
    float min1 = bsphere1.GetCenter ()[axis] - bsphere1.GetRadius ();
    if (max0 < min1-.01)        // Small threshold to avoid bad split location.
    {
      split_loc = max0 + (min1-max0) * 0.5;
      if (split_loc <= max0)
      {
        fprintf (stderr,
          "FindBestSplitLocation failed: split_loc(%g) <= max0(%g)\n",
          split_loc, max0);
        DumpNode ();
        DebugExit ();
      }
      if (split_loc >= min1)
      {
        fprintf (stderr,
          "FindBestSplitLocation failed: split_loc(%g) >= min1(%g)\n",
          split_loc, min1);
        DumpNode ();
        DebugExit ();
      }
      return 10.0;      // Good quality split.
    }
    float min0 = bsphere0.GetCenter ()[axis] - bsphere0.GetRadius ();
    float max1 = bsphere1.GetCenter ()[axis] + bsphere1.GetRadius ();
    if (max1 < min0-.01)
    {
      split_loc = max1 + (min0-max1) * 0.5;
      if (split_loc <= max1)
      {
        fprintf (stderr,
          "FindBestSplitLocation failed: split_loc(%g) <= max1(%g)\n",
          split_loc, max1);
        DumpNode ();
        DebugExit ();
      }
      if (split_loc >= min0)
      {
        fprintf (stderr,
          "FindBestSplitLocation failed: split_loc(%g) >= min0(%g)\n",
          split_loc, min0);
        DumpNode ();
        DebugExit ();
      }
      return 10.0;      // Good quality split.
    }
    return -1.0;                // Very bad quality split.
  }

  // Calculate minimum and maximum value along the axis.
  CS_ALLOC_STACK_ARRAY_FALLBACK (float, objectsMin, num_objects, 50000);
  CS_ALLOC_STACK_ARRAY_FALLBACK (float, objectsMax, num_objects, 50000);
  const csSphere& bs = objects[0]->bsphere;
  float mina = bs.GetCenter ()[axis]-bs.GetRadius ();
  objectsMin[0] = mina;
  float maxa = bs.GetCenter ()[axis]+bs.GetRadius ();
  objectsMax[0] = maxa;
  for (i = 1 ; i < num_objects ; i++)
  {
    const csSphere& bsphere = objects[i]->bsphere;
    float mi = bsphere.GetCenter ()[axis] - bsphere.GetRadius ();
    objectsMin[i] = mi;
    float ma = bsphere.GetCenter ()[axis] + bsphere.GetRadius ();
    objectsMax[i] = ma;
    if (mi < mina) mina = mi;
    if (ma > maxa) maxa = ma;
  }
  // Make sure we don't go outside node_box.
  if (mina < node_bbox.Min (axis)) mina = node_bbox.Min (axis);
  if (maxa > node_bbox.Max (axis)) maxa = node_bbox.Max (axis);

  // If mina and maxa are almost the same then reject.
  if (fabs (mina-maxa) < 0.0001f) return -1.0f;

# define FBSL_ATTEMPTS 5

  float best_qual = -2.0;
  float inv_num_objects = 1.0 / float (num_objects);

  for (int attempt = 0 ; attempt < FBSL_ATTEMPTS ; attempt++)
  {
    // Set 'a' to the middle. We will start to find a good split location from there.
    float a = (mina + maxa) / 2.0f;
    // Now count the number of objects that are completely
    // on the left and the number of objects completely on the right
    // side. The remaining objects are cut by this position.
    int left = 0;
    int right = 0;
    for (j = 0 ; j < num_objects ; j++)
    {
      float mi = objectsMin[j];
      float ma = objectsMax[j];
      // The .0001 is for safety.
      if (ma < a-.0001) left++;
      else if (mi > a+.0001) right++;
    }
    int cut = num_objects-left-right;
    // If we have no object on the left or right then this is a bad
    // split which we should never take.
    float qual;
    if (left == 0 || right == 0)
    {
      qual = -1.0;
    }
    else
    {
      float qual_cut = 1.0 - (float (cut) * inv_num_objects);
      float qual_balance = 1.0 - (float (ABS (left-right)) * inv_num_objects);
      qual = 3.0 * qual_cut + qual_balance;
    }
    if (qual > best_qual)
    {
      best_qual = qual;
      split_loc = a;
    }
    if (left <= right) maxa = a;
    else mina = a;
  }
# undef FBSL_ATTEMPTS
  return best_qual;
}

void KDTree::DistributeLeafObjects ()
{
  if (split_axis < CS_KDTREE_AXISX || split_axis > CS_KDTREE_AXISZ)
  {
    fprintf (stderr,
          "DistributeLeafObjects failed: split_axis=%d\n",
          split_axis);
    DumpNode ();
    DebugExit ();
  }
  for (int i = 0 ; i < num_objects ; i++)
  {
    const csSphere& bsphere = objects[i]->bsphere;
    float mi = bsphere.GetCenter ()[split_axis] - bsphere.GetRadius ();
    float ma = bsphere.GetCenter ()[split_axis] + bsphere.GetRadius ();
    bool leaf_replaced = false;
    // The SMALL_EPSILON is used to ensure that when bbox_min
    // is equal to bbox_max we don't get a situation where
    // both of the if's are not used.
    if (mi-SMALL_EPSILON <= split_location)
    {
      objects[i]->ReplaceLeaf (this, child1);
      leaf_replaced = true;
      child1->AddObject (objects[i]);
    }
    if (ma >= split_location)
    {
      if (leaf_replaced)
      {
        // If we also added this object to child1
        // we need to call AddLeaf() instead of ReplaceLeaf().
        objects[i]->AddLeaf (child2);
      }
      else
      {
        objects[i]->ReplaceLeaf (this, child2);
        leaf_replaced = true;
      }
      child2->AddObject (objects[i]);
    }
    if (!leaf_replaced)
    {
      DumpNode ("DistributeLeafObjects failed: !leaf_replaced\n");
      DebugExit ();
    }
  }

  num_objects = 0;
  // @@@ Clean up objects array if there are too many objects?
  // There should be some threshold at least.
}

void KDTree::AddObjectInt (KDTreeChild* obj)
{
  // Add this object to the list of objects to be distributed
  // later.
  if (disallow_distribute > 0)
    disallow_distribute--;
  obj->AddLeaf (this);
  AddObject (obj);
}

KDTreeChild* KDTree::AddObject (const csSphere& bsphere, void* object)
{
  KDTreeChild* obj = TreeAlloc()->tree_children.Alloc ();
  obj->object = object;
  obj->bsphere = bsphere;
  AddObjectInt (obj);
  return obj;
}

void KDTree::UnlinkObject (KDTreeChild* object)
{
  int i;
  for (i = 0 ; i < object->num_leafs ; i++)
  {
    KDTree* leaf = object->leafs[i];
    int idx = leaf->FindObject (object);
    if (idx == -1)
    {
      csPrintfErr ("UnlinkObject failed: idx == -1!\n");
      DumpObject (object, "  Trying to unlink object: %s!\n");
      DumpNode ();
      DebugExit ();
    }
    leaf->RemoveObject (idx);
    if (leaf->disallow_distribute > 0)
      leaf->disallow_distribute--;      // Give distribute a new chance.
  }
  object->num_leafs = 0;
}

void KDTree::RemoveObject (KDTreeChild* object)
{
  UnlinkObject (object);
  TreeAlloc()->tree_children.Free (object);
}

void KDTree::MoveObject (KDTreeChild* object, const csSphere& new_bsphere)
{
  const csVector3& newcenter = new_bsphere.GetCenter ();
  float newradius = new_bsphere.GetRadius ();

  // First check if the bounding box actually changed.
  csVector3 dc = object->bsphere.GetCenter () - newcenter;
  float dr = object->bsphere.GetRadius () - newradius;
  if ((dc < .00001f) && (dr < .00001f))
  {
    return;
  }

  // If the object is in only one leaf then we test if this object
  // will still be in the bounding box of that leaf after moving it around.
  if (object->num_leafs == 1)
  {
    const csBox3& nodeBox = object->leafs[0]->GetNodeBBox ();

    if (nodeBox.In (newcenter) ||
	csIntersect3::BoxSphere (nodeBox, newcenter, newradius))
    {
      // Even after moving we are still completely inside the bounding box
      // of the current leaf.
      object->bsphere = new_bsphere;
      if (object->leafs[0]->disallow_distribute > 0)
        object->leafs[0]->disallow_distribute--;
      return;
    }
  }

  object->bsphere = new_bsphere;

  // When updating the bounding box of an object we move the object upwards
  // in the tree until we find a node that completely contains the
  // new bounding box. For small movements this usually means that the
  // object will stay in its current node. Note that the case above already
  // catches that situation but we keep the above case because it is
  // slightly more efficient even.

  // The following counter makes sure we flatten the top-most parent
  // node every 50 times an object has moved. This ensures the tree
  // will keep reasonable quality. We don't do this every time because
  // Flatten() itself has some overhead.
  bool do_flatten = false;
#if 0
  static int cnt = 50;
  cnt--;
  if (cnt < 0)
  {
    cnt = 50;
    do_flatten = true;
  }
#endif

  KDTree* node = this;
  if (object->num_leafs > 0)
  {
    node = object->leafs[0];
    if (!do_flatten) UnlinkObject (object);
    const csBox3& nodeBox = node->GetNodeBBox ();
    while (node->parent && !(nodeBox.In (newcenter) ||
	csIntersect3::BoxSphere (nodeBox, newcenter, newradius)))
    {
      node = node->parent;
    }
    if (do_flatten)
      node->Flatten ();
    else
      node->AddObjectInt (object);
  }
}

void KDTree::Distribute ()
{
  // Check if there are objects to distribute or if distribution
  // is not allowed.
  if (num_objects == 0 || disallow_distribute > 0) return;

  CS_ASSERT ((child1 == 0) == (child2 == 0));
  if (child1)
  {
    // This node has children. So we have to see to what child (or both)
    // we distribute the objects in the this node.
    DistributeLeafObjects ();
    if (num_objects != 0)
    {
      DumpNode ("Distribute failed(1): distributing leaf objects!\n");
      DebugExit ();
    }

    // Update the bounding box of this node.
    estimate_total_objects = child1->GetEstimatedObjectCount ()
        + child2->GetEstimatedObjectCount ();
  }
  else
  {
    // This node doesn't have children yet.

    // If we don't have enough objects we do nothing.
    if (num_objects <= min_split_objects) return;

    // Here we have 2 or more objects.
    // We use FindBestSplitLocation() to see how we can best split this
    // node.
    float split_loc_x, split_loc_y, split_loc_z;
    float qual_x = FindBestSplitLocation (CS_KDTREE_AXISX, split_loc_x);
    float qual_y = FindBestSplitLocation (CS_KDTREE_AXISY, split_loc_y);
    float qual_z = FindBestSplitLocation (CS_KDTREE_AXISZ, split_loc_z);
    if (qual_x >= 0 && qual_x >= qual_y && qual_x >= qual_z)
    {
      split_axis = CS_KDTREE_AXISX;
      split_location = split_loc_x;
    }
    else if (qual_y >= 0 && qual_y >= qual_x && qual_y >= qual_z)
    {
      split_axis = CS_KDTREE_AXISY;
      split_location = split_loc_y;
    }
    else if (qual_z >= 0)
    {
      split_axis = CS_KDTREE_AXISZ;
      split_location = split_loc_z;
    }
    else
    {
      // No good quality split was found so we don't split. This
      // can happen if all objects are placed on top of each other.
      disallow_distribute = DISALLOW_DISTRIBUTE_TIME;
    }
    if (disallow_distribute == 0)
    {
      child1 = TreeAlloc()->tree_nodes.Alloc ();
      child1->SetParent (this);
      child1->min_split_objects = min_split_objects;
      child1->SetObjectDescriptor (descriptor);
      child2 = TreeAlloc()->tree_nodes.Alloc ();
      child2->SetParent (this);
      child2->min_split_objects = min_split_objects;
      child2->SetObjectDescriptor (descriptor);
      DistributeLeafObjects ();
      if (num_objects != 0)
      {
        DumpNode ("Distribute failed(2): distributing leaf objects!\n");
        DebugExit ();
      }
      // Update the bounding box of this node.
      child1->node_bbox = GetNodeBBox ();
      child1->node_bbox.SetMax (split_axis, split_location);
      child2->node_bbox = GetNodeBBox ();
      child2->node_bbox.SetMin (split_axis, split_location);
      estimate_total_objects = child1->GetEstimatedObjectCount ()
        + child2->GetEstimatedObjectCount ();
    }
    else
    {
      estimate_total_objects = num_objects;
    }
  }
}

void KDTree::FullDistribute ()
{
  Distribute ();
  if (child1)
  {
    child1->FullDistribute ();
    CS_ASSERT (child2 != 0);
    child2->FullDistribute ();
  }
}

void KDTree::FlattenTo (KDTree* node)
{
  if (!child1) return;  // Nothing to do.

  // First flatten the children.
  // @@@ Is this the most optimal solution?
  child1->FlattenTo (node);
  child2->FlattenTo (node);

  KDTree* c1 = child1; child1 = 0;
  KDTree* c2 = child2; child2 = 0;

  int i;
  for (i = 0 ; i < c1->num_objects ; i++)
  {
    KDTreeChild* obj = c1->objects[i];
    if (obj->num_leafs == 1)
    {
      if (obj->leafs[0] != c1)
      {
        csPrintfErr ("FlattenTo failed(1)!\n");
        DumpObject (obj, "  Processing object: %s!\n");
        DumpNode ();
        DebugExit ();
      }
      obj->leafs[0] = node;
      node->AddObject (obj);
    }
    else
    {
      if (obj->FindLeaf (node) == -1)
      {
        obj->ReplaceLeaf (c1, node);
        node->AddObject (obj);
      }
      else
      {
        obj->RemoveLeaf (c1);
      }
    }
  }
  for (i = 0 ; i < c2->num_objects ; i++)
  {
    KDTreeChild* obj = c2->objects[i];
    if (obj->num_leafs == 1)
    {
      if (obj->leafs[0] != c2)
      {
        csPrintfErr ("FlattenTo failed(2)!\n");
        DumpObject (obj, "  Processing object: %s!\n");
        DumpNode ();
        DebugExit ();
      }
      obj->leafs[0] = node;
      node->AddObject (obj);
    }
    else
    {
      if (obj->FindLeaf (node) == -1)
      {
        obj->ReplaceLeaf (c2, node);
        node->AddObject (obj);
      }
      else
      {
        obj->RemoveLeaf (c2);
      }
    }
  }
  delete[] c1->objects;
  c1->objects = 0;
  c1->num_objects = 0;
  c1->max_objects = 0;
  delete[] c2->objects;
  c2->objects = 0;
  c2->num_objects = 0;
  c2->max_objects = 0;
  TreeAlloc()->tree_nodes.Free (c1);
  TreeAlloc()->tree_nodes.Free (c2);
  estimate_total_objects = num_objects;
}

void KDTree::Flatten ()
{
  if (!child1) return;  // Nothing to do.

  disallow_distribute = 0;

  FlattenTo (this);
  return;
}

void KDTree::TraverseRandom (KDTreeVisitFunc* func,
        void* userdata, uint32 cur_timestamp, uint32 frustum_mask)
{
  CS_ASSERT (this != 0);
  if (!func (this, userdata, cur_timestamp, frustum_mask))
    return;
  if (child1)
  {
    // There are children.
    child1->TraverseRandom (func, userdata, cur_timestamp, frustum_mask);
    CS_ASSERT (child2 != 0);
    child2->TraverseRandom (func, userdata, cur_timestamp, frustum_mask);
  }
}

void KDTree::Front2Back (const csVector3& pos, KDTreeVisitFunc* func,
        void* userdata, uint32 cur_timestamp, uint32 frustum_mask)
{
  CS_ASSERT (this != 0);
  if (!func (this, userdata, cur_timestamp, frustum_mask))
    return;
  if (child1)
  {
    // There are children.
    if (pos[split_axis] <= split_location)
    {
      child1->Front2Back (pos, func, userdata, cur_timestamp, frustum_mask);
      CS_ASSERT (child2 != 0);
      child2->Front2Back (pos, func, userdata, cur_timestamp, frustum_mask);
    }
    else
    {
      child2->Front2Back (pos, func, userdata, cur_timestamp, frustum_mask);
      CS_ASSERT (child1 != 0);
      child1->Front2Back (pos, func, userdata, cur_timestamp, frustum_mask);
    }
  }
}

void KDTree::ResetTimestamps ()
{
  int i;
  for (i = 0 ; i < num_objects ; i++)
    objects[i]->timestamp = 0;
  if (child1)
  {
    child1->ResetTimestamps ();
    child2->ResetTimestamps ();
  }
}

uint32 KDTree::NewTraversal ()
{
  if (global_timestamp > 4000000000u)
  {
    // For safety reasons we will reset all timestamps to 0
    // for all objects in the tree and also set the global
    // timestamp to 1 again. This should be very rare (every
    // 4000000000 calls of Front2Back :-)
    ResetTimestamps ();
    global_timestamp = 1;
  }
  else
  {
    global_timestamp++;
  }
  return global_timestamp;
}

void KDTree::TraverseRandom (KDTreeVisitFunc* func,
        void* userdata, uint32 frustum_mask)
{
  NewTraversal ();
  TraverseRandom (func, userdata, global_timestamp, frustum_mask);
}

void KDTree::Front2Back (const csVector3& pos, KDTreeVisitFunc* func,
        void* userdata, uint32 frustum_mask)
{
  NewTraversal ();
  Front2Back (pos, func, userdata, global_timestamp, frustum_mask);
}

#define KDT_ASSERT_BOOL(test,msg) \
  if (!(test)) \
  { \
    csString ss; \
    ss.Format ("KDTree failure (%d,%s): %s\n", int(__LINE__), \
        #msg, #test); \
    str.Append (ss); \
    return rc; \
  }

#define KDT_ASSERT(test,msg) \
  if (!(test)) \
  { \
    csString ss; \
    ss.Format ("KDTree failure (%d,%s): %s\n", int(__LINE__), \
        #msg, #test); \
    str.Append (ss); \
    return csPtr<iString> (rc); \
  }

bool KDTree::Debug_CheckTree (csString& str)
{
  bool rc = false;

  KDT_ASSERT_BOOL ((child1 == 0) == (child2 == 0), "child consistency");

  if (child1)
  {
    //-------
    // Test-cases in case this is a node.
    //-------

    KDT_ASSERT_BOOL (split_axis >= CS_KDTREE_AXISX && split_axis <= CS_KDTREE_AXISZ,
        "axis");
    KDT_ASSERT_BOOL (GetNodeBBox ().Contains (child1->GetNodeBBox ()),
        "node_bbox mismatch");
    KDT_ASSERT_BOOL (GetNodeBBox ().Contains (child2->GetNodeBBox ()),
        "node_bbox mismatch");

    KDT_ASSERT_BOOL (split_location >= GetNodeBBox ().Min (split_axis),
        "split/node");
    KDT_ASSERT_BOOL (split_location <= GetNodeBBox ().Max (split_axis),
        "split/node");

    csBox3 new_node_bbox = child1->GetNodeBBox ();
    new_node_bbox += child2->GetNodeBBox ();
    KDT_ASSERT_BOOL (new_node_bbox == GetNodeBBox (), "node_bbox mismatch");
    KDT_ASSERT_BOOL (child1->parent == this, "parent check");
    KDT_ASSERT_BOOL (child2->parent == this, "parent check");

    if (!child1->Debug_CheckTree (str))
      return false;
    if (!child2->Debug_CheckTree (str))
      return false;
  }

  //-------
  // Test-cases in case this is a leaf (or not a leaf but has
  // objects waiting for distribution).
  //-------

  KDT_ASSERT_BOOL (num_objects <= max_objects, "object list");

  int i, j;
  for (i = 0 ; i < num_objects ; i++)
  {
    KDTreeChild* o = objects[i];

    KDT_ASSERT_BOOL (o->num_leafs <= o->max_leafs, "leaf list");
    int parcnt = 0;
    for (j = 0 ; j < o->num_leafs ; j++)
    {
      if (o->leafs[j] == this)
      {
        parcnt++;
        KDT_ASSERT_BOOL (parcnt <= 1, "parent occurs multiple times");
      }
    }
    KDT_ASSERT_BOOL (parcnt == 1, "leaf list doesn't contain parent");
  }

  return true;
}

static float rnd (float range)
{
  return float ((rand () >> 4) % 1000) * range / 1000.0;
}

// Number of objects we use for UnitTest().
#define CS_UNITTEST_OBJECTS 500

struct Debug_TraverseData
{
  int obj_counter;
  // The min_node_bbox array contains pointers to the node bounding
  // boxes we encounter before traversing further objects.
  const csBox3* min_node_bbox[CS_UNITTEST_OBJECTS*10];

  // The obj_bbox array contains pointers to the object bounding
  // boxes we encounter during traversing a node. Or 0
  // if this is a node entry.
  const csBox3* obj_bbox[CS_UNITTEST_OBJECTS*10];

  // Counter of the number of pointers in the above three arrays.
  int num_bbox_pointers;
};

#ifdef DEBUG_TRAVERSE_FUNC_UNUSED
static bool Debug_TraverseFunc (KDTree* treenode, void* userdata,
        uint32 cur_timestamp, uint32&)
{
  Debug_TraverseData* data = (Debug_TraverseData*)userdata;

  treenode->Distribute ();

  data->min_node_bbox[data->num_bbox_pointers] = &(treenode->GetNodeBBox ());
  data->obj_bbox[data->num_bbox_pointers] = 0;
  data->num_bbox_pointers++;
  CS_ASSERT (data->num_bbox_pointers < CS_UNITTEST_OBJECTS * 10);

  int num_objects = treenode->GetObjectCount ();
  KDTreeChild** objects = treenode->GetObjects ();
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      data->obj_counter++;

      data->min_node_bbox[data->num_bbox_pointers] = &(treenode->GetNodeBBox ());
      data->obj_bbox[data->num_bbox_pointers] = &(objects[i]->GetBBox ());
      data->num_bbox_pointers++;
      CS_ASSERT (data->num_bbox_pointers < CS_UNITTEST_OBJECTS * 10);
    }
  }

  return true;
}
#endif


static bool Debug_TraverseFuncBenchmark (KDTree* treenode, void*,
        uint32 cur_timestamp, uint32&)
{
  treenode->Distribute ();

  int num_objects = treenode->GetObjectCount ();
  KDTreeChild** objects = treenode->GetObjects ();
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
      objects[i]->timestamp = cur_timestamp;
  }

  return true;
}

csTicks KDTree::Debug_Benchmark (int num_iterations)
{
  int i, j;

  srand (12345678);

  csTicks pass0 = csGetTicks ();

  csSphere b;
  for (i = 0 ; i < num_iterations ; i++)
  {
    Clear ();
    for (j = 0 ; j < 500 ; j++)
    {
      float x = rnd (100.0)-50.0;
      float y = rnd (100.0)-50.0;
      float z = rnd (100.0)-50.0;
      float r = rnd (100.0)+0.5;
      b.SetCenter (csVector3 (x, y, z));
      b.SetRadius (r);
      AddObject (b, (void*)0);
      if (i % 20 == 0) FullDistribute ();
    }
  }

  csTicks pass1 = csGetTicks ();

  for (i = 0 ; i < num_iterations ; i++)
  {
    Front2Back (csVector3 (0, 0, 0), Debug_TraverseFuncBenchmark, 0, 0);
  }

  csTicks pass2 = csGetTicks ();

  for (i = 0 ; i < num_iterations ; i++)
  {
    Flatten ();
    FullDistribute ();
  }

  csTicks pass3 = csGetTicks ();

  for (i = 0 ; i < num_iterations ; i++)
  {
    Front2Back (csVector3 (0, 0, 0), Debug_TraverseFuncBenchmark, 0, 0);
  }

  csTicks pass4 = csGetTicks ();

  csPrintf ("Creating the tree:        %u ms\n", pass1-pass0);
  csPrintf ("Unoptimized Front2Back:   %u ms\n", pass2-pass1);
  csPrintf ("Flatten + FullDistribute: %u ms\n", pass3-pass2);
  csPrintf ("Optimized Front2Back:     %u ms\n", pass4-pass3);

  return pass4-pass0;
}

void KDTree::Debug_Statistics (int& tot_objects,
        int& tot_nodes, int& tot_leaves, int depth, int& max_depth,
        float& balance_quality)
{
  tot_objects += num_objects;
  if (child1) tot_nodes++;
  else tot_leaves++;
  depth++;
  if (depth > max_depth) max_depth = depth;
  if (child1)
  {
    int left = 0;
    int right = 0;
    child1->Debug_Statistics (left, tot_nodes,
        tot_leaves, depth, max_depth, balance_quality);
    child2->Debug_Statistics (right, tot_nodes,
        tot_leaves, depth, max_depth, balance_quality);
    tot_objects += left;
    tot_objects += right;

    float qual_balance = 1.0 - float (ABS (left-right)) / float (left+right);
    balance_quality += qual_balance;
  }
}

csPtr<iString> KDTree::Debug_Statistics ()
{
  scfString* rc = new scfString ();
  csString& str = rc->GetCsString ();

  int tot_objects, tot_nodes, tot_leaves, max_depth;
  float balance_quality;
  tot_objects = 0;
  tot_nodes = 0;
  tot_leaves = 0;
  max_depth = 0;
  balance_quality = 0.0;
  Debug_Statistics (tot_objects, tot_nodes, tot_leaves, 0, max_depth,
        balance_quality);
  str.Format ("#o=%d #n=%d #l=%d maxd=%d balqual=%g\n",
        tot_objects, tot_nodes, tot_leaves, max_depth,
        balance_quality / float (tot_nodes));

  return csPtr<iString> ((iString*)rc);
}

void KDTree::Debug_Dump (csString& str, int indent)
{
  char* spaces = new char[indent+1];
  char* s = spaces;
  int ind = indent;
  while (ind >= 10)
  {
    strcpy (s, "          ");
    s += 10;
    ind -= 10;
  }
  while (ind > 0) { *s++ = ' '; ind--; }
  *s = 0;

  csString ss;
  csRef<iString> stats = Debug_Statistics ();
  ss.Format ("%s KDT disallow_dist=%d\n%s     node_bbox=(%g,%g,%g)-(%g,%g,%g)\n%s %s",
        spaces,
        (int)disallow_distribute,
        spaces, GetNodeBBox ().MinX (), GetNodeBBox ().MinY (),
        GetNodeBBox ().MinZ (), GetNodeBBox ().MaxX (),
        GetNodeBBox ().MaxY (), GetNodeBBox ().MaxZ (),
        spaces, stats->GetData ());
  str.Append (ss);
  if (child1)
  {
    ss.Format ("%s   axis=%c loc=%g\n",
        spaces,
        split_axis == CS_KDTREE_AXISX ? 'x' :
        split_axis == CS_KDTREE_AXISY ? 'y' : 'z',
        split_location);
    str.Append (ss);
    child1->Debug_Dump (str, indent+2);
    child2->Debug_Dump (str, indent+2);
  }
  else
  {
    ss.Format ("%s   %d objects\n", spaces, num_objects);
    str.Append (ss);
  }
}

}
}

