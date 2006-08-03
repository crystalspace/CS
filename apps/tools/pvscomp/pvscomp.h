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

struct Polygon 
{
  csVector3* vertices;
  int numVertices;
  int* index;
  bool freeData;
};

// Facility for loading CS levels into a KD tree, performing PVS calculation,
// and saving data to a file.
class Compiler
{
  csRef<iEngine> engine;
  iObjectRegistry* reg;
  iMeshList* meshlist;
  csStaticKDTree* pvstree;

  void MakeTree ();
  void ConstructPVS(csStaticKDTree* node);
  void ConstructPVSForRegion(const csBox3& region, PVSArray& pvs);
  void ConstructPVSForFace(const Polygon* p, PVSArray& pvs);
  void PropogatePVS(csStaticKDTree* node);

public:
  Compiler ();
  ~Compiler ();
  bool Initialize (int argc, char** argv);
  bool LoadWorld (const char* path, const char* filename);
  void DoWork ();
  void PrintObjects ();
  void Save (const char* name);
};

typedef float PluckerPoint[6];
typedef float PluckerPlane[6];

// polyhedron.cpp?
class PolyhedronTree
{
  PluckerPlane split;
  struct PolyhedronTreeLeaf* leafNode;

public:
  static PolyhedronTree* Construct (const Polygon* source,
      const Polygon* target, const char* objectName);
  
  void Union(const PolyhedronTree* other);
  void MakeSizeSet();
  void CollectPVS(PVSArray& pvs) const;
};

struct PolyhedronTreeLeaf
{
  int type;  
  float angle;
  csString objectName;
  Polygon* poly;
};

/*

enum PLANE_INTERSECTION { FRONT, BACK, SPLIT };

class HPolyhedron
{
  csArray<PluckerPlane> planes;

public:
  void AddPlane (const PluckerPlane& p);
  VPolyhedron* ConstructVPolyhedron ();
};

class VPolyhedron
{
  csArray<PluckerPoint> vertices;
  HPolyhedron* hrep;
  dd_PolyhedraPtr poly;

public:
  PLANE_INTERSECTION PlaneTest (const PluckerPlane& p);
  void PluckerIntersection ();
  HPolyhedron* GetHPolyhedron ();
}; */

#endif
