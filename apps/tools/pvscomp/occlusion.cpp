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
#include "csutil/csstring.h"
#include "csutil/array.h"
#include "pvscomp.h"

static void PVSSetAdd (PVSArray* array, const csString& objectName);
static void PushArray (csArray<Plucker>& dest, const csArray<Plucker>& source);

static void PVSSetAdd (PVSArray* array, const csString& objectName)
{
  for (unsigned int i = 0; i < array->Length (); i++)
  {
    if ((*array)[i] == objectName)
      return;
    if ((*array)[i] > objectName)
    {
      array->Insert (i, objectName);
      return;
    }
  }
}

static void PushArray (csArray<Plucker>& dest, const csArray<Plucker>& source)
{
  for (unsigned int i = 0 ; i < source.Length (); i++)
  {
    dest.Push (source[i]);
  }
}

//////////////////////////////////////////////////////////////////////////////

// A 5D polyhedra that represents a set of lines that intersect a source
// polygon and an occluder polygon.
class BlockerPolyhedron
{
public:
  csString objectName;
  Polygon* sourcePoly;
  Polygon* blockerPoly;

  // Vertex representation
  csArray<Plucker> vertices;
  // Plane representation
  csArray<Plucker> planes;

  // Construct a blocker polyhedron.
  BlockerPolyhedron (const Polygon* source, const Polygon* occluder, 
      const char* objectName);
  // Push the necessary occlusion tree hyperplanes onto fill.
  void AddOTPlanes (const csArray<Plucker>& splitPlanes,
      csArray<Plucker>& fill);
  // Returns <0 if poly lies in negative halfspace, >0 if poly lies in positive
  // halfspace, 0 if poly lies on both.
  int Test (const Plucker& plane);
};

BlockerPolyhedron::BlockerPolyhedron (const Polygon* source, 
    const Polygon* occluder, const char* objectName)
{
  ExtremalPluckerPoints (source, occluder, vertices);
  PluckerPlanes (source, occluder, planes);
}

void BlockerPolyhedron::AddOTPlanes (const csArray<Plucker>& splitPlanes,
      csArray<Plucker>& fill)
{
  PushArray (fill, splitPlanes);
  PushArray (fill, planes);
  CapPlanes (vertices, fill);
}

int BlockerPolyhedron::Test (const Plucker& plane)
{
  // TODO:  should this use a thickness value?
  
  int value = 0;
  for (unsigned int i = 0; i < vertices.Length (); i++)
  {
    float distance = plane.Distance (vertices[i]);
    if (value == 0)
    {
      // First iteration or all previous vertices have been on the plane
      if (distance > 0)
        value = 1;
      else if (distance < 0)
        value = 0;
    }
    else if (value == 1)  
    {
      // All previous vertices were in positive halfspace
      if (distance < 0)
        return 0;
    }
    else if (value == -1)
    {
      // All previous vertices were in negative halfspace
      if (distance > 0)
        return 0;
    }
  }

  // Polyhedron should not be located on just one plane!
  CS_ASSERT (value != 0);

  return value;
}

//////////////////////////////////////////////////////////////////////////////

#define IN_NODE 0
#define OUT_NODE 1

// TODO NOTE:  does not need to be in .h
struct OcclusionTreeLeaf
{
  // IN node or OUT node
  int type;  
  // For IN nodes: filled in MakeSizeSet
  float angle;
  // For IN Nodes: Name of object that poly came from
  csString objectName;
  // For IN nodes: polygon the blocker belongs to
  Polygon* poly;
  // For IN nodes: the polyhedron fragment
  csArray<Plucker> fragment;
};

//////////////////////////////////////////////////////////////////////////////

OcclusionTree::OcclusionTree (int type, const char* objectName, 
    Polygon* p)
{
  leafNode = new OcclusionTreeLeaf ();
  leafNode->type = type;
  leafNode->angle = -1;
  leafNode->objectName = objectName;
  leafNode->poly = p;
  posChild = NULL;
  negChild = NULL;
}

OcclusionTree::OcclusionTree (const Plucker& split)
{
  splitPlane = split;
}

void OcclusionTree::ReplaceWithElementaryOT (
    const csArray<Plucker>& splitPlanes, BlockerPolyhedron* poly)
{
  OcclusionTree* currentPoly = NULL;

  csArray<Plucker> planes;
  poly->AddOTPlanes (splitPlanes, planes);

  for (unsigned int i = 0; i < planes.Length (); i++)
  {
    if (currentPoly == NULL)
    {
      // First time through loop
      currentPoly = this;
    }
    else
    {
      // Next plane goes to positive halfspace
      currentPoly->negChild = new OcclusionTree (planes[i]);
      currentPoly = currentPoly->negChild;
    }

    // In leaf goes to negative halfspace
    currentPoly->posChild = new OcclusionTree (OUT_NODE, NULL, NULL);
  }

  // Last plane node has a leaf child with an in node
  currentPoly->negChild = new OcclusionTree (IN_NODE, poly->objectName, 
      poly->blockerPoly);
  VertexRepresentation (planes, currentPoly->negChild->leafNode->fragment);
}

void OcclusionTree::Union (BlockerPolyhedron* polyhedron,
    csArray<Plucker> splitPlanes)
{
  if (leafNode)
  {
    if (leafNode->type == OUT_NODE)
    {
      delete leafNode;

      //
//      ReplaceNode (polyhedron);
    }
    else  // In node
    {
//      if (FaceTest (leafNode->poly, polyhedron->blockerPoly) < 0)
//      {
        // leafNode's polygon is behind our current occluder.  Merge.
//        OcclusionTreeLeaf* leaf = leafNode;
 //       ReplaceNode (polyhedron);
//        ReplaceOutNodes ();
//      }
    }
  }
  else  // Interior node
  {
    int test = polyhedron->Test (splitPlane);
    if (test > 0)
    {
      // poly in front
      posChild->Union (polyhedron, splitPlanes);
    }
    else if (test < 0)
    {
      // poly in back
      negChild->Union (polyhedron, splitPlanes);
    }
    else
    {
      // poly is in both sides of splitPlane
      splitPlanes.Push (splitPlane);
      posChild->Union (polyhedron, splitPlanes);
      splitPlanes[splitPlanes.Length () - 1].Negate ();
      negChild->Union (polyhedron, splitPlanes);
    }
  }
}

OcclusionTree::OcclusionTree ()
{
  leafNode = new OcclusionTreeLeaf ();
  leafNode->type = OUT_NODE;
  posChild = NULL;
  negChild = NULL;
}

OcclusionTree::~OcclusionTree ()
{
  if (leafNode)
  {
    delete leafNode;
  }
  else
  {
    delete negChild;
    delete posChild;
  }
}

void OcclusionTree::Union (const Polygon* source, const Polygon* target, 
    const char* objectName)
{
  BlockerPolyhedron blocker (source, target, objectName);
  csArray<Plucker> empty;
  Union (&blocker, empty);
}

void OcclusionTree::MakeSizeSet ()
{
  if (leafNode)
  {
    if (leafNode->type == IN_NODE)
    {
    }
  }
  else
  {
    posChild->MakeSizeSet ();
    negChild->MakeSizeSet ();
  }
}

void OcclusionTree::CollectPVS (PVSArray& pvs) const
{
  if (leafNode)
  {
    if (leafNode->type == IN_NODE)
    {
      if (leafNode->angle > 0)
      {
        PVSSetAdd (&pvs, leafNode->objectName);
      }
    }
  }
  else
  {
    posChild->CollectPVS (pvs);
    posChild->CollectPVS (pvs);
  }
}
