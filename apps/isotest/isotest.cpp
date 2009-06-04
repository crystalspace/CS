/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include "isotest.h"

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

IsoTest::IsoTest ()
{
  SetApplicationName ("CrystalSpace.IsoTest");

  current_view = 0;
  views[0].SetOrigOffset (csVector3 (-1, 1, -1), 4); // true isometric perspective.
  views[1].SetOrigOffset (csVector3 (-1, 1, -1), 9); // zoomed out.
  views[2].SetOrigOffset (csVector3 (1, 0.75, -1), 4); // diablo style perspective.
  views[3].SetOrigOffset (csVector3 (0, 1, -1), 4); // zelda style perspective.

  actor_is_walking = false;
}

IsoTest::~IsoTest ()
{
}

void IsoTest::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 90);

  if (kbd->GetModifierState (CSKEY_SHIFT_LEFT) 
    || kbd->GetModifierState (CSKEY_SHIFT_RIGHT))
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      views[current_view].angle += speed*15.f;
    if (kbd->GetKeyState (CSKEY_LEFT))
      views[current_view].angle -= speed*15.f;
    if (kbd->GetKeyState (CSKEY_UP))
      views[current_view].zoom -= 0.25f*speed;
    if (kbd->GetKeyState (CSKEY_DOWN))
      views[current_view].zoom += 0.25f*speed;
    SetupIsoView(views[current_view]);
  }
  else
  {
    float facing = 0.f; // in degrees
    bool moved = false;
    if (kbd->GetKeyState (CSKEY_RIGHT))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (speed, 0, 0));
      facing = 270.f;
    }
    if (kbd->GetKeyState (CSKEY_LEFT))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (-speed, 0, 0));
      facing = 90.f;
    }
    if (kbd->GetKeyState (CSKEY_UP))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (0, 0, speed));
      facing = 0.f;
    }
    if (kbd->GetKeyState (CSKEY_DOWN))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (0, 0, -speed));
      facing = 180.f;
    }

    if(kbd->GetKeyState (CSKEY_DOWN) && kbd->GetKeyState (CSKEY_LEFT))
      facing = 135;
    if(kbd->GetKeyState (CSKEY_DOWN) && kbd->GetKeyState (CSKEY_RIGHT))
      facing = 225;
    if(kbd->GetKeyState (CSKEY_UP) && kbd->GetKeyState (CSKEY_LEFT))
      facing = 45;
    if(kbd->GetKeyState (CSKEY_UP) && kbd->GetKeyState (CSKEY_RIGHT))
      facing = 315;

    if(moved)
    {
      csYRotMatrix3 r(facing*PI/180.0);
      actor->GetMovable ()->SetTransform(r);
    }
    // update animation state
    csRef<iGeneralMeshState> spstate (
      scfQueryInterface<iGeneralMeshState> (actor->GetMeshObject ()));
    csRef<iGenMeshSkeletonControlState> animcontrol (

      scfQueryInterface<iGenMeshSkeletonControlState> (spstate->GetAnimationControl ()));
    iSkeleton* skeleton = animcontrol->GetSkeleton ();
    if(actor_is_walking && !moved)
    {
      skeleton->StopAll();
      skeleton->Execute("idle");
    }
    if(!actor_is_walking && moved)
    {
      skeleton->StopAll();
      skeleton->Execute("run");
    }
    actor_is_walking = moved;
  }

  // Make sure actor is constant distance above plane.
  csVector3 actor_pos = actor->GetMovable ()->GetPosition ();
  actor_pos.y += 10.0;	// Make sure we start beam high enough.
  csVector3 end_pos, isect;
  end_pos = actor_pos; end_pos.y -= 100.0;
  csHitBeamResult rc = plane->HitBeamObject (actor_pos, end_pos);
  actor_pos.y = rc.isect.y;

  actor->GetMovable ()->SetPosition (actor_pos);
  actor->GetMovable ()->UpdateMove ();

  // Move the light.
  actor_light->SetCenter (actor_pos+csVector3 (0, 2, -1));

  CameraIsoLookat(view->GetCustomMatrixCamera(), views[current_view], actor_pos);

  rm->RenderView (view);

  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  csVector2 lpos(0,0);
  lpos = view->GetCamera()->Perspective(
    view->GetCamera()->GetTransform ().Other2This(csVector3 (-4.7f, 1.0f, 5.5f)));
  // display a helpful little text.
  int txtw=0, txth=0;
  font->GetMaxSize(txtw, txth);
  if(txth == -1) txth = 20;
  int white = g3d->GetDriver2D ()->FindRGB (255, 255, 255);
  g3d->GetDriver2D ()->DrawBox((int)lpos.x-2,
    g3d->GetDriver2D ()->GetHeight()-(int)lpos.y-2,4,4,white);
  int ypos = g3d->GetDriver2D ()->GetHeight () - txth*4 - 1;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "Isometric demo keys (esc to exit):");
  ypos += txth;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "   arrow keys: move around");
  ypos += txth;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "   shift+arrow keys: rotate/zoom camera");
  ypos += txth;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "   tab key: cycle through camera presets");
}

bool IsoTest::OnKeyboard(iEvent& ev)
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
    else if (code == CSKEY_TAB)
    {
      current_view++;
      if (current_view >= 4) current_view = 0;
    }
  }
  return false;
}

void IsoTest::CameraIsoLookat(iCustomMatrixCamera* customCam, const IsoView& isoview,
                              const csVector3& lookat)
{
  iCamera* cam = customCam->GetCamera();
  cam->SetViewportSize (g3d->GetWidth(), g3d->GetHeight());
  // set center and lookat
  csOrthoTransform& cam_trans = cam->GetTransform ();
  cam_trans.SetOrigin (lookat + isoview.camera_offset);
  cam_trans.LookAt (lookat-cam_trans.GetOrigin (), csVector3 (0, 1, 0));

  CS::Math::Matrix4 orthoMatrix (
    CS::Math::Projections::Ortho (
      -1.0f*isoview.zoom, 1.0f*isoview.zoom,
      -0.75*isoview.zoom, 0.75*isoview.zoom,
      -100, csVector3::Norm (cam_trans.GetOrigin ())*isoview.zoom));
  customCam->SetProjectionMatrix (orthoMatrix);
}

void IsoTest::SetupIsoView(IsoView& isoview)
{
  // clamp
  if(isoview.angle < 0.f) isoview.angle += 360.f;
  if(isoview.angle > 360.f) isoview.angle -= 360.f;
  if(isoview.zoom < 0.05f) isoview.zoom = 0.05f;
  if(views[current_view].zoom > 10.f) isoview.zoom = 10.f;
  // setup
  csYRotMatrix3 r(isoview.angle * PI / 180.0);
  isoview.camera_offset = (r*isoview.original_offset);
}

bool IsoTest::OnInitialize(int /*argc*/, char* /*argv*/ [])
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
    CS_REQUEST_PLUGIN("crystalspace.font.server.multiplexer", iFontServer),
    "crystalspace.font.server.freetype2", "iFontServer.1", 
    scfInterfaceTraits<iFontServer>::GetID(), 
    scfInterfaceTraits<iFontServer>::GetVersion(),
    "crystalspace.font.server.default", "iFontServer.2", 
    scfInterfaceTraits<iFontServer>::GetID(), 
    scfInterfaceTraits<iFontServer>::GetVersion(),
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

void IsoTest::OnExit()
{
  // Shut down the event handlers we spawned earlier.
  drawer.Invalidate();
  printer.Invalidate();
}

bool IsoTest::Application()
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

bool IsoTest::SetupModules ()
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
  // The camera will be an orthographic one, we need a custom matrix cam for that
  csRef<iCustomMatrixCamera> customCam (engine->CreateCustomMatrixCamera());
  view->SetCustomMatrixCamera (customCam);
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  font = g3d->GetDriver2D ()->GetFontServer()->LoadFont
    ("/fonts/ttf/Vera.ttf", 10);
  if(!font) // fallback
    font = g3d->GetDriver2D ()->GetFontServer()->LoadFont(CSFONT_LARGE);

  if (!LoadMap ()) return false;
  if (!CreateActor ()) return false;
  engine->Prepare ();
  rm = engine->GetRenderManager();

  // We use some other "helper" event handlers to handle 
  // pushing our work into the 3D engine and rendering it
  // to the screen.
  //drawer.AttachNew(new FrameBegin3DDraw (GetObjectRegistry (), view));
  printer.AttachNew(new FramePrinter (GetObjectRegistry ()));

  return true;
}

bool IsoTest::LoadMap ()
{
  // Set VFS current directory to the level we want to load.
  csRef<iVFS> VFS (csQueryRegistry<iVFS> (object_reg));
  VFS->ChDir ("/lev/isomap");
  // Load the level file which is called 'world'.
  if (!loader->LoadMapFile ("world"))
  {
    return ReportError ("Couldn't load level!");
  }

  // Find the starting position in this level.
  csVector3 pos (0, 0, 0);
  if (engine->GetCameraPositions ()->GetCount () > 0)
  {
    // There is a valid starting position defined in the level file.
    iCameraPosition* campos = engine->GetCameraPositions ()->Get (0);
    room = engine->GetSectors ()->FindByName (campos->GetSector ());
    pos = campos->GetPosition ();
  }
  else
  {
    // We didn't find a valid starting position. So we default
    // to going to room called 'room' at position (0,0,0).
    room = engine->GetSectors ()->FindByName ("room");
  }
  if (!room)
  {
    return ReportError ("Can't find a valid starting position!");
  }

  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);

  iLightList* ll = room->GetLights ();
  actor_light = engine->CreateLight (0, csVector3 (-3, 5, 0), 5,
    csColor (1, 1, 1));
  ll->Add (actor_light);

  csRef<iLight> statuelight = engine->CreateLight ("statuelight",
    csVector3 (-4.7f, 1.0f, 5.5f), 4, csColor(1.2f,0.2f,0.2f));
  statuelight->CreateNovaHalo (1278, 15, 0.3f);
  ll->Add (statuelight);

  plane = engine->FindMeshObject ("Plane");

  return true;
}

bool IsoTest::CreateActor ()
{
  csRef<iVFS> vfs (csQueryRegistry<iVFS> (object_reg));
  vfs->PushDir ();
  vfs->ChDir ("/lib/kwartz");
  if (!loader->LoadLibraryFile ("kwartz.lib"))
  {
    return ReportError ("Error loading kwartz!");
  }
  vfs->PopDir ();

  iMeshFactoryWrapper* imeshfact = engine->FindMeshFactory ("kwartz_fact");

  //csMatrix3 m; m.Identity ();
  //imeshfact->HardTransform (csReversibleTransform (m, csVector3 (0, -1, 0)));

  // Create the sprite and add it to the engine.
  actor = engine->CreateMeshWrapper (
    imeshfact, "MySprite", room, csVector3 (-3, 1, 3));
  actor->GetMovable ()->UpdateMove ();
  csRef<iGeneralMeshState> spstate (
    scfQueryInterface<iGeneralMeshState> (actor->GetMeshObject ()));
  csRef<iGenMeshSkeletonControlState> animcontrol (

    scfQueryInterface<iGenMeshSkeletonControlState> (spstate->GetAnimationControl ()));
  iSkeleton* skel = animcontrol->GetSkeleton ();
  skel->StopAll();
  skel->Execute("idle");

  // The following two calls are not needed since CS_ZBUF_USE and
  // Object render priority are the default but they show how you
  // can do this.
  actor->SetZBufMode (CS_ZBUF_USE);
  actor->SetRenderPriority (engine->GetObjectRenderPriority ());
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
  return csApplicationRunner<IsoTest>::Run (argc, argv);
}
