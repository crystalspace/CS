/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __WALKTEST_H__
#define __WALKTEST_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/box.h"
#include "cstool/collider.h"
#include "csutil/cscolor.h"
#include "csutil/parray.h"
#include "wentity.h"
#include "iengine/engine.h"
#include "iengine/fview.h"
#include "ivaria/conout.h"
#include "ivaria/conin.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "ivideo/fontserv.h"
#include "bot.h"

#include "iengine/engine.h"

class WalkTest;
class csPixmap;
class csWireFrameCam;
class InfiniteMaze;
struct iEngine;
struct iRegion;
struct iSoundHandle;
struct iCollideSystem;
struct iObjectRegistry;
struct iPluginManager;
struct iConfigFile;
struct iMaterialHandle;
struct iLoader;
struct iMeshWrapper;
struct iLight;
struct iView;
struct iSoundRender;
struct iModelConverter;
struct iCrossBuilder;
struct iKeyboardDriver;
struct iVirtualClock;
struct iGraphics3D;
struct iGraphics2D;

// Several map modes.
#define MAP_OFF 0
#define MAP_OVERLAY 1
#define MAP_ON 2
#define MAP_TXT 3

SCF_VERSION (WalkDataObject, 0, 0, 1);
class WalkDataObject : public csObject
{
protected:
  /// Pointer to data.
  void* data;

public:
  /// Initialize this object with data pointer initialized to 'd'
  WalkDataObject (void *d) : csObject (), data (d)
  {
  }
  /// Destroy object.
  virtual ~WalkDataObject ()
  {
  }
  /// Get the data associated with this object
  void* GetData () const
  { return data; }
  /**
   * Get first data pointer associated with other object.
   */
  static void* GetData (iObject* obj)
  {
    csRef<WalkDataObject> d (CS_GET_CHILD_OBJECT (obj, WalkDataObject));
    void *res = (d ? d->GetData () : 0);
    return res;
  }

  SCF_DECLARE_IBASE_EXT (csObject);
};

///
struct csKeyMap
{
  csKeyMap* next, * prev;
  utf32_char key;
  bool shift, alt, ctrl;
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
  bool mirror;
  iSector* sector;
  char *cmd;
  char *arg;
  ~csRecordedCamera ()
  { delete [] cmd; delete [] arg; }
};

/**
 * A recorded entry saved in a file.
 */
struct csRecordedCameraFile
{
  int32 m11, m12, m13;
  int32 m21, m22, m23;
  int32 m31, m32, m33;
  int32 x, y, z;
  unsigned char mirror;
};

/**
 * A vector which holds the recorded items and cleans them up if needed.
 */
typedef csPDelArray<csRecordedCamera> csRecordVector;

struct csMapToLoad
{
  /// The startup directory on VFS with needed map file
  char* map_dir;
  csMapToLoad* next_map;
};

///
class WalkTest
{
public:
  iObjectRegistry* object_reg;
  csRef<iPluginManager> plugin_mgr;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;

  csRefArray<iLight> dynamic_lights;

  int FrameWidth, FrameHeight;

  /// All maps we want to load.
  csMapToLoad* first_map, * last_map;
  int num_maps;
  csMapToLoad* cache_map;	// If 0 no cache: entry was given.
  /// A script to execute at startup.
  char* auto_script;

  /// Player position, orientation, and velocity
  csVector3 pos;
  csVector3 velocity;

  /// Camera angles. X and Y are user controllable, Z is not.
  //csVector3 angle;

  /**
   * Angular velocity: angle_velocity.x is constantly added to angle.x
   * and so on.
   */
  csVector3 angle_velocity;

  /// Colliders for "legs" and "body". Intersections are handled differently.
  csRef<iCollider> body;
  csRef<iCollider> legs;
  csVector3 body_radius, body_center, legs_radius, legs_center;

  /// A list with all busy entities.
  csArray<csWalkEntity*> busy_entities;
  /// A vector that is used to temporarily store references to busy entities.
  csArray<csWalkEntity*> busy_vector;
  /// Vector with recorded camera transformations and commands.
  csRecordVector recording;
  /// This frames current recorded cmd and arg
  char *recorded_cmd;
  char *recorded_arg;
  /// Time when we started playing back the recording.
  csTicks record_start_time;
  /// Number of frames that have passed since we started playing back recording.
  int record_frame_count;
  // Various configuration values for collision detection.
  /// If >= 0 then we're recording. The value is the current frame entry.
  int cfg_recording;
  /// If >= 0 then we're playing a recording.
  int cfg_playrecording;
  /// If true the demo recording loops.
  bool cfg_playloop;
  /// Initial speed of jumping.
  float cfg_jumpspeed;
  /// Walk acceleration.
  float cfg_walk_accelerate;
  /// Walk maximum speed.
  float cfg_walk_maxspeed;
  /// Multiplier for maximum speed.
  float cfg_walk_maxspeed_mult;
  /// Is multiplier used?
  float cfg_walk_maxspeed_multreal;
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
   * If this flag is true we move in 3D (old system). Otherwise we move more
   * like common 3D games do.
   */
  static bool move_3d;

  /// Color for the TXT map background.
  int bgcolor_txtmap;

  /// Clear color for other map modes.
  int bgcolor_map;

  /**
   * If true we show edges around all polygons (debugging).
   */
  bool do_edges;

  // Various settings for fullscreen effects.
  bool do_fs_inter;
  float fs_inter_amount;
  float fs_inter_anim;
  float fs_inter_length;

  bool do_fs_fadeout;
  float fs_fadeout_fade;
  bool fs_fadeout_dir;

  bool do_fs_fadecol;
  float fs_fadecol_fade;
  bool fs_fadecol_dir;
  csColor fs_fadecol_color;

  bool do_fs_fadetxt;
  float fs_fadetxt_fade;
  bool fs_fadetxt_dir;
  iMaterialHandle* fs_fadetxt_mat;

  bool do_fs_red;
  float fs_red_fade;
  bool fs_red_dir;
  bool do_fs_green;
  float fs_green_fade;
  bool fs_green_dir;
  bool do_fs_blue;
  float fs_blue_fade;
  bool fs_blue_dir;

  bool do_fs_whiteout;
  float fs_whiteout_fade;
  bool fs_whiteout_dir;

  bool do_fs_shadevert;
  csColor fs_shadevert_topcol;
  csColor fs_shadevert_botcol;

  /**
   * The main engine interface
   * (when interface will be complete, csEngine will not be needed anymore)
   */
  csRef<iEngine> Engine;
  /// The level loader
  csRef<iLoader> LevelLoader;
  /// The model importer and crossbuilder
  csRef<iModelConverter> ModelConverter;
  csRef<iCrossBuilder> CrossBuilder;
  ///
  csRef<iGraphics2D> myG2D;
  csRef<iGraphics3D> myG3D;
  csRef<iConsoleOutput> myConsole;
  csRef<iVFS> myVFS;
  csRef<iSoundRender> mySound;

  /// The view on the world.
  iView* view;

  /// A pointer to a skybox to animate (if any).
  iMeshWrapper* anim_sky;
  /// Speed of this animation (with 1 meaning 1 full rotation in a second).
  float anim_sky_speed;
  /// Rotation direction (0=x, 1=y, 2=z)
  int anim_sky_rot;

  /// A pointer to the terrain for which we animate the dirlight.
  iMeshWrapper* anim_dirlight;

  /// A sprite to display the Crystal Space Logo
  csPixmap* cslogo;

  /// Our infinite maze object if used.
  InfiniteMaze* infinite_maze;

  /// Some sounds.
  iSoundHandle* wMissile_boom;
  iSoundHandle* wMissile_whoosh;

  /// Some flags.
  bool do_show_coord;
  bool busy_perf_test;
  bool do_show_z;
  bool do_show_palette;
  bool do_infinite;
  bool do_huge;
  bool do_cd;
  bool do_freelook;
  bool do_gravity;
  bool do_light_frust;
  bool do_logo;
  bool doSave;
  int cfg_debug_check_frustum;
  int fgcolor_stats;

  /// The selected light.
  iLight* selected_light;
  /// The selected polygon.
  int selected_polygon;

  /// Debug box 1.
  csBox3 debug_box1;
  /// Debug box 2.
  csBox3 debug_box2;
  /// If true then show both debug boxes.
  bool do_show_debug_boxes;

  bool on_ground;
  bool inverse_mouse;
  bool move_forward;

  /// Collision detection plugin.
  csRef<iCollideSystem> collide_system;

  /// Player's body (as a 3D model) and legs
  //csRef<iMeshWrapper> plbody, pllegs;

  /// The console input plugin
  csRef<iConsoleInput> ConsoleInput;
  /// Is the console smaller than the screen?
  bool SmallConsole;

  /// The font we'll use for writing
  csRef<iFont> Font;

  /// Value to indicate split state
  /// -1 = not split, other value is index of current view
  int split;
  csRef<iView> views[2];

  /// is actually anything visible on the canvas?
  bool canvas_exposed;

  csArray<iMeshWrapper*> ghosts;

public:
  ///
  WalkTest ();

  ///
  virtual ~WalkTest ();

  /// Perform some initialization work
  bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);

  /// Report something to the reporter.
  void Report (int severity, const char* msg, ...);

  /// Fire all commands on an object (run time execution).
  virtual void ActivateObject (iObject* object);

  /**
   * Find all key commands attached to an object and execute
   * them (load time execution).
   */
  virtual void ParseKeyCmds (iObject* src);

  /**
   * Find all SEED_MESH_OBJ nodes
   * commands attached to an object and create them (load time execution).
   */
  void ParseKeyNodes (iObject* src);

  /**
   * Find all key commands attached to objects and execute
   * them (load time execution).
   */
  virtual void ParseKeyCmds ();

  /**
   * Set the current VFS dir to the given map directory.
   */
  bool SetMapDir (const char* map_dir);

  /// Draw the frame.
  void SetupFrame ();
  /// Finalize the frame.
  void FinishFrame ();
  ///
  void PrepareFrame (csTicks elapsed_time, csTicks current_time);

  /// Move bots, particle systems, players, etc. for each frame.
  virtual void MoveSystems (csTicks elapsed_time, csTicks current_time);

  /**
   * Draw all things related to debugging (mostly edge drawing).
   * Must be called with G3D set in 2D mode.
   */
  void DrawFrameDebug ();

  /**
   * Draw all things related to debugging in 3D mode instead of 2D.
   * Must be called with G3D set in 3D mode.
   */
  void DrawFrameDebug3D ();

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
  virtual void DrawFrame (csTicks elapsed_time, csTicks current_time);
  /// Draws 3D objects to screen
  virtual void DrawFrame3D (int drawflags, csTicks current_time);
  /// Draws 2D objects to screen
  virtual void DrawFrame2D (void);
  /// Draw 3D fullscreen effects.
  virtual void DrawFullScreenFX3D (csTicks elapsed_time, csTicks current_time);
  /// Draw 2D fullscreen effects.
  virtual void DrawFullScreenFX2D (csTicks elapsed_time, csTicks current_time);

  /// Load all the graphics libraries needed
  virtual void LoadLibraryData (iRegion* region);
  virtual void Inititalize2DTextures ();
  virtual void Create2DSprites ();

  ///
  bool WalkHandleEvent (iEvent &Event);

  /// Handle additional configuration defaults.
  void SetDefaults ();
  /// Commandline help for WalkTest.
  void Help ();

  /// Inits all the collision detection stuff
  virtual void InitCollDet (iEngine* engine, iRegion* region);

  /// Destroys all the collision detection stuff
  virtual void EndEngine();

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

  void RotateCam(float x, float y);
  
  ///
  void eatkeypress (iEvent* Event);

  /// Handle mouse click events
  virtual void MouseClick1Handler(iEvent &Event);
  virtual void MouseClick2Handler(iEvent &Event);
  virtual void MouseClick3Handler(iEvent &Event);

  void GfxWrite (int x, int y, int fg, int bg, char *str, ...);

  // Bot stuff
  csPDelArray<Bot> bots;
  void add_bot (float size, iSector* where, csVector3 const& pos,
    float dyn_radius);
  void del_bot ();
  void move_bots (csTicks);
};

extern csVector2 coord_check_vector;

#define FRAME_WIDTH Sys->FrameWidth
#define FRAME_HEIGHT Sys->FrameHeight

extern void perf_test (int num);
extern void CaptureScreen ();
extern void free_keymap ();

extern void SaveCamera (iVFS*, const char *fName);
extern bool LoadCamera (iVFS*, const char *fName);

// Use a view's clipping rect to calculate a bounding box
void BoundingBoxForView(iView *view, csBox2 *box);

// The global system driver
extern WalkTest *Sys;

#endif // __WALKTEST_H__
