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
#include "cstool/cspixmap.h"
#include "cstool/genmeshbuilder.h"
#include "cstool/materialbuilder.h"
#include "cstool/csdemoapplication.h"

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
namespace Demo {

// ------------------------ HUDManager ------------------------

HUDManager::HUDManager ()
  : cslogo (nullptr), enabled (true), frameCount (0), frameTime (0), currentFPS (0.0f),
    currentKeyPage (0)
{
}

HUDManager::~HUDManager ()
{
  // Unregister from the event queue
  UnregisterQueue ();

  // Delete the logo
  delete cslogo;
}

void HUDManager::Initialize (iObjectRegistry* registry)
{
  // Find references to the engine objects
  g3d = csQueryRegistry<iGraphics3D> (registry);
  if (!g3d) ReportError (registry, "crystalspace.demo.hudmanager", "Failed to locate 3D renderer!");

  g2d = csQueryRegistry<iGraphics2D> (registry);
  if (!g2d) ReportError (registry, "crystalspace.demo.hudmanager", "Failed to locate 2D renderer!");

  vc = csQueryRegistry<iVirtualClock> (registry);
  if (!vc) ReportError (registry, "crystalspace.demo.hudmanager", "Failed to locate virtual clock!");

  csRef<iLoader> loader = csQueryRegistry<iLoader> (registry);
  if (!loader) ReportError (registry, "crystalspace.demo.hudmanager", "Failed to locate main loader!");

  // Load the font
  csRef<iFontServer> fontServer = g2d->GetFontServer ();
  if (!fontServer)
    ReportError (registry, "crystalspace.demo.hudmanager", "Failed to locate font server!");
  else
  {
    font = fontServer->LoadFont (CSFONT_COURIER);
    if (!font)
      ReportError (registry, "crystalspace.demo.hudmanager", "Failed to load font!");
  }

  // Load the Crystal Space logo image
  csRef<iTextureWrapper> texture = loader->LoadTexture
    ("cslogo2", "/lib/std/cslogo2.png", CS_TEXTURE_2D, 0, true, true, true);
  if (!texture.IsValid ())
    ReportError (registry, "crystalspace.demo.hudmanager", "Failed to load CS logo!\n");

  else
  {
    // Create a 2D sprite for the logo
    iTextureHandle* textureHandle = texture->GetTextureHandle ();
    if (textureHandle)
      cslogo = new csSimplePixmap (textureHandle);
  }

  // Register to the event queue
  csBaseEventHandler::Initialize (registry);
  if (!RegisterQueue (registry, csevAllEvents (registry)))
    ReportError (registry, "crystalspace.demo.hudmanager", "Failed to setup the event handler!");
}

void HUDManager::Frame ()
{
#ifdef CS_DEBUG
  if (!g3d || !g2d || !vc) return;
#endif

  if (!enabled) return;

  // Tell the 3D driver we're going to display 2D things.
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  // Get the elasped time
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Update the Frame Per Second data
  frameCount++;
  frameTime += elapsed_time;

  if (frameTime > 500)
  {
    currentFPS = ((float) (frameCount * 1000)) / (float) frameTime;
    frameCount = 0;
    frameTime = 0;
  }

  int margin = 15;
  int logoMargin = 5;
  int lineSize = 18;
  int fontColor = g2d->FindRGB (255, 150, 100);

  // Display the available keys
  if (keyDescriptions.GetSize ())
  {
    int y = margin;

    // Check if there is enough room to display all keys
    maxKeys = (uint) ((g2d->GetHeight () - 2 * margin
		       - (stateDescriptions.GetSize () + 5) * lineSize)
		      / lineSize);
    if (keyDescriptions.GetSize () < maxKeys)
    {
      currentKeyPage = 0;
      WriteShadow (margin, y, fontColor, "Keys available:");
    }
    else
      WriteShadow (margin, y, fontColor, "Keys available (%i/%i):",
		   currentKeyPage + 1, keyDescriptions.GetSize () / maxKeys + 1);
    y += lineSize;

    // Write all keys
    uint index = 0;
    for (csStringArray::Iterator it = keyDescriptions.GetIterator ();
	 it.HasNext (); index++)
    {
      if (index / maxKeys == currentKeyPage)
      {
	WriteShadow (margin + 5, y, fontColor, it.Next ());
	y += lineSize;
      }
      else
	it.Next ();
    }

    if (keyDescriptions.GetSize () > maxKeys)
    {
      WriteShadow (margin, y, fontColor, "F1: more keys");
      y += lineSize;
    }
  }

  // Display the state descriptions
  int y = g2d->GetHeight () - margin - lineSize;

  WriteShadow (margin, y, fontColor, "FPS: %.2f", currentFPS);
  y -= lineSize;

  for (csStringArray::ReverseIterator it = stateDescriptions.GetReverseIterator ();
       it.HasNext (); )
  {
    WriteShadow (margin, y, fontColor, it.Next ());
    y -= lineSize;
  }

  // Display the Crystal Space logo
  if (cslogo)
    cslogo->Draw (g3d,
		  g2d->GetWidth () - cslogo->Width() - logoMargin,
		  logoMargin);
}

void HUDManager::SwitchKeysPage ()
{
  if (keyDescriptions.GetSize ())
    currentKeyPage =
      (currentKeyPage + 1) % (keyDescriptions.GetSize () / maxKeys + 1);
}

void HUDManager::WriteShadow (int x, int y, int fg, const char *str,...)
{
  csString buf;
  va_list arg;

  va_start (arg, str);
  buf.FormatV (str, arg);
  va_end (arg);

  Write (x + 1, y - 1, 0, -1, "%s", buf.GetData ());
  Write (x, y, fg, -1, "%s", buf.GetData ());
}

void HUDManager::Write (int x, int y, int fg, int bg, const char *str,...)
{
#ifdef CS_DEBUG
  if (!g2d) return;
#endif

  csString buf;
  va_list arg;

  va_start (arg,str);
  buf.FormatV (str, arg);
  va_end (arg);

  g2d->Write (font, x, y, fg, bg, buf);
}

void HUDManager::SetEnabled (bool enabled)
{
  this->enabled = enabled;
}

bool HUDManager::GetEnabled ()
{
  return enabled;
}

// ------------------------ CameraManager ------------------------

CameraManager::CameraManager ()
  : cameraMode (CAMERA_MOVE_FREE), mouseMoveEnabled (true),
    hasStartPosition (false), currentCameraPosition (0), cameraTarget (0.0f),
    minimumDistance (0.1f), cameraDistance (0.0f), cameraYaw (0.0f), cameraPitch (0.0f),
    cameraModePan (false), cameraModeRotate (false), cameraModeZoom (false),
    motionSpeed (5.0f), rotationSpeed (2.0f), wasUpdated (false),
    previousMouseX (0), previousMouseY (0)
{
}

CameraManager::~CameraManager ()
{
  // Unregister from the event queue
  csBaseEventHandler::UnregisterQueue ();
}

void CameraManager::Initialize (iObjectRegistry* registry)
{
  // Find references to the engine objects
  engine = csQueryRegistry<iEngine> (registry);
  if (!engine) ReportError (registry, "crystalspace.demo.cameramanager", "Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (registry);
  if (!vc) ReportError (registry, "crystalspace.demo.cameramanager", "Failed to locate virtual clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (registry);
  if (!kbd) ReportError (registry, "crystalspace.demo.cameramanager", "Failed to locate keyboard driver!");

  mouse = csQueryRegistry<iMouseDriver> (registry);
  if (!mouse) ReportError (registry, "crystalspace.demo.cameramanager", "Failed to locate mouse driver!");

  // Register to the event queue
  csBaseEventHandler::Initialize (registry);
  if (!RegisterQueue (registry, csevAllEvents (registry)))
    ReportError (registry, "crystalspace.demo.cameramanager", "Failed to setup the event handler!");
}

void CameraManager::SetCamera (iCamera* camera)
{
  this->camera = camera;

  if (camera && cameraMode != CAMERA_NO_MOVE)
    ResetCamera ();
}

iCamera* CameraManager::GetCamera ()
{
  return camera;
}

void CameraManager::UpdateCamera ()
{
#ifdef CS_DEBUG
  if (!vc || !kbd || !mouse) return;
#endif

  if (!camera || cameraMode == CAMERA_NO_MOVE) return;

  // Compute the speed of the camera
  float elapsedTime = ((float) vc->GetElapsedTicks ()) / 1000.0f;
  float motionDelta = elapsedTime * motionSpeed;
  float rotationDelta = elapsedTime * rotationSpeed;

  if (kbd->GetKeyState (CSKEY_CTRL))
  {
    motionDelta *= 10.0f;
    rotationDelta *= 3.0f;
  }

  // Update the camera
  csVector3 cameraOrigin = camera->GetTransform ().GetOrigin ();

  switch (cameraMode)
  {
  case CAMERA_MOVE_FREE:
    {
      if (kbd->GetKeyState (CSKEY_SHIFT))
      {
	// If the user is holding down shift, the arrow keys will cause
	// the camera to strafe up, down, left or right from it's
	// current position.
	if (kbd->GetKeyState (CSKEY_RIGHT))
	  camera->Move (CS_VEC_RIGHT * motionDelta);
	if (kbd->GetKeyState (CSKEY_LEFT))
	  camera->Move (CS_VEC_LEFT * motionDelta);
	if (kbd->GetKeyState (CSKEY_UP))
	  camera->Move (CS_VEC_UP * motionDelta);
	if (kbd->GetKeyState (CSKEY_DOWN))
	  camera->Move (CS_VEC_DOWN * motionDelta);
      }
      else
      {
	// left and right cause the camera to rotate on the global Y
	// axis; page up and page down cause the camera to rotate on the
	// camera's X axis, and up and down arrows cause the camera to
	// go forwards and backwards.
	if (kbd->GetKeyState (CSKEY_RIGHT))
	  camera->GetTransform ().RotateOther (csVector3 (0.0f, 1.0f, 0.0f), rotationDelta);
	if (kbd->GetKeyState (CSKEY_LEFT))
	  camera->GetTransform ().RotateOther (csVector3 (0.0f, 1.0f, 0.0f), -rotationDelta);
	if (kbd->GetKeyState (CSKEY_PGUP))
	  camera->GetTransform ().RotateThis (csVector3 (1.0f, 0.0f, 0.0f), -rotationDelta);
	if (kbd->GetKeyState (CSKEY_PGDN))
	  camera->GetTransform ().RotateThis (csVector3 (1.0f, 0.0f, 0.0f), rotationDelta);
	if (kbd->GetKeyState (CSKEY_UP))
	  camera->Move (CS_VEC_FORWARD * motionDelta);
	if (kbd->GetKeyState (CSKEY_DOWN))
	  camera->Move (CS_VEC_BACKWARD * motionDelta);
      }

      break;
    }

  case CAMERA_MOVE_LOOKAT:
    {
      // The arrow keys move the position of the camera, after that it is rotated
      // to look at the target
      if (kbd->GetKeyState (CSKEY_DOWN))
	cameraOrigin.z -= motionDelta;
      if (kbd->GetKeyState (CSKEY_UP))
	cameraOrigin.z += motionDelta;
      if (kbd->GetKeyState (CSKEY_LEFT))
	cameraOrigin.x -= motionDelta;
      if (kbd->GetKeyState (CSKEY_RIGHT))
	cameraOrigin.x += motionDelta;
      if (kbd->GetKeyState (CSKEY_PGUP))
	cameraOrigin.y += motionDelta;
      if (kbd->GetKeyState (CSKEY_PGDN))
	cameraOrigin.y -= motionDelta;

      camera->GetTransform ().SetOrigin (cameraOrigin);
      camera->GetTransform ().LookAt (cameraTarget - cameraOrigin, csVector3 (0,1,0));

      break;
    }

  case CAMERA_ROTATE:
    {
      // The arrow keys rotate the camera around the target
      // Pressing shift or using page up/down makes the camera closer/farther
      if (kbd->GetKeyState (CSKEY_LEFT))
	cameraYaw += rotationDelta;
      if (kbd->GetKeyState (CSKEY_RIGHT))
	cameraYaw -= rotationDelta;
      if (kbd->GetKeyState (CSKEY_UP))
      {
	if (kbd->GetKeyState (CSKEY_SHIFT))
	  cameraDistance =
	    csMax<float> (minimumDistance, cameraDistance - motionDelta);
	else
	  cameraPitch = csMax<float> (-3.14159f * 0.5f + 0.01f, cameraPitch - rotationDelta);
      }
      if (kbd->GetKeyState (CSKEY_DOWN))
      {
	if (kbd->GetKeyState (CSKEY_SHIFT))
	  cameraDistance += motionDelta;
	else
	  cameraPitch = csMin<float> (3.14159f * 0.5f - 0.01f, cameraPitch + rotationDelta);
      }
      if (kbd->GetKeyState (CSKEY_PGUP))
	cameraDistance =
	  csMax<float> (minimumDistance, cameraDistance - motionDelta);
      if (kbd->GetKeyState (CSKEY_PGDN))
	cameraDistance += motionDelta;

      ApplyPositionParameters ();

      break;
    }
  default:
    break;
  }

  wasUpdated = true;
}

void CameraManager::Frame ()
{
  // Update the camera if it was not yet made manually
  if (!wasUpdated)
    UpdateCamera ();

  wasUpdated = false;
}

bool CameraManager::OnMouseDown (iEvent &event)
{
#ifdef CS_DEBUG
  if (!vc || !kbd || !mouse) return false;
#endif

  if (!camera || !mouseMoveEnabled)
    return false;

  if (cameraMode != CAMERA_ROTATE && cameraMode != CAMERA_MOVE_LOOKAT)
    UpdatePositionParameters (camera->GetTransform ().GetOrigin ());

  previousMouseX = mouse->GetLastX ();
  previousMouseY = mouse->GetLastY ();

  const float mouseWheelZoomAmount = motionSpeed / 20.0f;

  uint button = csMouseEventHelper::GetButton (&event);
  switch (button)
  {
  case 0:
    cameraModePan = true;
    panCameraTarget = cameraTarget;
    break;
  case 1:
    cameraModeRotate = true;
    break;
  case 2:
    cameraModeZoom = true;
    break;
  case 3:
    cameraDistance = csMax<float> (minimumDistance, cameraDistance - mouseWheelZoomAmount);
    ApplyPositionParameters ();
    break;
  case 4:
    cameraDistance = csMax<float> (minimumDistance, cameraDistance + mouseWheelZoomAmount);
    ApplyPositionParameters ();
    break;
  }
  return false;
}

bool CameraManager::OnMouseUp (iEvent &event)
{
#ifdef CS_DEBUG
  if (!vc || !kbd || !mouse) return false;
#endif

  if (!camera || !mouseMoveEnabled)
    return false;

  uint button = csMouseEventHelper::GetButton (&event);
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

bool CameraManager::OnMouseMove (iEvent &event)
{
#ifdef CS_DEBUG
  if (!vc || !kbd || !mouse) return false;
#endif

  int x = csMouseEventHelper::GetX (&event) - previousMouseX;
  int y = csMouseEventHelper::GetY (&event) - previousMouseY;
  if (!x && !y)
    return false;

  float dx = (float) (x) * 0.004f;
  float dy = (float) (y) * -0.004f;

  previousMouseX = csMouseEventHelper::GetX (&event);
  previousMouseY = csMouseEventHelper::GetY (&event);

  if (cameraModePan)
  {
    panCameraTarget +=
      camera->GetTransform ().This2OtherRelative (csVector3 (1,0,0)) * dx * motionSpeed / 10.0f
      + camera->GetTransform ().This2OtherRelative (csVector3 (0,1,0)) * dy * motionSpeed / 10.0f;
    ApplyPositionParameters ();
  }

  if (cameraModeRotate)
  {
    cameraYaw += dx * rotationSpeed;
    cameraPitch += dy * rotationSpeed;
    cameraPitch = csMax<float> (-3.14159f * 0.5f + 0.01f, cameraPitch);
    cameraPitch = csMin<float> (3.14159f * 0.5f - 0.01f, cameraPitch);
    ApplyPositionParameters ();
  }

  if (cameraModeZoom)
  {
    cameraDistance = csMax<float> (minimumDistance, cameraDistance - (dx + dy) * motionSpeed);
    ApplyPositionParameters ();
  }

  return false;
}

void CameraManager::SetCameraMode (CameraMode cameraMode)
{
  if (camera && this->cameraMode != cameraMode
      && (cameraMode == CAMERA_ROTATE || cameraMode == CAMERA_MOVE_LOOKAT))
    UpdatePositionParameters (camera->GetTransform ().GetOrigin ());

  this->cameraMode = cameraMode;
}

CameraMode CameraManager::GetCameraMode ()
{
  return cameraMode;
}

void CameraManager::SetStartPosition (csVector3 position)
{
  startPosition = position;
  hasStartPosition = true;
}

csVector3 CameraManager::GetStartPosition ()
{
  if (hasStartPosition)
    return startPosition;
  return csVector3 (0.0f);
}

void CameraManager::ClearStartPosition ()
{
  hasStartPosition = false;
}

bool CameraManager::HasStartPosition ()
{
  return hasStartPosition;
}

void CameraManager::SwitchCameraPosition ()
{
  iCameraPositionList* positions = engine->GetCameraPositions ();
  if (positions->GetCount ())
    currentCameraPosition = (currentCameraPosition + 1) % positions->GetCount ();
  ResetCamera ();
}

void CameraManager::SetCameraTarget (csVector3 position)
{
  cameraTarget = position;
}

csVector3 CameraManager::GetCameraTarget ()
{
  return cameraTarget;
}

void CameraManager::SetCameraMinimumDistance (float distance)
{
  minimumDistance = distance;
}

float CameraManager::GetCameraMinimumDistance ()
{
  return minimumDistance;
}

void CameraManager::SetMouseMoveEnabled (bool enabled)
{
  mouseMoveEnabled = enabled;
}

bool CameraManager::GetMouseMoveEnabled ()
{
  return mouseMoveEnabled;
}

void CameraManager::ResetCamera ()
{
  if (camera)
  {
    if (hasStartPosition)
    {
      csOrthoTransform transform (csMatrix3 (), startPosition);
      camera->SetTransform (transform);
    }

    else
    {
      iCameraPositionList* positions = engine->GetCameraPositions ();
      if (positions->GetCount ())
	positions->Get (currentCameraPosition)->Load (camera, engine);
    }

    if (cameraMode == CAMERA_ROTATE || cameraMode == CAMERA_MOVE_LOOKAT)
      UpdatePositionParameters (camera->GetTransform ().GetOrigin ());
  }
}

void CameraManager::SetMotionSpeed (float speed)
{
  motionSpeed = speed;
}

float CameraManager::GetMotionSpeed ()
{
  return motionSpeed;
}

void CameraManager::SetRotationSpeed (float speed)
{
  rotationSpeed = speed;
}

float CameraManager::GetRotationSpeed ()
{
  return rotationSpeed;
}

void CameraManager::UpdatePositionParameters (const csVector3& newPosition)
{
  // Compute the distance, yaw, and pitch values from the target to the new position
  cameraDistance = (cameraTarget - newPosition).Norm ();

  if (fabs (cameraDistance) < EPSILON)
  {
    cameraPitch = 0.0f;
    cameraYaw = 0.0f;
  }

  else
  {
    cameraPitch =
      (float) asin ((cameraTarget.y - newPosition.y) / cameraDistance);

    if (fabs (cameraPitch) < EPSILON)
    {
      cameraYaw = (float) asin ((cameraTarget.x - newPosition.x) / cameraDistance);

      if ((cameraTarget.z - newPosition.z) / (cameraDistance) < 0.0f)
	cameraYaw = 3.14159f - cameraYaw;
    }

    else
    {
      cameraYaw = (float) asin ((cameraTarget.x - newPosition.x)
				/ (cameraDistance * (float) cos (cameraPitch)));

      if ((cameraTarget.z - newPosition.z)
	  / (cameraDistance * (float) cos (cameraPitch)) < 0.0f)
	cameraYaw = 3.14159f - cameraYaw;
    }
  }
}

void CameraManager::ApplyPositionParameters ()
{
  // Apply the distance, yaw, and pitch values to the camera
  csVector3 position;
  csVector3 target = cameraModePan ? panCameraTarget : cameraTarget;

  position.x = target.x
    - cameraDistance * (float) cos (cameraPitch) * (float) sin (cameraYaw);
  position.y = target.y
    - cameraDistance * (float) sin (cameraPitch);
  position.z = target.z
    - cameraDistance * (float) cos (cameraPitch) * (float) cos (cameraYaw);

  camera->GetTransform ().SetOrigin (position);
  camera->GetTransform ().LookAt
    (target - position, csVector3 (0.0f, 1.0f, 0.0f));
}

// ------------------------ DemoApplication ------------------------

DemoApplication::DemoApplication (const char* applicationName)
  : mouseInitialized (false)
{
  SetApplicationName (applicationName);

  // Initialize the default key descriptions
  hudManager.keyDescriptions.Push ("arrow keys: move camera");
  hudManager.keyDescriptions.Push ("SHIFT-arrow keys: lateral motion");
  hudManager.keyDescriptions.Push ("CTRL-arrow keys: speedier motion");
  hudManager.keyDescriptions.Push ("F5: next camera position");
  hudManager.keyDescriptions.Push ("F9: toggle HUD");
  hudManager.keyDescriptions.Push ("F12: screenshot");
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

  visualDebugger = csQueryRegistry<CS::Debug::iVisualDebugger> (GetObjectRegistry ());
  if (!visualDebugger) return ReportError ("Failed to locate visual debugger!");

  printer.AttachNew (new FramePrinter (GetObjectRegistry ()));

  // Create a view to display things
  view = csPtr<iView> (new csView (engine, g3d));
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Load the configuration file
  config.AddConfig (GetObjectRegistry (), "/config/csdemoapplication.cfg");

  // Load the screenshot configuration
  screenshotFormat = config->GetStr ("DemoApplication.Screenshot.ImageFormat", "jpg");
  csString screenshotMask = config->GetStr ("DemoApplication.Screenshot.FilenameFormat",
					    "/tmp/CS_screenshot_0000");
  screenshotHelper.SetMask (screenshotMask + "." + screenshotFormat);

  // Initialize the camera and HUD managers
  hudManager.Initialize (GetObjectRegistry ());
  cameraManager.Initialize (GetObjectRegistry ());

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
  cameraManager.SetCamera (view->GetCamera ());

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

  // Update the camera before the rendering of the view
  cameraManager.UpdateCamera ();

  // Tell the 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  // Display of visual debugging informations
  visualDebugger->Display (view);
}

bool DemoApplication::OnKeyboard (iEvent &event)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&event);
  if (eventtype == csKeyEventTypeDown)
  {
    // Help key
    if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_F1)
    {
      hudManager.SwitchKeysPage ();
      return true;
    }

    // Switch to next camera position
    if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_F5)
    {
      cameraManager.SwitchCameraPosition ();
      return true;
    }

    // Toggle HUD
    if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_F9)
    {
      hudManager.SetEnabled (!hudManager.GetEnabled ());
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

bool DemoApplication::OnMouseMove (iEvent &event)
{
  previousMouse.x = mouse->GetLastX ();
  previousMouse.y = mouse->GetLastY ();

  return false;
}

} // namespace Demo
} // namespace CS
