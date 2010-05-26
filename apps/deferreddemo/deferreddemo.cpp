/*
  Copyright (C) 2008 by Joe Forte

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "deferreddemo.h"

//----------------------------------------------------------------------
DeferredDemo::DeferredDemo()
:
viewRotX(0.0f),
viewRotY(0.0f),
shouldShutdown(false)
{
  SetApplicationName ("CrystalSpace.DeferredDemo");
}

//----------------------------------------------------------------------
DeferredDemo::~DeferredDemo()
{
}

//----------------------------------------------------------------------
bool DeferredDemo::OnInitialize(int argc, char *argv[])
{
  if (!csInitializer::RequestPlugins (GetObjectRegistry(),
      CS_REQUEST_VFS,
      CS_REQUEST_OPENGL3D,
      CS_REQUEST_ENGINE,
      CS_REQUEST_FONTSERVER,
      CS_REQUEST_IMAGELOADER,
      CS_REQUEST_LEVELLOADER,
      CS_REQUEST_REPORTER,
      CS_REQUEST_REPORTERLISTENER,
      CS_REQUEST_END))
  {
    return ReportError("Failed to initialize plugins!");
  }

  csBaseEventHandler::Initialize (GetObjectRegistry());
  csEventID events[] = 
  {
    /* List of events to listen to. */

    csevQuit (GetObjectRegistry()),
    csevFrame (GetObjectRegistry()),
    csevKeyboardEvent (GetObjectRegistry()),

    CS_EVENTLIST_END
  };

  if (!RegisterQueue (GetObjectRegistry(), events))
  {
    return ReportError("Failed to set up event handler!");
  }

  // Setup default values.
  viewRotX = 0.0f;
  viewRotY = 0.0f;

  shouldShutdown = false;
  
  // Setup chached event names.
  quitEventID = csevQuit (GetObjectRegistry());

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::SetupModules()
{
  eventQueue = csQueryRegistry<iEventQueue> (GetObjectRegistry());
  if (!eventQueue) 
    return ReportError("Failed to locate Event Queue!");

  graphics2D = csQueryRegistry<iGraphics2D> (GetObjectRegistry());
  if (!graphics2D) 
    return ReportError("Failed to locate 2D renderer!");

  graphics3D = csQueryRegistry<iGraphics3D> (GetObjectRegistry());
  if (!graphics3D) 
    return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry());
  if (!engine) 
    return ReportError("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry());
  if (!vc) 
    return ReportError("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry());
  if (!kbd) 
    return ReportError("Failed to locate Keyboard Driver!");

  md = csQueryRegistry<iMouseDriver> (GetObjectRegistry());
  if (!md) 
    return ReportError("Failed to locate Mouse Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry());
  if (!loader) 
    return ReportError("Failed to locate Loader!");

  rm = csLoadPlugin<iRenderManager> (GetObjectRegistry(), "crystalspace.rendermanager.deferred");
  if (!rm)
    return ReportError("Failed to load deferred Render Manager!");

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::LoadScene()
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs->ChDir ("/lev/castle"))
    return ReportError("Could not navigate to level directory!");

  if (!loader->LoadMapFile ("world"))
     return ReportError("Could not load level!");

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::SetupScene()
{
  // Setup camera
  csRef<iSector> room;
  csVector3 pos (0, 0, 0);
  if (engine->GetCameraPositions ()->GetCount () > 0)
  {
    iCameraPosition* campos = engine->GetCameraPositions ()->Get (0);
    room = engine->GetSectors ()->FindByName (campos->GetSector ());
    pos = campos->GetPosition ();
  }
  else
  {
    // There are no valid starting point for the camera so we just start at the origin.
    room = engine->GetSectors ()->FindByName ("room");
    pos = csVector3 (0, 0, 0);
  }

  if (!room)
    return ReportError("Can't find a valid starting position!");

  view.AttachNew( new csView (engine, graphics3D) );
  view->SetRectangle (0, 0, graphics2D->GetWidth (), graphics2D->GetHeight ());
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);

  // Checks for support of at least 4 color buffer attachment points.
  const csGraphics3DCaps *caps = graphics3D->GetCaps();
  if (caps->MaxRTColorAttachments < 3)
    return ReportError("Graphics3D does not support at least 3 color buffer attachments!");
  else
    ReportInfo("Graphics3D supports %d color buffer attachments.", caps->MaxRTColorAttachments);

  engine->SetClearScreen (true);
  engine->Prepare ();

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::Application()
{
  if (!OpenApplication (GetObjectRegistry()))
  {
    return ReportError("Error opening system!");
  }

  if (SetupModules() && LoadScene() && SetupScene())
  {
    RunDemo();
  }

  return true;
}

//----------------------------------------------------------------------
void DeferredDemo::RunDemo()
{
  // Fetch the minimal elapsed ticks per second.
  csConfigAccess cfgacc (GetObjectRegistry(), "/config/system.cfg");
  csTicks min_elapsed = (csTicks)cfgacc->GetInt ("System.MinimumElapsedTicks", 0);

  while (!ShouldShutdown())
  {
    vc->Advance ();

    csTicks previous = csGetTicks ();

    eventQueue->Process ();

    // Limit fps.
    csTicks elapsed = csGetTicks () - previous;
    if (elapsed < min_elapsed)
      csSleep (min_elapsed - elapsed);
  }
}

//----------------------------------------------------------------------
void DeferredDemo::UpdateCamera()
{
  const float MOVE_SPEED = 5.0f;
  const float ROTATE_SPEED = 2.0f;

  float dt = (float)(vc->GetElapsedTicks () / 1000.0f);

  // Handles camera movment.
  iCamera *c = view->GetCamera ();
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * MOVE_SPEED * dt);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * MOVE_SPEED * dt);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * MOVE_SPEED * dt);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * MOVE_SPEED * dt);
  }
  else
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      viewRotY += ROTATE_SPEED * dt;
    if (kbd->GetKeyState (CSKEY_LEFT))
      viewRotY -= ROTATE_SPEED * dt;
    if (kbd->GetKeyState (CSKEY_PGUP))
      viewRotX += ROTATE_SPEED * dt;
    if (kbd->GetKeyState (CSKEY_PGDN))
      viewRotX -= ROTATE_SPEED * dt;
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_FORWARD * MOVE_SPEED * dt);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * MOVE_SPEED * dt);
  }

  csMatrix3 Rx = csXRotMatrix3 (viewRotX);
  csMatrix3 Ry = csYRotMatrix3 (viewRotY);
  csOrthoTransform V (Rx * Ry, c->GetTransform ().GetOrigin ());

  c->SetTransform (V);
}

//----------------------------------------------------------------------
void DeferredDemo::Frame ()
{
  UpdateCamera ();

  engine->SetRenderManager (rm);

  view->Draw ();

  graphics3D->FinishDraw ();
  graphics3D->Print (NULL);
}

//----------------------------------------------------------------------
bool DeferredDemo::OnUnhandledEvent (iEvent &event)
{
  if (event.Name == quitEventID)
    return OnQuit (event);

  return false;
}

//----------------------------------------------------------------------
bool DeferredDemo::OnKeyboard(iEvent &event)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType (&event);
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode(&event);
    if(code == CSKEY_ESC)
    {
      if (eventQueue.IsValid ()) 
      {
        eventQueue->GetEventOutlet ()->Broadcast( csevQuit(GetObjectRegistry()) );
        return true;
      }
    }
  }

  return false;
}

//----------------------------------------------------------------------
bool DeferredDemo::OnQuit (iEvent &event)
{
  shouldShutdown = true;
  return true;
}
