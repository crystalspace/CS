/*
    Copyright (C) 2008 by Scott Johnson

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

#include "tri3dtest.h"

CS_IMPLEMENT_APPLICATION

using CS::Geometry::csContour3;

Tri3DTest::Tri3DTest()
{
    SetApplicationName ("CrystalSpace.Tri3DTest");
    //untrimesh = 0;
}

Tri3DTest::~Tri3DTest()
{
}

void Tri3DTest::Frame()
{
  csSimpleRenderMesh rMeshObj;
  rMeshObj.vertexCount = 0;
  rMeshObj = ConvertToRenderMesh(tm);

  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  iCamera* c = view->GetCamera();

  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the arrow keys will cause
    // the camera to strafe up, down, left or right from it's
    // current position.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * 4 * speed);
  }
  else
  {
    // left and right cause the camera to rotate on the global Y
    // axis; page up and page down cause the camera to rotate on the
    // _camera's_ X axis (more on this in a second) and up and down
    // arrows cause the camera to go forwards and backwards.
    if (kbd->GetKeyState (CSKEY_RIGHT))
    {
      rotY += speed;
    }
    if (kbd->GetKeyState (CSKEY_LEFT))
      rotY -= speed;
    if (kbd->GetKeyState (CSKEY_PGUP))
      rotX += speed;
    if (kbd->GetKeyState (CSKEY_PGDN))
      rotX -= speed;
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_FORWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * 4 * speed);
  }

  // We now assign a new rotation transformation to the camera.  You
  // can think of the rotation this way: starting from the zero
  // position, you first rotate "rotY" radians on your Y axis to get
  // the first rotation.  From there you rotate "rotX" radians on the
  // your X axis to get the final rotation.  We multiply the
  // individual rotations on each axis together to get a single
  // rotation matrix.  The rotations are applied in right to left
  // order .
  csMatrix3 rot = csXRotMatrix3 (rotX + 90) * csYRotMatrix3 (rotY);
  csOrthoTransform ot (rot, c->GetTransform().GetOrigin ());
  rMeshObj.object2world *= ot;

  //c->SetTransform (ot);
  //g3d->SetWorldToCamera(csReversibleTransform());

  rm->RenderView (view);

  // if the user wants the item to be displayed triangulated, 
  // then do so
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
  {
    ReportError("Cannot prepare renderer for 3D drawing.");
  }

  if (rMeshObj.vertexCount > 0)
  {
    g3d->DrawSimpleMesh(rMeshObj, 0);
  }

  g3d->FinishDraw();

  if (rMeshObj.vertices != 0)
  {
    delete[] rMeshObj.vertices;
    rMeshObj.vertices = 0;
  }

  if (rMeshObj.colors != 0)
  {
    delete[] rMeshObj.colors;
    rMeshObj.colors = 0;
  }
}

bool Tri3DTest::OnKeyboard(iEvent& ev)
{
    // We got a keyboard event.
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      // The user pressed escape to exit the application.
      // The proper way to quit a Crystal Space application
      // is by broadcasting a csevQuit event. That will cause the
      // main runloop to stop. To do that we get the event queue from
      // the object registry and then post the event.
      csRef<iEventQueue> q = 
        csQueryRegistry<iEventQueue> (GetObjectRegistry());
      if (q.IsValid()) q->GetEventOutlet()->Broadcast(
      	csevQuit(GetObjectRegistry()));
    }
  }
  return false;
}

bool Tri3DTest::OnInitialize(int /*argc*/, char* /*argv*/ [])
{
    // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  // "Warm up" the event handler so it can interact with the world
  csBaseEventHandler::Initialize(GetObjectRegistry());

  // Now we need to register the event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  // Rather than simply handling all events, we subscribe to the
  // particular events we're interested in.
  csEventID events[] = {
    csevFrame (GetObjectRegistry()),
    csevKeyboardEvent (GetObjectRegistry()),
    CS_EVENTLIST_END
  };
  if (!RegisterQueue(GetObjectRegistry(), events))
    return ReportError("Failed to set up event handler!");

  // Report success
  return true;
}

void Tri3DTest::OnExit()
{
  // Shut down the event handlers we spawned earlier.
  drawer.Invalidate();
  printer.Invalidate();
}

bool Tri3DTest::Application()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  if (SetupModules())
  {
    iCamera* c = view->GetCamera();

    // camera at 0.00000, -1.5, 15.225 initially
    csOrthoTransform camTransf = c->GetTransform();
    camTransf.SetOrigin(csVector3(0.0f, -1.5f, 15.225f));
    c->SetTransform(camTransf);

    // This calls the default runloop. This will basically just keep
    // broadcasting process events to keep the game going.
    Run();
  }

  return true;
}

bool Tri3DTest::SetupModules ()
{
  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry());
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry());
  if (!engine) return ReportError("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry());
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry());
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry());
  if (!loader) return ReportError("Failed to locate Loader!");

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Setup our basic sector
  room = engine->CreateSector("room");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  //engine->SetLightingCacheMode (0);

  // Now we need light to see something.
  //csRef<iLight> light;
  //iLightList* ll = room->GetLights ();

  //light = engine->CreateLight(0, csVector3(-3, 5, 0), 10, csColor(2, 0, 0));
  //ll->Add (light);

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();
  rm = engine->GetRenderManager();

  // these are used store the current orientation of the camera
  rotY = rotX = 0;
  
  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));

  // We use some other "helper" event handlers to handle 
  // pushing our work into the 3D engine and rendering it
  // to the screen.
  //drawer.AttachNew(new FrameBegin3DDraw (GetObjectRegistry (), view));
  printer.AttachNew(new FramePrinter (GetObjectRegistry ()));

  // create our simple box
  CreateBox(untrimesh);

  return true;
}

bool Tri3DTest::CreateBox(csContour3& mesh)
{
  mesh.DeleteAll();

  // setup our untriangulated mesh
  mesh.Push(csVector3(-3, 0, -3));
  mesh.Push(csVector3(-3, 0, 3));
  mesh.Push(csVector3(0, 0, 5));
  mesh.Push(csVector3(3, 0, 3));
  mesh.Push(csVector3(3, 0, -3));

  // triangulate the mesh
  CS::Geometry::Triangulate3D::Process(mesh, tm);

  return true;
}

/*-------------------------------------------------------------------------*
 * Main function
 *-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  /* Runs the application. 
   *
   * csApplicationRunner<> is a small wrapper to support "restartable" 
   * applications (ie where CS needs to be completely shut down and loaded 
   * again). Simple1 does not use that functionality itself, however, it
   * allows you to later use "Simple.Restart();" and it'll just work.
   */
  return csApplicationRunner<Tri3DTest>::Run (argc, argv);
}

csSimpleRenderMesh Tri3DTest::ConvertToRenderMesh(const csTriangleMesh& t)
{
  csVector3* verts = 0;
  csVector3* tmVerts;
  csVector4* cols = 0;
  csSimpleRenderMesh rendMesh;
  rendMesh.vertexCount = 0;
  rendMesh.vertices = 0;
  rendMesh.colors = 0;


  verts = new csVector3[tm.GetTriangleCount() * 3];
  tmVerts = tm.GetVertices();

  csArray<csVector4> availableColors;
  availableColors.Push(csVector4(1.0f, 0.0f, 0.0f, 1.0f)); // red
  availableColors.Push(csVector4(1.0f, 0.5f, 0.0f, 1.0f)); // orange
  availableColors.Push(csVector4(1.0f, 1.0f, 0.0f, 1.0f)); // yellow
  availableColors.Push(csVector4(0.0f, 1.0f, 0.0f, 1.0f)); // green
  availableColors.Push(csVector4(0.0f, 0.0f, 1.0f, 1.0f)); // blue
  availableColors.Push(csVector4(0.4f, 0.0f, 1.0f, 1.0f)); // indigo
  availableColors.Push(csVector4(1.0f, 0.0f, 1.0f, 1.0f)); // violet

  int numAvabColors = (int)availableColors.GetSize();

  // also color each triangle differently
  cols = new csVector4[3*tm.GetTriangleCount()];

  for (size_t i = 0; i < tm.GetTriangleCount(); i++)
  {
    csTriangle curTri = tm.GetTriangle((int)i);

    int colorNumber = ((int)i)%numAvabColors;
    verts[3*i] = tmVerts[curTri.a];
    verts[3*i + 1] = tmVerts[curTri.b];
    verts[3*i + 2] = tmVerts[curTri.c];

    cols[3*i] = availableColors[colorNumber];
    cols[3*i + 1] = availableColors[colorNumber];
    cols[3*i + 2] = availableColors[colorNumber];
  }

  rendMesh.vertexCount = (uint)(3 * tm.GetTriangleCount());
  rendMesh.vertices = verts;

  rendMesh.colors = cols;

  rendMesh.meshtype = CS_MESHTYPE_TRIANGLES;
  csAlphaMode alf;
  alf.alphaType = alf.alphaSmooth;
  alf.autoAlphaMode = false;
  rendMesh.alphaType = alf;

  return rendMesh;
}

csSimpleRenderMesh Tri3DTest::ConvertToRenderMesh(const csContour3& c)
{
  csVector3* verts = 0;
  csVector4* cols = 0;

  csSimpleRenderMesh rendMesh;
  rendMesh.vertexCount = 0;
  rendMesh.vertices = 0;
  rendMesh.colors = 0;

  verts = new csVector3[c.GetSize()];
  cols = new csVector4[c.GetSize()];

  for (size_t i = 0; i < c.GetSize(); i++)
  {
    verts[i].Set(c[i]);
    cols[i].Set(1.0, 0.0, 0.0, 1.0);
  }

  rendMesh.vertexCount = (uint)(c.GetSize());
  rendMesh.vertices = verts;
  rendMesh.colors = cols;

  // ok, so this doesn't exactly work, since if we have a pentagon, it's not a quad
  rendMesh.meshtype = CS_MESHTYPE_QUADS;
  csAlphaMode alf;
  alf.alphaType = alf.alphaSmooth;
  alf.autoAlphaMode = false;
  rendMesh.alphaType = alf;

  return rendMesh;
}
