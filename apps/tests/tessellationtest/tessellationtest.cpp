/*
    Copyright (C) 2011 by Antony Martin

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

#include "tessellationtest.h"

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

TessellationTest::TessellationTest ()
{
  SetApplicationName ("CrystalSpace.TessellationTest");
}

TessellationTest::~TessellationTest ()
{
}

void TessellationTest::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  // Rotate camera, looking at origin
  if (kbd->GetKeyState (CSKEY_RIGHT))
    rotX -= speed;
  if (kbd->GetKeyState (CSKEY_LEFT))
    rotX += speed;
  if (kbd->GetKeyState (CSKEY_UP))
    rotY = csMax (rotY - speed, -0.5f*PI + EPSILON);
  if (kbd->GetKeyState (CSKEY_DOWN))
    rotY = csMin (rotY + speed, 0.5f*PI - EPSILON);

  // Compute new camera pos. (Lifted from viewmesh.)
  csVector3 camTarget (0);
  csVector3 camPos;

  camPos.x = camTarget.x - camDist * (float)cos(rotY) * (float)sin(rotX);
  camPos.y = camTarget.y - camDist * (float)sin(rotY);
  camPos.z = camTarget.z - camDist * (float)cos(rotY) * (float)cos(rotX);

  iCamera * c = view->GetCamera();
  c->GetTransform().SetOrigin(camPos);
  c->GetTransform().LookAt(camTarget - camPos, csVector3(0,1,0));

  rm->RenderView (view);
}

bool TessellationTest::OnKeyboard(iEvent& ev)
{
  // We got a keyboard event.
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    switch (code)
    {
    case CSKEY_ESC:
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
      break;
    }

    case '+':
      camDist -= 0.1f;
      break;
    case '-':
      camDist += 0.1f;
      break;
    case 'p':
      phong = !phong;
      svPhong->SetValue ((int)phong);
      break;
    case 'd':
      displace = !displace;
      svDisplace->SetValue ((int)displace);
      break;
    }
  }
  return false;
}

bool TessellationTest::OnInitialize(int /*argc*/, char* /*argv*/ [])
{
  if (!SetupConfigManager (object_reg, "data/tessellationtest/tess.cfg"))
  {
    ReportError ("Failed to initialize config!\n");
    return false;
  }
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

void TessellationTest::OnExit()
{
  // Shut down the event handlers we spawned earlier.
  drawer.Invalidate();
  printer.Invalidate();
}

bool TessellationTest::Application()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  if (SetupModules())
  {
    // This calls the default runloop. This will basically just keep
    // broadcasting process events to keep the game going.
    Run();
  }

  return true;
}

bool TessellationTest::SetupModules ()
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

  strings = csQueryRegistryTagInterface<iStringSet> 
      (GetObjectRegistry(), "crystalspace.shared.stringset");
  if (!strings) return ReportError("Failed to locate string set!");

  shstrings = csQueryRegistryTagInterface<iShaderVarStringSet> 
      (GetObjectRegistry(), "crystalspace.shader.variablenameset");
  if (!shstrings) return ReportError("Failed to locate variable name set!");
  
  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  CreateRoom ();
  CreateTeapot ();

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  rm = engine->GetRenderManager();

  // these are used store the current orientation/distance of the camera
  rotY = rotX = -PI * 0.20;
  camDist = 4.0;
 
  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));

  // Setup default state
  phong = displace = true;

  // We use some other "helper" event handlers to handle 
  // pushing our work into the 3D engine and rendering it
  // to the screen.
  printer.AttachNew(new FramePrinter (GetObjectRegistry ()));

  return true;
}

void TessellationTest::CreateRoom ()
{
  // We create a new sector called "room".
  room = engine->CreateSector ("room");
}

void TessellationTest::CreateTeapot ()
{
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("brick", "/lib/std/castle/brick1_d.jpg"))
    ReportError("Error loading %s texture!",
		CS::Quote::Single ("brick1_d"));

  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("brick");

  // Add height map
  {
    csRef<iTextureHandle> heightMap = loader->LoadTexture (
      "/lib/std/castle/brick1_h.png");
    csShaderVariable* svHeightMap =
      tm->GetMaterial()->GetVariableAdd (shstrings->Request ("tex height"));
    svHeightMap->SetValue (heightMap);
  }

  // Set shader
  {
    const char *shaderFile = "data/tessellationtest/shader.xml";
    iShader* shader = loader->LoadShader (shaderFile);

    // Booleans parameters: enable/disable phong smoothing/displacement mapping
    svPhong = shader->GetVariableAdd (shstrings->Request ("phong"));
    svPhong->SetValue ((int)phong);
    svDisplace = shader->GetVariableAdd (shstrings->Request("displace"));
    svDisplace->SetValue ((int)displace);

    // Change shader of teapot material
    tm->GetMaterial ()->SetShader (strings->Request ("base"), shader);
    tm->GetMaterial ()->SetShader (strings->Request ("diffuse"), shader);
  }

  // Load a mesh template from disk.
  if (!loader->LoadLibraryFile ("/lib/teapot/teapot.lib"))
  {
    ReportError("Error loading teapot library");
    return;
  }
  csRef<iMeshFactoryWrapper> imeshfact (engine->FindMeshFactory ("Teapot"));
  if (imeshfact == 0)
    ReportError("Error getting mesh object factory!");

  csRef<iMeshWrapper> pot (engine->CreateMeshWrapper (
    imeshfact, "MyTeapot", room,
    csVector3 (0, -0.5, 0)));
  pot->GetMeshObject ()->SetMaterialWrapper (tm);
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
   * again). TessellationTest does not use that functionality itself, however, it
   * allows you to later use "TessellationTest.Restart();" and it'll just work.
   */
  return csApplicationRunner<TessellationTest>::Run (argc, argv);
}
