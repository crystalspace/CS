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

using namespace std;
using CS::Geom::Triangulate3D;
using CS::Geom::csContour3;

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

  return true;
}

bool Tri3DTest::Application()
{
	if (!OpenApplication(GetObjectRegistry()))
		return ReportError("Error: Unable to fetch Object Registry!");
	
	csContour3 polygon;
	csVector3 point1(10, 0, 10);
	csVector3 point2(-10, 0, 10);
	csVector3 point3(10, 10, 0);
	csVector3 point4(-10, 10, 0);
	csVector3 point5(10, 0, -10);
	csVector3 point6(-10, 0, -10);

	polygon.Push(point1);
	polygon.Push(point2);
	polygon.Push(point3);
	polygon.Push(point4);
	polygon.Push(point5);
	polygon.Push(point6);

	csContour3 result_vertices;
	csTriangleMesh result;

	Triangulate3D::Process(polygon, result, result_vertices);

	view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle(0, 0, g2d->GetWidth(), g2d->GetHeight ());

	//csTriangle tri = result.GetTriangle(0);

	Run();

  return true;
}

int main(int argc, char** argv)
{
	return csApplicationRunner<Tri3DTest>::Run (argc, argv);
}