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
  if (!g3d->BeginDraw(engine->GetBeginDrawFlags() | CSDRAW_3DGRAPHICS))
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

  CreateRoom();

  csRef<iPluginManager> plugin_mgr;
  csRef<iCollideSystem> collide_system;

  plugin_mgr = csQueryRegistry<iPluginManager> (object_reg);

  const char* p = "crystalspace.collisiondetection.opcode";
  collide_system = csLoadPlugin<iCollideSystem> (plugin_mgr, p);
  if (!collide_system) return ReportError ("No Collision Detection plugin found!");
  object_reg->Register (collide_system, "iCollideSystem");

  csColliderHelper::InitializeCollisionWrappers (collide_system, engine);

  player.AttachNew(new Player(object_reg));

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
    else if (code == CSKEY_UP)
    {
      player->Step(1);
    }
    else if (code == CSKEY_DOWN)
    {
      player->Step(-1);
    }
    else if (code == CSKEY_LEFT)
    {
      player->Strafe(-1);
    }
    else if (code == CSKEY_RIGHT)
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
        //cegui->GetMouseCursorPtr()->show(); TODO: Why doesn't this work?
        do_freelook = false;
      }
      else
      {
        cegui->DisableMouseCapture();
        //cegui->GetMouseCursorPtr()->hide(); TODO: Why doesn't this work?
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
    if (code == CSKEY_UP || code == CSKEY_DOWN)
    {
      player->Step(0);
    }
    else if (code == CSKEY_LEFT || code == CSKEY_RIGHT)
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
  room = engine->CreateSector ("room");

  bool succ= vfs->Mount("/bias/", "$@data$/bias$/world.zip");
  vfs->ChDir("/bias/");
  bool suc = loader->LoadMapFile("/bias/world", false);

  if (engine->GetCameraPositions ()->GetCount () > 0)
  {
    iCameraPosition *cp = engine->GetCameraPositions ()->Get (0);
    cp->Load(view->GetCamera (), engine);
  }

  engine->Prepare ();
}


/*---------------*
 * Main function
 *---------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<Demo>::Run (argc, argv);
}
