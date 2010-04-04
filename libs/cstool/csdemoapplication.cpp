/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
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
#include "cstool/cspixmap.h"
#include "cstool/genmeshbuilder.h"
#include "cstool/materialbuilder.h"
#include "cstool/csdemoapplication.h"

csDemoApplication::csDemoApplication (const char* applicationName,
				      const char* applicationCommand,
				      const char* applicationCommandUsage,
				      const char* applicationDescription)
  : cameraMode (CSDEMO_CAMERA_MOVE_NORMAL),
    applicationCommand (applicationCommand),
    applicationCommandUsage (applicationCommandUsage),
    applicationDescription (applicationDescription), cslogo (0)
{
  SetApplicationName (applicationName);
}

csDemoApplication::~csDemoApplication ()
{
  delete cslogo;
}

void csDemoApplication::Frame ()
{
  csTicks elapsed_time = vc->GetElapsedTicks ();
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  // Update the camera
  iCamera* camera = view->GetCamera ();
  csVector3 cameraOrigin = camera->GetTransform ().GetOrigin ();

  switch (cameraMode)
  {
  case CSDEMO_CAMERA_MOVE_NORMAL:
    {
      if (kbd->GetKeyState (CSKEY_SHIFT))
      {
	if (kbd->GetKeyState (CSKEY_UP))
	  cameraTarget += camera->GetTransform ().This2OtherRelative
	    (csVector3 (0,1,0)) * 4 * speed;
	if (kbd->GetKeyState (CSKEY_DOWN))
	  cameraTarget -= camera->GetTransform ().This2OtherRelative
	    (csVector3 (0,1,0)) * 4 * speed;
	if (kbd->GetKeyState (CSKEY_RIGHT))
	  cameraTarget += camera->GetTransform ().This2OtherRelative
	    (csVector3 (1,0,0)) * 4 * speed;
	if (kbd->GetKeyState (CSKEY_LEFT))
	  cameraTarget -= camera->GetTransform ().This2OtherRelative
	    (csVector3 (1,0,0)) * 4 * speed;
      }

      else
      {
	if (kbd->GetKeyState (CSKEY_UP))
	  cameraTarget += (cameraTarget - cameraOrigin).Unit () * 4 * speed;
	if (kbd->GetKeyState (CSKEY_DOWN))
	  cameraTarget -= (cameraTarget - cameraOrigin).Unit () * 4 * speed;
      }

      UpdateCamera ();
      cameraOrigin = camera->GetTransform ().GetOrigin ();
      if (!kbd->GetKeyState (CSKEY_SHIFT))
      {
	if (kbd->GetKeyState (CSKEY_RIGHT))
	  cameraYaw += speed;
	if (kbd->GetKeyState (CSKEY_LEFT))
	  cameraYaw -= speed;
      }
      if (kbd->GetKeyState (CSKEY_PGUP))
	cameraPitch =
	  csMin<float> (3.14159f * 0.5f - 0.01f, cameraPitch + speed);
      if (kbd->GetKeyState (CSKEY_PGDN))
	cameraPitch =
	  csMax<float> (-3.14159f * 0.5f + 0.01f, cameraPitch - speed);

      UpdateCamera ();
      csVector3 deltaOrig = camera->GetTransform ().GetOrigin () - cameraOrigin;
      cameraTarget -= deltaOrig;
      UpdateCamera ();
      break;
    }

  case CSDEMO_CAMERA_MOVE_ORIGIN:
    {
      if (kbd->GetKeyState (CSKEY_DOWN))
	cameraOrigin.z -= 4 * speed;
      if (kbd->GetKeyState (CSKEY_UP))
	cameraOrigin.z += 4 * speed;
      if (kbd->GetKeyState (CSKEY_LEFT))
	cameraOrigin.x -= 4 * speed;
      if (kbd->GetKeyState (CSKEY_RIGHT))
	cameraOrigin.x += 4 * speed;
      if (kbd->GetKeyState (CSKEY_PGUP))
	cameraOrigin.y += 4 * speed;
      if (kbd->GetKeyState (CSKEY_PGDN))
	cameraOrigin.y -= 4 * speed;
      FixCameraForOrigin (cameraOrigin);
      UpdateCamera ();
      break;
    }

  case CSDEMO_CAMERA_ROTATE_ORIGIN:
    {
      if (kbd->GetKeyState (CSKEY_LEFT))
	cameraYaw += speed;
      if (kbd->GetKeyState (CSKEY_RIGHT))
	cameraYaw -= speed;
      if (kbd->GetKeyState (CSKEY_UP))
	cameraPitch =
	  csMin<float> (3.14159f * 0.5f - 0.01f, cameraPitch + speed);
      if (kbd->GetKeyState (CSKEY_DOWN))
	cameraPitch =
	  csMax<float> (-3.14159f * 0.5f + 0.01f, cameraPitch - speed);
      if (kbd->GetKeyState (CSKEY_PGUP))
	cameraDist =
	  csMax<float> (0.01f, cameraDist - speed * 4);
      if (kbd->GetKeyState (CSKEY_PGDN))
	cameraDist += speed * 4;
      UpdateCamera ();
      break;
    }
  default:
    break;
  }

  // Tell the 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  // Tell the 3D driver we're going to display 2D things.
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  int margin = 15;
  int logoMargin = 5;
  int lineSize = 18;
  int fontColor = g2d->FindRGB (255, 150, 100);

  // Display available keys
  if (keyDescriptions.GetSize ())
  {
    int y = margin;

    WriteShadow (margin, y, fontColor, "Keys available:");
    y += lineSize;

    for (csStringArray::Iterator it = keyDescriptions.GetIterator ();
	 it.HasNext (); )
    {
      WriteShadow (margin + 5, y, fontColor, it.Next ());
      y += lineSize;
    }
  }

  // Display state description
  int y = g2d->GetHeight () - margin - lineSize;

  if (speed != 0.0f)
    WriteShadow (margin, y, fontColor, "FPS: %.2f", 1.0f / speed);
  y -= lineSize;

  for (csStringArray::ReverseIterator it = stateDescriptions.GetReverseIterator ();
       it.HasNext (); )
  {
    WriteShadow (margin, y, fontColor, it.Next ());
    y -= lineSize;
  }

  // Display Crystal Space logo
  if (cslogo)
    cslogo->Draw (g3d, g2d->GetWidth () - cslogo->Width() - logoMargin,
		  logoMargin);
}

void csDemoApplication::ResetCamera ()
{
  cameraTarget.Set (0,0,0);

  cameraDist = 3.5f;
  cameraYaw = 0.0f;
  cameraPitch = -0.2f;
}

void csDemoApplication::UpdateCamera ()
{
  csVector3 cameraPos;

  cameraPos.x = cameraTarget.x
    - cameraDist * (float) cos (cameraPitch) * (float) sin (cameraYaw);
  cameraPos.y = cameraTarget.y
    - cameraDist * (float) sin (cameraPitch);
  cameraPos.z = cameraTarget.z
    - cameraDist * (float) cos (cameraPitch) * (float) cos (cameraYaw);

  iCamera* camera = view->GetCamera ();
  camera->GetTransform ().SetOrigin (cameraPos);
  camera->GetTransform ().LookAt
    (cameraTarget - cameraPos, csVector3 (0,1,0));
}

void csDemoApplication::FixCameraForOrigin
(const csVector3 & desiredOrigin)
{
  // calculate distance, yaw, and pitch values that will put the
  // origin at the desired origin
  cameraDist = (cameraTarget - desiredOrigin).Norm ();

  cameraPitch =
    (float) asin ((cameraTarget.y - desiredOrigin.y) / cameraDist);

  cameraYaw = (float) asin ((cameraTarget.x - desiredOrigin.x)
      / (cameraDist * (float) cos (cameraPitch)));
  if ((cameraTarget.z - desiredOrigin.z)
      / (cameraDist * (float) cos (cameraPitch)) < 0.0f)
      cameraYaw = 3.14159f - cameraYaw;
}

bool csDemoApplication::OnKeyboard (iEvent &event)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&event);
  if (eventtype == csKeyEventTypeDown)
  {
    // Check for ESC key
    if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_ESC)
    {
      csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (GetObjectRegistry ()));
      if (q) q->GetEventOutlet()->Broadcast (csevQuit (GetObjectRegistry ()));
      return true;
    }
  }

  return false;
}

bool csDemoApplication::OnMouseDown (iEvent& e)
{
  if (cameraMode == CSDEMO_CAMERA_NONE)
    return false;

  const float mouseWheelZoomAmount = 0.25f;

  uint button = csMouseEventHelper::GetButton (&e);
  switch (button)
  {
  case 0:
    cameraModePan = true;
    break;
  case 1:
    cameraModeRotate = true;
    break;
  case 2:
    cameraModeZoom = true;
    break;
  case 3:
    cameraDist = csMax<float> (0.1f, cameraDist - mouseWheelZoomAmount);
    UpdateCamera ();
    break;
  case 4:
    cameraDist = csMax<float> (0.1f, cameraDist + mouseWheelZoomAmount);
    UpdateCamera ();
    break;
  }
  return false;
}

bool csDemoApplication::OnMouseUp (iEvent& e)
{
  if (cameraMode == CSDEMO_CAMERA_NONE)
    return false;

  uint button = csMouseEventHelper::GetButton (&e);
  switch (button)
  {
  case 0:
    cameraModePan = false;
    break;
  case 1:
    cameraModeRotate = false;
    break;
  case 2:
    cameraModeZoom = false;
    break;
  }
  return false;
}

bool csDemoApplication::OnMouseMove (iEvent& e)
{
  if (cameraMode == CSDEMO_CAMERA_NONE)
    return false;

  int x = csMouseEventHelper::GetX (&e);
  int y = csMouseEventHelper::GetY (&e);
  float dx = (float) (x - lastMouseX) * 0.02f;
  float dy = (float) (y - lastMouseY) * -0.02f;
  iCamera* camera = view->GetCamera ();

  lastMouseX = x;
  lastMouseY = y;

  if (cameraModePan)
  {
    cameraTarget +=
      camera->GetTransform ().This2OtherRelative (csVector3 (1,0,0)) * dx 
      + camera->GetTransform ().This2OtherRelative (csVector3 (0,1,0)) * dy;
  }
  if (cameraModeRotate)
  {
    cameraYaw += dx;
    cameraPitch += dy;
  }
  if (cameraModeZoom)
  {
    cameraDist = csMax<float> (0.1f, cameraDist - (dx + dy));
  }

  if (cameraModePan || cameraModeRotate || cameraModePan)
    UpdateCamera ();

  return false;
}

void csDemoApplication::WriteShadow (int x, int y, int fg, const char *str,...)
{
  csString buf;
  va_list arg;

  va_start (arg, str);
  buf.FormatV (str, arg);
  va_end (arg);

  Write (x + 1, y - 1, 0, -1, "%s", buf.GetData ());
  Write (x, y, fg, -1, "%s", buf.GetData ());
}

void csDemoApplication::Write (int x, int y, int fg, int bg, const char *str,...)
{
  csString buf;
  va_list arg;

  va_start (arg,str);
  buf.FormatV (str, arg);
  va_end (arg);

  g2d->Write (font, x, y, fg, bg, buf);
}

void csDemoApplication::OnExit ()
{
  printer.Invalidate ();
}

bool csDemoApplication::OnInitialize (int argc, char* argv[])
{
  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (GetObjectRegistry ()))
  {
    csPrintf ("%s\n\n", applicationDescription.GetData ());
    csPrintf ("Usage: %s\n\n", applicationCommandUsage.GetData ());
    csPrintf ("Available options:\n\n");

    if (commandOptions.GetSize ())
      csPrintf ("Specific options for %s:\n", applicationCommand.GetData ());

    for (csArray<CommandOption>::Iterator it = commandOptions.GetIterator ();
	 it.HasNext (); )
    {
      CommandOption commandOption = it.Next ();
      csPrintf ("  -%-*s%s\n", 18, commandOption.option.GetData (),
		commandOption.description.GetData ());
    }

    if (commandOptions.GetSize ())
      csPrintf ("\n");

    csCommandLineHelper::Help (GetObjectRegistry ());
    return false;
  }

  // Request for the engine plugins
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

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  if (!RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ())))
    return ReportError ("Failed to set up event handler!");

  return true;
}

bool csDemoApplication::Application ()
{
  // Open the application and load engine objects
  if (!OpenApplication (GetObjectRegistry ()))
    return ReportError ("Error opening system!");

  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry ());
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry ());
  if (!engine) return ReportError("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry ());
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry ());
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry ());
  if (!loader) return ReportError("Failed to locate Loader!");

  g2d = csQueryRegistry<iGraphics2D> (GetObjectRegistry ());
  if (!g2d) return ReportError("Failed to locate 2D renderer!");

  printer.AttachNew (new FramePrinter (GetObjectRegistry ()));

  // Initialize the view
  view = csPtr<iView> (new csView (engine, g3d));
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Load the font
  csRef<iFontServer> fontServer = g2d->GetFontServer ();
  if (fontServer)
    font = fontServer->LoadFont (CSFONT_COURIER);
  else ReportError ("Failed to locate font server!");

  // Load the Crystal Space logo image
  csRef<iTextureWrapper> texture = loader->LoadTexture
    ("cslogo2", "/lib/std/cslogo2.png", CS_TEXTURE_2D, 0, true, true, true);
  if (!texture.IsValid ())
    ReportWarning ("Failed to load CS logo!\n");

  else
  {
    // Create a 2D sprite for the logo
    iTextureHandle* textureHandle = texture->GetTextureHandle ();
    if (textureHandle)
      cslogo = new csSimplePixmap (textureHandle);
  }

  // Define the default text for the available keys
  keyDescriptions.Push ("arrow keys: move camera");
  keyDescriptions.Push ("SHIFT-arrow keys: move camera sideways");

  return true;
}

bool csDemoApplication::CreateRoom ()
{
  // Create the main sector
  room = engine->CreateSector ("room");
  view->GetCamera ()->SetSector (room);

  // Initialize the camera
  if (cameraMode != CSDEMO_CAMERA_NONE)
  {
    ResetCamera ();
    UpdateCamera ();

    cameraModePan = false;
    cameraModeRotate = false;
    cameraModeZoom = false;
  }

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
  light = engine->CreateLight(0, csVector3(1, 1, 0), 9000, csColor (1));
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
  CS::Lighting::SimpleStaticLighter::ShineLights (room, engine, 4);

  return true;
}
