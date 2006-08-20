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

#ifndef __CS_PVSCOMP_H
#define __CS_PVSCOMP_H

#include "cssysdef.h"
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csutil/ref.h"
#include "csgeom/box.h"
#include "iengine/engine.h"
#include "csgeom/statickdtree.h"

typedef csArray<csString> PVSArray;

// polygon.cpp
// A polygon either from CS or created from 4 vertices.
struct Polygon 
{
  csVector3* vertices;
  int numVertices;
  int* index;
  bool freeData;

  Polygon (const csVector3& a, const csVector3& b, const csVector3& c,
      const csVector3& d);
  Polygon (struct iPolygonMesh* mesh, int polygonIndex);
  ~Polygon ();
  void Print () const;
  csVector3 FindCenter () const;

  static void Fill (iMeshWrapper* wrapper, csArray<Polygon*>& array);
  static void Fill (const struct csBox3& region, csArray<Polygon*>& fill);
  static void Print (const csArray<Polygon*>& array);
  static void Free (csArray<Polygon*>& array);
};

// A plucker point or plane.
class Plucker
{
  float data[6];

public:
  Plucker ()
  {
  }

  Plucker (float a, float b, float c, float d, float e, float f)
  {
    data[0] = a;
    data[1] = b;
    data[2] = c;
    data[3] = d;
    data[4] = e;
    data[5] = f;
  }

  // Construct a plucker point that represents a directed line passing through
  // u to v.
  Plucker (const csVector3& u, const csVector3& v)
  {
    data[0] = v[0] - u[0];
    data[1] = v[1] - u[1];
    data[2] = v[2] - u[2];
    data[3] = u[1] * v[2] - u[2] * v[1];
    data[4] = u[2] * v[0] - u[0] * v[2];
    data[5] = u[0] * v[1] - u[1] * v[0];
  }

  float operator[] (int index) const
  {
    return data[index];
  }

  float& operator[] (int index)
  {
    return data[index];
  }

  void Negate ()
  {
    for (int i = 0; i < 6; i++)
      data[i] = -data[i];
  }

  Plucker Dual ()
  {
    Plucker p;
    p[0] = data[3];
    p[1] = data[4];
    p[2] = data[5];
    p[3] = data[0];
    p[4] = data[1];
    p[5] = data[2];
    return p;
  }

  float Distance (Plucker& other) const
  {
    return data[0] * other[0] + data[1] * other[1] + data[2] * other[2] +
      data[3] * other[3] + data[4] * other[4] + data[5] * other[5];
  }
};

// polyenum.cpp
void ExtremalPluckerPoints (const Polygon* source, const Polygon* occluder,
    csArray<Plucker>& fill);
void PluckerPlanes (const Polygon* source, const Polygon* occluder,
    csArray<Plucker>& fill);
void VertexRepresentation (const csArray<Plucker>& planes,
    csArray<Plucker>& fill);
// void GetEdges ();
void CapPlanes (const csArray<Plucker>& vertices, csArray<Plucker>& fill);

// Represents all scene events from a source polygon.
class OcclusionTree
{
  Plucker splitPlane;
  struct OcclusionTreeLeaf* leafNode;
  OcclusionTree* posChild;
  OcclusionTree* negChild;
  OcclusionTree* parent;

  // Constructs a blocker polyhedron representing lines that pass through the
  // space of this node.
  class BlockerPolyhedron* ConstructBlockerForNode ();
  // Construct a leaf node.
  OcclusionTree (int type, const char* objectName, Polygon* p);
  // Construct an interior node.
  OcclusionTree (const Plucker& split);
  // Construct an IN node, may return NULL if blocker polyhedron is empty.
  OcclusionTree* ConstructInNode (struct BlockerPolyhedron* poly,
    const csArray<Plucker>& splitPlanes);
  // Constructs an OUT node.
  OcclusionTree* ConstructOutNode ();
  // Construct an elementary polyhedron and replace current node with it.
  void ReplaceWithElementaryOT (const csArray<Plucker>& otplanes,
      OcclusionTree* inLeaf);
  // Unions a BlockerPolyhedron that was originally an IN node.
  void OcclusionTree::InUnion (BlockerPolyhedron* blocker,
      csArray<Plucker> splitPlanes);
  // Unions a BlockerPolyhedron.
  void Union (struct BlockerPolyhedron* polyhedron, 
      csArray<Plucker> splitPlanes);

public:
  // Start a new empty occlusion tree.
  OcclusionTree ();
  // Standard destructor.
  ~OcclusionTree ();
  
  // Add a polyhedra to the scene.
  void Union (const struct Polygon* source,
      const struct Polygon* target, const char* objectName);
  // Determine the size of the set of lines.  If below a certain threshold,
  // the IN-NODE will not be included in the PVS.
  void MakeSizeSet ();
  // Add all objects that are visible to PVS array, making sure to not add any
  // duplicates (i.e. treat array as a set).
  void CollectPVS (PVSArray& pvs) const;
};

// pvscomp.cpp
// Facility for loading CS levels into a KD tree, performing PVS calculation,
// and saving data to a file.
class Compiler
{
  csRef<iEngine> engine;
  iObjectRegistry* reg;
  iMeshList* meshlist;
  csStaticKDTree* pvstree;

  // Creates a tree from the list of meshes.
  void MakeTree ();
  // Every node's data is initialized to an empty set of PVSArray type.
  void InitializePVSArrays (csStaticKDTree* node);
  // Change PVSArray to csPVSNodeData for all nodes.
  void ConvertTree (csStaticKDTree* node);
  // Frees csPVSNodeData memory from the tree.
  void FreeTreeData (csStaticKDTree* node);

  // Recursively fills in PVSArray for all leaves.
  void ConstructPVS (csStaticKDTree* node);
  // Finds PVS for a particular boxed region.
  void ConstructPVSForRegion (const csBox3& region, PVSArray& pvs);
  // Finds PVS for a particular face for rays that point in the same direction
  // as the normal.
  void ConstructPVSForFace (const struct Polygon* p, PVSArray& pvs);
  // Creates a polyhedron that represents the rays passing from the source
  // polygon to the rest of the scene.
  static OcclusionTree* ConstructOT (const struct Polygon* p,
      csStaticKDTree* node, class OcclusionTree* polyhedron);
  // Distribute the PVS data to non-leaf nodes.
  void PropogatePVS (csStaticKDTree* node);

public:
  Compiler ();
  ~Compiler ();
  // Initializes Crystal Space and the necessary modules.
  bool Initialize (int argc, char** argv);
  // Loads a world file.
  bool LoadWorld (const char* path, const char* filename);
  // Construct PVS for the scene.
  void DoWork ();
  // Show the tree and what it contains, given that node data has csPVSNodeData
  void PrintObjects ();
  // Save PVS tree to a file.
  void Save (const char* name);
};

#endif
