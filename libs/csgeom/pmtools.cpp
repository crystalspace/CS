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
#include "csqsqrt.h"
#include "csutil/hash.h"
#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/pmtools.h"
#include "csgeom/polymesh.h"
#include "csgeom/poly3d.h"
#include "csgeom/math3d.h"
#include "igeom/polymesh.h"

void csPolygonMeshTools::CalculateNormals (iPolygonMesh* mesh,
  csVector3* normals)
{
  int p;
  csVector3* verts = mesh->GetVertices ();
  int num_verts = mesh->GetVertexCount ();
  (void)num_verts;
  int num_poly = mesh->GetPolygonCount ();
  csMeshedPolygon* poly = mesh->GetPolygons ();
  for (p = 0 ; p < num_poly ; p++)
  {
    float ayz = 0;
    float azx = 0;
    float axy = 0;
    int i, i1;
    float x1, y1, z1, x, y, z;

    int* vi = poly->vertices;
    i1 = poly->num_vertices - 1;
    CS_ASSERT (vi[i1] >= 0 && vi[i1] < num_verts);
    x1 = verts[vi[i1]].x;
    y1 = verts[vi[i1]].y;
    z1 = verts[vi[i1]].z;
    for (i = 0 ; i < poly->num_vertices ; i++)
    {
      CS_ASSERT (vi[i] >= 0 && vi[i] < num_verts);
      x = verts[vi[i]].x;
      y = verts[vi[i]].y;
      z = verts[vi[i]].z;
      ayz += (z1 + z) * (y - y1);
      azx += (x1 + x) * (z - z1);
      axy += (y1 + y) * (x - x1);
      x1 = x;
      y1 = y;
      z1 = z;
    }

    float sqd = ayz * ayz + azx * azx + axy * axy;
    float invd;
    if (sqd < SMALL_EPSILON)
      invd = 1.0f / SMALL_EPSILON;
    else
      invd = csQisqrt (sqd);
    normals[p].Set (ayz * invd, azx * invd, axy * invd);

    poly++;
  }
}

void csPolygonMeshTools::CalculatePlanes (iPolygonMesh* mesh,
  csPlane3* planes)
{
  int p;
  csVector3* verts = mesh->GetVertices ();
  int num_verts = mesh->GetVertexCount ();
  (void)num_verts;
  int num_poly = mesh->GetPolygonCount ();
  csMeshedPolygon* poly = mesh->GetPolygons ();
  for (p = 0 ; p < num_poly ; p++)
  {
    float ayz = 0;
    float azx = 0;
    float axy = 0;
    int i, i1;
    float x1, y1, z1, x, y, z;

    int* vi = poly->vertices;
    i1 = poly->num_vertices - 1;
    CS_ASSERT (vi[i1] >= 0 && vi[i1] < num_verts);
    x1 = verts[vi[i1]].x;
    y1 = verts[vi[i1]].y;
    z1 = verts[vi[i1]].z;
    for (i = 0 ; i < poly->num_vertices ; i++)
    {
      CS_ASSERT (vi[i] >= 0 && vi[i] < num_verts);
      x = verts[vi[i]].x;
      y = verts[vi[i]].y;
      z = verts[vi[i]].z;
      ayz += (z1 + z) * (y - y1);
      azx += (x1 + x) * (z - z1);
      axy += (y1 + y) * (x - x1);
      x1 = x;
      y1 = y;
      z1 = z;
    }

    float sqd = ayz * ayz + azx * azx + axy * axy;
    float invd;
    if (sqd < SMALL_EPSILON)
      invd = 1.0f / SMALL_EPSILON;
    else
      invd = csQisqrt (sqd);
    planes[p].norm.Set (ayz * invd, azx * invd, axy * invd);
    planes[p].DD = - planes[p].norm * verts[vi[0]];

    poly++;
  }
}

void csPolygonMeshTools::CalculatePlanes (csVector3* vertices,
	csTriangleMinMax* tris, int num_tris,
  	csPlane3* planes)
{
  int p;
  for (p = 0 ; p < num_tris ; p++)
  {
    planes[p].Set (
    	vertices[tris[p].a],
    	vertices[tris[p].b],
    	vertices[tris[p].c]);
  }
}

/// used by CalculateEdges()
struct LinkedEdge : public csPolygonMeshEdge
{
  LinkedEdge* next;
};
/// used by CalculateEdges()
struct _FreeEdge
{
  LinkedEdge* list;
  LinkedEdge* next;
  
  _FreeEdge(): list(0), next(0) {};
  ~_FreeEdge()
  {
    while (list)
    {
      next = list->next;
      delete list;
      list = next;
    }
  }
};

CS_IMPLEMENT_STATIC_VAR (GetEdgeList, _FreeEdge, ())

csPolygonMeshEdge* csPolygonMeshTools::CalculateEdges (iPolygonMesh* mesh,
	int& num_edges)
{
  int num_vertices = mesh->GetVertexCount ();
  int num_polygons = mesh->GetPolygonCount ();
  if (num_polygons == 0 || num_vertices == 0) return 0;
  _FreeEdge* FreeEdge = GetEdgeList ();

  // First we create a table indexed by the first vertex index of every
  // edge. Every entry of this table will then contain a linked list of
  // edges which all start with that vertex.
  LinkedEdge** edge_table = new LinkedEdge* [num_vertices];
  memset (edge_table, 0, sizeof (LinkedEdge*) * num_vertices);

  // Loop over every polygon and add the edges.
  num_edges = 0;
  LinkedEdge* edge_collector = 0;	// Here we collect edges.
  int i, j;
  csMeshedPolygon* poly = mesh->GetPolygons ();
  for (i = 0 ; i < num_polygons ; i++, poly++)
  {
    int* vi = poly->vertices;
    int vt2 = vi[poly->num_vertices-1];
    for (j = 0 ; j < poly->num_vertices ; j++)
    {
      int vt1 = vi[j];

      // Handle an edge from vt1 to vt2.
      CS_ASSERT (vt1 >= 0 && vt1 < num_vertices);
      CS_ASSERT (vt2 >= 0 && vt2 < num_vertices);
      int vt1s, vt2s;
      if (vt1 < vt2) { vt1s = vt1; vt2s = vt2; }
      else { vt1s = vt2; vt2s = vt1; }
      LinkedEdge* le = edge_table[vt1s];

      // There are two cases here. Either we find the edge
      // in the edge_table. In that case we previously encountered
      // another polygon that shares that edge. In that case we will
      // add the edge to the linked list of real edges and remove
      // it from the edge table. If we later discover another polygon
      // with that edge then it will become a new edge.
      // If the edge is not in the table yet we will add it.
      LinkedEdge* prev = 0;
      while (le)
      {
        CS_ASSERT (le->vt1 == vt1s);
        if (le->vt2 == vt2s)
	{
	  // Found!
	  le->poly2 = i;
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
	if (FreeEdge->list)
	{
	  le = FreeEdge->list;
	  FreeEdge->list = FreeEdge->list->next;
	}
	else
	{
	  le = new LinkedEdge ();
	}
	le->vt1 = vt1s;
	le->vt2 = vt2s;
	le->poly1 = i;
	le->poly2 = -1;
	le->next = edge_table[vt1s];
	edge_table[vt1s] = le;
      }

      vt2 = vt1;
    }
  }

  // After the previous loop we basically have two sets of edges.
  // First there are the edges we are fully processed (have two attached
  // polygons). These were added to edge_collector. Secondly we have
  // the edges that have no second polygon. These will still be in
  // the edge_table. Here we add them together.
  // 'num_edges' will contain the correct number of total edges already.
  csPolygonMeshEdge* edges = new csPolygonMeshEdge [num_edges];
  csPolygonMeshEdge* e = edges;
  i = 0;
  while (edge_collector)
  {
    e->vt1 = edge_collector->vt1;
    e->vt2 = edge_collector->vt2;
    e->poly1 = edge_collector->poly1;
    e->poly2 = edge_collector->poly2;
    e++;
    i++;
    LinkedEdge* el = edge_collector;
    edge_collector = edge_collector->next;
    el->next = FreeEdge->list;
    FreeEdge->list = el;
  }
  for (j = 0 ; j < num_vertices ; j++)
  {
    LinkedEdge* et = edge_table[j];
    while (et)
    {
      e->vt1 = et->vt1;
      e->vt2 = et->vt2;
      e->poly1 = et->poly1;
      e->poly2 = et->poly2;
      e++;
      i++;
      LinkedEdge* el = et;
      et = et->next;
      el->next = FreeEdge->list;
      FreeEdge->list = el;
    }
  }
  delete[] edge_table;
  CS_ASSERT (i == num_edges);
  return edges;
}

int csPolygonMeshTools::CheckActiveEdges (
	csPolygonMeshEdge* edges, int num_edges,
  	csPlane3* planes)
{
  int i;
  csPolygonMeshEdge* e = edges;
  int active = 0;
  for (i = 0 ; i < num_edges ; i++, e++)
  {
    if (e->poly2 == -1)
    {
      e->active = true;
      active++;
    }
    else
    {
      // Since we know the two planes connect we only have to check
      // the normals.
      if ((planes[e->poly1].norm - planes[e->poly2].norm) < EPSILON)
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

void csPolygonMeshTools::CalculateOutline (
	csPolygonMeshEdge* edges, int num_edges,
  	csPlane3* planes, int num_vertices,
	const csVector3& pos,
	int* outline_edges, int& num_outline_edges,
	bool* outline_verts,
	float& valid_radius)
{
  int i;
  csPolygonMeshEdge* e = edges;
  num_outline_edges = 0;
  valid_radius = 10000000.0;
  for (i = 0 ; i < num_vertices ; i++)
  {
    outline_verts[i] = false;
  }
  for (i = 0 ; i < num_edges ; i++, e++)
  {
    if (!e->active) continue;

    if (e->poly2 == -1)
    {
      // If the edge is attached to only one edge it is an outline
      // edge automatically.
      *outline_edges++ = e->vt1;
      *outline_edges++ = e->vt2;
      num_outline_edges++;
      CS_ASSERT (num_outline_edges <= num_edges);
      CS_ASSERT (e->vt1 >= 0 && e->vt1 < num_vertices);
      CS_ASSERT (e->vt2 >= 0 && e->vt2 < num_vertices);
      outline_verts[e->vt1] = true;	// Mark vertices as in use.
      outline_verts[e->vt2] = true;
    }
    else
    {
      // Otherwise we have to compare the plane equations of the two
      // adjacent planes.
      const csPlane3& pl1 = planes[e->poly1];
      const csPlane3& pl2 = planes[e->poly2];
      float cl1 = pl1.Classify (pos);
      float cl2 = pl2.Classify (pos);
      if ((cl1 < 0 && cl2 > 0) || (cl1 > 0 && cl2 < 0))
      {
        // Sign is different. So this is an outline.
        *outline_edges++ = e->vt1;
        *outline_edges++ = e->vt2;
        num_outline_edges++;
        CS_ASSERT (num_outline_edges <= num_edges);
        CS_ASSERT (e->vt1 >= 0 && e->vt1 < num_vertices);
        CS_ASSERT (e->vt2 >= 0 && e->vt2 < num_vertices);
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

struct PolyEdge
{
  int v1, v2;
  bool flipped;

  PolyEdge (int v1, int v2)
  {
    if (v1 > v2)
    {
      PolyEdge::v1 = v2;
      PolyEdge::v2 = v1;
      flipped = true;
    }
    else
    {
      PolyEdge::v1 = v1;
      PolyEdge::v2 = v2;
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

class PolyEdgeHashKeyHandler
{
public:
  static uint32 ComputeHash (const PolyEdge& key)
  {
    uint32 key2 = 
      (key.v2 >> 24) | ((key.v2 >> 8) & 0xff00) | 
      ((key.v2 << 8) & 0xff0000) | (key.v2 << 24);
    return (((uint32)key.v1) ^ key2);
  }

  static bool CompareKeys (const PolyEdge& key1, const PolyEdge& key2)
  {
    return ((key1.v1 == key2.v1) && (key1.v2 == key2.v2));
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
bool csPolygonMeshTools::IsMeshClosed (iPolygonMesh* polyMesh)
{
  csHash<AdjacencyCounter, PolyEdge, PolyEdgeHashKeyHandler>
    adjacency;

  int numIncorrect = 0;

  int pc = polyMesh->GetPolygonCount ();
  csMeshedPolygon* polys = polyMesh->GetPolygons ();
  int p;
  for (p = 0; p < pc; p++)
  {
    const csMeshedPolygon& poly = polys[p];
    int v1 = poly.vertices[poly.num_vertices - 1];
    int v;
    for (v = 0; v < poly.num_vertices; v++)
    {
      int v2 = poly.vertices[v];

      PolyEdge edge (v1, v2);
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

bool csPolygonMeshTools::IsMeshConvex (iPolygonMesh* polyMesh)
{
  int num_edges;
  csPolygonMeshEdge* edges = CalculateEdges (polyMesh, num_edges);

  int pccount = polyMesh->GetPolygonCount ();
  csArray<int>* adjacent_polygons;
  adjacent_polygons = new csArray<int> [pccount];
  csMeshedPolygon* polys = polyMesh->GetPolygons ();
  csVector3* verts = polyMesh->GetVertices ();
  int i;

  // Calculate adjacency information for polygons.
  for (i = 0 ; i < num_edges ; i++)
  {
    csPolygonMeshEdge& me = edges[i];
    if (me.poly2 != -1)
    {
      adjacent_polygons[me.poly1].Push (me.poly2);
      adjacent_polygons[me.poly2].Push (me.poly1);
    }
    else
    {
      delete[] adjacent_polygons;
      delete[] edges;
      return false;
    }
  }

  // Calculate the center of every polygon.
  csVector3* poly_centers = new csVector3 [pccount];
  for (i = 0 ; i < pccount ; i++)
  {
    int* vi = polys[i].vertices;
    csVector3 center = verts[vi[0]];
    int j;
    for (j = 1 ; j < polys[i].num_vertices ; j++)
      center += verts[vi[j]];
    center /= float (polys[i].num_vertices);
    poly_centers[i] = center;
  }

  // For every polygon we test if the adjacent polygons are all
  // at the same side of the testing polygon.
  bool convex = false;
  int side = 0;
  for (i = 0 ; i < pccount ; i++)
  {
    int* vi = polys[i].vertices;
    csPlane3 plane = csPoly3D::ComputePlane (vi, polys[i].num_vertices,
    	verts);
    size_t j;
    const csArray<int>& ap = adjacent_polygons[i];
    for (j = 0 ; j < ap.Length () ; j++)
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
  delete[] adjacent_polygons;
  delete[] edges;

  return convex;
}

void csPolygonMeshTools::CloseMesh (iPolygonMesh* polyMesh, 
				 csArray<csMeshedPolygon>& newPolys,
				 int*& vertidx, int& vertidx_len)
{
  if (polyMesh->GetFlags ().Check (CS_POLYMESH_TRIANGLEMESH))
  {
    int tc = polyMesh->GetTriangleCount ();
    csTriangle* tris = polyMesh->GetTriangles ();
    int p;
    vertidx_len = tc * 3;
    vertidx = new int[vertidx_len];
    int* vertidx_p = vertidx;

    for (p = 0; p < tc; p++)
    {
      const csTriangle& tri = tris[p];
      csMeshedPolygon newPoly;
      newPoly.num_vertices = 3;
      newPoly.vertices = vertidx_p;
      *vertidx_p++ = tri.c;
      *vertidx_p++ = tri.b;
      *vertidx_p++ = tri.a;
      newPolys.Push (newPoly);
    }
  }
  else
  {
    int pc = polyMesh->GetPolygonCount ();
    csMeshedPolygon* polys = polyMesh->GetPolygons ();
    int p;
    vertidx_len = 0;
    for (p = 0; p < pc; p++)
    {
      const csMeshedPolygon& poly = polys[p];
      vertidx_len += poly.num_vertices;
    }
    vertidx = new int[vertidx_len];
    int* vertidx_p = vertidx;

    for (p = 0; p < pc; p++)
    {
      const csMeshedPolygon& poly = polys[p];
      csMeshedPolygon newPoly;
      newPoly.num_vertices = poly.num_vertices;
      newPoly.vertices = vertidx_p;
      int v;
      for (v = 0; v < poly.num_vertices; v++)
      {
        newPoly.vertices[v] = poly.vertices[poly.num_vertices - 1 - v];
      }
      vertidx_p += poly.num_vertices;
      newPolys.Push (newPoly);
    }
  }
}

void csPolygonMeshTools::Triangulate (iPolygonMesh* polymesh,
	csTriangle*& tris, int& tri_count)
{
  tri_count = 0;
  int pc = polymesh->GetPolygonCount ();
  if (!pc) { tris = 0; return; }
  csMeshedPolygon* polys = polymesh->GetPolygons ();
  int p;
  for (p = 0 ; p < pc ; p++)
  {
    const csMeshedPolygon& poly = polys[p];
    tri_count += poly.num_vertices-2;
  }

  tris = new csTriangle[tri_count];
  tri_count = 0;
  for (p = 0 ; p < pc ; p++)
  {
    const csMeshedPolygon& poly = polys[p];
    int i;
    for (i = 2 ; i < poly.num_vertices ; i++)
    {
      tris[tri_count].a = poly.vertices[i-1];
      tris[tri_count].b = poly.vertices[i];
      tris[tri_count].c = poly.vertices[0];
      tri_count++;
    }
  }
}

void csPolygonMeshTools::Polygonize (iPolygonMesh* polymesh,
  	csMeshedPolygon*& polygons, int& poly_count)
{
  poly_count = polymesh->GetTriangleCount ();
  csTriangle* tris = polymesh->GetTriangles ();
  polygons = new csMeshedPolygon[poly_count];
  int p;
  for (p = 0 ; p < poly_count ; p++)
  {
    csMeshedPolygon& poly = polygons[p];
    poly.num_vertices = 3;
    poly.vertices = (int*)&tris[p];
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

void csPolygonMeshTools::SortTrianglesX (iPolygonMesh* polymesh,
  	csTriangleMinMax*& tris, int& tri_count,
	csPlane3*& planes)
{
  csTriangle* mesh_tris;
  bool delete_mesh_tris = false;
  int i;
  if (polymesh->GetFlags ().Check (CS_POLYMESH_TRIANGLEMESH))
  {
    tri_count = polymesh->GetTriangleCount ();
    mesh_tris = polymesh->GetTriangles ();
  }
  else
  {
    Triangulate (polymesh, mesh_tris, tri_count);
    delete_mesh_tris = true;
  }
  tris = new csTriangleMinMax[tri_count];
  for (i = 0 ; i < tri_count ; i++)
  {
    tris[i].a = mesh_tris[i].a;
    tris[i].b = mesh_tris[i].b;
    tris[i].c = mesh_tris[i].c;
  }
  if (delete_mesh_tris) delete[] mesh_tris;

  csVector3* vertices = polymesh->GetVertices ();
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
  CalculatePlanes (polymesh->GetVertices (),
	tris, tri_count, planes);
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

bool csPolygonMeshTools::PointInClosedMesh (const csVector3& point,
  	csVector3* vertices,
  	csTriangleMinMax* tris, int tri_count,
	csPlane3* planes)
{
  // This algorithm assumes the triangles are sorted from left to right
  // (minimum x). That way we can do some quick rejections.
  float nearest_found_x = 1000000000.0f;
  int nearest_idx = -1;
  int i;
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

  if (nearest_idx == -1)
  {
    // We found no triangle. So we are certainly outside.
    return false;
  }

  // Now we found the triangle that is closest to our point. We
  // check if we can see that triangle (backface culling). If we can
  // then it means we are outside the object. Otherwise we are inside.
  return planes[nearest_idx].Classify (point) < 0;
}

bool csPolygonMeshTools::LineInClosedMesh (
	const csVector3& p1, const csVector3& p2,
  	csVector3* vertices,
  	csTriangleMinMax* tris, int tri_count,
	csPlane3* planes)
{
  int i;
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

bool csPolygonMeshTools::BoxInClosedMesh (const csBox3& box,
  	csVector3* vertices,
  	csTriangleMinMax* tris, int tri_count,
	csPlane3* planes)
{
  int i;
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

