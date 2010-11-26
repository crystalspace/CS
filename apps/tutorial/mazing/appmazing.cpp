/*
    Copyright (C) 2006 by Jorrit Tyberghein

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

#include <cssysdef.h>
#include "appmazing.h"

CS_IMPLEMENT_APPLICATION

AppMazing::AppMazing() : csApplicationFramework(), game (this)
{
  SetApplicationName("mazing");
}

AppMazing::~AppMazing()
{
}

void AppMazing::Frame()
{
  csTicks elapsed_time = vc->GetElapsedTicks ();
  game.Handle (elapsed_time);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  view->Draw ();
}

bool AppMazing::OnKeyboard(iEvent& ev)
{
  // We got a keyboard event.
  if (csKeyEventHelper::GetEventType(&ev) == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      // The user pressed escape, so terminate the application.  The proper
      // way to terminate a Crystal Space application is by broadcasting a
      // csevQuit event.  That will cause the main run loop to stop.  To do
      // so we retrieve the event queue from the object registry and then
      // post the event.
      csRef<iEventQueue> q =
        csQueryRegistry<iEventQueue> (GetObjectRegistry());
      if (q.IsValid())
        q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
    }
    else
    {
      return game.OnKeyboard (ev);
    }
  }
  return false;
}

bool AppMazing::OnInitialize(int /*argc*/, char* /*argv*/[])
{
  iObjectRegistry* r = GetObjectRegistry();

  // Load application-specific configuration file.
  if (!csInitializer::SetupConfigManager(r, 0, GetApplicationName()))
    return ReportError("Failed to initialize configuration manager!");

  // RequestPlugins() will load all plugins we specify.  In addition it will
  // also check if there are plugins that need to be loaded from the
  // configuration system (both the application configuration and CS or global
  // configurations).  It also supports specifying plugins on the command line
  // via the --plugin= option.
  if (!csInitializer::RequestPlugins(r,
	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
        CS_REQUEST_PLUGIN ("crystalspace.collisiondetection.opcode",
		iCollideSystem),
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  // "Warm up" the event handler so it can interact with the world
  csBaseEventHandler::Initialize(r);
 
  // Set up an event handler for the application.  Crystal Space is fully
  // event-driven.  Everything (except for this initialization) happens in
  // response to an event.
  if (!RegisterQueue (r, csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void AppMazing::OnExit()
{
  printer.Invalidate ();
}

bool AppMazing::Application()
{
  iObjectRegistry* r = GetObjectRegistry();

  // Open the main system. This will open all the previously loaded plugins
  // (i.e. all windows will be opened).
  if (!OpenApplication(r))
    return ReportError("Error opening system!");

  // Now get the pointer to various modules we need.  We fetch them from the
  // object registry.  The RequestPlugins() call we did earlier registered all
  // loaded plugins with the object registry.  It is also possible to load
  // plugins manually on-demand.
  g3d = csQueryRegistry<iGraphics3D> (r);
  if (!g3d)
    return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (r);
  if (!engine)
    return ReportError("Failed to locate 3D engine!");

  loader = csQueryRegistry<iLoader> (r);
  if (!loader)
    return ReportError("Failed to locate the map loader!");

  cdsys = csQueryRegistry<iCollideSystem> (r);
  if (!cdsys)
    return ReportError("Failed to locate the collision detection system!");

  vc = csQueryRegistry<iVirtualClock> (r);
  if (!vc)
    return ReportError("Failed to locate the virtual clock!");

  strings = csQueryRegistryTagInterface<iStringSet> (r,
      "crystalspace.shared.stringset");
  if (!strings)
    return ReportError("Failed to locate the standard stringset!");

  // Setup game.
  if (!game.SetupGame ())
    return false;

  // Create a view.
  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (engine->FindSector ("room_0_0_0"));
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  printer.AttachNew (new FramePrinter (object_reg));

  // Start the default run/event loop.  This will return only when some code,
  // such as OnKeyboard(), has asked the run loop to terminate.
  Run();

  return true;
}

int main(int argc, char** argv)
{
  csPrintf ("mazing version 1.0 by Jorrit Tyberghein.\n");

  /* Runs the application.  
   *
   * csApplicationRunner<> cares about creating an application instance 
   * which will perform initialization and event handling for the entire game. 
   *
   * The underlying csApplicationFramework also performs some core 
   * initialization.  It will set up the configuration manager, event queue, 
   * object registry, and much more.  The object registry is very important, 
   * and it is stored in your main application class (again, by 
   * csApplicationFramework). 
   */
  return csApplicationRunner<AppMazing>::Run (argc, argv);
}
