/*
  Copyright (C) 2010-11 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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
#include "cssysdef.h"

#include "cstool/csview.h"
#include "cstool/genmeshbuilder.h"
#include "cstool/materialbuilder.h"
#include "cstool/demoapplication.h"

#include "iengine/campos.h"

/**
 * Error reporting
 */
void ReportError (iObjectRegistry* registry, const char* objectId, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (registry, CS_REPORTER_SEVERITY_ERROR, objectId, description, arg);
  va_end (arg);
}

void ReportWarning (iObjectRegistry* registry, const char* objectId, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (registry, CS_REPORTER_SEVERITY_WARNING, objectId, description, arg);
  va_end (arg);
}

namespace CS {
namespace Utility {

// ------------------------ DemoApplication ------------------------

DemoApplication::DemoApplication (const char* applicationName)
  : mouseInitialized (false)
{
  SetApplicationName (applicationName);
}

DemoApplication::~DemoApplication ()
{
  // Unregister from the event queue
  csBaseEventHandler::UnregisterQueue ();
}

bool DemoApplication::OnInitialize (int argc, char* argv[])
{
  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (GetObjectRegistry ()))
  {
    PrintHelp ();
    return false;
  }

  // Load the base plugins
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN ("crystalspace.utilities.visualdebugger",
		       CS::Debug::iVisualDebugger),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize some plugins!");

  // Register to the event queue
  csBaseEventHandler::Initialize (GetObjectRegistry ());
  if (!RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ())))
    return ReportError ("Failed to setup the event handler!");

  return true;
}

bool DemoApplication::Application ()
{
  // Open the application and load engine objects
  if (!OpenApplication (GetObjectRegistry ()))
    return ReportError ("Error opening system!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry ());
  if (!engine) return ReportError ("Failed to locate 3D engine!");

  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry ());
  if (!g3d) return ReportError ("Failed to locate 3D renderer!");

  g2d = csQueryRegistry<iGraphics2D> (GetObjectRegistry ());
  if (!g2d) return ReportError ("Failed to locate 2D renderer!");

  vfs = csQueryRegistry<iVFS> (GetObjectRegistry ());
  if (!vfs) return ReportError ("Failed to locate Virtual File System!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry ());
  if (!loader) return ReportError ("Failed to locate main loader!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry ());
  if (!kbd) return ReportError ("Failed to locate keyboard driver!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry ());
  if (!vc) return ReportError ("Failed to locate virtual clock!");

  mouse = csQueryRegistry<iMouseDriver> (GetObjectRegistry ());
  if (!mouse) return ReportError ("Failed to locate mouse driver!");

  printer.AttachNew (new FramePrinter (GetObjectRegistry ()));

  // Create a view to display things
  view = csPtr<iView> (new csView (engine, g3d));
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Load remaining plugins
  csRef<iPluginManager> pluginManager = csQueryRegistry<iPluginManager> (GetObjectRegistry ());

  hudManager = csLoadPlugin<CS::Utility::iHUDManager>
    (pluginManager, "crystalspace.utilities.texthud");
  if (!hudManager) return ReportError ("Failed to locate HUD manager!");

  cameraManager = csLoadPlugin<CS::Utility::iCameraManager>
    (pluginManager, "crystalspace.utilities.cameramanager");
  if (!cameraManager) return ReportError ("Failed to locate camera manager!");

  visualDebugger = csQueryRegistry<CS::Debug::iVisualDebugger> (GetObjectRegistry ());
  if (!visualDebugger) return ReportError ("Failed to locate visual debugger!");

  // Load the configuration file
  config.AddConfig (GetObjectRegistry (), "/config/csdemoapplication.cfg");

  // Load the screenshot configuration
  screenshotFormat = config->GetStr ("DemoApplication.Screenshot.ImageFormat", "jpg");
  csString screenshotMask = config->GetStr ("DemoApplication.Screenshot.FilenameFormat",
					    "/tmp/CS_screenshot_0000");
  screenshotHelper.SetMask (screenshotMask + "." + screenshotFormat);

  // Initialize the default key descriptions
  hudManager->GetKeyDescriptions ()->Push ("arrow keys: move camera");
  hudManager->GetKeyDescriptions ()->Push ("SHIFT-arrow keys: lateral motion");
  hudManager->GetKeyDescriptions ()->Push ("CTRL-arrow keys: speedier motion");
  hudManager->GetKeyDescriptions ()->Push ("F5: next camera position");
  hudManager->GetKeyDescriptions ()->Push ("F9: toggle HUD");
  hudManager->GetKeyDescriptions ()->Push ("F12: screenshot");

  return true;
}

void DemoApplication::PrintHelp ()
{
  csCommandLineHelper::Help (GetObjectRegistry ());
}

bool DemoApplication::CreateRoom ()
{
  // Create the main sector
  room = engine->CreateSector ("room");

  // Setup the camera
  view->GetCamera ()->SetSector (room);
  cameraManager->SetCamera (view->GetCamera ());

  // Creating the background
  // First we make a primitive for our geometry.
  CS::Geometry::DensityTextureMapper bgMapper (0.3f);
  CS::Geometry::TesselatedBox bgBox (csVector3 (-4000.0f), csVector3 (4000.0f));
  bgBox.SetMapper (&bgMapper);
  bgBox.SetFlags (CS::Geometry::Primitives::CS_PRIMBOX_INSIDE);
  
  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> background =
    CS::Geometry::GeneralMeshBuilder::CreateFactoryAndMesh (engine, room,
				   "background", "background_factory", &bgBox);
  background->SetRenderPriority (engine->GetRenderPriority ("sky"));

  csRef<iMaterialWrapper> bgMaterial =
    CS::Material::MaterialBuilder::CreateColorMaterial
    (GetObjectRegistry (), "background", csColor (0.398f));
  background->GetMeshObject()->SetMaterialWrapper (bgMaterial);

  // Creating lights
  csRef<iLight> light;
  iLightList* lightList = room->GetLights ();

  // This light is for the background
  light = engine->CreateLight (0, csVector3 (-1, -1, 0), 9000, csColor (1));
  light->SetAttenuationMode (CS_ATTN_NONE);
  lightList->Add (light);

  // Other lights
  light = engine->CreateLight (0, csVector3 (1, 0, 0), 8, csColor4 (1, 1, 1, 1));
  light->SetAttenuationMode (CS_ATTN_REALISTIC);
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (-3, 0,  0), 8, csColor (1));
  light->SetAttenuationMode (CS_ATTN_REALISTIC);
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 0, -3), 8, csColor (1));
  light->SetAttenuationMode (CS_ATTN_REALISTIC);
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 0, 3), 8, csColor (1));
  light->SetAttenuationMode (CS_ATTN_REALISTIC);
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (0, -3, 0), 8, csColor (1));
  light->SetAttenuationMode (CS_ATTN_REALISTIC);
  lightList->Add (light);

  engine->Prepare ();
  CS::Lighting::SimpleStaticLighter::ShineLights
    (room, engine, CS::Lighting::SimpleStaticLighter::CS_SHADOW_FULL);

  return true;
}

void DemoApplication::Frame ()
{
  // Initialize the mouse cursor position if we are at the first frame
  if (!mouseInitialized)
  {
    previousMouse.x = mouse->GetLastX ();
    previousMouse.y = mouse->GetLastY ();
    mouseInitialized = true;
  }

  // Tell the 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  // Display of visual debugging informations
  visualDebugger->Display (view);
}

bool DemoApplication::OnKeyboard (iEvent& event)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&event);
  if (eventtype == csKeyEventTypeDown)
  {
    // Help key
    if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_F1)
    {
      hudManager->SwitchKeysPage ();
      return true;
    }

    // Switch to next camera position
    if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_F5)
    {
      cameraManager->SwitchCameraPosition ();
      return true;
    }

    // Toggle HUD
    if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_F9)
    {
      hudManager->SetEnabled (!hudManager->GetEnabled ());
      return true;
    }

    // Screenshot key
    //if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_PRINTSCREEN)
    if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_F12)
    {
      // Get the screenshot
      csRef<iImage> screenshot = g2d->ScreenShot ();

      // Convert the screenshot to the target image format
      csRef<iImageIO> imageIO = csQueryRegistry<iImageIO> (GetObjectRegistry ());
      if (!screenshot || !imageIO)
	return false;

      csRef<iDataBuffer> data =
	imageIO->Save (screenshot, csString ().Format ("image/%s", screenshotFormat.GetData ()));

      if (!data)
      {
	ReportError ("Could not export screenshot image to format %s!",
		     CS::Quote::Single (screenshotFormat.GetData ()));
	return false;
      }

      // Save the file
      csString filename = screenshotHelper.FindNextFilename (vfs);
      if (data && vfs->WriteFile (filename, data->GetData (), data->GetSize ()))
      {
	csRef<iDataBuffer> path = vfs->GetRealPath (filename.GetData ());
	ReportInfo ("Screenshot saved to %s...",
		    CS::Quote::Single (path->GetData ()));
      }

      return true;
    }

    // ESC key
    if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_ESC)
    {
      csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (GetObjectRegistry ()));
      if (q) q->GetEventOutlet()->Broadcast (csevQuit (GetObjectRegistry ()));
      return true;
    }
  }

  return false;
}

void DemoApplication::OnExit ()
{
  printer.Invalidate ();
}

bool DemoApplication::OnMouseMove (iEvent& event)
{
  previousMouse.x = mouse->GetLastX ();
  previousMouse.y = mouse->GetLastY ();

  return false;
}

} // namespace Utility
} // namespace CS
