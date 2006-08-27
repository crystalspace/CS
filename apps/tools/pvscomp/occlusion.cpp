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

#define PLANE_THICKNESS 0.0001

// Unions array and the set containing objectName.
static void PVSSetAdd (PVSArray* array, const csString& objectName);
// Pushes all the elements of source onto dest.
static void PushArray (csArray<Plucker>& dest, const csArray<Plucker>& source);
// Returns <0 if poly lies in negative halfspace, >0 if poly lies in positive
// halfspace, 0 if poly lies on both.
static int Test (csArray<Plucker>& vertices, const Plucker& plane);
// Same as above but tests for 2D polygons and planes.
static int Test (const Polygon* p, const csVector3& normal, float planeD);
// Returns the face that is in front (<0 if lhs, >0 if rhs).
static int FaceTest (const Polygon* lhs, const Polygon* rhs,
    const Polygon* source);

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

static int Test (csArray<Plucker>& vertices, const Plucker& plane)
{
  int value = 0;
  for (unsigned int i = 0; i < vertices.Length (); i++)
  {
    float distance = plane.Distance (vertices[i]);
    if (value == 0)
    {
      // First iteration or all previous vertices have been on the plane
      if (distance > PLANE_THICKNESS)
        value = 1;
      else if (distance < -PLANE_THICKNESS)
        value = 0;
    }
    else if (value == 1)  
    {
      // All previous vertices were in positive halfspace
      if (distance < -PLANE_THICKNESS)
        return 0;
    }
    else if (value == -1)
    {
      // All previous vertices were in negative halfspace
      if (distance > PLANE_THICKNESS)
        return 0;
    }
  }

  // Polyhedron should not be located on just one plane!
  CS_ASSERT (value != 0);

  return value;
}

static int Test (const Polygon* p, const csVector3& normal, float planeD)
{
  int value = 0;
  for (int i = 0; i < p->numVertices; i++)
  {
    float distance = p->vertices[p->index[i]] * normal - planeD;
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

  return value;
}

static int FaceTest (const Polygon* lhs, const Polygon* rhs, 
    const Polygon* source)
{
  // First see if either polygon lies in the positive or negative halfspace
  // defined by the other polygon.
  csVector3 normal;
  float distance;
  int otherTest, sourceTest;

  // LHS defines the plane
  normal.Cross (
      (lhs->vertices[lhs->index[1]] - lhs->vertices[lhs->index[0]]),
      (lhs->vertices[lhs->index[2]] - lhs->vertices[lhs->index[0]]));
  normal.Normalize ();
  distance = normal * lhs->vertices[lhs->index[0]];
  sourceTest = Test (source, normal, distance);
  otherTest = Test (rhs, normal, distance);
  if (sourceTest != 0 && otherTest != 0)
  {
    if (otherTest > 0 && sourceTest > 0)
    {
      // RHS and source are on same side, RHS in front
      return 1;
    }
    else
    {
      return -1;
    }
  }

  // The above test was inconclusive, so use the distances from the center
  // of the source polygon to the centers of the occluders to determine.

  // TODO:  does this really work?
  
  float lhsdistance = (lhs->FindCenter () - source->FindCenter ()).Norm ();
  float rhsdistance = (rhs->FindCenter () - source->FindCenter ()).Norm ();
  if (lhsdistance < rhsdistance)
  {
    return -1;
  }
  else
  {
    return 1;
  }
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

// A 5D polyhedra that represents a set of lines that intersect a source
// polygon and an occluder polygon.
// OR
// Represents a set of rays that passes through a source polygon and an IN
// node's volume.
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

  // Default constructor
  BlockerPolyhedron () {}
  // Construct a blocker polyhedron from an occluder polygon.
  BlockerPolyhedron (const Polygon* source, const Polygon* occluder, 
      const char* objectName);
  // Prints the vertices of the polyhedron.
  void PrintVertices ();
};

BlockerPolyhedron::BlockerPolyhedron (const Polygon* source, 
    const Polygon* occluder, const char* objectName)
{
  ExtremalPluckerPoints (source, occluder, vertices);
  PluckerPlanes (source, planes);
  PluckerPlanes (occluder, planes);
}

void BlockerPolyhedron::PrintVertices ()
{
  printf("Polyhedral vertices (%d total):\n", vertices.Length ());
  for (unsigned int i = 0; i < vertices.Length (); i++)
  {
    Plucker& v = vertices[i];
    printf("  Vertex: (%f,%f,%f,%f,%f,%f)\n", v[0], 
        v[1], v[2], v[3], v[4], v[5]);
  }
}

//////////////////////////////////////////////////////////////////////////////

BlockerPolyhedron* OcclusionTree::ConstructBlockerForNode ()
{
  CS_ASSERT (leafNode);
  CS_ASSERT (leafNode->type == IN_NODE);

  BlockerPolyhedron* blocker = new BlockerPolyhedron ();
  blocker->objectName = leafNode->objectName;
  blocker->sourcePoly = NULL;
  blocker->blockerPoly = leafNode->poly;
  PushArray (blocker->vertices, leafNode->fragment);

  for (OcclusionTree* current = this; current; current = current->parent)
  {
    blocker->planes.Push (current->splitPlane);
  }

  return blocker;
}

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
  parent = NULL;
}

OcclusionTree::OcclusionTree (const Plucker& split)
{
  splitPlane = split;
  leafNode = NULL;
  parent = NULL;
}

OcclusionTree* OcclusionTree::ConstructOutNode ()
{
  return new OcclusionTree (OUT_NODE, NULL, NULL);
}

OcclusionTree* OcclusionTree::ConstructInNode (const BlockerPolyhedron* poly,
    const csArray<Plucker>& splitPlanes)
{
  csArray<Plucker> planes;
  OcclusionTree* inNode = new OcclusionTree (IN_NODE, poly->objectName,
      poly->blockerPoly);

  // First find the new vertex representation.
  if (splitPlanes.IsEmpty ())
  {
    // There is no need to recalculate Vertex form.
    PushArray (inNode->leafNode->fragment, poly->vertices);
  }
  else
  {
    // Since there were split planes the polygon enumeration algorithm must
    // be run.

    // Make sure planes contains every hyperplane defining the polyhedron.
    // and then fill the fragment with the vertex representation.
    PushArray (planes, splitPlanes);
    PushArray (planes, poly->planes);
    CapPlanes (poly->vertices, planes);
    VertexRepresentation (planes, inNode->leafNode->fragment);
  }

  // If fragment comes up empty, this is not an in node.
  if (inNode->leafNode->fragment.IsEmpty ())
  {
    delete inNode;
    return NULL;
  }
  else
  {
    return inNode;
  }
}

void OcclusionTree::ReplaceWithElementaryOT (
    const csArray<Plucker>& otplanes, OcclusionTree* inLeaf)
{
  delete leafNode;
  leafNode = NULL;

  if (otplanes.Length () > 0)
  {
    OcclusionTree* currentPoly = NULL;
   
    for (unsigned int i = 0; i < otplanes.Length (); i++)
    {
      if (currentPoly == NULL)
      {
        // First time through loop
        currentPoly = this;
        splitPlane = otplanes[i];
      }
      else
      {
        // Next plane goes to positive halfspace
        currentPoly->negChild = new OcclusionTree (otplanes[i]);
        currentPoly->negChild->parent = currentPoly;
        currentPoly = currentPoly->negChild;
      }

      // In leaf goes to negative halfspace
      currentPoly->posChild = new OcclusionTree (OUT_NODE, NULL, NULL);
      currentPoly->posChild->parent = currentPoly;
    }

    // Last plane node has a leaf child with an in node
    currentPoly->negChild = inLeaf;
    inLeaf->parent = currentPoly;
  }
  else
  {
    // The current node must be replaced with the IN node.
    leafNode = inLeaf->leafNode;  // Steal replacement's soul!
    inLeaf->leafNode = NULL;
    delete inLeaf;
  }
}

void OcclusionTree::InUnion (BlockerPolyhedron* blocker,
    csArray<Plucker> splitPlanes)
{
  if (leafNode)
  {
    if (leafNode->type == OUT_NODE)
    {
      OcclusionTree* replacement = ConstructInNode (blocker, splitPlanes);
      if (replacement)
      {
        // TODO:  just inserting the IN node should be enough, right?
        ReplaceWithElementaryOT (csArray<Plucker> (), replacement);
      }
    }
  }
  else  // Interior node
  {
    int test = Test (leafNode->fragment, splitPlane);
    if (test > 0)
    {
      // poly in front
      posChild->Union (blocker, splitPlanes);
    }
    else if (test < 0)
    {
      // poly in back
      negChild->Union (blocker, splitPlanes);
    }
    else
    {
      // poly is on both sides of splitPlane
      splitPlanes.Push (splitPlane);
      posChild->Union (blocker, splitPlanes);
      splitPlanes[splitPlanes.Length () - 1].Negate ();
      negChild->Union (blocker, splitPlanes);
    }
  }
}

void OcclusionTree::Union (BlockerPolyhedron* polyhedron,
    csArray<Plucker> splitPlanes)
{
  // TODO:  do we need splitplanes in e-OT as well?
  // TODO:  can we look at e-OT's planes and the distances from the
  //        polyhedron to determine if they still bound it?

  if (leafNode)
  {
    if (leafNode->type == OUT_NODE)
    {
      printf ("Arrived at OUT node.\n");
      OcclusionTree* inLeaf = ConstructInNode (polyhedron, splitPlanes);
      if (inLeaf)
      {
        ReplaceWithElementaryOT (polyhedron->planes, inLeaf);
      }
    }
    else  // In node
    {
      printf ("Arrived at IN node.\n");

      // TODO:  I don't think we care about what polygon defines this node
      // as an IN NODE.  We might be able to completely skip the IN node, or
      // possibly make one node with two IN node children.
      if (FaceTest (leafNode->poly, polyhedron->blockerPoly, 
            polyhedron->sourcePoly) < 0)
      {
        printf ("Special merge.\n");
        // leafNode's polygon is behind our current occluder.

        OcclusionTree* inLeaf = ConstructInNode (polyhedron, splitPlanes);
        if (inLeaf)
        {
          BlockerPolyhedron* old = ConstructBlockerForNode ();
          ReplaceWithElementaryOT (polyhedron->planes, inLeaf);
          InUnion (old, csArray<Plucker> ());
        }
      }
    }
  }
  else  // Interior node
  {
    int test = Test (polyhedron->vertices, splitPlane);
    if (test > 0)
    {
      printf ("Polyhedron lies on positive side.\n");
      // poly in front
      posChild->Union (polyhedron, splitPlanes);
    }
    else if (test < 0)
    {
      printf ("Polyhedron lies on negative side.\n");
      // poly in back
      negChild->Union (polyhedron, splitPlanes);
    }
    else
    {
      printf ("Polyhedron lies on both sides.\n");

      // poly is on both sides of splitPlane
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

void OcclusionTree::PrintTree () const
{
  if (leafNode)
  {
    if (leafNode->type == OUT_NODE)
    {
      printf("OUT node.\n");
    }
    else
    {
      printf("IN node.\n");
      printf("Polyhedral fragments:\n");
      for (unsigned int i = 0; i < leafNode->fragment.Length (); i++)
      {
        Plucker& v = leafNode->fragment[i];
        printf("  Vertex: (%f,%f,%f,%f,%f,%f)\n", v[0], 
            v[1], v[2], v[3], v[4], v[5]);
      }
    }
  }
  else
  {
    printf("Split plane: (%f,%f,%f,%f,%f,%f)\n", splitPlane[0], splitPlane[1],
        splitPlane[2], splitPlane[3], splitPlane[4], splitPlane[5]);
    posChild->PrintTree ();
    negChild->PrintTree ();
  }
}

void TestUnionTree ()
{
  Polygon p1 (csVector3 (0, 0, 0), csVector3 (1, 0, 0), csVector3 (1, 1, 0),
      csVector3 (0, 1, 0));
  Polygon p2 (csVector3 (0, 0, 1), csVector3 (0, 1, 1), csVector3 (1, 1, 0),
      csVector3 (1, 0, 1));
  Polygon p3 (csVector3 (.5, 0, .5), csVector3 (.5, 1, .5), 
      csVector3 (1.5, 1, .5), csVector3 (1.5, 0, .5));
  BlockerPolyhedron (&p1, &p3, "").PrintVertices ();
  OcclusionTree* test = new OcclusionTree ();
  test->Union (&p1, &p2, "number 1");
  test->Union (&p1, &p3, "number 2");
  test->PrintTree ();
  delete test;
}
