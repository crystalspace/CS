/*
    Copyright (C) 2007 by Scott Johnson

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this application; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "tri3dtest.h"
#include "csgeom/plane3.h"

using namespace std;
using CS::Geometry::Triangulate3D;
using CS::Geometry::csContour3;

CS_IMPLEMENT_APPLICATION

Tri3DTest::Tri3DTest()
{
	SetApplicationName("crystalspace.tri3dtest");
}

bool Tri3DTest::OnInitialize(int argc, char* argv[])
{
  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
//		CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
		CS_REQUEST_END))
		return ReportError("Failed to initialize plugins!");

	object_reg = GetObjectRegistry();

  csBaseEventHandler::Initialize(object_reg);
  if (!RegisterQueue(object_reg, csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

		plugManager = csQueryRegistry<iPluginManager> (object_reg);
	
		g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry());
		if (!g3d) return ReportError("Failed to locate 3D renderer!");

		engine = csQueryRegistry<iEngine> (GetObjectRegistry());
		if (!engine) return ReportError("Failed to locate 3D engine!");

		report = csQueryRegistry<iReporter> (GetObjectRegistry());
		if (!report) return ReportError("Unable to load reporter!");

  return true;
}

bool Tri3DTest::Application()
{
	if (!OpenApplication(GetObjectRegistry()))
		return ReportError("Error: Unable to fetch Object Registry!");

	/* csPlane3 testing code

	csVector3 norm(1, 0, 0);

	csPlane3 myPlane(norm);

	csVector3 p (3, 4, 16);
	csVector3 newVector = myPlane.ProjectOnto(p);

	ReportWarning("Projected vector: (%f, %f, %f)", newVector.x, newVector.y, newVector.z);
	
	End of csPlane3 testing code */

	/* Old Testing Code -- Will be reinserted later */
	csContour3 polygon;

	csVector3 point1(1, 1, 1);
	csVector3 point2(1, 2, 1);
	csVector3 point3(1, 3, 1);
	csVector3 point4 (1, 4, 1);

	polygon.Insert(0, point1);
	polygon.Insert(1, point2);
	polygon.Insert(2, point3);
	polygon.Insert(3, point4);

	/*
	csVector3 point1(0, 10.0, 10.0);
	csVector3 point2(0, -10.0, 10.0);
	csVector3 point3(0.0, -10.0, -10.0);
	csVector3 point4(20.0, 5.0, -10.0);
	csVector3 point5(10.0, 0.0, -10.0);
	csVector3 point6(0, 10.0, -10.0);

	polygon.Insert(0, point1);
	polygon.Insert(1, point2);
	polygon.Insert(2, point3);
	polygon.Insert(3, point4);
	polygon.Insert(4, point5);
	polygon.Insert(5, point6);
	*/

	csContour3 result_vertices;
	csTriangleMesh result;

	Triangulate3D::Process(polygon, result, report);
	/* End of Old testing code */

	/* Testing Code for new addition to csTriangleMesh: AddTriangleMesh()

	csTriangleMesh mesh1, mesh2;
	csVector3 *verts1, *verts2;
	csTriangle *tris1, *tris2;

	mesh1.AddVertex(csVector3(0.0, 0.0, 0.0));
	mesh1.AddVertex(csVector3(1.0, 1.0, 1.0));
	mesh1.AddVertex(csVector3(2.0, 2.0, 2.0));
	mesh1.AddTriangle(0, 1, 2);

	verts1 = mesh1.GetVertices();
	tris1 = mesh1.GetTriangles();

	mesh2.AddVertex(csVector3(11.0, 11.0, 11.0));
	mesh2.AddVertex(csVector3(15.0, 15.0, 15.0));
	mesh2.AddVertex(csVector3(20.0, 20.0, 20.0));
	mesh2.AddTriangle(0, 1, 2);
	
	verts2 = mesh2.GetVertices();
	tris2 = mesh2.GetTriangles();

	ReportWarning("Mesh 1 contains %d vertices.  Mesh 2 contains %d vertices", mesh1.GetVertexCount(), mesh2.GetVertexCount());
	ReportWarning("Mesh 1 contains %d triangles.  Mesh 2 contains %d triangles", mesh1.GetTriangleCount(), mesh2.GetTriangleCount());

	for (int i = 0; i < (int)mesh1.GetVertexCount(); i++)
	{
		ReportWarning("Mesh 1 vertex number %d: (%f, %f, %f)", i, verts1[i].x, verts1[i].y, verts1[i].z);
	}
	
	for (int i = 0; i < (int)mesh2.GetVertexCount(); i++)
	{
		ReportWarning("Mesh 2 vertex number %d: (%f, %f, %f)", i, verts2[i].x, verts2[i].y, verts2[i].z);
	}

	for (int i = 0; i < (int)mesh1.GetTriangleCount(); i++)
	{
		ReportWarning("Mesh 1 Triangle number %d connects vertices %d, %d, and %d", i, tris1[i].a, tris1[i].b, tris1[i].c);
	}

	for (int i = 0; i < (int)mesh2.GetTriangleCount(); i++)
	{
		ReportWarning("Mesh 2 Triangle number %d connects vertices %d, %d, and %d", i, tris2[i].a, tris2[i].b, tris2[i].c);
	}

	ReportWarning("Merging mesh 1 into mesh2...");

	mesh2.AddTriangleMesh(mesh1);
	
	for (int i = 0; i < (int)mesh2.GetVertexCount(); i++)
	{
		ReportWarning("Mesh 2 vertex number %d: (%f, %f, %f)", i, verts2[i].x, verts2[i].y, verts2[i].z);
	}

	for (int i = 0; i < (int)mesh2.GetTriangleCount(); i++)
	{
		ReportWarning("Mesh 2 Triangle number %d connects vertices %d, %d, and %d", i, tris2[i].a, tris2[i].b, tris2[i].c);
	}
	*/

	view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle(0, 0, g2d->GetWidth(), g2d->GetHeight ());

	//csTriangle tri = result.GetTriangle(0);

	//size_t numTris = result.GetTriangleCount();

	//ReportWarning("Number of Triangles: %d", numTris);

	Run();

  return true;
}

int main(int argc, char** argv)
{
	return csApplicationRunner<Tri3DTest>::Run (argc, argv);
}
