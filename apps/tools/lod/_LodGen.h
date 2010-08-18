/*
    Copyright (C) 2010 by Eduardo Poyart

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

#ifndef __LODGEN_H__
#define __LODGEN_H__

void PointTriangleDistanceUnitTests();

typedef csArray<size_t> IncidentTris;

struct Edge
{
  int v0;
  int v1;
  Edge() {}
  Edge(int a, int b): v0(a), v1(b) { assert(a != b); }
  inline bool operator==(const Edge& e1) const
  {
    return (v0 == e1.v0 && v1 == e1.v1);
  }
};

struct SlidingWindow
{
  int start_index;
  int end_index;
};

struct WorkMesh
{
  csArray<csTriangle> tri_buffer;
  csArray<size_t> tri_indices;
  csArray<IncidentTris> incident_tris; // map from vertices to incident triangles
  csArray<SlidingWindow> sliding_windows;
  void AddTriangle(const csTriangle& tri)
  {
    tri_buffer.Push(tri);
    size_t itri = tri_buffer.GetSize()-1;
    assert(tri_indices.Find(itri) == csArrayItemNotFound);
    tri_indices.Push(itri);
    for (int i = 0; i < 3; i++)
      incident_tris[tri[i]].PushSmart(itri);
  }
  const SlidingWindow& GetLastWindow() const { return sliding_windows[sliding_windows.GetSize()-1]; }
  void SetLastWindow(const SlidingWindow& sw) { sliding_windows[sliding_windows.GetSize()-1] = sw; }
  const csTriangle& GetTriangle(int idx) const { return tri_buffer[tri_indices[idx]]; }
};

typedef csArray<int> VertexIndexList;

enum ErrorMetricType { ERROR_METRIC_FAST, ERROR_METRIC_PRECISE };

class LodGen
{
protected:
  csArray<csVector3> vertices;
  csArray<csTriangle> triangles;
  csArray<VertexIndexList> coincident_vertices;
  
  WorkMesh k;
  csArray<csTriangle> ordered_tris;
  int top_limit;
  ErrorMetricType error_metric_type;
  bool verbose;

public:
  LodGen(): error_metric_type(ERROR_METRIC_FAST), verbose(false) {}

  /// Set an error metric to be used when generating LODs
  void SetErrorMetricType(ErrorMetricType em) { error_metric_type = em; }

  /// Set verbose mode
  void SetVerbose(bool v) { verbose = v; }

  /// Add a vertex to the mesh
  void AddVertex(const csVector3& v) { vertices.Push(v); }

  /// Add a triangle to the mesh
  void AddTriangle(const csTriangle& t) { triangles.Push(t); }

  /**
   * Main LOD generation method.
   */
  void GenerateLODs();

  // The methods below allow retrieval of results.

  /// Get the number of triangles in the processed mesh
  size_t GetTriangleCount() const { return ordered_tris.GetSize(); }

  /// Get a triangle from the processed mesh
  const csTriangle& GetTriangle(size_t i) const { return ordered_tris[i]; }

  /// Get the number of sliding windows in the processed mesh
  size_t GetSlidingWindowCount() const { return k.sliding_windows.GetSize(); }

  /// Get a sliding window from the processed mesh
  const SlidingWindow& GetSlidingWindow(size_t i) const { return k.sliding_windows[i]; }
  
protected:
  /**
   * Initialize array of coincident vertices.
   * For each vertex, create a list of all other vertices that share the same position.
   */
  void InitCoincidentVertices();

  /**
   * Checks if a triangle is degenerate through identical indices.
   */
  bool IsDegenerate(const csTriangle& tri) const;

  /**
   * Remove a triangle from the list of incident triangles of each of its 3 vertices.
   */
  void RemoveTriangleFromIncidentTris(WorkMesh& k, size_t itri);

  /**
   * Perform edge collapse from v0 to v1.
   */
  bool Collapse(WorkMesh& k, int v0, int v1);

  /**
   * Compute an error metric for the difference between two meshes.
   */
  float ErrorMetric(const WorkMesh& k, int start_index) const;

  /**
   * Slower and more precise error metric.
   * Examines all triangles at each step.
   */
  float ErrorMetricPrecise(const WorkMesh& k) const;

  /**
   * Quicker and less precise error metric.
   * Examines only modified triangles at each step.
   */
  float ErrorMetricFastOld(const WorkMesh& k, int start_index) const;

  float ErrorMetricFast(const WorkMesh& k, int start_index) const;

  /**
   * Finds a triangle in the sliding window and returns its position in the triangle buffer
   */
  int FindInWindow(const WorkMesh& k, const SlidingWindow& sw, size_t itri) const;

  /**
   * Swaps two triangles.
   */
  void SwapIndex(WorkMesh& k, int i0, int i1);

  /**
   * Verifies if a mesh is consistent, for debugging purposes.
   */
  void VerifyMesh(WorkMesh& k);

  /**
   * Checks if a triangle is coincident with another through comparison of their indices.
   */
  bool IsTriangleCoincident(const csTriangle& t0, const csTriangle& t1) const;

  /**
   * Checks if a triangle is coincident with any of its neighboring triangles
   */
  bool IsCoincident(const WorkMesh& k, const csTriangle& tri) const;

  /**
   * Utility function - prints out a message when in verbose mode
   */
  int Message(const char* format, ...) const;
};

#endif // __LODGEN_H__
