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

#include "simpvs.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

Simple::Simple ()
{
  SetApplicationName ("CrystalSpace.SimpVS");
}

Simple::~Simple ()
{
}

void Simple::OnExit ()
{
  printer.Invalidate ();
}

bool Simple::Setup ()
{
  // The virtual clock.
  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry ());
  if (!vc)
    return ReportError("Can't find the virtual clock!");

  // Find the pointer to engine plugin
  engine = csQueryRegistry<iEngine> (GetObjectRegistry ());
  if (!engine)
    return ReportError("No iEngine plugin!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry ());
  if (!loader)
    return ReportError("No iLoader plugin!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry ());
  if (!kbd)
    return ReportError("No iKeyboardDriver plugin!");

  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry ());
  if (!g3d)
    return ReportError("No iGraphics3D plugin!");

  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs) 
    return ReportError("Failed to locate Virtual FileSystem!");

  cegui = csQueryRegistry<iCEGUI> (GetObjectRegistry());
  if (!cegui) 
    return ReportError("Failed to locate CEGUI plugin");

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    return ReportError("Error loading 'stone4' texture!");
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");

  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
      engine, room, "walls", "walls_factory", &box);
  walls->GetMeshObject ()->SetMaterialWrapper (tm);


  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 5, 0), 10,
  	csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 5,  0), 10,
  	csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 5, -3), 10,
  	csColor (0, 1, 0));
  ll->Add (light);

  engine->Prepare ();

  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);

  view.AttachNew (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  CreateGui();

  printer.AttachNew (new FramePrinter (object_reg));

  return true;
}

void Simple::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  iCamera* c = view->GetCamera();
  if (kbd->GetKeyState (CSKEY_RIGHT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    c->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    c->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    c->Move (CS_VEC_FORWARD * 4 * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    c->Move (CS_VEC_BACKWARD * 4 * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) 
    return;

  cegui->Render();
}


bool Simple::OnKeyboard (iEvent& ev)
{
  bool res = false;
  if ((ev.Name == csevKeyboardDown(object_reg)) &&
    (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_ESC))
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (GetObjectRegistry ()));
    if (q)
      q->GetEventOutlet()->Broadcast (csevQuit(object_reg));
    res = true;
  }

  return res;
}

void Simple::SaveVideoPreference()
{
  csRef<iConfigFile> userConfig (csGetPlatformConfig (GetApplicationName()));
  csConfigAccess config (GetObjectRegistry (), userConfig, 
    iConfigManager::ConfigPriorityUserApp);
  config->SetStr ("System.Plugins.iGraphics3D", mode);
}

bool Simple::OnInitialize(int /*argc*/, char* /*argv*/ [])
{
  if (!csInitializer::SetupConfigManager (GetObjectRegistry (), "/config/simpvs.cfg",
    GetApplicationName()))
    return ReportError("Failed to initialize config!");

  KeyboardDown = csevKeyboardDown (GetObjectRegistry ());
  Quit = csevQuit (GetObjectRegistry ());

  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
	CS_REQUEST_VFS,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
    return ReportError("Can't initialize plugins!");

  csBaseEventHandler::Initialize(GetObjectRegistry());

  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void Simple::CreateGui()
{
  // Initialize CEGUI wrapper
  cegui->Initialize ();

  // Set the logging level
  cegui->GetLoggerPtr ()->setLoggingLevel(CEGUI::Informative);

  vfs->ChDir ("/ceguitest/0.5/");

  // Load the ice skin (which uses Falagard skinning system)
  cegui->GetSchemeManagerPtr ()->create("ice.scheme");

  cegui->GetSystemPtr ()->setDefaultMouseCursor("ice", "MouseArrow");

  CEGUI::Font& font = cegui->GetFontManagerPtr ()->createFreeTypeFont("Vera", 10, true, "/fonts/ttf/Vera.ttf");

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  // Load layout and set as root
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("simpvs.layout"));

  CEGUI::Window* btn = 0;
  // ----[ GENERAL ]---------------------------------------------------------

  btn = winMgr->getWindow("Root/ButtonPane/SoftwareButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&Simple::SetSoftware, this));

  btn = winMgr->getWindow("Root/ButtonPane/OpenGLButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&Simple::SetOpenGL, this));
}

bool Simple::Application()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open())
    return ReportError("Error opening system!");

  if (!Setup ())
    return false;

  Run();

  return true;
}

bool Simple::SetSoftware (const CEGUI::EventArgs& e)
{
  csPrintf ("Software mode!\n"); fflush (stdout);

  mode = "crystalspace.graphics3d.software";

  SaveVideoPreference();
  Restart();

  return true;
}

bool Simple::SetOpenGL (const CEGUI::EventArgs& e)
{
  csPrintf ("OpenGL mode!\n"); fflush (stdout);

  mode = "crystalspace.graphics3d.opengl";

  SaveVideoPreference();
  Restart();

  return true;
}

/*-------------------------------------------------------------------------*
 * Main function
 *-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<Simple>::Run (argc, argv);
}
