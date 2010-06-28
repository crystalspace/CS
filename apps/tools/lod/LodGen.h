/*
 *  LodGen.h
 *  cs
 *
 *  Created by Eduardo Poyart on 6/14/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

void PointTriangleDistanceUnitTests();

typedef csArray<int> IncidentTris;

struct Edge
{
  int v0;
  int v1;
  Edge(int a, int b) { assert(a != b); if (a < b) { v0 = a; v1 = b; } else { v0 = b; v1 = a; } }
  inline bool operator==(const Edge& e1) const
  {
    assert(v0 < v1 && e1.v0 < e1.v1);
    return (v0 == e1.v0 && v1 == e1.v1);
  }
};

struct WorkMesh
{
  csArray<csTriangle> tri_buffer;
  csArray<int> tri_indices;
  csArray<IncidentTris> incident_tris; // map from vertices to incident triangles
};

struct SlidingWindow
{
  int start_index;
  int end_index;
};

enum UpdateEdges { NO_UPDATE_EDGES, UPDATE_EDGES };

class LodGen
{
protected:
  const csArray<csVector3>& vertices;
  const csArray<csTriangle>& triangles;
  int num_vertices;
  int num_triangles;
  
  csArray<Edge> edges;
  WorkMesh k;
  csArray<int> removed_tris;
  csArray<csTriangle> ordered_tris;
  csArray<SlidingWindow> sliding_windows;

public:
  LodGen(const csArray<csVector3>& v, const csArray<csTriangle>& t): 
    vertices(v), triangles(t), num_vertices(v.GetSize()), num_triangles(t.GetSize()) {}
  void GenerateLODs();
  int GetTriangleCount() const { return ordered_tris.GetSize(); }
  const csTriangle& GetTriangle(int i) const { return ordered_tris[i]; }
  int GetSlidingWindowCount() const { return sliding_windows.GetSize(); }
  const SlidingWindow& GetSlidingWindow(int i) const { return sliding_windows[i]; }
  
protected:
  bool IsDegenerate(const csTriangle& tri) const;
  void AddTriangle(WorkMesh& k, int itri);
  void RemoveTriangle(WorkMesh& k, int itri);
  bool Collapse(WorkMesh& k, int v0, int v1, UpdateEdges u = NO_UPDATE_EDGES);
  float SumOfSquareDist(const WorkMesh& k) const;
};
