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

#include "transparentwindow.h"

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

TransparentWindow::TransparentWindow ()
{
  SetApplicationName ("CrystalSpace.TransparentWindow");
}

TransparentWindow::~TransparentWindow ()
{
}

void TransparentWindow::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  // Rotate camera, looking at origin
  if (kbd->GetKeyState (CSKEY_RIGHT))
    rotX -= speed;
  if (kbd->GetKeyState (CSKEY_LEFT))
    rotX += speed;
  if (kbd->GetKeyState (CSKEY_UP))
    rotY = csMax (rotY - speed, -0.5f*PI + EPSILON);
  if (kbd->GetKeyState (CSKEY_DOWN))
    rotY = csMin (rotY + speed, 0.5f*PI - EPSILON);

  // Compute new camera pos. (Lifted from viewmesh.)
  csVector3 camTarget (0);
  csVector3 camPos;
  float camDist = 4;

  camPos.x = camTarget.x - camDist * (float)cos(rotY) * (float)sin(rotX);
  camPos.y = camTarget.y - camDist * (float)sin(rotY);
  camPos.z = camTarget.z - camDist * (float)cos(rotY) * (float)cos(rotX);

  iCamera * c = view->GetCamera();
  c->GetTransform().SetOrigin(camPos);
  c->GetTransform().LookAt(camTarget - camPos, csVector3(0,1,0));

  rm->RenderView (view);

  g3d->BeginDraw (CSDRAW_2DGRAPHICS);
  DrawLogo ();
  
  if (natwin)
  {
    static const char* statusLines[] = 
    {
      "Window (t)ransparency: "
    };
    size_t numLines = sizeof (statusLines)/sizeof (statusLines[0]);
    
    int colWidth = 0;
    for (size_t l = 0; l < numLines; l++)
    {
      int textW, textH;
      font->GetDimensions (statusLines[l], textW, textH);
      colWidth = csMax (textW, colWidth);
    }
    
    int x = 4, y = 4;
    bool transpState = natwin->GetWindowTransparent();
    DrawOutlineText (font, x, y, statusLines[0]);
    csString statusString;
    statusString = transpState ? "on" : "off";
    if (transpState != transpRequested)
    {
      statusString.Append (" (couldn't be changed)");
    }
    DrawOutlineText (font, x + colWidth + 4, y, statusString);
  }
  
  g3d->FinishDraw();
}

void TransparentWindow::DrawLogo()
{
  if (!logoTex) return;
  
  int w, h;
  logoTex->GetRendererDimensions (w, h);

  int screenW = g3d->GetDriver2D()->GetWidth ();

  // Margin to the edge of the screen, as a fraction of screen width
  const float marginFraction = 0.01f;
  const int margin = (int)screenW * marginFraction;

  // Width of the logo, as a fraction of screen width
  const float widthFraction = 0.3f;
  const int width = (int)screenW * widthFraction;
  const int height = width * h / w;

  g3d->DrawPixmap (logoTex, 
		   screenW - width - margin, 
		   margin,
		   width,
		   height,
		   0, 0, w, h, 0);
}

void TransparentWindow::DrawOutlineText (iFont* font, int x, int y, const char* text)
{
  iGraphics2D* g2d = g3d->GetDriver2D();
  int black = g2d->FindRGB (0, 0, 0);
  int white = g2d->FindRGB (255, 255, 255);
  for (int dy = -1; dy < 2; dy++)
  {
    for (int dx = -1; dx < 2; dx++)
    {
      if ((dx == 0) && (dy == 0)) continue;
      g2d->Write (font, dx+x, dy+y, black, -1, text);
    }
  }
  g2d->Write (font, x, y, white, -1, text);
}

bool TransparentWindow::OnKeyboard (iEvent& ev)
{
  // We got a keyboard event.
  csKeyEventType eventtype = csKeyEventHelper::GetEventType (&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode (&ev);
    switch (code)
    {
    case CSKEY_ESC:
      {
	// The user pressed escape to exit the application.
	// The proper way to quit a Crystal Space application
	// is by broadcasting a csevQuit event. That will cause the
	// main runloop to stop. To do that we get the event queue from
	// the object registry and then post the event.
	csRef<iEventQueue> q = 
	  csQueryRegistry<iEventQueue> (GetObjectRegistry ());
	if (q.IsValid ()) q->GetEventOutlet ()->Broadcast(
	  csevQuit (GetObjectRegistry ()));
      }
      break;
    case 't':
      if (natwin)
      {
	transpRequested = !natwin->GetWindowTransparent();
	natwin->SetWindowTransparent (transpRequested);
      }
      break;
    }
  }

  return false;
}

bool TransparentWindow::OnInitialize (int /*argc*/, char* /*argv*/ [])
{
  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  // "Warm up" the event handler so it can interact with the world
  csBaseEventHandler::Initialize (GetObjectRegistry ());

  // Now we need to register the event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  // Rather than simply handling all events, we subscribe to the
  // particular events we're interested in.
  csEventID events[] = {
    csevFrame (GetObjectRegistry ()),
    csevKeyboardEvent (GetObjectRegistry ()),
    CS_EVENTLIST_END
  };

  if (!RegisterQueue (GetObjectRegistry (), events))
    return ReportError ("Failed to set up event handler!");

  // Report success
  return true;
}

void TransparentWindow::OnExit ()
{
  // Shut down the event handlers we spawned earlier.
  printer.Invalidate ();
}

bool TransparentWindow::Application ()
{
  // Set up window transparency. Must happen _before_ system is opened!
  csRef<iGraphics2D> g2d = csQueryRegistry<iGraphics2D> (GetObjectRegistry ());
  if (!g2d) return ReportError ("Failed to obtain canvas!");
  natwin = scfQueryInterface<iNativeWindow> (g2d);
  if (natwin)
  {
    ReportInfo ("Window transparency available: %s",
                natwin->IsWindowTransparencyAvailable() ? "yes" : "no");
    bool transpResult = natwin->SetWindowTransparent (true);
    ReportInfo ("Window transparency could be enabled: %s",
                transpResult ? "yes" : "no");
  }
  transpRequested = true;

  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication (GetObjectRegistry ()))
    return ReportError ("Error opening system!");

  if (SetupModules ())
  {
    // This calls the default runloop. This will basically just keep
    // broadcasting process events to keep the game going.
    Run ();
  }

  return true;
}

bool TransparentWindow::SetupModules ()
{
  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry ());
  if (!g3d) return ReportError ("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry ());
  if (!engine) return ReportError ("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry ());
  if (!vc) return ReportError ("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry ());
  if (!kbd) return ReportError ("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry ());
  if (!loader) return ReportError ("Failed to locate Loader!");

  csRef<iFontServer> fontServ (csQueryRegistry<iFontServer> (GetObjectRegistry ()));
  if (!fontServ) return ReportError ("Failed to obtain font server!");
  font = fontServ->LoadFont (CSFONT_LARGE);

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Here we create our world.
  CreateRoom ();

  CreateTeapot ();
  
  static const char logoFile[] = "/lib/std/cslogo2.png";
  logoTex = loader->LoadTexture (logoFile, CS_TEXTURE_2D);
  if (!logoTex.IsValid ())
  {
    ReportWarning ("Could not load logo %s!",
		   CS::Quote::Single (logoFile));
  }
  
  // Let the engine prepare the meshes and textures.
  engine->Prepare ();

  rm = engine->GetRenderManager ();

  // These are used store the current orientation of the camera
  rotY = -0.1f;
  rotX = 0;
 
  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  //view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));

  // We use some other "helper" event handlers to handle 
  // pushing our work into the 3D engine and rendering it
  // to the screen.
  printer.AttachNew (new FramePrinter (GetObjectRegistry ()));

  return true;
}

void TransparentWindow::CreateRoom ()
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

void TransparentWindow::CreateTeapot ()
{
  // Load a mesh template from disk.
  if (!loader->LoadLibraryFile ("/lib/teapot/teapot.lib"))
  {
    ReportError("Error loading teapot library");
    return;
  }
  csRef<iMeshFactoryWrapper> imeshfact (engine->FindMeshFactory ("Teapot"));
  if (imeshfact == 0)
    ReportError("Error getting mesh object factory!");

  // Create the sprite and add it to the engine.
  csRef<iMeshWrapper> pot (engine->CreateMeshWrapper (
    imeshfact, "MyTeapot", room,
    csVector3 (0, -0.5, 0)));
  csMatrix3 m; m.Identity ();
  pot->GetMovable ()->SetTransform (m);
  pot->GetMovable ()->UpdateMove ();
}

/*-------------------------------------------------------------------------*
 * Main function
 *-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<TransparentWindow>::Run (argc, argv);
}
