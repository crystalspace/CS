/*
    Copyright (C) 2011 by Jorrit Tyberghein and Jelle Hellemans

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

#include "startme.h"

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

StartMe::StartMe ()
{
  SetApplicationName ("CrystalSpace.StartMe");
  rotate = true;
}

StartMe::~StartMe ()
{
}

void StartMe::Frame ()
{
  // First get elapsed time from the virtual clock.
  double time = vc->GetCurrentTicks () / 4000.0;

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
  
  /* CEGUI rendering is done by the CEGUI plugin itself since
     we called SetAutoRender (true). */

  static bool rotating = true;
  if (rotate) rotating = true;

  if (rotating)
  {
    CEGUI::Window* description = cegui->GetWindowManagerPtr()->getWindow("Description");
    
    size_t all = demos.GetSize ();
    float offset = (PI*2)/(float)all;
	  for (size_t i = 0 ; i < demos.GetSize (); i++)
    {
	    float y = cos(i*offset-time);
	    float x = sin(i*offset-time);
	    bool left = x < -0.6f && y < -0.6f;
      bool leftPeak = x < -0.7f && y < -0.7f;
	    x = (x * 350.0f) - 40.0f;
	    y = (y * 350.0f) + 70.0f;
      x -= (left?128:64);
      y -= (left?128:64);

      float size = left?192.0f:128.0f;
      demos[i].window->setSize(CEGUI::UVector2(CEGUI::UDim(0.0f, size), CEGUI::UDim(0.0f, size)));
      demos[i].window->setPosition(CEGUI::UVector2(CEGUI::UDim(1.0f, x), CEGUI::UDim(1.0f, y)));
      demos[i].window->setEnabled(left);
      demos[i].window->setProperty("Alpha", left?"1":"0.6");
      if (left) description->setText(demos[i].description);
      
      if (leftPeak && !rotate) { rotating = false; }
	  }
  }
}

bool StartMe::OnKeyboard(iEvent& ev)
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
      if (q.IsValid()) 
        q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
    }
  }
  return false;
}

bool StartMe::OnInitialize(int /*argc*/, char* /*argv*/ [])
{
  if (!csInitializer::SetupConfigManager (GetObjectRegistry (),
  	"/config/startme.cfg"))
    return ReportError ("Error reading config file %s!",
			CS::Quote::Single ("startme.cfg"));

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
      CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
      CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  csBaseEventHandler::Initialize(GetObjectRegistry());

  // Now we need to setup an event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  if (!RegisterQueue (GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError ("Failed to set up event handler!");

  return true;
}

void StartMe::OnExit()
{
  printer.Invalidate ();
}

bool StartMe::Application()
{
  // Set up window transparency. Must happen _before_ system is opened!
  csRef<iGraphics2D> g2d = csQueryRegistry<iGraphics2D> (GetObjectRegistry ());
  if (!g2d) return ReportError ("Failed to obtain canvas!");
  natwin = scfQueryInterface<iNativeWindow> (g2d);
  if (natwin)
  {
    natwin->SetWindowTransparent (true);
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  // The window is open, so lets make it dissappear! 
  if (natwin)
  {
    natwin->SetWindowDecoration (iNativeWindow::decoCaption, false);
	  natwin->SetWindowDecoration (iNativeWindow::decoClientFrame, false);
  }

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

  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs) return ReportError("Failed to locate VFS!");

  confman = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  if (!confman) return ReportError("Failed to locate Config Manager!");

   cegui = csQueryRegistry<iCEGUI> (GetObjectRegistry());
  if (!cegui) return ReportError("Failed to locate CEGUI plugin");

  // Initialize CEGUI wrapper
  cegui->Initialize ();
  
  /* Let CEGUI plugin install an event handler that takes care of rendering
     every frame */
  cegui->SetAutoRender (true);
  
  // Set the logging level
  cegui->GetLoggerPtr ()->setLoggingLevel(CEGUI::Informative);

  vfs->ChDir ("/cegui/");

  // Load the ice skin (which uses Falagard skinning system)
  cegui->GetSchemeManagerPtr ()->create("ice.scheme");

  cegui->GetSystemPtr ()->setDefaultMouseCursor("ice", "MouseArrow");

  cegui->GetFontManagerPtr ()->createFreeTypeFont("DejaVuSans", 10, true, "/fonts/ttf/DejaVuSans.ttf");

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  // Load layout and set as root
  vfs->ChDir ("/data/startme/");
  cegui->GetSchemeManagerPtr ()->create("crystal.scheme");
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("startme.layout"));

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  LoadConfig ();

  CEGUI::Window* logo = winMgr->getWindow("Logo");
  logo->subscribeEvent(CEGUI::Window::EventMouseClick,
      CEGUI::Event::Subscriber(&StartMe::OnLogoClicked, this));

  vfs->ChDir ("/lib/startme");
  CEGUI::Window* root = winMgr->getWindow("root");
  for (size_t i = 0 ; i < demos.GetSize () ; i++)
  {
    demos[i].window = winMgr->createWindow("crystal/StaticImage");
    demos[i].window->setSize(CEGUI::UVector2(CEGUI::UDim(0.0f, 128.0f), CEGUI::UDim(0.0f, 128.0f)));
    demos[i].window->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0f, 0.0f), CEGUI::UDim(0.0f, 0.0f)));
    CEGUI::ImagesetManager* imsetmgr = cegui->GetImagesetManagerPtr();
    if (!imsetmgr->isDefined(demos[i].image))
    {
      imsetmgr->createFromImageFile(demos[i].image, demos[i].image);
      std::string img = "set:"+std::string(demos[i].image)+" image:full_image";
      demos[i].window->setProperty("Image", img);
    }
    root->addChildWindow(demos[i].window);

    demos[i].window->subscribeEvent(CEGUI::Window::EventMouseClick,
      CEGUI::Event::Subscriber(&StartMe::OnClick, this));

    ///TODO: Using 'EventMouseEntersArea' is more correct but is only available
    /// in 0.7.2+
    demos[i].window->subscribeEvent(CEGUI::Window::EventMouseEnters,
      CEGUI::Event::Subscriber(&StartMe::OnEnter, this));
    demos[i].window->subscribeEvent(CEGUI::Window::EventMouseLeaves,
      CEGUI::Event::Subscriber(&StartMe::OnLeave, this));
  }

  // Here we create our world.
  CreateRoom ();

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
  view->GetCamera ()->SetViewportSize (g2d->GetWidth(), g2d->GetHeight ());

  printer.AttachNew (new FramePrinter (object_reg));

  // This calls the default runloop. This will basically just keep
  // broadcasting process events to keep the game going.
  Run();

  return true;
}

bool StartMe::OnLogoClicked (const CEGUI::EventArgs& e)
{
  csRef<iEventQueue> q =
    csQueryRegistry<iEventQueue> (GetObjectRegistry());
  if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
  return true;
}

bool StartMe::OnClick (const CEGUI::EventArgs& e)
{
  const CEGUI::WindowEventArgs& args =static_cast<const CEGUI::WindowEventArgs&>(e);

  for (size_t i = 0 ; i < demos.GetSize () ; i++)
    if (demos[i].window == args.window)
    {
      csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (GetObjectRegistry());
      csString appdir = cmdline->GetAppDir ();
      system (csString("\"") << appdir << CS_PATH_SEPARATOR <<
        csInstallationPathsHelper::GetAppFilename (
          demos[i].exec) << "\" " << 
          demos[i].args);
    }
  return true;
}

bool StartMe::OnEnter (const CEGUI::EventArgs& e)
{
  rotate = false;
  return true;
}

bool StartMe::OnLeave (const CEGUI::EventArgs& e)
{
  rotate = true;
  return true;
}

void StartMe::CreateRoom ()
{
    // We create a new sector called "room".
  room = engine->CreateSector ("room");

  // Note: no walls are created - to get the point of this demo across...

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 5, -3), 20, csColor (1, 1, 1));
  ll->Add (light);
}

void StartMe::LoadConfig ()
{  
  // Retrieve demo programs informations.
  size_t i = 0;
  csString pattern;
  while (confman->SubsectionExists (pattern.Format ("StartMe.%zu.", i)))
  {
    DemoData demo;
    csRef<iConfigIterator> iterator (confman->Enumerate (pattern.GetData()));
    while (iterator->HasNext ())
    {
      iterator->Next();
      csString key (iterator->GetKey ());
      csString leaf;
      key.SubString (leaf,
          key.FindLast ('.', key.Length ()) + 1,
          key.Length ());
      if (!strcmp(leaf.GetData (), "name"))
        demo.name = iterator->GetStr ();
      else if (!strcmp(leaf.GetData (), "exec"))
        demo.exec = iterator->GetStr ();
      else if (!strcmp(leaf.GetData (), "args"))
        demo.args = iterator->GetStr ();
      else if (!strcmp(leaf.GetData (), "image"))
        demo.image = iterator->GetStr ();
      else
      {
        demo.description += iterator->GetStr ();
        demo.description += "\n";
      }
    }
    demos.Push (demo);
    i++;
  }
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
   * again). StartMe1 does not use that functionality itself, however, it
   * allows you to later use "StartMe.Restart();" and it'll just work.
   */
  return csApplicationRunner<StartMe>::Run (argc, argv);
}
