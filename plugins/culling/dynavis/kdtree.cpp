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
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "qint.h"
#include "qsqrt.h"
#include "csgeom/math3d.h"
#include "kdtree.h"

//---------------------------------------------------------------------------

csKDTreeChild::csKDTreeChild ()
{
  object = NULL;
  num_leafs = 0;
  max_leafs = 2;
  leafs = new csKDTree* [max_leafs];
}

csKDTreeChild::~csKDTreeChild ()
{
  delete[] leafs;
}

void csKDTreeChild::AddLeaf (csKDTree* leaf)
{
  if (num_leafs >= max_leafs)
  {
    max_leafs += 3;
    csKDTree** new_leafs = new csKDTree* [max_leafs];
    if (leafs && num_leafs > 0)
      memcpy (new_leafs, leafs, sizeof (csKDTree*) * num_leafs);
    delete[] leafs;
    leafs = new_leafs;
  }

  leafs[num_leafs++] = leaf;
}

void csKDTreeChild::RemoveLeaf (int idx)
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
    	sizeof (csKDTree*) * (num_leafs-idx-1));
  num_leafs--;
}

void csKDTreeChild::RemoveLeaf (csKDTree* leaf)
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
  CS_ASSERT (false);
}

void csKDTreeChild::ReplaceLeaf (csKDTree* old_leaf, csKDTree* new_leaf)
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
  CS_ASSERT (false);
}

int csKDTreeChild::FindLeaf (csKDTree* leaf)
{
  int i;
  for (i = 0 ; i < num_leafs ; i++)
  {
    if (leafs[i] == leaf) return i;
  }
  return -1;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csKDTree)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csKDTree::DebugHelper)
  SCF_IMPLEMENTS_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csKDTree::csKDTree ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);

  child1 = NULL;
  child2 = NULL;
  objects = NULL;
  num_objects = max_objects = 0;
  tree_bbox.StartBoundingBox ();
  disallow_distribute = false;
}

csKDTree::~csKDTree ()
{
  Clear ();
}

void csKDTree::Clear ()
{
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    objects[i]->RemoveLeaf (this);
    // Remove this object if there are no more leafs refering to it.
    if (objects[i]->num_leafs == 0)
      delete objects[i];
  }
  delete[] objects;
  objects = NULL;
  num_objects = 0;
  max_objects = 0;
  delete child1; child1 = NULL;
  delete child2; child2 = NULL;
  disallow_distribute = false;
  tree_bbox.StartBoundingBox ();
}

void csKDTree::AddObject (csKDTreeChild* obj)
{
  CS_ASSERT ((max_objects == 0) == (objects == NULL));
  if (num_objects >= max_objects)
  {
    max_objects += 3;
    csKDTreeChild** new_objects = new csKDTreeChild* [max_objects];
    if (objects && num_objects > 0)
      memcpy (new_objects, objects, sizeof (csKDTreeChild*) * num_objects);
    delete[] objects;
    objects = new_objects;
  }

  objects[num_objects++] = obj;
}

void csKDTree::RemoveObject (int idx)
{
  CS_ASSERT (idx >= 0 && idx < num_objects);
  if (num_objects == 1)
  {
    // Easy case.
    num_objects = 0;
    return;
  }

  if (idx < num_objects-1)
    memmove (&objects[idx], &objects[idx+1], num_objects-idx-1);
  num_objects--;
}

float csKDTree::FindBestSplitLocation (int axis, float& split_loc)
{
  int i, j;

//@@@@@@@@@@@@ Use 'int' type for quality!!!

  // If we have only two objects we use the average location between
  // the two objects.
  if (num_objects == 2)
  {
    const csBox3& bbox0 = objects[0]->bbox;
    const csBox3& bbox1 = objects[1]->bbox;
    float max0 = bbox0.Max (axis);
    float min1 = bbox1.Min (axis);
    if (max0 < min1-.01)	// Small thresshold to avoid bad split location.
    {
      split_loc = max0 + (min1-max0) * 0.5;
      CS_ASSERT (split_loc > max0);
      CS_ASSERT (split_loc < min1);
      return 1.0;	// Good quality split.
    }
    float min0 = bbox0.Min (axis);
    float max1 = bbox1.Max (axis);
    if (max1 < min0-.01)
    {
      split_loc = max1 + (min0-max1) * 0.5;
      CS_ASSERT (split_loc > max1);
      CS_ASSERT (split_loc < min0);
      return 1.0;	// Good quality split.
    }
    return -1.0;	// Very bad quality split.
  }

  // Calculate minimum and maximum value along the axis.
  float mina = objects[0]->bbox.Min (axis);
  float maxa = objects[0]->bbox.Max (axis);
  for (i = 1 ; i < num_objects ; i++)
  {
    const csBox3& bbox = objects[i]->bbox;
    if (bbox.Min (axis) < mina) mina = bbox.Min (axis);
    if (bbox.Max (axis) > maxa) maxa = bbox.Max (axis);
  }

  // Do 10 tests to find best split location. This should
  // probably be a configurable parameter.

  // @@@ Is the routine below very efficient?
# define FBSL_ATTEMPTS 10
  float a;
  float best_qual = -2.0;
  float inv_num_objects = 1.0 / float (num_objects);
  for (i = 0 ; i < FBSL_ATTEMPTS ; i++)
  {
    // Calculate a possible split location.
    a = mina + float (i+1)*(maxa-mina)/float (FBSL_ATTEMPTS+1.0);
    // Now count the number of objects that are completely
    // on the left and the number of objects completely on the right
    // side. The remaining objects are cut by this position.
    int left = 0;
    int right = 0;
    for (j = 0 ; j < num_objects ; j++)
    {
      const csBox3& bbox = objects[j]->bbox;
      if (bbox.Max (axis) < a) left++;
      else if (bbox.Min (axis) > a) right++;
    }
    int cut = num_objects-left-right;
    // If we have left, right, or cut equal to the number
    // objects we have an extremely bad situation that we will not
    // allow to be used.
    float qual;
    if (cut == num_objects || left == num_objects || right == num_objects)
    {
      qual = -1.0;
    }
    else
    {
      float qual_cut = 1.0 - float (cut) * inv_num_objects;
      float qual_balance = 1.0 - float (ABS (left-right)) * inv_num_objects;
      // Currently we just give 'cut' and 'balance' quality an equal share.
      // We should consider if that is a good measure.
      qual = qual_cut * qual_balance;
    }
    if (qual > best_qual)
    {
      best_qual = qual;
      split_loc = a;
    }
  }
# undef FBSL_ATTEMPTS
  return best_qual;
}

void csKDTree::UpdateBBox (const csBox3& bbox)
{
  // This function assumes that the object is already
  // added to this node.
  if (num_objects > 1 || child1)
    tree_bbox += bbox;
  else
    tree_bbox = bbox;
}

void csKDTree::DistributeLeafObjects ()
{
  CS_ASSERT (split_axis >= CS_KDTREE_AXISX && split_axis <= CS_KDTREE_AXISZ);
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    const csBox3& bbox = objects[i]->bbox;
    float bbox_min = bbox.Min (split_axis);
    float bbox_max = bbox.Max (split_axis);
    bool leaf_replaced = false;
    if (bbox_min <= split_location)
    {
      objects[i]->ReplaceLeaf (this, child1);
      leaf_replaced = true;
      child1->AddObject (objects[i]);
      child1->UpdateBBox (bbox);
    }
    if (bbox_max >= split_location)
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
      child2->UpdateBBox (bbox);
    }
    CS_ASSERT (leaf_replaced);
  }

  num_objects = 0;
  // @@@ Clean up objects array if there are too many objects?
  // There should be some thresshold at least.
}

void csKDTree::AddObject (const csBox3& bbox, csKDTreeChild* obj)
{
  // Add this object to the list of objects to be distributed
  // later.
  disallow_distribute = false;
  obj->AddLeaf (this);
  AddObject (obj);
  UpdateBBox (bbox);
}

csKDTreeChild* csKDTree::AddObject (const csBox3& bbox, void* object)
{
  csKDTreeChild* obj = new csKDTreeChild ();
  obj->object = object;
  obj->bbox = bbox;
  AddObject (bbox, obj);
  return obj;
}

void csKDTree::Distribute ()
{
  // Check if there are objects to distribute or if distribution
  // is not allowed.
  if (num_objects == 0 || disallow_distribute) return;

  CS_ASSERT ((child1 == NULL) == (child2 == NULL));
  if (child1)
  {
    // This node has children. So we have to see to what child (or both)
    // we distribute the objects in the this node.
    DistributeLeafObjects ();
    CS_ASSERT (num_objects == 0);

    // Update the bounding box of this node.
    tree_bbox = child1->tree_bbox;
    tree_bbox += child2->tree_bbox;
  }
  else
  {
    // This node doesn't have children yet.

    // If we only have one object we do nothing.
    if (num_objects == 1) return;

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
      disallow_distribute = true;
    }
    if (!disallow_distribute)
    {
      child1 = new csKDTree ();
      child2 = new csKDTree ();
      DistributeLeafObjects ();
      CS_ASSERT (num_objects == 0);
      // Update the bounding box of this node.
      tree_bbox = child1->tree_bbox;
      tree_bbox += child2->tree_bbox;
    }
  }
}

void csKDTree::FullDistribute ()
{
  Distribute ();
  if (child1)
  {
    child1->FullDistribute ();
    CS_ASSERT (child2 != NULL);
    child2->FullDistribute ();
  }
}

void csKDTree::Flatten ()
{
  if (!child1) return;	// Nothing to do.

  // First flatten the children.
  // @@@ Is this the most optimal solution?
  child1->Flatten ();
  child2->Flatten ();

  csKDTree* c1 = child1; child1 = NULL;
  csKDTree* c2 = child2; child2 = NULL;

  int i;
  for (i = 0 ; i < c1->num_objects ; i++)
  {
    csKDTreeChild* obj = c1->objects[i];
    if (obj->num_leafs == 1)
    {
      CS_ASSERT (obj->leafs[0] == c1);
      obj->leafs[0] = this;
      AddObject (obj);
      UpdateBBox (obj->bbox);
    }
    else
    {
      if (obj->FindLeaf (this) == -1)
      {
        obj->ReplaceLeaf (c1, this);
	AddObject (obj);
        UpdateBBox (obj->bbox);
      }
    }
  }
  for (i = 0 ; i < c2->num_objects ; i++)
  {
    csKDTreeChild* obj = c2->objects[i];
    if (obj->num_leafs == 1)
    {
      CS_ASSERT (obj->leafs[0] == c2);
      obj->leafs[0] = this;
      AddObject (obj);
      UpdateBBox (obj->bbox);
    }
    else
    {
      if (obj->FindLeaf (this) == -1)
      {
        obj->ReplaceLeaf (c2, this);
	AddObject (obj);
        UpdateBBox (obj->bbox);
      }
    }
  }
}

#define KDT_ASSERT(test,msg) \
  if (!(test)) \
  { \
    csString ss; \
    ss.Format ("csKDTree failure (%d,%s): %s\n", int(__LINE__), \
    	#msg, #test); \
    str.Append (ss); \
    return rc; \
  }

bool csKDTree::Debug_CheckTree (csString& str)
{
  bool rc = false;

  KDT_ASSERT ((child1 == NULL) == (child2 == NULL), "child consistency");

  if (child1)
  {
    //-------
    // Test-cases in case this is a node.
    //-------

    KDT_ASSERT (split_axis >= CS_KDTREE_AXISX && split_axis <= CS_KDTREE_AXISZ,
    	"axis");
    KDT_ASSERT (tree_bbox.Contains (child1->tree_bbox), "tree_bbox mismatch");
    KDT_ASSERT (tree_bbox.Contains (child2->tree_bbox), "tree_bbox mismatch");

    if (!child1->Debug_CheckTree (str))
      return false;
    if (!child2->Debug_CheckTree (str))
      return false;
  }

  //-------
  // Test-cases in case this is a leaf (or not a leaf but has
  // objects waiting for distribution).
  //-------

  KDT_ASSERT (num_objects <= max_objects, "object list");

  int i, j;
  for (i = 0 ; i < num_objects ; i++)
  {
    csKDTreeChild* o = objects[i];

    KDT_ASSERT (tree_bbox.Contains (o->bbox), "object not in tree_bbox");

    KDT_ASSERT (o->num_leafs <= o->max_leafs, "leaf list");
    int parcnt = 0;
    for (j = 0 ; j < o->num_leafs ; j++)
    {
      if (o->leafs[j] == this)
      {
	parcnt++;
        KDT_ASSERT (parcnt <= 1, "parent occurs multiple times");
      }
    }
    KDT_ASSERT (parcnt == 1, "leaf list doesn't contain parent");
  }

  return true;
}

static float rnd (float range)
{
  return float ((rand () >> 4) % 1000) * range / 1000.0;
}

iString* csKDTree::Debug_UnitTest ()
{
  csTicks seed = csGetTicks ();
  srand (seed);

  scfString* rc = new scfString ();
  csString& str = rc->GetCsString ();

  str.Format ("Seed %d\n", seed);

  Clear ();
  KDT_ASSERT (child1 == NULL, "clear ok?");
  KDT_ASSERT (child2 == NULL, "clear ok?");
  KDT_ASSERT (objects == NULL, "clear ok?");
  KDT_ASSERT (num_objects == 0, "clear ok?");
  KDT_ASSERT (max_objects == 0, "clear ok?");

  csBox3 b;

  b.Set (9, 7, 8, 11, 9, 10);
  AddObject (b, (void*)1);
  b.Set (12, 7, 8, 13, 9, 10);
  AddObject (b, (void*)2);
  b.Set (10, 11, 8, 14, 12, 10);
  AddObject (b, (void*)3);
  b.Set (10, 11, 11, 14, 12, 14);
  AddObject (b, (void*)3);
  b.Set (10, 11, 11, 14, 12, 14);
  AddObject (b, (void*)4);

  iString* dbdump;

  if (!Debug_CheckTree (str)) return rc;
  Distribute ();
  if (!Debug_CheckTree (str)) return rc;
  FullDistribute ();
  if (!Debug_CheckTree (str)) return rc;

  Clear ();

  int i;
  for (i = 0 ; i < 500 ; i++)
  {
    float x = rnd (100.0)-50.0;
    float y = rnd (100.0)-50.0;
    float z = rnd (100.0)-50.0;
    b.Set (x, y, z, x+rnd (1.0)+.5, y+rnd (1.0)+.5, z+rnd (1.0)+.5);
    AddObject (b, (void*)0);
    if (!Debug_CheckTree (str)) return rc;
    if (i % 20 == 0)
    {
      FullDistribute ();
      if (!Debug_CheckTree (str)) return rc;
    }
  }
  if (!Debug_CheckTree (str)) return rc;
  dbdump = Debug_Statistics ();
  printf ("%s", dbdump->GetData ()); fflush (stdout);

  Distribute ();
  if (!Debug_CheckTree (str)) return rc;
  dbdump = Debug_Statistics ();
  printf ("%s", dbdump->GetData ()); fflush (stdout);

  FullDistribute ();
  if (!Debug_CheckTree (str)) return rc;
  dbdump = Debug_Statistics ();
  printf ("%s", dbdump->GetData ()); fflush (stdout);

  Flatten ();
  if (!Debug_CheckTree (str)) return rc;
  dbdump = Debug_Statistics ();
  printf ("%s", dbdump->GetData ()); fflush (stdout);

  FullDistribute ();
  if (!Debug_CheckTree (str)) return rc;
  dbdump = Debug_Statistics ();
  printf ("%s", dbdump->GetData ()); fflush (stdout);

  rc->DecRef ();
  return NULL;
}

void csKDTree::Debug_Statistics (int& tot_objects,
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

iString* csKDTree::Debug_Statistics ()
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

  return rc;
}

void csKDTree::Debug_Dump (csString& str, int indent)
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
  iString* stats = Debug_Statistics ();
  ss.Format ("%s KDT bbox(%g,%g,%g)-(%g,%g,%g) disallow_dist=%d\n%s %s",
  	spaces, tree_bbox.MinX (), tree_bbox.MinY (), tree_bbox.MinZ (),
	tree_bbox.MaxX (), tree_bbox.MaxY (), tree_bbox.MaxZ (),
	disallow_distribute,
  	spaces, stats->GetData ());
  stats->DecRef ();
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

