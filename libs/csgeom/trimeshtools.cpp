/*
  Copyright (C) 2007 by Jorrit Tyberghein

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
#include "csqsqrt.h"
#include "csutil/hash.h"
#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/trimeshtools.h"
#include "csgeom/trimesh.h"
#include "csgeom/poly3d.h"
#include "csgeom/math3d.h"
#include "igeom/trimesh.h"

void csTriangleMeshTools::CalculateNormals (iTriangleMesh* mesh,
  csVector3* normals)
{
  size_t p;
  csVector3* verts = mesh->GetVertices ();
  size_t num_verts = mesh->GetVertexCount ();
  (void)num_verts;
  size_t num_tri = mesh->GetTriangleCount ();
  csTriangle* tri = mesh->GetTriangles ();
  for (p = 0 ; p < num_tri ; p++)
  {
    int a = tri->a;
    int b = tri->b;
    int c = tri->c;
    CS_ASSERT (a >= 0 && a < (int)num_verts);
    CS_ASSERT (b >= 0 && b < (int)num_verts);
    CS_ASSERT (c >= 0 && c < (int)num_verts);
    csMath3::CalcNormal (normals[p], verts[a], verts[b], verts[c]);
    tri++;
  }
}

void csTriangleMeshTools::CalculatePlanes (iTriangleMesh* mesh,
  csPlane3* planes)
{
  size_t p;
  csVector3* verts = mesh->GetVertices ();
  size_t num_verts = mesh->GetVertexCount ();
  (void)num_verts;
  size_t num_tri = mesh->GetTriangleCount ();
  csTriangle* tri = mesh->GetTriangles ();
  for (p = 0 ; p < num_tri ; p++)
  {
    int a = tri->a;
    int b = tri->b;
    int c = tri->c;
    CS_ASSERT (a >= 0 && a < (int)num_verts);
    CS_ASSERT (b >= 0 && b < (int)num_verts);
    CS_ASSERT (c >= 0 && c < (int)num_verts);
    planes[p].Set (verts[c], verts[b], verts[a]);
    planes[p].Normalize ();
    tri++;
  }
}

void csTriangleMeshTools::CalculatePlanes (csVector3* vertices,
	csTriangleMinMax* tris, size_t num_tris,
  	csPlane3* planes)
{
  size_t p;
  for (p = 0 ; p < num_tris ; p++)
  {
    planes[p].Set (
    	vertices[tris[p].a],
    	vertices[tris[p].b],
    	vertices[tris[p].c]);
  }
}

/// used by CalculateEdges()
struct LinkedTriEdge : public csTriangleMeshEdge
{
  LinkedTriEdge* next;
};
/// used by CalculateEdges()
struct _FreeTriEdge
{
  LinkedTriEdge* list;
  LinkedTriEdge* next;
  
  _FreeTriEdge(): list(0), next(0) {};
  ~_FreeTriEdge()
  {
    while (list)
    {
      next = list->next;
      delete list;
      list = next;
    }
  }
};

CS_IMPLEMENT_STATIC_VAR (GetTriEdgeList, _FreeTriEdge, ())

csTriangleMeshEdge* csTriangleMeshTools::CalculateEdges (iTriangleMesh* mesh,
	size_t& num_edges)
{
  size_t num_vertices = mesh->GetVertexCount ();
  size_t num_tris = mesh->GetTriangleCount ();
  if (num_tris == 0 || num_vertices == 0) return 0;
  _FreeTriEdge* FreeTriEdge = GetTriEdgeList ();

  // First we create a table indexed by the first vertex index of every
  // edge. Every entry of this table will then contain a linked list of
  // edges which all start with that vertex.
  LinkedTriEdge** edge_table = new LinkedTriEdge* [num_vertices];
  memset (edge_table, 0, sizeof (LinkedTriEdge*) * num_vertices);

  // Loop over every polygon and add the edges.
  num_edges = 0;
  LinkedTriEdge* edge_collector = 0;	// Here we collect edges.
  size_t i, j;
  csTriangle* tri = mesh->GetTriangles ();
  for (i = 0 ; i < num_tris ; i++, tri++)
  {
    int vt2 = tri->c;
    for (j = 0 ; j < 3 ; j++)
    {
      int vt1 = (*tri)[j];

      // Handle an edge from vt1 to vt2.
      CS_ASSERT (vt1 >= 0 && vt1 < (int)num_vertices);
      CS_ASSERT (vt2 >= 0 && vt2 < (int)num_vertices);
      int vt1s, vt2s;
      if (vt1 < vt2) { vt1s = vt1; vt2s = vt2; }
      else { vt1s = vt2; vt2s = vt1; }
      LinkedTriEdge* le = edge_table[vt1s];

      // There are two cases here. Either we find the edge
      // in the edge_table. In that case we previously encountered
      // another triangle that shares that edge. In that case we will
      // add the edge to the linked list of real edges and remove
      // it from the edge table. If we later discover another triangle
      // with that edge then it will become a new edge.
      // If the edge is not in the table yet we will add it.
      LinkedTriEdge* prev = 0;
      while (le)
      {
        CS_ASSERT (le->vt1 == vt1s);
        if (le->vt2 == vt2s)
	{
	  // Found!
	  le->tri2 = int (i);
	  if (prev) prev->next = le->next;
	  else edge_table[vt1s] = le->next;
	  le->next = edge_collector;
	  edge_collector = le;
	  break;
	}
	prev = le;
	le = le->next;
      }
      if (!le)
      {
        // Not found!
	num_edges++;
	// Take a free edge from free_edges if any.
	if (FreeTriEdge->list)
	{
	  le = FreeTriEdge->list;
	  FreeTriEdge->list = FreeTriEdge->list->next;
	}
	else
	{
	  le = new LinkedTriEdge ();
	}
	le->vt1 = vt1s;
	le->vt2 = vt2s;
	le->tri1 = int (i);
	le->tri2 = -1;
	le->next = edge_table[vt1s];
	edge_table[vt1s] = le;
      }

      vt2 = vt1;
    }
  }

  // After the previous loop we basically have two sets of edges.
  // First there are the edges we are fully processed (have two attached
  // triangles). These were added to edge_collector. Secondly we have
  // the edges that have no second triangle. These will still be in
  // the edge_table. Here we add them together.
  // 'num_edges' will contain the correct number of total edges already.
  csTriangleMeshEdge* edges = new csTriangleMeshEdge [num_edges];
  csTriangleMeshEdge* e = edges;
  i = 0;
  while (edge_collector)
  {
    e->vt1 = edge_collector->vt1;
    e->vt2 = edge_collector->vt2;
    e->tri1 = edge_collector->tri1;
    e->tri2 = edge_collector->tri2;
    e++;
    i++;
    LinkedTriEdge* el = edge_collector;
    edge_collector = edge_collector->next;
    el->next = FreeTriEdge->list;
    FreeTriEdge->list = el;
  }
  for (j = 0 ; j < num_vertices ; j++)
  {
    LinkedTriEdge* et = edge_table[j];
    while (et)
    {
      e->vt1 = et->vt1;
      e->vt2 = et->vt2;
      e->tri1 = et->tri1;
      e->tri2 = et->tri2;
      e++;
      i++;
      LinkedTriEdge* el = et;
      et = et->next;
      el->next = FreeTriEdge->list;
      FreeTriEdge->list = el;
    }
  }
  delete[] edge_table;
  CS_ASSERT (i == num_edges);
  return edges;
}

size_t csTriangleMeshTools::CheckActiveEdges (
	csTriangleMeshEdge* edges, size_t num_edges,
  	csPlane3* planes)
{
  if (num_edges == (size_t)~0) return 0;
  size_t i;
  csTriangleMeshEdge* e = edges;
  int active = 0;
  for (i = 0 ; i < num_edges ; i++, e++)
  {
    if (e->tri2 == -1)
    {
      e->active = true;
      active++;
    }
    else
    {
      // Since we know the two planes connect we only have to check
      // the normals.
      if ((planes[e->tri1].norm - planes[e->tri2].norm) < EPSILON)
        e->active = false;
      else
      {
        e->active = true;
        active++;
      }
    }
  }
  return active;
}

void csTriangleMeshTools::CalculateOutline (
	csTriangleMeshEdge* edges, size_t num_edges,
  	csPlane3* planes, size_t num_vertices,
	const csVector3& pos,
	size_t* outline_edges, size_t& num_outline_edges,
	bool* outline_verts,
	float& valid_radius)
{
  size_t i;
  csTriangleMeshEdge* e = edges;
  num_outline_edges = 0;
  valid_radius = 10000000.0;
  for (i = 0 ; i < num_vertices ; i++)
  {
    outline_verts[i] = false;
  }
  for (i = 0 ; i < num_edges ; i++, e++)
  {
    if (!e->active) continue;

    if (e->tri2 == -1)
    {
      // If the edge is attached to only one edge it is an outline
      // edge automatically.
      *outline_edges++ = e->vt1;
      *outline_edges++ = e->vt2;
      num_outline_edges++;
      CS_ASSERT (num_outline_edges <= num_edges);
      CS_ASSERT (e->vt1 >= 0 && e->vt1 < (int)num_vertices);
      CS_ASSERT (e->vt2 >= 0 && e->vt2 < (int)num_vertices);
      outline_verts[e->vt1] = true;	// Mark vertices as in use.
      outline_verts[e->vt2] = true;
    }
    else
    {
      // Otherwise we have to compare the plane equations of the two
      // adjacent planes.
      const csPlane3& pl1 = planes[e->tri1];
      const csPlane3& pl2 = planes[e->tri2];
      float cl1 = pl1.Classify (pos);
      float cl2 = pl2.Classify (pos);
      if ((cl1 < 0 && cl2 > 0) || (cl1 > 0 && cl2 < 0))
      {
        // Sign is different. So this is an outline.
        *outline_edges++ = e->vt1;
        *outline_edges++ = e->vt2;
        num_outline_edges++;
        CS_ASSERT (num_outline_edges <= num_edges);
        CS_ASSERT (e->vt1 >= 0 && e->vt1 < (int)num_vertices);
        CS_ASSERT (e->vt2 >= 0 && e->vt2 < (int)num_vertices);
        outline_verts[e->vt1] = true;	// Mark vertices as in use.
        outline_verts[e->vt2] = true;
      }
      // Calculate minimum distance at which this edge changes status
      // (from in to out or vice versa).
      float cl = MIN (ABS (cl1), ABS (cl2));
      if (cl < valid_radius) valid_radius = cl;
    }
  }
}

struct TriEdge
{
  int v1, v2;
  bool flipped;

  TriEdge (int v1, int v2)
  {
    if (v1 > v2)
    {
      TriEdge::v1 = v2;
      TriEdge::v2 = v1;
      flipped = true;
    }
    else
    {
      TriEdge::v1 = v1;
      TriEdge::v2 = v2;
      flipped = false;
    }
  }
};

struct AdjacencyCounter
{
  int adjNormal, adjFlipped;

  AdjacencyCounter () : adjNormal(0), adjFlipped(0)
  {
  }
  AdjacencyCounter (const AdjacencyCounter* other) : adjNormal(0), 
    adjFlipped(0)
  {
    if (other)
      *this = *other;
  }
};

template<>
class csComparator<TriEdge, TriEdge>
{
public:
  static int Compare (TriEdge const& key1, TriEdge const& key2)
  {
    if (key1.v1 != key2.v1)
      return key1.v1 - key2.v1;
    else
      return key1.v2 - key2.v2;
  }
};

template<>
class csHashComputer<TriEdge>
{
public:
  static uint ComputeHash (TriEdge const& key)
  {
    uint32 key2 = 
      (key.v2 >> 24) | ((key.v2 >> 8) & 0xff00) | 
      ((key.v2 << 8) & 0xff0000) | (key.v2 << 24);
    return (((uint32)key.v1) ^ key2);
  }
};

/*
  The closedness test works by counting the faces to adjacent edges.
  An even number of faces facing into the same direction must be adjacent
  to every edge.

  Given two vertices A and B, we count the number of faces that are
  adjacent to A->B and B->A. (If face 1 has the A->B edge, in an adjacent
  face that shares that edge and and faces into the same direction as
  face 1 this edge will appear as B->A. This is due that vertex order
  defines the direction.) The number of faces that have A->B in them must
  be equal to the number of faces that contain B->A. Otherwise, the mesh
  isn't closed.
 */
bool csTriangleMeshTools::IsMeshClosed (iTriangleMesh* trimesh)
{
  csHash<AdjacencyCounter, TriEdge> adjacency;

  size_t numIncorrect = 0;

  size_t pc = trimesh->GetTriangleCount ();
  csTriangle* tris = trimesh->GetTriangles ();
  size_t p;
  for (p = 0; p < pc; p++)
  {
    const csTriangle& tri = tris[p];
    int v1 = tri.c;
    int v;
    for (v = 0; v < 3; v++)
    {
      int v2 = tri[v];

      TriEdge edge (v1, v2);
      AdjacencyCounter counter (adjacency.GetElementPointer (edge));

      if (counter.adjFlipped != counter.adjNormal)
      {
	numIncorrect--;
      }

      if (edge.flipped)
      {
	counter.adjFlipped++;
      }
      else
      {
	counter.adjNormal++;
      }

      if (counter.adjFlipped != counter.adjNormal)
      {
	numIncorrect++;
      }

      adjacency.PutUnique (edge, counter);

      v1 = v2;
    }
  }

  return (numIncorrect == 0);
}

bool csTriangleMeshTools::IsMeshConvex (iTriangleMesh* trimesh)
{
  size_t num_edges;
  csTriangleMeshEdge* edges = CalculateEdges (trimesh, num_edges);

  size_t pccount = trimesh->GetTriangleCount ();
  csArray<size_t>* adjacent_tris;
  adjacent_tris = new csArray<size_t> [pccount];
  csTriangle* tris = trimesh->GetTriangles ();
  csVector3* verts = trimesh->GetVertices ();
  size_t i;

  // Calculate adjacency information for polygons.
  for (i = 0 ; i < num_edges ; i++)
  {
    csTriangleMeshEdge& me = edges[i];
    if (me.tri2 != -1)
    {
      adjacent_tris[me.tri1].Push (me.tri2);
      adjacent_tris[me.tri2].Push (me.tri1);
    }
    else
    {
      delete[] adjacent_tris;
      delete[] edges;
      return false;
    }
  }

  // Calculate the center of every triangle.
  csVector3* poly_centers = new csVector3 [pccount];
  for (i = 0 ; i < pccount ; i++)
  {
    const csTriangle& tri = tris[i];
    csVector3 center = verts[tri.a] + verts[tri.b] + verts[tri.c];
    center /= 3.0f;
    poly_centers[i] = center;
  }

  // For every polygon we test if the adjacent polygons are all
  // at the same side of the testing polygon.
  bool convex = false;
  int side = 0;
  for (i = 0 ; i < pccount ; i++)
  {
    const csTriangle& tri = tris[i];
    csPlane3 plane (verts[tri.a], verts[tri.b], verts[tri.c]);
    size_t j;
    const csArray<size_t>& ap = adjacent_tris[i];
    for (j = 0 ; j < ap.GetSize () ; j++)
    {
      float cl = plane.Classify (poly_centers[ap[j]]);
      if (cl > SMALL_EPSILON)
      {
        if (side == -1) goto end;
	side = 1;
      }
      if (cl < -SMALL_EPSILON)
      {
        if (side == 1) goto end;
	side = -1;
        goto end;
      }
    }
  }
  convex = true;

end:
  delete[] poly_centers;
  delete[] adjacent_tris;
  delete[] edges;

  return convex;
}

void csTriangleMeshTools::CloseMesh (iTriangleMesh* trimesh, 
				 csArray<csTriangle>& newtris)
{
  size_t tc = trimesh->GetTriangleCount ();
  csTriangle* tris = trimesh->GetTriangles ();
  size_t p;
  newtris.SetMinimalCapacity (tc);
  for (p = 0; p < tc; p++)
  {
    const csTriangle& tri = tris[p];
    csTriangle newtri (tri.c, tri.b, tri.a);
    newtris.Push (newtri);
  }
}

static int compare_sort_x (const void* v1, const void* v2)
{
  const csTriangleMinMax* t1 = (csTriangleMinMax*)v1;
  const csTriangleMinMax* t2 = (csTriangleMinMax*)v2;

  float minx1 = t1->minx;
  float minx2 = t2->minx;

  if (minx1 < minx2) return -1;
  else if (minx1 > minx2) return 1;
  else return 0;
}

void csTriangleMeshTools::SortTrianglesX (iTriangleMesh* trimesh,
  	csTriangleMinMax*& tris, size_t& tri_count,
	csPlane3*& planes)
{
  csTriangle* mesh_tris;
  size_t i;
  tri_count = trimesh->GetTriangleCount ();
  mesh_tris = trimesh->GetTriangles ();

  tris = new csTriangleMinMax[tri_count];
  for (i = 0 ; i < tri_count ; i++)
  {
    tris[i].a = mesh_tris[i].a;
    tris[i].b = mesh_tris[i].b;
    tris[i].c = mesh_tris[i].c;
  }

  csVector3* vertices = trimesh->GetVertices ();
  for (i = 0 ; i < tri_count ; i++)
  {
    float x;
    float minx1 = vertices[tris[i].a].x;
    float maxx1 = minx1;
    x = vertices[tris[i].b].x;
    if (x < minx1) minx1 = x;
    if (x > maxx1) maxx1 = x;
    x = vertices[tris[i].c].x;
    if (x < minx1) minx1 = x;
    if (x > maxx1) maxx1 = x;
    tris[i].minx = minx1;
    tris[i].maxx = maxx1;
  }

  qsort (tris, tri_count, sizeof (csTriangleMinMax), compare_sort_x);

  planes = new csPlane3[tri_count];
  CalculatePlanes (trimesh->GetVertices (), tris, tri_count, planes);
}

static bool IntersectPlane_X (
	const csVector3& point,
	const csPlane3& p,
	csVector3& isect)
{
  float denom = -p.norm.x;
  if (fabs (denom) < SMALL_EPSILON) return false;
  float dist = (p.norm * point + p.DD) / denom;
  if (dist < -SMALL_EPSILON) return false;

  isect.Set (point.x + dist, point.y, point.z);
  return true;
}

static int WhichSide2D_X (const csVector3& v,
                          const csVector3& s1, const csVector3& s2)
{
  float k  = (s1.z - v.z)*(s2.y - s1.y);
  float k1 = (s1.y - v.y)*(s2.z - s1.z);
  if (k < k1) return -1;
  else if (k > k1) return 1;
  else return 0;
}

static bool In2D_X (const csVector3& v1, const csVector3& v2,
	const csVector3& v3, const csVector3 &v)
{
  int side = WhichSide2D_X (v, v1, v2);
  int s = WhichSide2D_X (v, v2, v3);
  if ((side < 0 && s > 0) || (side > 0 && s < 0)) return false;
  s = WhichSide2D_X (v, v3, v1);
  if ((side < 0 && s > 0) || (side > 0 && s < 0)) return false;
  return true;
}

bool csTriangleMeshTools::PointInClosedMesh (const csVector3& point,
  	csVector3* vertices,
  	csTriangleMinMax* tris, size_t tri_count,
	csPlane3* planes)
{
  // This algorithm assumes the triangles are sorted from left to right
  // (minimum x). That way we can do some quick rejections.
  float nearest_found_x = 1000000000.0f;
  size_t nearest_idx = (size_t)-1;
  size_t i;
  for (i = 0 ; i < tri_count ; i++)
  {
    // If the minimum x is greater then the nearest found x then
    // we can stop testing.
    if (tris[i].minx > nearest_found_x) break;

    // If the maximum x is smaller then point.x then we don't have
    // to test this triangle.
    if (tris[i].maxx <= point.x) continue;

    // Try to intersect.
    csVector3 isect;
    if (IntersectPlane_X (point, planes[i], isect))
    {
      // If the intersection point is greater then nearest_found_x
      // then we don't have to test further.
      if (isect.x < nearest_found_x)
        if (In2D_X (vertices[tris[i].a], vertices[tris[i].b],
      	  vertices[tris[i].c], point))
        {
	  nearest_idx = i;
	  nearest_found_x = isect.x;
        }
    }
  }

  if (nearest_idx == (size_t)-1)
  {
    // We found no triangle. So we are certainly outside.
    return false;
  }

  // Now we found the triangle that is closest to our point. We
  // check if we can see that triangle (backface culling). If we can
  // then it means we are outside the object. Otherwise we are inside.
  return planes[nearest_idx].Classify (point) < 0;
}

bool csTriangleMeshTools::LineInClosedMesh (
	const csVector3& p1, const csVector3& p2,
  	csVector3* vertices,
  	csTriangleMinMax* tris, size_t tri_count,
	csPlane3* /*planes*/)
{
  size_t i;
  float minx = p1.x;
  float maxx = minx;
  if (p2.x < minx) minx = p2.x;
  if (p2.x > maxx) maxx = p2.x;
  csSegment3 seg (p1, p2);

  for (i = 0 ; i < tri_count ; i++)
  {
    // Quick reject of triangles that are outside minx/maxx range.
    if (tris[i].maxx < minx) continue;
    if (tris[i].minx > maxx) continue;

    // Try to intersect.
    csVector3 isect;
    if (csIntersect3::SegmentTriangle (seg, vertices[tris[i].a],
    	vertices[tris[i].b], vertices[tris[i].c], isect))
    {
      return false;
    }
  }

  return true;
}

bool csTriangleMeshTools::BoxInClosedMesh (const csBox3& box,
  	csVector3* vertices,
  	csTriangleMinMax* tris, size_t tri_count,
	csPlane3* /*planes*/)
{
  size_t i;
  float minx = box.MinX ();
  float maxx = box.MaxX ();

  for (i = 0 ; i < tri_count ; i++)
  {
    // Quick reject of triangles that are outside minx/maxx range.
    if (tris[i].maxx < minx) continue;
    if (tris[i].minx > maxx) continue;

    // Try to intersect.
    if (csIntersect3::BoxTriangle (box, vertices[tris[i].a],
    	vertices[tris[i].b], vertices[tris[i].c]))
    {
      return false;
    }
  }

  return true;
}

csArray<csArray<int> >* csTriangleMeshTools::CalculateVertexConnections (
    iTriangleMesh* mesh)
{
  size_t i,j,k;
  size_t vert_count = mesh->GetVertexCount ();
  csArray<csArray<int> > *link_array = new csArray<csArray<int> >(vert_count);

  // Initialize array
  csArray<int> newarray;
  for (i = 0 ; i < vert_count ; i++)
    link_array->Put(i,newarray);

  // Fill connection information.
  size_t tri_count = mesh->GetTriangleCount ();
  csTriangle* tri = mesh->GetTriangles ();
  for (i = 0 ; i < tri_count ; i++, tri++)
  {
    for (j = 0 ; j < 3 ; j++)
    {
      int vt1 = (*tri)[j];
      csArray<int> *vset = &link_array->Get(vt1);
      for (k = 0 ; k < 3 ; k++)
      {
        int vt2 = (*tri)[k];
        if (vt1 != vt2)
        {
          vset->PushSmart(vt2);
        }
      }
    }
  }
  return link_array;
}

