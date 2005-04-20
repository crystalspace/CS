/*
    Copyright (C) 2002 by Jorrit Tyberghein
    Copyright (C) 2005 by Eric Sunshine <sunshine@sunshineco.com>

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

#include "csgeom/math3d.h"
#include "csgeom/pmtools.h"
#include "csgeom/polymesh.h"

static bool contains_two_poly(csPolygonMeshEdge* edges, int num_edges,
			      int poly1, int poly2)
{
  for (int i = 0; i < num_edges; i++)
  {
    if ((edges[i].poly1 == poly1 && edges[i].poly2 == poly2) ||
	(edges[i].poly2 == poly1 && edges[i].poly1 == poly2))
      return true;
  }
  return false;
}

static bool contains_edge(int* outline_edges, int num_outline_edges,
			  int vt1, int vt2)
{
  for (int i = 0; i < num_outline_edges; i++)
  {
    int const e1 = *outline_edges++;
    int const e2 = *outline_edges++;
    if (e1 == vt1 && e2 == vt2)
      return true;
  }
  return false;
}

static bool contains_edge(csPolygonMeshEdge* edges, int num_edges,
			  int vt1, int vt2)
{
  for (int i = 0; i < num_edges; i++)
    if (edges[i].vt1 == vt1 && edges[i].vt2 == vt2)
      return true;
  return false;
}

/**
 * A cube mesh for unit testing.
 */
class csUnitCubeMesh : public iPolygonMesh
{
private:
  csVector3 verts[8];
  csMeshedPolygon poly[6];
  int vertices[4*6];
  csFlags flags;
  csTriangle* triangles;
  
public:
  csUnitCubeMesh();
  virtual ~csUnitCubeMesh();

  //---------- iPolygonMesh implementation ----------
  SCF_DECLARE_IBASE;
  virtual int GetVertexCount() { return 8; }
  virtual csVector3* GetVertices() { return verts; }
  virtual int GetPolygonCount() { return 6; }
  virtual csMeshedPolygon* GetPolygons() { return poly; }
  virtual int GetTriangleCount() { return 12; }
  virtual csTriangle* GetTriangles() { return triangles; }
  virtual void Lock() {}
  virtual void Unlock() {}
  virtual csFlags& GetFlags() { return flags; }
  virtual uint32 GetChangeNumber() const { return 0; }
};

SCF_IMPLEMENT_IBASE(csUnitCubeMesh)
  SCF_IMPLEMENTS_INTERFACE(iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

csUnitCubeMesh::csUnitCubeMesh()
{
  SCF_CONSTRUCT_IBASE(0);
  csVector3 dim(1, 1, 1);
  csVector3 d = dim * .5;
  verts[0].Set(-d.x, -d.y, -d.z);
  verts[1].Set( d.x, -d.y, -d.z);
  verts[2].Set(-d.x, -d.y,  d.z);
  verts[3].Set( d.x, -d.y,  d.z);
  verts[4].Set(-d.x,  d.y, -d.z);
  verts[5].Set( d.x,  d.y, -d.z);
  verts[6].Set(-d.x,  d.y,  d.z);
  verts[7].Set( d.x,  d.y,  d.z);
  for (int i = 0; i < 6; i++)
  {
    poly[i].num_vertices = 4;
    poly[i].vertices = &vertices[i*4];
  }
  vertices[0*4+0] = 4;
  vertices[0*4+1] = 5;
  vertices[0*4+2] = 1;
  vertices[0*4+3] = 0;
  vertices[1*4+0] = 5;
  vertices[1*4+1] = 7;
  vertices[1*4+2] = 3;
  vertices[1*4+3] = 1;
  vertices[2*4+0] = 7;
  vertices[2*4+1] = 6;
  vertices[2*4+2] = 2;
  vertices[2*4+3] = 3;
  vertices[3*4+0] = 6;
  vertices[3*4+1] = 4;
  vertices[3*4+2] = 0;
  vertices[3*4+3] = 2;
  vertices[4*4+0] = 6;
  vertices[4*4+1] = 7;
  vertices[4*4+2] = 5;
  vertices[4*4+3] = 4;
  vertices[5*4+0] = 0;
  vertices[5*4+1] = 1;
  vertices[5*4+2] = 3;
  vertices[5*4+3] = 2;
  int tc;
  csPolygonMeshTools::Triangulate(this, triangles, tc);
  flags.Set(CS_POLYMESH_TRIANGLEMESH);
}

csUnitCubeMesh::~csUnitCubeMesh()
{
  delete[] triangles;
  SCF_DESTRUCT_IBASE();
}

/**
 * Test csPolygonMesh operations.
 */
class csPolygonMeshTest : public CppUnit::TestFixture
{
private:
  csUnitCubeMesh mesh;
  csPolygonMeshEdge* edges;
  int nedges;
  csPlane3 planes[6];

public:
  void setUp();
  void tearDown();

  void testEdges();
  void testPlanes();
  void testOutline1();
  void testOutline2();
  void testOutline3();
  void testBorderCase();

  CPPUNIT_TEST_SUITE(csPolygonMeshTest);
    CPPUNIT_TEST(testEdges);
    CPPUNIT_TEST(testPlanes);
    CPPUNIT_TEST(testOutline1);
    CPPUNIT_TEST(testOutline2);
    CPPUNIT_TEST(testOutline3);
    CPPUNIT_TEST(testBorderCase);
  CPPUNIT_TEST_SUITE_END();
};

void csPolygonMeshTest::setUp()
{
  edges = csPolygonMeshTools::CalculateEdges(&mesh, nedges);
  csPolygonMeshTools::CalculatePlanes(&mesh, planes);
}

void csPolygonMeshTest::tearDown()
{
  delete[] edges;
}

void csPolygonMeshTest::testEdges()
{
  CPPUNIT_ASSERT_EQUAL(nedges, 12);
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 0, 1));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 0, 2));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 0, 4));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 1, 3));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 1, 5));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 2, 3));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 2, 6));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 3, 7));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 4, 5));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 4, 6));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 5, 7));
  CPPUNIT_ASSERT(contains_edge(edges, nedges, 6, 7));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 0, 1));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 1, 2));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 2, 3));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 3, 0));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 4, 0));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 4, 1));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 4, 2));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 4, 3));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 5, 0));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 5, 1));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 5, 2));
  CPPUNIT_ASSERT(contains_two_poly(edges, nedges, 5, 3));
}

void csPolygonMeshTest::testPlanes()
{
  int aedge = csPolygonMeshTools::CheckActiveEdges(edges, nedges, planes);
  CPPUNIT_ASSERT_EQUAL(aedge, 12);
  CPPUNIT_ASSERT(edges[ 0].active);
  CPPUNIT_ASSERT(edges[ 1].active);
  CPPUNIT_ASSERT(edges[ 2].active);
  CPPUNIT_ASSERT(edges[ 3].active);
  CPPUNIT_ASSERT(edges[ 4].active);
  CPPUNIT_ASSERT(edges[ 5].active);
  CPPUNIT_ASSERT(edges[ 6].active);
  CPPUNIT_ASSERT(edges[ 7].active);
  CPPUNIT_ASSERT(edges[ 8].active);
  CPPUNIT_ASSERT(edges[ 9].active);
  CPPUNIT_ASSERT(edges[10].active);
  CPPUNIT_ASSERT(edges[11].active);
}

void csPolygonMeshTest::testOutline1()
{
  int   outline_edges[24];
  bool  outline_verts[8];
  int   num_outline_edges;
  float valid_radius;

  csPolygonMeshTools::CalculateOutline(edges, nedges, planes,
    mesh.GetVertexCount(), csVector3(0, 0, -10), outline_edges,
    num_outline_edges, outline_verts, valid_radius);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(valid_radius, 0.50, 0.01);
  CPPUNIT_ASSERT_EQUAL(num_outline_edges, 4);
  CPPUNIT_ASSERT( outline_verts[0]);
  CPPUNIT_ASSERT( outline_verts[1]);
  CPPUNIT_ASSERT(!outline_verts[2]);
  CPPUNIT_ASSERT(!outline_verts[3]);
  CPPUNIT_ASSERT( outline_verts[4]);
  CPPUNIT_ASSERT( outline_verts[5]);
  CPPUNIT_ASSERT(!outline_verts[6]);
  CPPUNIT_ASSERT(!outline_verts[7]);
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 0, 1));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 0, 4));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 1, 5));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 4, 5));
}

void csPolygonMeshTest::testOutline2()
{
  int   outline_edges[24];
  bool  outline_verts[8];
  int   num_outline_edges;
  float valid_radius;

  csPolygonMeshTools::CalculateOutline(edges, nedges, planes,
    mesh.GetVertexCount(), csVector3(2, 0, -2), outline_edges,
    num_outline_edges, outline_verts, valid_radius);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(valid_radius, 0.50, 0.01);
  CPPUNIT_ASSERT_EQUAL(num_outline_edges, 6);
  CPPUNIT_ASSERT( outline_verts[0]);
  CPPUNIT_ASSERT( outline_verts[1]);
  CPPUNIT_ASSERT(!outline_verts[2]);
  CPPUNIT_ASSERT( outline_verts[3]);
  CPPUNIT_ASSERT( outline_verts[4]);
  CPPUNIT_ASSERT( outline_verts[5]);
  CPPUNIT_ASSERT(!outline_verts[6]);
  CPPUNIT_ASSERT( outline_verts[7]);
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 0, 1));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 0, 4));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 4, 5));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 1, 3));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 3, 7));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 5, 7));
}
 
void csPolygonMeshTest::testOutline3()
{
  int   outline_edges[24];
  bool  outline_verts[8];
  int   num_outline_edges;
  float valid_radius;

  csPolygonMeshTools::CalculateOutline(edges, nedges, planes,
    mesh.GetVertexCount(), csVector3(2, 2, -2), outline_edges,
    num_outline_edges, outline_verts, valid_radius);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(valid_radius, 1.50, 0.01);
  CPPUNIT_ASSERT_EQUAL(num_outline_edges, 6);
  CPPUNIT_ASSERT( outline_verts[0]);
  CPPUNIT_ASSERT( outline_verts[1]);
  CPPUNIT_ASSERT(!outline_verts[2]);
  CPPUNIT_ASSERT( outline_verts[3]);
  CPPUNIT_ASSERT( outline_verts[4]);
  CPPUNIT_ASSERT(!outline_verts[5]);
  CPPUNIT_ASSERT( outline_verts[6]);
  CPPUNIT_ASSERT( outline_verts[7]);
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 0, 1));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 1, 3));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 3, 7));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 6, 7));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 4, 6));
  CPPUNIT_ASSERT(contains_edge(outline_edges, num_outline_edges, 0, 4));
}

void csPolygonMeshTest::testBorderCase()
{
  int   outline_edges[24];
  bool  outline_verts[8];
  int   num_outline_edges;
  float valid_radius;

  csPolygonMeshTools::CalculateOutline(edges, nedges, planes,
    mesh.GetVertexCount(), csVector3(.5, 0, -10), outline_edges,
    num_outline_edges, outline_verts, valid_radius);
  CPPUNIT_ASSERT(valid_radius < 0.01);
}
