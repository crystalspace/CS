/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __ISOTEST_H__
#define __ISOTEST_H__

#include <stdarg.h>
#include "csutil/ref.h"
#include "csgeom/vector3.h"
#include "iutil/eventnames.h"
#include "imesh/nskeleton.h"
#include "imesh/skelanim.h"

struct iEngine;
struct iLoader;
struct iGraphics3D;
struct iKeyboardDriver;
struct iVirtualClock;
struct iObjectRegistry;
struct iEvent;
struct iSector;
struct iSkeleton;
struct iSkeletonBone;
struct iView;
struct iMeshWrapper;
struct iLight;
struct iCamera;
struct iFont;

/**
 * Capture an isometric camera viewpoint.
 * After changing angle or distance, call SetupIsoView() on it.
 * Every frame the camera moves or changes angle or distance call
 *   CameraIsoLookat with the view as argument.
 */
struct IsoView
{
  /// offset to apply to the camera.
  csVector3 camera_offset;
  /// original camera offset.
  csVector3 original_offset;
  /// angle of rotation, in degrees, 0.0 is the original rotation.
  float angle;
  /// distance from the lookat spot. 1.0 is original distance.
  float distance;

  /// initialize with original offset of camera from the spot you look at.
  void SetOrigOffset(const csVector3& v) 
  { camera_offset = original_offset = v; angle = 0.f; distance = 1.f; }
};

class IsoTest
{
private:
  /*enum ManipulationMode
  {
    TRAN_GRAB = 0,
    TRAN_ROTATE
  } manipmode;
  enum Axis
  {
    AXIS_X = 0,
    AXIS_Y,
    AXIS_Z
  } transaxis;*/

  int selboneid;
  iSkeletonBone* selbone;
  iSkeleton* skeleton;
  Skeleton::iSkeleton* myskel;

  csRef<Skeleton::Animation::iAnimation> anim_punch;
  csRef<Skeleton::Animation::iBlendNode> blend;
  csRef<Skeleton::Animation::iAccumulateNode> blpen;
  size_t walkid, standid, punchid, otherid;

  enum
  {
    STAND = 0,
    WALK,
    STAND_WALK,
    WALK_STAND
  } feather, pfeather;
  float feather_duration;
  float pfeather_duration;

  iObjectRegistry* object_reg;
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  size_t last_time;
  csRef<Skeleton::iGraveyard> skelgrave;
  iSector* room;
  csRef<iView> view;
  csRef<iMeshWrapper> actor;
  iMeshWrapper* plane;
  csRef<iLight> actor_light;
  csRef<iFont> font;

  csEventID Process;
  csEventID FinalProcess;
  csEventID KeyboardDown;

  int current_view;
  IsoView views[4];
  /// is the main actor walking
  bool actor_is_walking;

  static bool IsoTestEventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();
  void FinishFrame ();

  bool CreateActor ();
  bool LoadMap ();

  /// make the camera look at given position using isometric viewpoint.
  void CameraIsoLookat(csRef<iCamera> cam, const IsoView& isoview, 
    const csVector3& lookat);
  /// setup an isometric view to be ready for display, call after rotating
  void SetupIsoView(IsoView& isoview);

  bool LoadKwartzAnim ();
  bool LoadKirchdorferAnim ();

  /**
   * Draw a cool looking bone like in Blender.
   */
  static void DrawBone (float length, iGraphics3D* g3d);

public:
  IsoTest (iObjectRegistry* object_reg);
  ~IsoTest ();

  bool Initialize ();
  void Start ();
};

#endif // __ISOTEST_H__

