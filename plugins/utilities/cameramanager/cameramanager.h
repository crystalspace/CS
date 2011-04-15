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
#ifndef __CS_CAMERAMANAGER_H__
#define __CS_CAMERAMANAGER_H__

#include "cssysdef.h"
#include "csgeom/vector3.h"
#include "csutil/scf_implementation.h"
#include "csutil/eventhandlers.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iutil/comp.h"
#include "iutil/csinput.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/virtclk.h"
#include "ivaria/cameramanager.h"

CS_PLUGIN_NAMESPACE_BEGIN(CameraManager)
{

/**
 * Camera motion manager
 */

class CameraManager
  : public scfImplementation3<CameraManager,
  CS::Utility::iCameraManager,
  iEventHandler,
  iComponent>
{
public:
  CameraManager (iBase* parent);
  ~CameraManager ();

  //-- iComponent
  bool Initialize (iObjectRegistry* registry);

  //-- iEventHandler
  bool HandleEvent (iEvent& event);

  //-- CS::Utility::iCameraManager
  virtual void SetCamera (iCamera* camera);
  virtual iCamera* GetCamera ();

  virtual void SetCameraMode (CS::Utility::CameraMode cameraMode);
  virtual CS::Utility::CameraMode GetCameraMode ();

  virtual void SetStartPosition (csVector3 position);
  virtual csVector3 GetStartPosition ();
  virtual void ClearStartPosition ();
  virtual bool HasStartPosition ();
  virtual void SwitchCameraPosition ();

  virtual void SetCameraTarget (csVector3 position);
  virtual csVector3 GetCameraTarget ();

  virtual void SetCameraMinimumDistance (float distance);
  virtual float GetCameraMinimumDistance ();

  virtual void SetMouseMoveEnabled (bool enabled);
  virtual bool GetMouseMoveEnabled ();

  virtual void ResetCamera ();

  virtual void SetMotionSpeed (float speed);
  virtual float GetMotionSpeed ();

  virtual void SetRotationSpeed (float speed);
  virtual float GetRotationSpeed ();

private:
  void Frame ();
  bool OnMouseDown (iEvent& event);
  bool OnMouseUp (iEvent& event);
  bool OnMouseMove (iEvent& event);

  void UpdatePositionParameters (const csVector3& newPosition);
  void ApplyPositionParameters ();

  iObjectRegistry* registry;

  // Reference to the engine
  csRef<iEngine> engine;
  // Reference to the keyboard driver
  csRef<iKeyboardDriver> kbd;
  // Reference to the virtual clock
  csRef<iVirtualClock> vc;
  // Reference to the mouse driver
  csRef<iMouseDriver> mouse;
  // Reference to the event queue
  csRef<iEventQueue> eventQueue;

  // Reference to the camera
  csRef<iCamera> camera;

  CS::Utility::CameraMode cameraMode;
  bool mouseMoveEnabled;

  csVector3 startPosition;
  bool hasStartPosition;
  size_t currentCameraPosition;  

  csVector3 cameraTarget;
  float minimumDistance;

  csVector3 panCameraTarget;
  float cameraDistance;
  float cameraYaw;
  float cameraPitch;

  bool cameraModePan;
  bool cameraModeRotate;
  bool cameraModeZoom;

  float motionSpeed;
  float rotationSpeed;

  int previousMouseX, previousMouseY;

  // Declare this event handler as listening to the '2D' frame phase
  CS_EVENTHANDLER_PHASE_2D ("crystalspace.utilities.cameramanager");
};

}
CS_PLUGIN_NAMESPACE_END(CameraManager)

#endif // __CS_CAMERAMANAGER_H__
