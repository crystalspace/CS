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

#include "simpmap.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to simple
Simple *simple;

Simple::Simple (iObjectRegistry* object_reg)
{
  Simple::object_reg = object_reg;
}

Simple::~Simple ()
{
}

void Simple::SetupFrame ()
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
}

void Simple::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool Simple::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    simple->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    simple->FinishFrame ();
    return true;
  }
  else if ((ev.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&ev) == csKeyEventTypeDown))
  {
    if (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_ESC)
    {
      csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
      if (q)
	q->GetEventOutlet()->Broadcast (cscmdQuit);
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'l')
    {
      csDebuggingGraph::Dump (0);
      engine->DeleteAll ();
      csDebuggingGraph::Dump (0);
      csDebuggingGraph::Clear (0);
      LoadMap ();
      return true;
    }
  }

  return false;
}

bool Simple::SimpleEventHandler (iEvent& ev)
{
  return simple->HandleEvent (ev);
}

bool Simple::LoadMap ()
{
  // Set VFS current directory to the level we want to load.
  csRef<iVFS> VFS (CS_QUERY_REGISTRY (object_reg, iVFS));
  VFS->ChDir ("/lev/partsys");
  // Load the level file which is called 'world'.
  if (!loader->LoadMapFile ("world"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simpmap",
    	"Couldn't load level!");
    return false;
  }
  engine->Prepare ();

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
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simpmap",
      	"Can't find a valid starting position!");
    return false;
  }

  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);
  return true;
}

bool Simple::Initialize ()
{
  csDebuggingGraph::SetupGraph (object_reg);

  if (!csInitializer::RequestPlugins (object_reg,
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
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simpmap",
	"Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, SimpleEventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simpmap",
	"Can't initialize event handler!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    return false;
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (!vc)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simpmap",
	"Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simpmap",
	"No iEngine plugin!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simpmap",
    	"No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!g3d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simpmap",
    	"No iGraphics3D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simpmap",
    	"No iKeyboardDriver plugin!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.simpmap",
    	"Error opening system!");
    return false;
  }

  view = csPtr<iView> (new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  if (!LoadMap ()) return false;
  return true;
}

void Simple::Start ()
{
  csDefaultRunLoop (object_reg);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;
  simple = new Simple (object_reg);

  if (simple->Initialize ())
    simple->Start ();

  delete simple;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}
