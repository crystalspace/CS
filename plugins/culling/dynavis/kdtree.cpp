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

void csKDTree::AddObjectToSingleChildLeaf (
	csKDTree*& child_new, csKDTree*& child_old,
	csKDTreeChild* obj_new)
{
  CS_ASSERT (num_objects == 1);

  // Add the new object.
  child_new = new csKDTree ();
  child_new->AddObject (obj_new);
  child_new->tree_bbox = obj_new->bbox;
  obj_new->AddLeaf (child_new);

  // Remove the original object from this leaf and move it
  // to the right child.
  child_old = new csKDTree ();
  csKDTreeChild* obj_old = objects[0];
  obj_old->ReplaceLeaf (this, child_old);
  child_old->AddObject (obj_old);
  RemoveObject (0);
  child_old->tree_bbox = obj_old->bbox;

  tree_bbox = child_new->tree_bbox;
  tree_bbox += child_old->tree_bbox;
}

float csKDTree::FindBestSplitLocation (int axis, float& split_loc)
{
  int i, j;

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
  float best_qual = -1.0;
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
    float qual_cut = 1.0 - float (cut) * inv_num_objects;
    float qual_balance = 1.0 - float (ABS (left-right)) * inv_num_objects;
    // Currently we just give 'cut' and 'balance' quality an equal share.
    // We should consider if that is a good measure.
    float qual = qual_cut * qual_balance;
    if (qual > best_qual)
    {
      best_qual = qual;
      split_loc = a;
    }
  }
# undef FBSL_ATTEMPTS
  return best_qual;
}

void csKDTree::DistributeLeafObjects ()
{
  child1 = new csKDTree ();
  child2 = new csKDTree ();

  int i;
  bool c1_bbox = false;
  bool c2_bbox = false;
  for (i = 0 ; i < num_objects ; i++)
  {
    const csBox3& bbox = objects[i]->bbox;
    float bbox_min = bbox.Min (split_axis);
    float bbox_max = bbox.Max (split_axis);
    if (bbox_min <= split_location)
    {
      objects[i]->ReplaceLeaf (this, child1);
      child1->AddObject (objects[i]);
      if (c1_bbox)
        child1->tree_bbox += bbox;
      else
      {
        child1->tree_bbox = bbox;
	c1_bbox = true;
      }
    }
    if (bbox_max >= split_location)
    {
      if (bbox_min <= split_location)
      {
        // If we also added this object to child1
	// we need to call AddLeaf() instead of ReplaceLeaf().
        objects[i]->AddLeaf (child2);
      }
      else objects[i]->ReplaceLeaf (this, child2);
      child2->AddObject (objects[i]);
      if (c2_bbox)
        child2->tree_bbox += bbox;
      else
      {
        child2->tree_bbox = bbox;
	c2_bbox = true;
      }
    }
  }

  num_objects = 0;
  // @@@ Clean up objects array if there are too many objects?
  // There should be some thresshold at least.
}

void csKDTree::AddObject (const csBox3& bbox, csKDTreeChild* obj)
{
  CS_ASSERT ((child1 == NULL) == (child2 == NULL));
  if (child1)
  {
    // This node has children. So we have to see to what child (or both)
    // we distribute this object.
    CS_ASSERT (split_axis >= CS_KDTREE_AXISX && split_axis <= CS_KDTREE_AXISZ);
    float bbox_min = bbox.Min (split_axis);
    float bbox_max = bbox.Max (split_axis);
    if (bbox_min <= split_location)
      child1->AddObject (bbox, obj);
    if (bbox_max >= split_location)
      child2->AddObject (bbox, obj);
    tree_bbox = child1->tree_bbox;
    tree_bbox += child2->tree_bbox;
    return;
  }
  else
  {
    // This node doesn't have children yet.
    // If it also doesn't have any other objects we will create
    // it here.
    if (num_objects == 0)
    {
      obj->AddLeaf (this);
      AddObject (obj);
      tree_bbox = bbox;
      return;
    }

    // If we already have one object we check if we are going to
    // split or not.
    if (num_objects == 1)
    {
      // Otherwise we check if there is a clean split between the
      // two objects. A clean split is an axis aligned plane that
      // doesn't cut any of the two objects. If no such clean cut
      // exists we are just going to put this object in this leaf
      // as well.
      csKDTreeChild* oobj = objects[0];
      const csBox3& obbox = oobj->bbox;
      if (bbox.MaxX () < obbox.MinX ())
      {
	split_axis = CS_KDTREE_AXISX;
	split_location = bbox.MaxX () + (obbox.MinX ()-bbox.MaxX ()) / 2.0;
	AddObjectToSingleChildLeaf (child1, child2, obj);
      }
      else if (obbox.MaxX () < bbox.MinX ())
      {
	split_axis = CS_KDTREE_AXISX;
	split_location = obbox.MaxX () + (bbox.MinX ()-obbox.MaxX ()) / 2.0;
	AddObjectToSingleChildLeaf (child2, child1, obj);
      }
      else if (bbox.MaxY () < obbox.MinY ())
      {
	split_axis = CS_KDTREE_AXISY;
	split_location = bbox.MaxY () + (obbox.MinY ()-bbox.MaxY ()) / 2.0;
	AddObjectToSingleChildLeaf (child1, child2, obj);
      }
      else if (obbox.MaxY () < bbox.MinY ())
      {
	split_axis = CS_KDTREE_AXISY;
	split_location = obbox.MaxY () + (bbox.MinY ()-obbox.MaxY ()) / 2.0;
	AddObjectToSingleChildLeaf (child2, child1, obj);
      }
      else if (bbox.MaxZ () < obbox.MinZ ())
      {
	split_axis = CS_KDTREE_AXISZ;
	split_location = bbox.MaxZ () + (obbox.MinZ ()-bbox.MaxZ ()) / 2.0;
	AddObjectToSingleChildLeaf (child1, child2, obj);
      }
      else if (obbox.MaxZ () < bbox.MinZ ())
      {
	split_axis = CS_KDTREE_AXISZ;
	split_location = obbox.MaxZ () + (bbox.MinZ ()-obbox.MaxZ ()) / 2.0;
	AddObjectToSingleChildLeaf (child2, child1, obj);
      }
      else
      {
        // We didn't find a suitable clean cut. So we just
	// add this object to this leaf.
	obj->AddLeaf (this);
	AddObject (obj);
        tree_bbox += bbox;
      }
      return;
    }

    // Here we have 2 or more objects already in the leaf.
    // We just add the new object and then run FindBestSplitLocation()
    // for all three axis so we can see what the best way to split is.
    float split_loc_x, split_loc_y, split_loc_z;
    float qual_x = FindBestSplitLocation (CS_KDTREE_AXISX, split_loc_x);
    float qual_y = FindBestSplitLocation (CS_KDTREE_AXISY, split_loc_y);
    float qual_z = FindBestSplitLocation (CS_KDTREE_AXISZ, split_loc_z);
    if (qual_x >= qual_y && qual_x >= qual_z)
    {
      split_axis = CS_KDTREE_AXISX;
      split_location = split_loc_x;
    }
    else if (qual_y >= qual_x && qual_y >= qual_z)
    {
      split_axis = CS_KDTREE_AXISY;
      split_location = split_loc_y;
    }
    else
    {
      split_axis = CS_KDTREE_AXISZ;
      split_location = split_loc_z;
    }
    DistributeLeafObjects ();

    tree_bbox = child1->tree_bbox;
    tree_bbox += child2->tree_bbox;
  }
}

csKDTreeChild* csKDTree::AddObject (const csBox3& bbox, void* object)
{
  csKDTreeChild* obj = new csKDTreeChild ();
  obj->object = object;
  obj->bbox = bbox;
  AddObject (bbox, obj);
  return obj;
}

#define KDT_ASSERT(test,msg) \
  if (!(test)) \
  { \
    str.Format ("csKDTree failure (%d,%s): %s\n", int(__LINE__), \
    	#msg, #test); \
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
    KDT_ASSERT (num_objects == 0, "leaf vs node");

    KDT_ASSERT (tree_bbox.Contains (child1->tree_bbox), "tree_bbox mismatch");
    KDT_ASSERT (tree_bbox.Contains (child2->tree_bbox), "tree_bbox mismatch");

    if (!child1->Debug_CheckTree (str))
      return false;
    if (!child2->Debug_CheckTree (str))
      return false;
  }
  else
  {
    //-------
    // Test-cases in case this is a leaf.
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

  }

  return true;
}

static float rnd (float range)
{
  return float ((rand () >> 4) % 1000) * range / 1000.0;
}

iString* csKDTree::Debug_UnitTest ()
{
  scfString* rc = new scfString ();
  csString& str = rc->GetCsString ();

  Clear ();
  KDT_ASSERT (child1 == NULL, "clear ok?");
  KDT_ASSERT (child2 == NULL, "clear ok?");
  KDT_ASSERT (objects == NULL, "clear ok?");
  KDT_ASSERT (num_objects == 0, "clear ok?");
  KDT_ASSERT (max_objects == 0, "clear ok?");

  csBox3 b;

  b.Set (9, 7, 8, 11, 9, 10);
  AddObject (b, (void*)1);
  KDT_ASSERT (child1 == NULL, "added one obj");
  KDT_ASSERT (child2 == NULL, "added one obj");
  KDT_ASSERT (objects != NULL, "added one obj");
  KDT_ASSERT (num_objects == 1, "added one obj");
  KDT_ASSERT (objects[0]->object == (void*)1, "added one obj");
  KDT_ASSERT (objects[0]->leafs[0] == this, "added one obj");
  KDT_ASSERT (max_objects > 0, "added one obj");
  KDT_ASSERT (tree_bbox.Contains (b), "added one obj");

  b.Set (12, 7, 8, 13, 9, 10);
  AddObject (b, (void*)2);
  KDT_ASSERT (child1 != NULL, "added two obj");
  KDT_ASSERT (child2 != NULL, "added two obj");
  KDT_ASSERT (num_objects == 0, "added two obj");
  KDT_ASSERT (split_axis == CS_KDTREE_AXISX, "added two obj");
  KDT_ASSERT (split_location > 11, "added two obj");
  KDT_ASSERT (split_location < 12, "added two obj");
  KDT_ASSERT (tree_bbox.Contains (b), "added one obj");

  KDT_ASSERT (child1->child1 == NULL, "added two obj");
  KDT_ASSERT (child1->child2 == NULL, "added two obj");
  KDT_ASSERT (child1->objects != NULL, "added two obj");
  KDT_ASSERT (child1->num_objects == 1, "added two obj");
  KDT_ASSERT (child1->objects[0]->object == (void*)1, "added two obj");
  KDT_ASSERT (child1->objects[0]->leafs[0] == child1, "added two obj");

  KDT_ASSERT (child2->child1 == NULL, "added two obj");
  KDT_ASSERT (child2->child2 == NULL, "added two obj");
  KDT_ASSERT (child2->objects != NULL, "added two obj");
  KDT_ASSERT (child2->num_objects == 1, "added two obj");
  KDT_ASSERT (child2->objects[0]->object == (void*)2, "added two obj");
  KDT_ASSERT (child2->objects[0]->leafs[0] == child2, "added two obj");

  if (!Debug_CheckTree (str))
    return rc;

  Clear ();

  int i;
  for (i = 0 ; i < 100 ; i++)
  {
    float x = rnd (100.0)-50.0;
    float y = rnd (100.0)-50.0;
    float z = rnd (100.0)-50.0;
    b.Set (x, y, z, x+rnd (1.0)+.5, y+rnd (1.0)+.5, z+rnd (1.0)+.5);
    AddObject (b, (void*)0);
    if (!Debug_CheckTree (str))
      return rc;
  }

  rc->DecRef ();
  return NULL;
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
  ss.Format ("%s KDT bbox(%g,%g,%g)-(%g,%g,%g)\n",
  	spaces, tree_bbox.MinX (), tree_bbox.MinY (), tree_bbox.MinZ (),
	tree_bbox.MaxX (), tree_bbox.MaxY (), tree_bbox.MaxZ ());
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

