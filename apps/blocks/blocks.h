/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

class csThingTemplate;
class csTextureHandle;
class csSector;
class csWorld;
class csThing;
class csDynLight;

class KeyMapping
{
public:
  int key;
  bool shift, alt, ctrl;

public:
  bool Match (int k, bool s, bool a, bool c)
  {
    return key == k && shift == s && alt == a && ctrl == c;
  }
};

enum BlRotType
{
  ROT_NONE = 0,
  ROT_PX,
  ROT_PY,
  ROT_PZ,
  ROT_MX,
  ROT_MY,
  ROT_MZ
};

enum BlShapeType
{
  SHAPE_RANDOM = -1,
  SHAPE_R1,
  SHAPE_R2,
  SHAPE_R3,
  SHAPE_R4,
  SHAPE_L1,
  SHAPE_L2,
  SHAPE_T1,
  SHAPE_T2,
  SHAPE_FLAT,
  SHAPE_CUBE,
  SHAPE_L3,
  SHAPE_U,
  SHAPE_S,
  SHAPE_T1X,
  SHAPE_FLATX,
  SHAPE_FLATXX,
  SHAPE_DEMO_B,
  SHAPE_DEMO_L,
  SHAPE_DEMO_O,
  SHAPE_DEMO_C,
  SHAPE_DEMO_K,
  SHAPE_DEMO_S
};

#define NUM_EASY_SHAPE (SHAPE_T1+1)
#define NUM_MEDIUM_SHAPE (SHAPE_U+1)
#define NUM_HARD_SHAPE (SHAPE_FLATXX+1)

#define CUBE_DIM .4
#define RAST_DIM .02

// By zone we mean the space where the shapes move/fall.
#define ZONE_DIM 6
#define ZONE_HEIGHT 15

// When blocks freeze at this height the game is over.
#define GAMEOVER_HEIGHT ZONE_HEIGHT-3

// This is a kind of padding around the world(zone).
#define ZONE_SAFETY 2

// Max cubes in a shape.
#define MAX_CUBES 30

// Maximum speed (fall down speed).
#define MAX_FALL_SPEED 12
// Maximum speed in game.
#define MAX_SPEED 10
// Slowest speed.
#define MIN_SPEED .2

// Menus.
#define MENU_NOVICE 0
#define MENU_AVERAGE 1
#define MENU_EXPERT 2
#define MENU_HIGHSCORES 3
#define MENU_SETUP 4
#define MENU_QUIT 5
#define MENU_3X3 6
#define MENU_4X4 7
#define MENU_5X5 8
#define MENU_6X6 9
#define MENU_BOARDSIZE 10
#define MENU_KEYCONFIG 11
#define MENU_TOTAL 12	// Total number of menu entries in system.

#define MAX_MENUS 20	// Maximum number of menus visible at same time.

// Screens.
#define SCREEN_STARTUP 1
#define SCREEN_GAME 2
#define SCREEN_KEYCONFIG 3

struct CubeInfo
{
  csThing* thing;
  float dx, dy, dz;
};

class TextEntryMenu
{
private:
  struct TextEntry
  {
    char* txt;
    char* entry;
    void* userdata;
    TextEntry* next;
  };
  TextEntry* entries;
  TextEntry* last;
  int num_entries;
  int selected;

public:
  TextEntryMenu () { entries = last = NULL; num_entries = 0; selected = 0; }
  ~TextEntryMenu ();
  void Clear ();
  void Add (char* txt, char* entry, void* userdata);
  void Draw ();
  int GetSelected () { return selected; }
  int GetNumEntries () { return num_entries; }
  void SetSelected (int sel) { selected = sel; }
  void SelDown () { selected++; if (selected >= num_entries) selected = num_entries-1; }
  void SelUp () { selected--; if (selected < 0) selected = 0; }
};

class Blocks : public SysSystemDriver
{
private:
  csThingTemplate* cube_tmpl;
  csThingTemplate* pillar_tmpl;
  csThingTemplate* vrast_tmpl;
  csThingTemplate* hrast_tmpl;
  csTextureHandle* cube_txt;
  csTextureHandle* pillar_txt;
  csTextureHandle* raster_txt;
  csSector* room;
  csSector* demo_room;
  csDynLight* dynlight;
  float dynlight_dx;
  float dynlight_x;
  float dynlight_y;
  float dynlight_z;

  csMatrix3 full_rotate_x;
  csMatrix3 full_rotate_x_reverse;
  csMatrix3 full_rotate_y;
  csMatrix3 full_rotate_y_reverse;  
  csMatrix3 full_rotate_z;
  csMatrix3 full_rotate_z_reverse;

  // For the menu.
  csThing* menus[MAX_MENUS];
  int idx_menus[MAX_MENUS];
  csThing* src_menus[MENU_TOTAL];
  int cur_menu;
  int old_cur_menu;
  float menu_todo;
  int num_menus;	// Current number of active menu entries.
  TextEntryMenu* keyconf_menu;

  csVector3 view_origin;

  // Shift value for rotating a shape.
  // Normally shapes are rotated around (0,0,0).
  // With this vector you can shift this half a block
  // to make sure that rotation always happens around
  // the visual center of the shape and not the logical
  // center.
  csVector3 shift_rotate;

  /*
   * How much distance does the camera still need to move (with
   * 1 being the full distance to move)?
   */
  float cam_move_dist;
  csVector3 cam_move_src, cam_move_dest;
  csVector3 cam_move_up;
  csVector3 destinations[4][5];	// [horizontal][vertical]
  int dest_move_right_dx[4];
  int dest_move_right_dy[4];
  int dest_move_down_dx[4];
  int dest_move_down_dy[4];
  int cur_hor_dest, cur_ver_dest;

  /*
   * How much rotation around the specified axis do we still
   * need to do for the current cube?
   */
  float rot_px_todo;
  float rot_py_todo;
  float rot_pz_todo;
  float rot_mx_todo;
  float rot_my_todo;
  float rot_mz_todo;
  BlRotType queue_rot_todo;

  /*
   * How much do we have to move down before we reach another
   * cube-level?
   */
  float move_down_todo;

  /*
   * How much distance do we have to move horizontally and in
   * what direction?
   */
  float move_hor_todo;
  int move_hor_dx;
  int move_hor_dy;
  int queue_move_dx_todo;
  int queue_move_dy_todo;
  
  /*
   * The following four flags indicate how the movement keys work.
   * This is to make sure that pressing the right arrow key will always
   * move the block to the right for example.
   */
  int move_right_dx;      
  int move_right_dy;
  int move_down_dx;
  int move_down_dy;

  // Current dimensions of game area.
  int zone_dim;

  // Tells us wheather a cell is occupied. It's padded at both ends along
  // each axis. It is not recomended to access it directly (eg I forgot
  // abotut ZONE_SAFETY, and ka-booom).
  bool game_cube[ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY]
  	[ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY]
	[ZONE_SAFETY+ZONE_HEIGHT+ZONE_SAFETY];

  int num_cubes;
  CubeInfo cube_info[MAX_CUBES];

  int cube_x, cube_y, cube_z;

  // Current speed.
  float speed;
  // Current speed depending on level.
  float cur_speed;

  /// If true we are paused.
  bool pause;

  // When true clear world and start a new game. Usually false,
  // 'cause we are playing.
  bool initscreen;

  // If true we have Game Over.
  bool gameover;

  // Type of screen we are using (one of SCREEN_xxx).
  int screen;

  // Difficulty setting.
  int difficulty;

  /* NOTE: by a plane we mean if a certain height level is full of cubes.
     It's the analogue of a line in tetris games (2d) */

  // If true we are in the process moving cubes/things lower because
  // at least one plane was made.
  bool transition;

  // Shows if we have a plane at a certain height.
  bool filled_planes[ZONE_HEIGHT];

  // Fog density.
  float fog_density;

  // This is the z of the plane which's dissapearance is handled right now.
  int gone_z;

  int score;

  // Keys...
  KeyMapping key_up;
  KeyMapping key_down;
  KeyMapping key_left;
  KeyMapping key_right;
  KeyMapping key_rotpx;
  KeyMapping key_rotmx;
  KeyMapping key_rotpy;
  KeyMapping key_rotmy;
  KeyMapping key_rotpz;
  KeyMapping key_rotmz;
  KeyMapping key_pause;
  KeyMapping key_drop;
  KeyMapping key_esc;
  KeyMapping key_viewleft;
  KeyMapping key_viewright;
  KeyMapping key_viewup;
  KeyMapping key_viewdown;
  KeyMapping key_zoomin;
  KeyMapping key_zoomout;

public:
  csWorld* world;
  iTextureManager* txtmgr;
  static int white, black, red;

public:
  Blocks ();
  ~Blocks ();

  // Initialization stuff and starting of game/demo.
  void InitTextures ();
  void InitWorld ();
  void InitGameRoom ();
  void InitDemoRoom ();
  void InitMainMenu ();
  void StartNewGame ();
  void StartDemo ();
  void StartKeyConfig ();
  void set_cube_room (csSector* s) { room = s; }
  void InitGame ();
  void CreateMenuEntry (char* txt, int menu_nr);
  void ChangePlaySize (int new_size);

  void ReadConfig ();
  void WriteConfig ();
  void NamedKey (char* keyname, KeyMapping& map);
  char* KeyName (const KeyMapping& map);

  void DrawMenu (int menu);
  void DrawMenu (float menu_transition, int old_menu, int new_menu);
  void InitMenu ();
  void AddMenuItem (int menu_nr);

  // Handling of basic events and frame drawing.
  virtual void NextFrame (time_t elapsed_time, time_t current_time);
  virtual bool HandleEvent (csEvent &Event);
  void HandleKey (int key, bool shift, bool alt, bool ctrl);
  void HandleGameKey (int key, bool shift, bool alt, bool ctrl);
  void HandleDemoKey (int key, bool shift, bool alt, bool ctrl);
  void HandleKeyConfigKey (int key, bool shift, bool alt, bool ctrl);

  // Creating cubes and other geometry.
  csThing* create_cube_thing (float dx, float dy, float dz);
  csThing* add_cube_thing (csSector* sect, float dx, float dy, float dz,
  	float x, float y, float z);
  void add_cube (float dx, float dy, float dz, float x, float y, float z);
  void add_pillar (int x, int y);
  void add_vrast (int x, int y, float dx, float dy, float rot_z);
  void add_hrast (int x, int y, float dx, float dy, float rot_z);

  // All the templates for creating geometry.
  void add_cube_template ();
  void add_pillar_template ();
  void add_vrast_template ();
  void add_hrast_template ();

  // Default textures for geometry.
  void set_cube_texture (csTextureHandle* ct) { cube_txt = ct; }
  void set_pillar_texture (csTextureHandle* ct) { pillar_txt = ct; }
  void set_raster_texture (csTextureHandle* ct) { raster_txt = ct; }

  // Handle all time dependent movement of the game and menu.
  // This function will call some of the Handle... routines below.
  void HandleMovement (time_t elapsed_time);
  // Is called when we are in transition mode instead of HandleMovement()
  // (this happens when a plane is moving down).
  void HandleTransition (time_t elapsed_time);
  // Handle the movement of the camera.
  void HandleCameraMovement ();
  // Handle movement for the startup screen.
  void HandleStartupMovement (time_t elapsed_time);
  // Handle movement for the game screen.
  void HandleGameMovement (time_t elapsed_time);
  // Handle lowering of planes.
  void HandleLoweringPlanes (time_t elapsed_time);


  // Update the score.
  void updateScore ();

  // Conveniance functions.
  csMatrix3 create_rotate_x (float angle);
  csMatrix3 create_rotate_y (float angle);
  csMatrix3 create_rotate_z (float angle);

  // Start to rotate the current falling block.
  void start_rotation (BlRotType type);
  // Start to move the current falling block horizontally.
  void start_horizontal_move (int dx, int dy);
  // If there is nothing falling down this function causes
  // a new shape to fall down.
  void StartNewShape ();
  // For demo purposes.
  void start_demo_shape (BlShapeType type, float x, float y, float z);

  void move_shape_internal (int dx, int dy, int dz);
  void rotate_shape_internal (const csMatrix3& rot);

  // Return true if free.
  bool check_new_shape_location (int dx, int dy, int dz);
  bool check_new_shape_rotation (const csMatrix3& rot);

  // Shape has stopped.
  void freeze_shape ();

  // Debugging.
  void dump_shape ();

  // Access the cubes in the playing field.
  bool get_cube (int x, int y, int z)
  { return game_cube[x+ZONE_SAFETY][y+ZONE_SAFETY][z+ZONE_SAFETY]; }
  void set_cube (int x, int y, int z, bool v)
  { game_cube[x+ZONE_SAFETY][y+ZONE_SAFETY][z+ZONE_SAFETY] = v; }

  // Checks to see if a plane was formed.
  void checkForPlane ();

  // Remove some plane.
  void removePlane (int z);
};

#endif // BLOCKS_H
