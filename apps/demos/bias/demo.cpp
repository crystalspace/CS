/*
  Copyright (C) 2010 Jelle Hellemans

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

#include "demo.h"

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include <CEGUIWindowManager.h>
#include <CEGUILogger.h>
#include "csutil/custom_new_enable.h"


CS_IMPLEMENT_APPLICATION

Demo::Demo()
{
  SetApplicationName ("Demo");

  do_freelook = true;
}

Demo::~Demo()
{
}

void Demo::Frame()
{
  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
  {
    ReportError("BeginDraw Failed!");
    return;
  }
 
  // Tell the camera to render into the frame buffer.
  view->Draw ();

  if (!do_freelook) cegui->Render ();
}

bool Demo::OnInitialize(int /*argc*/, char* /*argv*/ [])
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

void Demo::OnExit()
{
  printer.Invalidate ();
}

bool Demo::Application()
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

  iNativeWindow* nw = g3d->GetDriver2D()->GetNativeWindow ();
  if (nw) nw->SetTitle ("Peragro Tempus: Project Bias");


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
  vfs->ChDir ("/data/bias/");
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("bias.layout"));

  // Subscribe to the clicked event for the exit button
  CEGUI::Window* btn = winMgr->getWindow("Bias/Inventory/Quit");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&Demo::OnExitButtonClicked, this));

  // These are used store the current orientation of the camera.
  rotY = rotX = 0;

  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle(0, 0, g2d->GetWidth(), g2d->GetHeight ());

  object_reg->Register(view, "iView");

  printer.AttachNew (new FramePrinter (object_reg));

  csRef<iPluginManager> plugin_mgr;
  csRef<iCollideSystem> collide_system;
  plugin_mgr = csQueryRegistry<iPluginManager> (object_reg);

  const char* p = "crystalspace.collisiondetection.opcode";
  collide_system = csLoadPlugin<iCollideSystem> (plugin_mgr, p);
  if (!collide_system) return ReportError ("No Collision Detection plugin found!");
  object_reg->Register (collide_system, "iCollideSystem");

  CreateRoom();

  csColliderHelper::InitializeCollisionWrappers (collide_system, engine);

  player.AttachNew(new Player(object_reg));
  object_reg->Register(player, "Player");

  Run();

  return true;
}

bool Demo::OnExitButtonClicked (const CEGUI::EventArgs&)
{
  csRef<iEventQueue> q =
    csQueryRegistry<iEventQueue> (GetObjectRegistry());
  if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
  return true;
}

bool Demo::OnKeyboard(iEvent& ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  utf32_char code = csKeyEventHelper::GetCookedCode(&ev);

  if (eventtype == csKeyEventTypeDown)
  {
    if (code == CSKEY_ESC)
    {
      csRef<iEventQueue> q =
        csQueryRegistry<iEventQueue> (GetObjectRegistry());
      if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
    }
    else if ((code == CSKEY_UP) || (code == 'w'))
    {
      player->Step(1);
    }
    else if ((code == CSKEY_DOWN) || (code == 's'))
    {
      player->Step(-1);
    }
    else if ((code == CSKEY_LEFT) || (code == 'a'))
    {
      player->Strafe(-1);
    }
    else if ((code == CSKEY_RIGHT) || (code == 'd'))
    {
      player->Strafe(1);
    }
    else if (code == CSKEY_SPACE)
    {
      player->Jump();
    }
    else if (code == 'i')
    {
      if (do_freelook)
      {
        cegui->EnableMouseCapture();
        cegui->GetMouseCursorPtr()->show(); //TODO: Why doesn't this work?
        do_freelook = false;
      }
      else
      {
        cegui->DisableMouseCapture();
        cegui->GetMouseCursorPtr()->hide(); //TODO: Why doesn't this work?
        do_freelook = true;
      }
    }
    else if (code == 't')
    {
      csVector3 pos(-48.0f,13.0f,-260.0f);
      iSector* sector = engine->FindSector("outside");
      if (sector) 
      {
        view->GetCamera()->SetSector(sector);
        view->GetCamera()->GetTransform ().SetOrigin(pos);
      }
    }
  }
  else if (eventtype == csKeyEventTypeUp)
  {
    if (code == CSKEY_UP || code == CSKEY_DOWN || (code == 'w') || (code == 's'))
    {
      player->Step(0);
    }
    else if (code == CSKEY_LEFT || code == CSKEY_RIGHT || (code == 'a') || (code == 'd'))
    {
      player->Strafe(0);
    }
  }

  return false;
}

bool Demo::OnMouseDown(iEvent& ev)
{
  int last_x, last_y;
  last_x = csMouseEventHelper::GetX(&ev);
  last_y = csMouseEventHelper::GetY(&ev);

  player->Fire(last_x, last_y);
  return false;
}

bool Demo::OnMouseMove(iEvent& ev)
{
  if (do_freelook)
  {
    int last_x, last_y;
    last_x = csMouseEventHelper::GetX(&ev);
    last_y = csMouseEventHelper::GetY(&ev);
    float speed = 6.0f;

    int FRAME_HEIGHT = g3d->GetDriver2D()->GetHeight();
    int FRAME_WIDTH = g3d->GetDriver2D()->GetWidth();

    g3d->GetDriver2D()->SetMousePosition (FRAME_WIDTH / 2, FRAME_HEIGHT / 2);
    player->RotateCam (
      speed * (-((float)(last_y - (FRAME_HEIGHT / 2) ))
      / (FRAME_HEIGHT*2)),
      speed * (((float)(last_x - (FRAME_WIDTH / 2) ))
      / (FRAME_WIDTH*2)));
  }
  return true;
}

void Demo::CreateRoom ()
{
  // Read from command line which level to load
  csRef<iCommandLineParser> clp =
    csQueryRegistry<iCommandLineParser> (GetObjectRegistry ());

  if (clp->GetBoolOption ("help", false))
  {

  }

  csString levelName = clp->GetOption ("level");

  bool castleLevel = true;
  if (!levelName.IsEmpty () && levelName == "test")
    castleLevel = false;

  // Create the main sector
  room = engine->CreateSector ("room");

  // Load default castle level
  if (castleLevel)
  {
    bool suc = vfs->Mount("/bias/", "$@data$/bias$/world.zip");
    if (!suc)
    {
      ReportError ("Error: could not mount VFS path %s",
		   CS::Quote::Single ("$@data$/bias$/world.zip"));
      return;
    }

    vfs->ChDir("/bias/");
    suc = loader->LoadMapFile("/bias/world", false);
    if (!suc)
    {
      ReportError ("Error: could not load map file\n");
      return;
    }

    if (engine->GetCameraPositions ()->GetCount () > 0)
    {
      iCameraPosition *cp = engine->GetCameraPositions ()->Get (0);
      cp->Load(view->GetCamera (), engine);
    }
    engine->Prepare ();
  }

  // Create test level
  else
  {
    if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
      ReportError("Error loading %s texture!",
		  CS::Quote::Single ("stone4"));

    iMaterialWrapper* tm =
      engine->GetMaterialList ()->FindByName ("stone");

    // First we make a primitive for our geometry.
    using namespace CS::Geometry;
    DensityTextureMapper mapper (0.3f);
    TesselatedBox box (csVector3 (-50, 0, -50), csVector3 (50, 20, 50));
    box.SetLevel (3);
    box.SetMapper (&mapper);
    box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

    // Now we make a factory and a mesh at once.
    csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
	engine, room, "walls", "walls_factory", &box);
    walls->GetMeshObject ()->SetMaterialWrapper (tm);

    view->GetCamera()->SetSector(room);
    view->GetCamera()->GetTransform().SetOrigin(csVector3(0,2,0));

    csRef<iCollideSystem> collide_system (csQueryRegistry<iCollideSystem> (object_reg));
    csColliderHelper::InitializeCollisionWrappers (collide_system, engine);

    // Create the empty meshes for the monsters
    {
      DensityTextureMapper mapper (0.3f);
      TesselatedBox box (csVector3 (0, 0, 0), csVector3 (1, 2, 1));
      box.SetLevel (3);
      box.SetMapper (&mapper);
      box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

      // Now we make a factory and a mesh at once.
      csRef<iMeshWrapper> m = GeneralMeshBuilder::CreateFactoryAndMesh (
	engine, room, "entity_kwartz01.001", "monster_factory1", &box);
      m->GetMeshObject ()->SetMaterialWrapper (tm);
      m->GetMovable()->SetPosition(room, csVector3(5,1,5));
      m->GetMovable()->UpdateMove();
    }

    {
      DensityTextureMapper mapper (0.3f);
      TesselatedBox box (csVector3 (0, 0, 0), csVector3 (1, 2, 1));
      box.SetLevel (3);
      box.SetMapper (&mapper);
      box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

      // Now we make a factory and a mesh at once.
      csRef<iMeshWrapper> m = GeneralMeshBuilder::CreateFactoryAndMesh (
	engine, room, "entity_kwartz02.002", "monster_factory2", &box);
      m->GetMeshObject ()->SetMaterialWrapper (tm);
      m->GetMovable()->SetPosition(room, csVector3(-5,1,-5));
      m->GetMovable()->UpdateMove();
    }

    {
      DensityTextureMapper mapper (0.3f);
      TesselatedBox box (csVector3 (0, 0, 0), csVector3 (1, 2, 1));
      box.SetLevel (3);
      box.SetMapper (&mapper);
      box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

      // Now we make a factory and a mesh at once.
      csRef<iMeshWrapper> m = GeneralMeshBuilder::CreateFactoryAndMesh (
	engine, room, "entity_knight.003", "monster_factory3", &box);
      m->GetMeshObject ()->SetMaterialWrapper (tm);
      //m->GetMovable()->SetPosition(room, csVector3(5,1,-5));
      m->GetMovable()->SetPosition(room, csVector3(5,1,10));
      m->GetMovable()->UpdateMove();
    }

    {
      DensityTextureMapper mapper (0.3f);
      TesselatedBox box (csVector3 (0, 0, 0), csVector3 (1, 2, 1));
      box.SetLevel (3);
      box.SetMapper (&mapper);
      box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

      // Now we make a factory and a mesh at once.
      csRef<iMeshWrapper> m = GeneralMeshBuilder::CreateFactoryAndMesh (
	engine, room, "entity_catscratch.004", "monster_factory4", &box);
      m->GetMeshObject ()->SetMaterialWrapper (tm);
      m->GetMovable()->SetPosition(room, csVector3(-5,1,5));
      m->GetMovable()->UpdateMove();
    }

    {
      DensityTextureMapper mapper (0.3f);
      TesselatedBox box (csVector3 (0, 0, 0), csVector3 (5, 5, 5));
      box.SetLevel (3);
      box.SetMapper (&mapper);
      //box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

      // Now we make a factory and a mesh at once.
      csRef<iMeshWrapper> m = GeneralMeshBuilder::CreateFactoryAndMesh (
	engine, room, "mybox", "mybox2", &box);
      m->GetMeshObject ()->SetMaterialWrapper (tm);
      m->GetMovable()->SetPosition(room, csVector3(-5,0,10));
      m->GetMovable()->UpdateMove();
    }

    // Create some lights
    csRef<iLight> light;
    iLightList* ll = room->GetLights ();

    light = engine->CreateLight (0, csVector3 (-3, 5, 0), 1000,
	csColor (1, 0, 0), CS_LIGHT_DYNAMICTYPE_DYNAMIC);
    ll->Add (light);

    light = engine->CreateLight (0, csVector3 (3, 5,  0), 1000,
        csColor (0, 0, 1), CS_LIGHT_DYNAMICTYPE_DYNAMIC);
    ll->Add (light);

    light = engine->CreateLight (0, csVector3 (0, 5, -3), 1000,
        csColor (0, 1, 0), CS_LIGHT_DYNAMICTYPE_DYNAMIC);
    ll->Add (light);

    engine->Prepare ();

    using namespace CS::Lighting;
    SimpleStaticLighter::ShineLights (room, engine, 4);
  }

  // Switch the monster meshes with their real models and objects
  csArray<int> index;
  csRef<iMeshList> list = engine->GetMeshes();
  for (int i = 0; i < list->GetCount(); i++)
  {
    csRef<iMeshWrapper> mesh = list->Get(i);
    if (strncmp (mesh->QueryObject()->GetName(), "entity", 6) == 0)
    {
      printf("creating entity %s\n", mesh->QueryObject()->GetName());
      csRef<Monster> monster;
      monster.AttachNew(new Monster(object_reg));
      if (monster->Initialize (mesh))
	monsters.Push(monster);
      index.Push(i);
    }
  }
  while (index.GetSize() > 0)
  {
    list->Remove(index.Pop());
  }

  // Pre-load the 'gibs' mesh
  LoadMesh(GetObjectRegistry (), "gibs", "/data/bias/models/iceblocks/gibs");

  // Initialize the mouse position
  int FRAME_HEIGHT = g3d->GetDriver2D()->GetHeight();
  int FRAME_WIDTH = g3d->GetDriver2D()->GetWidth();
  g3d->GetDriver2D()->SetMousePosition (FRAME_WIDTH / 2, FRAME_HEIGHT / 2);
}


/*---------------*
 * Main function
 *---------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<Demo>::Run (argc, argv);
}
