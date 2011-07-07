/*
  Copyright (C) 2010-11 Christian Van Brussel, Institute of Information
  and Communication Technologies, Electronics and Applied Mathematics
  at Universite catholique de Louvain, Belgium
  http://www.uclouvain.be/en-icteam.html

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
#include "cameramanager.h"
#include "csutil/event.h"
#include "iengine/campos.h"
#include "iutil/evdefs.h"
#include "ivaria/reporter.h"

CS_PLUGIN_NAMESPACE_BEGIN(CameraManager)
{

/**
 * Error reporting
 */
  static const char* objectId = "crystalspace.utilities.cameramanager";

  bool ReportError (iObjectRegistry* registry, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (registry, CS_REPORTER_SEVERITY_ERROR, objectId, description, arg);
    va_end (arg);
    return false;
  }

  bool ReportWarning (iObjectRegistry* registry, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (registry, CS_REPORTER_SEVERITY_WARNING, objectId, description, arg);
    va_end (arg);
    return false;
  }

// ------------------------ CameraManager ------------------------

  SCF_IMPLEMENT_FACTORY (CameraManager);

  CameraManager::CameraManager (iBase* parent)
    : scfImplementationType (this, parent), cameraMode (CS::Utility::CAMERA_MOVE_FREE),
    mouseMoveEnabled (true), hasStartPosition (false), currentCameraPosition (0), cameraTarget (0.0f),
    minimumDistance (0.1f), cameraDistance (0.0f), cameraYaw (0.0f), cameraPitch (0.0f),
    cameraModePan (false), cameraModeRotate (false), cameraModeZoom (false),
    motionSpeed (5.0f), rotationSpeed (2.0f), previousMouseX (0), previousMouseY (0)
{
}

CameraManager::~CameraManager ()
{
  // Unregister from the event queue
  if (eventQueue)
    eventQueue->RemoveListener (this);
}

bool CameraManager::Initialize (iObjectRegistry* registry)
{
  this->registry = registry;

  // Find references to the engine objects
  engine = csQueryRegistry<iEngine> (registry);
  if (!engine) return ReportError (registry, "crystalspace.demo.cameramanager", "Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (registry);
  if (!vc) return ReportError (registry, "crystalspace.demo.cameramanager", "Failed to locate virtual clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (registry);
  if (!kbd) return ReportError (registry, "crystalspace.demo.cameramanager", "Failed to locate keyboard driver!");

  mouse = csQueryRegistry<iMouseDriver> (registry);
  if (!mouse) return ReportError (registry, "crystalspace.demo.cameramanager", "Failed to locate mouse driver!");

  eventQueue = csQueryRegistry<iEventQueue> (registry);
  if (!eventQueue) return ReportError (registry, "crystalspace.demo.cameramanager", "Failed to locate event queue!");

  // Register to the event queue
  csEventID events[3] = { csevFrame (registry), csevMouseEvent (registry), 
			  CS_EVENTLIST_END };
  eventQueue->RegisterListener (this, events);

  return true;
}

void CameraManager::SetCamera (iCamera* camera)
{
  this->camera = camera;

  if (camera && cameraMode != CS::Utility::CAMERA_NO_MOVE)
    ResetCamera ();
}

iCamera* CameraManager::GetCamera ()
{
  return camera;
}

bool CameraManager::HandleEvent (iEvent& event)
{
  if (event.Name == csevFrame (registry))
  {
    Frame();
    return true;
  }

  if (CS_IS_MOUSE_EVENT (registry, event))
    switch(csMouseEventHelper::GetEventType (&event))
    {
    case csMouseEventTypeMove:
      return OnMouseMove (event);
    case csMouseEventTypeUp:
      return OnMouseUp (event);
    case csMouseEventTypeDown:
      return OnMouseDown (event);
    default:
      break;
    }

  return false;
}

void CameraManager::Frame ()
{
  if (!camera || cameraMode == CS::Utility::CAMERA_NO_MOVE) return;

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
  case CS::Utility::CAMERA_MOVE_FREE:
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

  case CS::Utility::CAMERA_MOVE_LOOKAT:
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

  case CS::Utility::CAMERA_ROTATE:
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
}

bool CameraManager::OnMouseDown (iEvent& event)
{
  if (!camera || !mouseMoveEnabled)
    return false;

  if (cameraMode != CS::Utility::CAMERA_ROTATE && cameraMode != CS::Utility::CAMERA_MOVE_LOOKAT)
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

bool CameraManager::OnMouseUp (iEvent& event)
{
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

bool CameraManager::OnMouseMove (iEvent& event)
{
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

void CameraManager::SetCameraMode (CS::Utility::CameraMode cameraMode)
{
  if (camera && this->cameraMode != cameraMode
      && (cameraMode == CS::Utility::CAMERA_ROTATE || cameraMode == CS::Utility::CAMERA_MOVE_LOOKAT))
    UpdatePositionParameters (camera->GetTransform ().GetOrigin ());

  this->cameraMode = cameraMode;
}

CS::Utility::CameraMode CameraManager::GetCameraMode ()
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

    if (cameraMode == CS::Utility::CAMERA_ROTATE || cameraMode == CS::Utility::CAMERA_MOVE_LOOKAT)
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

}
CS_PLUGIN_NAMESPACE_END(CameraManager)
