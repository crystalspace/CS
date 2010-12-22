/*
  Copyright (C) 2009 by Jelle Hellemans

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "smoketest.h"

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include <CEGUIWindowManager.h>
#include <CEGUILogger.h>
#include "csutil/custom_new_enable.h"


CS_IMPLEMENT_APPLICATION

SmokeTest::SmokeTest() 
  : current(-1)
{
  SetApplicationName ("SmokeTest");
}

SmokeTest::~SmokeTest()
{
}

void SmokeTest::Frame()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

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
      rotY += speed;
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
  // the first rotation.  From there you rotate "rotX" radians on
  // your X axis to get the final rotation.  We multiply the
  // individual rotations on each axis together to get a single
  // rotation matrix.  The rotations are applied in right to left
  // order .
  csMatrix3 rot = csXRotMatrix3 (rotX) * csYRotMatrix3 (rotY);
  csOrthoTransform ot (rot, c->GetTransform().GetOrigin ());
  c->SetTransform (ot);
  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  
  // Tell the camera to render into the frame buffer.
  view->Draw ();

  cegui->Render ();

  if (current == (size_t)-1)
  {
    if (renderAllToPath)
    {
      for (size_t i = 0; i < paths.size(); i++)
        LoadPath(i, true);
    }
    else if (selectedIndex != (size_t)-1) 
      LoadPath(selectedIndex);
  }
}

bool SmokeTest::OnInitialize(int /*argc*/, char* /*argv*/ [])
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
    CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  csBaseEventHandler::Initialize(GetObjectRegistry());

  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void SmokeTest::OnExit()
{
  printer.Invalidate ();
}

bool SmokeTest::Application()
{
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs) return ReportError("Failed to locate VFS!");

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

  cegui = csQueryRegistry<iCEGUI> (GetObjectRegistry());
  if (!cegui) return ReportError("Failed to locate CEGUI plugin");

  cmdline = csQueryRegistry<iCommandLineParser> (object_reg);
  if (!cmdline) return ReportError("Failed to locate cmdline plugin");

  // Initialize CEGUI wrapper
  cegui->Initialize ();

  // Set the logging level
  cegui->GetLoggerPtr ()->setLoggingLevel(CEGUI::Informative);

  vfs->ChDir ("/cegui/");

  // Load the ice skin (which uses Falagard skinning system)
  cegui->GetSchemeManagerPtr ()->create("ice.scheme");

  cegui->GetSystemPtr ()->setDefaultMouseCursor("ice", "MouseArrow");

  cegui->GetFontManagerPtr ()->createFreeTypeFont("DejaVuSans", 10, true, "/fonts/ttf/DejaVuSans.ttf");

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  // Load layout and set as root
  vfs->ChDir ("/data/smoketest/");
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("smoketest.layout"));
  vfs->ChDir ("/cegui/");

  // These are used store the current orientation of the camera.
  rotY = rotX = 0;

  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle(0, 0, g2d->GetWidth(), g2d->GetHeight ());

  printer.AttachNew (new FramePrinter (object_reg));

  CreateRoom();
  view->GetCamera()->SetSector (room);
  view->GetCamera()->GetTransform().SetOrigin(csVector3 (0, 5, -12));

  // Subscribe
  CEGUI::Window* btn;
  btn = winMgr->getWindow("Browse/GO");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&SmokeTest::OnGoto, this));

  btn = winMgr->getWindow("Browse/Previous");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&SmokeTest::OnPrevious, this));

  btn = winMgr->getWindow("Browse/Next");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&SmokeTest::OnNext, this));


  btn = winMgr->getWindow("Preview");
  btn->subscribeEvent(CEGUI::Window::EventMouseEnters,
    CEGUI::Event::Subscriber(&SmokeTest::onMouseEnters, this));

  btn = winMgr->getWindow("Preview");
  btn->subscribeEvent(CEGUI::Window::EventMouseLeaves,
    CEGUI::Event::Subscriber(&SmokeTest::onMouseLeaves, this));

  if (cmdline->GetBoolOption ("help"))
  {
    csPrintf ("smoketest <options>\n");
    csPrintf ("  --help:\n");       
    csPrintf ("     Print this help.\n");
    csPrintf ("  -path=<path>:\n");       
    csPrintf ("     Get meshes from specified path (default: %s).\n",
	      CS::Quote::Single ("/data/tests/"));
    csPrintf ("  -index=<index>:\n");       
    csPrintf ("     Which index to load by default.\n");
    csPrintf ("  -renderall:\n");       
    csPrintf ("     Render all meshes into %s and include logs.\n",
	      CS::Quote::Single ("/this/regression.zip"));
    csPrintf ("  -renderalltopath:\n");       
    csPrintf ("     DON'T USE: Render all meshes to their preview images.\n");
    return true;
  }

  //selectedIndex = cmdline->GetOption ("index");
  selectedIndex = 0;
  renderAll = cmdline->GetBoolOption ("renderall");
  renderAllToPath = cmdline->GetBoolOption ("renderalltopath");

  ScanDirectory("/data/tests/");

  collection = engine->CreateCollection("Collection");

  Run();

  return true;
}

void SmokeTest::ScreenShot(const std::string& fileName)
{
  csRef<iImageIO> iio = csQueryRegistry<iImageIO> (object_reg);

  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
     return;

  // Manually draw to bypass CEGUI.
  view->Draw ();
  g3d->FinishDraw ();
  g3d->Print (0);

  csRef<iImage> shot = g3d->GetDriver2D()->ScreenShot();
  csRef<iDataBuffer> data = iio->Save (shot, "image/png");
  if (vfs->WriteFile (fileName.c_str(), data->GetData(), data->GetSize()))
    printf("Written %s...\n", CS::Quote::Single (fileName.c_str()));
}

void SmokeTest::ScanDirectory(const std::string& path)
{
  csRef<iStringArray> arrs = vfs->FindFiles(path.c_str());
  for (size_t i = 0; i < arrs->GetSize(); i++)
  {
    csString rel = arrs->Get(i);
    rel.FindReplace(path.c_str(), "");
    printf("ScanDirectory: %s\n", rel.GetData());
    if (rel == "world")
    {
      paths.push_back(path);
    }
    else if (rel.Length() && rel[rel.Length()-1] == '/')
    {
      ScanDirectory(path+rel.GetData());
    }
  }
}

void SmokeTest::LoadPath(size_t index, bool render)
{
  if (index >= paths.size()) return;

  current = index;

  // Update the titlebar for current index.
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();
  CEGUI::Window* btn = winMgr->getWindow("Browse");
  std::stringstream str;
  str << "Browse " << index+1 << "/" << paths.size();
  btn->setText(str.str());

  // Display the current file.
  btn = winMgr->getWindow("Browse/File");
  btn->setText(paths[index].c_str());

  // Set the current index in the goto input.
  btn = winMgr->getWindow("Browse/GO/Input");
  std::stringstream indexstr;
  indexstr << index+1;
  btn->setText(indexstr.str());

  {// If available set the description.
    std::string fileName = paths[index]+"description.txt";
    if ( vfs->Exists(fileName.c_str()) )
    {
      csRef<iDataBuffer> buf = vfs->ReadFile(fileName.c_str());
      if (buf)
      {
        btn = winMgr->getWindow("Description");
        btn->setText(buf->GetData());
      }
    }
  }

  // Load stuff.
  collection->ReleaseAllObjects();
  csLoadResult r = loader->Load((paths[index]+"world").c_str(), collection);
  if (!r.success) 
    printf("ERROR LOADING %s\n", (paths[index]+"world").c_str());

  // Take screenshot if wanted
  if (render) ScreenShot(paths[index]+"world.png");

  { // If available set the preview.
    CEGUI::ImagesetManager* imsetmgr = cegui->GetImagesetManagerPtr();
    imsetmgr->destroy("Preview");
    btn = winMgr->getWindow("Preview");

    std::string fileName = paths[index]+"world.png";
    if ( vfs->Exists(fileName.c_str()) )
    {
      imsetmgr->createFromImageFile("Preview", fileName.c_str());
      btn->setProperty("Image", "set:Preview image:full_image");
    }
    else
    {
      btn->setProperty("Image", "");
    }
  }
}

bool SmokeTest::OnGoto (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();
  CEGUI::Window* btn = winMgr->getWindow("Browse/GO/Input");
  size_t i;
  std::istringstream ss(btn->getText().c_str());
  if (ss>>i) LoadPath(i-1);
  return true;
}

bool SmokeTest::OnPrevious (const CEGUI::EventArgs& e)
{
  LoadPath(current-1);
  return true;
}

bool SmokeTest::OnNext (const CEGUI::EventArgs& e)
{
  LoadPath(current+1);
  return true;
}

bool SmokeTest::onMouseEnters (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();
  CEGUI::Window* btn = winMgr->getWindow("Preview");
  originalSize = btn->getSize();
  btn->setSize(CEGUI::UVector2(CEGUI::UDim(0.9f, 0.0f), CEGUI::UDim(0.9f, 0.0f)));
  btn->setAlwaysOnTop(true);

  return true;
}

bool SmokeTest::onMouseLeaves (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();
  CEGUI::Window* btn = winMgr->getWindow("Preview");
  btn->setSize(originalSize);
  btn->setAlwaysOnTop(false);

  return true;
}

bool SmokeTest::OnKeyboard(iEvent& ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      csRef<iEventQueue> q =
        csQueryRegistry<iEventQueue> (GetObjectRegistry());
      if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
    }
  }
  return false;
}

void SmokeTest::CreateRoom ()
{
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError("Error loading %s texture!",
		CS::Quote::Single ("stone4"));

  iMaterialWrapper* tm =
    engine->GetMaterialList ()->FindByName ("stone");

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
        csColor (1, 1, 1));//csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 5,  0), 10,
        csColor (1, 1, 1));//csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 5, -3), 10,
        csColor (1, 1, 1));//csColor (0, 1, 0));
  ll->Add (light);

  engine->Prepare ();

  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);
}


/*---------------*
 * Main function
 *---------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<SmokeTest>::Run (argc, argv);
}
