/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __WALKTEST_H
#define __WALKTEST_H

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "cssys/sysdriv.h"
#include "csengine/collider.h"
#include "csengine/light.h"
#include "csutil/inifile.h"
#include "csutil/vfs.h"
#include "walktest/wentity.h"
#include "iworld.h"

class Polygon3D;
class WalkTest;
class csView;
class csSoundData;
class csWorld;
class csPixmap;
class csWireFrameCam;
class PhysicsLibrary;
class InfiniteMaze;
class HugeRoom;
class csPolygonSet;
struct iCollideSystem;

// Several map modes.
#define MAP_OFF 0
#define MAP_OVERLAY 1
#define MAP_ON 2
#define MAP_TXT 3

///
struct csKeyMap
{
  csKeyMap* next, * prev;
  int key, shift, alt, ctrl;
  char* cmd;
  int need_status,is_on;
};

/**
 * An entry for the record function.
 */
struct csRecordedCamera
{
  csMatrix3 mat;
  csVector3 vec;
  csVector3 angle;
  bool mirror;
  csSector* sector;
};

/**
 * A recorded entry saved in a file.
 */
struct csRecordedCameraFile
{
  long m11, m12, m13;
  long m21, m22, m23;
  long m31, m32, m33;
  long x, y, z;
  long ax, ay, az;
  unsigned char mirror;
};

/**
 * A vector which holds the recorded items and cleans them up if needed.
 */
class csRecordVector : public csVector
{
public:
  /// Free a single element of the array
  virtual bool FreeItem (csSome Item)
  {
    csRecordedCamera* reccam = (csRecordedCamera*)Item;
    delete reccam;
    return true;
  }
};

///
class WalkTest : public SysSystemDriver
{
  typedef SysSystemDriver superclass;

public:
  /// The startup directory on VFS with needed world file
  static char world_dir [100];
  /// A script to execute at startup.
  char* auto_script;

  /// Player position, orientation, and velocity
  csVector3 pos;
  csMatrix3 orient;
  csVector3 velocity;

  /// Camera angles. X and Y are user controllable, Z is not.
  csVector3 angle;

  /**
   * Angular velocity: angle_velocity.x is constantly added to angle.x
   * and so on.
   */
  csVector3 angle_velocity;

  /// Colliders for "legs" and "body". Intersections are handled differently.
  csCollider *legs;
  csCollider *body;
  csVector3 body_radius, legs_radius;

  /// Vector with recorded camera transformations.
  csRecordVector recording;

  /// A list with all busy entities.
  csEntityList busy_entities;
  /// A vector that is used to temporarily store references to busy entities.
  csVector busy_vector;

  // Various configuration values for collision detection.
  /// If >= 0 then we're recording. The value is the current frame entry.
  int cfg_recording;
  /// If >= 0 then we're playing a recording.
  int cfg_playrecording;
  /// Initial speed of jumping.
  float cfg_jumpspeed;
  /// Walk acceleration.
  float cfg_walk_accelerate;
  /// Walk maximum speed.
  float cfg_walk_maxspeed;
  /// Walk brake deceleration.
  float cfg_walk_brake;
  /// Rotate acceleration.
  float cfg_rotate_accelerate;
  /// Rotate maximum speed.
  float cfg_rotate_maxspeed;
  /// Rotate brake deceleration.
  float cfg_rotate_brake;
  /// Look acceleration.
  float cfg_look_accelerate;
  /// Body height.
  float cfg_body_height;
  /// Body width.
  float cfg_body_width;
  /// Body depth.
  float cfg_body_depth;
  /// Eye offset.
  float cfg_eye_offset;
  /// Legs width.
  float cfg_legs_width;
  /// Legs depth.
  float cfg_legs_depth;
  /// Legs offset.
  float cfg_legs_offset;

  /// Was player already spawned?..
  bool player_spawned;

  /**
   * If this flag is true we move in 3D (old system). Otherwise we move more like
   * common 3D games do.
   */
  static bool move_3d;

  /// Color for the TXT map background.
  int bgcolor_txtmap;

  /// Clear color with 'fclear'.
  int bgcolor_fclear;

  /// Clear color for other map modes.
  int bgcolor_map;

  /**
   * If true we show edges around all polygons (debugging).
   */
  bool do_edges;

  /// The world.
  csWorld *world;
  /// The main engine interface
  /// (when interface will be complete, csWorld will not be needed anymore)
  iWorld *World;

  /// The view on the world.
  csView* view;

  /// A pointer to a skybox to animate (if any).
  csThing* anim_sky;

  /// Speed of this animation (with 1 meaning 1 full rotation in a second).
  float anim_sky_speed;

  /// Rotation direction (0=x, 1=y, 2=z)
  int anim_sky_rot;

  /// A sprite to display the Crystal Space Logo
  csPixmap* cslogo;

  /// Our infinite maze object if used.
  InfiniteMaze* infinite_maze;

  /// Our huge room object if used.
  HugeRoom* huge_room;

  /// Some sounds.
  csSoundData* wMissile_boom;
  csSoundData* wMissile_whoosh;

  PhysicsLibrary *pl;

  /// Some flags.
  bool do_fps;
  bool do_stats;
  bool do_clear;
  bool do_show_coord;
  bool do_show_cbuffer;
  bool busy_perf_test;
  bool do_show_z;
  bool do_show_palette;
  bool do_infinite;
  bool do_huge;
  bool do_cd;
  bool do_freelook;
  bool do_gravity;
  bool do_light_frust;
  int cfg_draw_octree;
  int cfg_debug_check_frustum;

  /// The selected light.
  csLight* selected_light;
  /// The selected polygon.
  csPolygon3D* selected_polygon;

  /// A WireFrame object for the map.
  csWireFrameCam* wf;
  /// Map mode.
  int map_mode;
  /// The map projection mode (WF_PROJ_xxx)
  int map_projection;

  /// Timing.
  float timeFPS;

  bool on_ground;
  bool inverse_mouse;
  bool move_forward;

  /// Collision detection plugin.
  iCollideSystem* collide_system;

  /// Player's body (as a 3D model) and legs
  csPolygonSet *plbody, *pllegs;

public:
  ///
  WalkTest ();

  ///
  virtual ~WalkTest ();

  /// Perform some initialization work
  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);

  /// Fire all commands on an object (run time execution).
  virtual void ActivateObject (csObject* object);

  /**
   * Find all key commands attached to an object and execute
   * them (load time execution).
   */
  virtual void ParseKeyCmds (csObject* src);

  /**
   * Find all key commands attached to objects and execute
   * them (load time execution).
   */
  virtual void ParseKeyCmds ();

  ///
  virtual void NextFrame (time_t elapsed_time, time_t current_time);
  ///
  void PrepareFrame (time_t elapsed_time, time_t current_time);

  /// Move bots, particle systems, players, etc. for each frame.
  virtual void MoveSystems (time_t elapsed_time, time_t current_time);

  /**
   * Draw a frame in map mode.
   */
  void DrawFrameMap ();

  /**
   * Draw all things related to debugging (mostly edge drawing).
   * Must be called with G3D set in 2D mode.
   */
  void DrawFrameDebug ();

  /**
   * Another debug DrawFrame version which is called last. This
   * one is temporary and contains debugging stuff active for the
   * current thing that is being debugged.
   */
  void DrawFrameExtraDebug ();

  /**
   * Draw everything for the console.
   */
  virtual void DrawFrameConsole ();

  /**
   * Draw everything for a frame. This includes 3D graphics
   * and everything related to 2D drawing as well (console, debugging, ...).
   */
  virtual void DrawFrame (time_t elapsed_time, time_t current_time);
  /// Draws 3D objects to screen
  virtual void DrawFrame3D (int drawflags, time_t current_time);
  /// Draws 2D objects to screen
  virtual void DrawFrame2D (void);

  /// Load all the graphics libraries needed
  virtual void LoadLibraryData(void);
  virtual void Inititalize2DTextures(void);
  virtual void Create2DSprites(void);

  ///
  virtual bool HandleEvent (csEvent &Event);

  /// Override SetSystemDefaults to handle additional configuration defaults.
  virtual void SetSystemDefaults (csIniFile*);
  /// Override Help to show additional arguments help
  virtual void Help ();

  /// Inits all the collision detection stuff
  virtual void InitWorld(csWorld* world, csCamera* /*camera*/);

  /// Destroys all the collision detection stuff
  virtual void EndWorld();

  /// Creates Colliders
  virtual void CreateColliders();

  /// Gravity correct movement function: strafe().
  void strafe(float speed,int keep_old);
  /// Gravity correct movement function: step().
  void step(float speed,int keep_old);
  /// Gravity correct movement function: rotate().
  void rotate(float speed,int keep_old);
  /// Gravity correct movement function: look().
  void look(float speed,int keep_old);

  /// Immediate gravity incorrect movement functions.
  void imm_forward (float speed, bool slow, bool fast);
  void imm_backward (float speed, bool slow, bool fast);
  void imm_left (float speed, bool slow, bool fast);
  void imm_right (float speed, bool slow, bool fast);
  void imm_up (float speed, bool slow, bool fast);
  void imm_down (float speed, bool slow, bool fast);
  void imm_rot_left_camera (float speed, bool slow, bool fast);
  void imm_rot_left_world (float speed, bool slow, bool fast);
  void imm_rot_right_camera (float speed, bool slow, bool fast);
  void imm_rot_right_world (float speed, bool slow, bool fast);
  void imm_rot_left_xaxis (float speed, bool slow, bool fast);
  void imm_rot_right_xaxis (float speed, bool slow, bool fast);
  void imm_rot_left_zaxis (float speed, bool slow, bool fast);
  void imm_rot_right_zaxis (float speed, bool slow, bool fast);

  ///
  void handle_key_forward (float speed, bool shift, bool alt, bool ctrl);
  ///
  void handle_key_backwards (float speed, bool shift, bool alt, bool ctrl);
  ///
  void handle_key_left (float speed, bool shift, bool alt, bool ctrl);
  ///
  void handle_key_right (float speed, bool shift, bool alt, bool ctrl);
  ///
  void handle_key_pgup (float, bool shift, bool alt, bool ctrl);
  ///
  void handle_key_pgdn (float, bool shift, bool alt, bool ctrl);
  ///
  void eatkeypress (int status, int key, bool shift, bool alt, bool ctrl);

  /// Handle mouse click events
  virtual void MouseClick1Handler(csEvent &Event);
  virtual void MouseClick2Handler(csEvent &Event);
  virtual void MouseClick3Handler(csEvent &Event);
};

extern csVector2 coord_check_vector;

#define FRAME_WIDTH Sys->FrameWidth
#define FRAME_HEIGHT Sys->FrameHeight

extern void perf_test (int num);
extern void CaptureScreen ();
extern void free_keymap ();

extern void SaveCamera (iVFS*, const char *fName);
extern bool LoadCamera (iVFS*, const char *fName);

/// Apply lights to all static objects (currently only sprites)
void light_statics ();

#endif // __WALKTEST_H
