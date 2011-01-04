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

#define DEFAULT_ROTATION_SPEED 0.0005f

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

StartMe::StartMe ()
  : position (0.0f), lastPosition (0.0f), rotationStatus (ROTATE_NORMAL),
    rotationSpeed (DEFAULT_ROTATION_SPEED)
{
  SetApplicationName ("CrystalSpace.StartMe");
}

StartMe::~StartMe ()
{
}

void StartMe::Frame ()
{
  // First get elapsed time from the virtual clock.
  double time = vc->GetElapsedTicks ();

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
  
  /* CEGUI rendering is done by the CEGUI plugin itself since
     we called SetAutoRender (true). */

  // TODO: don't rotate if demos.GetSize () too small

  // Compute the new position for the rotation
  if (rotationStatus == ROTATE_SELECTING)
  {
    int closest = position + 0.5f;
    float distance = position - closest;
    if (fabs (distance) < 0.01f)
      position = closest;

    else if (distance < 0.0f)
      position += time * DEFAULT_ROTATION_SPEED * 3.0f;

    else
      position -= time * DEFAULT_ROTATION_SPEED * 3.0f;
  }

  else
    position += time * rotationSpeed;

  while (position > (float) demos.GetSize ())
    position -= (float) demos.GetSize ();
  while (position < 0.0f)
    position += (float) demos.GetSize ();

  if (fabs (position - lastPosition) < EPSILON)
    return;
  lastPosition = position;

  // Compute the active demo
  int currentDemo = position + 0.5f;
  currentDemo = (currentDemo + 1) % (int) demos.GetSize ();

  // Update each demo's window
  for (int i = 0 ; i < (int) demos.GetSize (); i++)
  {
    // Compute the distance from this index to the current position
    float distance1 = ((float) i) - position;
    float distance2 = ((float) demos.GetSize ()) - position + ((float) i);
    float distance3 = ((float) i) - ((float) demos.GetSize ()) - position;
    float distance;
    if (fabs (distance1) < fabs (distance2))
    {
      if (fabs (distance1) < fabs (distance3))
	distance = distance1;
      else distance = distance3;
    }
    else
    {
      if (fabs (distance2) < fabs (distance3))
	distance = distance2;
      else distance = distance3;
    }

    // Check if the window is visible
    if (distance < -0.3f || distance > 2.5f)
    {
      demos[i].window->setVisible(false);
      demos[i].window->setEnabled(false);
      continue;
    }

    // Compute the position of the window
    float angle = distance * PI * 0.2f - PI;
    float x = sin (angle);
    float y = cos (angle);
    x = (x * 350.0f) - 40.0f;
    y = (y * 350.0f) + 70.0f;

    // Check if this is the currently selected demo
    float size;
    bool selected = currentDemo == i;
    if (selected)
    {
      // Compute the size of the window
      int closest = position + 0.5f;
      float distance = (0.5f - fabs (position - closest)) * 2.0f;
      size = 128.0f + distance * 64.0f;
      x -= 64.0f + distance * 64.0f;
      y -= 64.0f + distance * 64.0f;
    }

    else
    {
      x -= 64;
      y -= 64;
      size = 128.0f;
    }

    // Setup the window
    demos[i].window->setSize(CEGUI::UVector2(CEGUI::UDim(0.0f, size), CEGUI::UDim(0.0f, size)));
    demos[i].window->setPosition(CEGUI::UVector2(CEGUI::UDim(1.0f, x), CEGUI::UDim(1.0f, y)));
    demos[i].window->setVisible(true);
    demos[i].window->setEnabled(true);
    demos[i].window->setProperty("Alpha", selected ? "1" : "0.6");

    if (selected && rotationStatus != OVER_EXIT)
    {
      CEGUI::Window* description = cegui->GetWindowManagerPtr()->getWindow("Description");
      description->setText(demos[i].description);
    }
  }

  // Display the "Exit" message if needed
  if (rotationStatus == OVER_EXIT)
  {
    CEGUI::Window* description = cegui->GetWindowManagerPtr()->getWindow("Description");
    description->setText("Click to exit application");
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

  ///TODO: Using 'EventMouseEntersArea' is more correct but is only available 
  /// in 0.7.2+
  logo->subscribeEvent(CEGUI::Window::EventMouseEnters,
      CEGUI::Event::Subscriber(&StartMe::OnEnterLogo, this));
  logo->subscribeEvent(CEGUI::Window::EventMouseLeaves,
      CEGUI::Event::Subscriber(&StartMe::OnLeaveLogo, this));

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

bool StartMe::OnClick (const CEGUI::EventArgs& e)
{
  if (rotationStatus != ROTATE_SELECTING)
    return true;

  const CEGUI::WindowEventArgs& args = static_cast<const CEGUI::WindowEventArgs&>(e);

  for (size_t i = 0 ; i < demos.GetSize () ; i++)
    if (demos[i].window == args.window)
    {
      csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (GetObjectRegistry());
      csString appdir = cmdline->GetAppDir ();
      system (csString("\"") << appdir << CS_PATH_SEPARATOR <<
        csInstallationPathsHelper::GetAppFilename (
          demos[i].exec) << "\" " << 
          demos[i].args);
      break;
    }
  return true;
}

bool StartMe::OnLogoClicked (const CEGUI::EventArgs& e)
{
  csRef<iEventQueue> q =
    csQueryRegistry<iEventQueue> (GetObjectRegistry());
  if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
  return true;
}

bool StartMe::OnEnterLogo (const CEGUI::EventArgs& e)
{
  rotationStatus = OVER_EXIT;
  rotationSpeed = DEFAULT_ROTATION_SPEED;
  return true;
}

bool StartMe::OnLeaveLogo (const CEGUI::EventArgs& e) 
{
  rotationStatus = ROTATE_NORMAL;
  return true;
}

bool StartMe::OnMouseMove (iEvent& ev)
{
  if (rotationStatus == OVER_EXIT)
    return false;

  // Compute the angle and distance to the bottom right corner of the window
  csRef<iGraphics2D> g2d = csQueryRegistry<iGraphics2D> (GetObjectRegistry ());
  float x = g2d->GetWidth () - csMouseEventHelper::GetX(&ev);
  float y = g2d->GetHeight () - csMouseEventHelper::GetY(&ev);
  float distance = csQsqrt (x * x + y * y);
  float angle = atan2 (y, x);

  // If the mouse is too far away then rotate at default speed
  if (distance < 250.0f || distance > 475.0f)
  {
    rotationStatus = ROTATE_NORMAL;
    rotationSpeed = DEFAULT_ROTATION_SPEED;
    return false;
  }

  // If the mouse is near the center then stop the rotation
  if (angle > PI * 0.166f && angle < PI * 0.333f)
    rotationStatus = ROTATE_SELECTING;

  // If the mouse is on the side, then rotate faster
  else
  {
    rotationStatus = ROTATE_SEARCHING;

    if (angle < PI * 0.166f)
    {
      float distance = PI * 0.166f - angle;
      rotationSpeed = - DEFAULT_ROTATION_SPEED * distance * 15.0f;
    }

    else
    {
      float distance = angle - PI * 0.333f;
      rotationSpeed = DEFAULT_ROTATION_SPEED * distance * 15.0f;
    }
  }

  return false;
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
