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

enum BlRotType
{
  ROT_PX,
  ROT_PY,
  ROT_PZ,
  ROT_MX,
  ROT_MY,
  ROT_MZ
};

enum BlShapeType
{
  SHAPE_R1 = 0,
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

// By zone we mean the space where the shapes move/fall.
#define ZONE_DIM 6
#define ZONE_HEIGHT 15

// This is a kind of padding around the world(zone).
#define ZONE_SAFETY 2

// Max cubes in a shape. This is set high for the demo. @@@
#define MAX_CUBES 300

struct CubeInfo
{
  csThing* thing;
  int dx, dy, dz;
};

///
class Blocks : public SysSystemDriver
{
private:
  csThingTemplate* cube_tmpl;
  csThingTemplate* pilar_tmpl;
  csTextureHandle* cube_txt;
  csTextureHandle* pilar_txt;
  csSector* room;
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

  csVector3 view_origin;

  /**
   * How much distance does the camera still need to move (with
   * 1 being the full distance to move)?
   */
  float cam_move_dist;
  csVector3 cam_move_src, cam_move_dest;
  csVector3 cam_move_up;
  csVector3 destinations[4][4];
  int dest_move_right_dx[4];
  int dest_move_right_dy[4];
  int dest_move_down_dx[4];
  int dest_move_down_dy[4];
  int cur_hor_dest, cur_ver_dest;

  /**
   * How much rotation around the specified axis do we still
   * need to do for the current cube?
   */
  float rot_px_todo;
  float rot_py_todo;
  float rot_pz_todo;
  float rot_mx_todo;
  float rot_my_todo;
  float rot_mz_todo;

  /**
   * How much do we have to move down before we reach another
   * cube-level?
   */
  float move_down_todo;

  /**
   * How much distance do we have to move horizontally and in
   * what direction?
   */
  float move_hor_todo;
  int move_hor_dx;
  int move_hor_dy;
  
  /**
   * The following four flags indicate how the movement keys work.
   * This is to make sure that pressing the right arrow key will always
   * move the block to the right for example.
   */
  int move_right_dx;      
  int move_right_dy;
  int move_down_dx;
  int move_down_dy;

  // Tells us wheather a cell is occupied. It's padded at both ends along
  // each axis. It is not recomended to access it directly (eg I forgot
  // abotut ZONE_SAFETY, and ka-booom).
  bool game_cube[ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY]
  	[ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY]
	[ZONE_SAFETY+ZONE_HEIGHT+ZONE_SAFETY];

  int num_cubes;
  CubeInfo cube_info[MAX_CUBES];

  int cube_x, cube_y, cube_z;
  float speed;

  /// If true we are paused.
  bool pause;

  // When true clear world and start a new game. Usually false,
  // 'cause we are playing.
  bool newgame;

  // When true we are in startup screen mode.
  bool startup_screen;

  // Difficulty setting.
  int difficulty;

  /* NOTE: by a plane we mean if a certain height level is full of cubes.
     It's the analogue of a line in tetris games (2d) */

  // If true we are in the process moving cubes/things lower because
  // at least one plane was made.
  bool transition;


  // Shows if we have a plane at a certain height.
  bool filled_planes[ZONE_HEIGHT];

  // This is the z of the plane which's dissapearance is handled right now.
  int gone_z;

  int score;

public:
  csWorld* world;

public:
  ///
  Blocks ();


  ///
  void InitTextures ();
  ///
  void StartNewGame ();
  ///
  void StartDemo ();

  ///
  virtual void NextFrame (time_t elapsed_time, time_t current_time);

  ///
  virtual bool HandleEvent (csEvent &Event);

  ///
  void eatkeypress (int key, bool shift, bool alt, bool ctrl);

  ///
  void add_cube (int dx, int dy, int dz, int x, int y, int z);

  ///
  void add_pilar (int x, int y);

  ///
  void add_cube_template ();

  ///
  void add_pilar_template ();

  ///
  void set_cube_texture (csTextureHandle* ct) { cube_txt = ct; }

  ///
  void set_pilar_texture (csTextureHandle* ct) { pilar_txt = ct; }

  ///
  void set_cube_room (csSector* s) { room = s; }

  ///
  void move_cubes (time_t elapsed_time);

  ///
  void updateScore();

  /// Is called when it transition mode instead of move_cubes().
  void handleTransition (time_t elapsed_time);

  /**
   * This is called during a transition, similar to move_cubes(),
   * which is called usually.
   */
  void lower (time_t elapsed_time);

  ///
  void move_camera ();

  ///
  csMatrix3 create_rotate_x (float angle);

  ///
  csMatrix3 create_rotate_y (float angle);

  ///
  csMatrix3 create_rotate_z (float angle);

  ///
  void start_rotation (BlRotType type);

  ///
  void start_horizontal_move (int dx, int dy);

  ///
  void start_shape (BlShapeType type, int x, int y, int z);

  ///
  void move_shape_internal (int dx, int dy, int dz);

  ///
  void rotate_shape_internal (const csMatrix3& rot);

  /// Return true if free.
  bool check_new_shape_location (int dx, int dy, int dz);

  /// Return true if free.
  bool check_new_shape_rotation (const csMatrix3& rot);

  ///
  void freeze_shape ();

  ///
  void dump_shape ();

  ///
  void init_game ();

  ///
  bool get_cube (int x, int y, int z)
  { return game_cube[x+ZONE_SAFETY][y+ZONE_SAFETY][z+ZONE_SAFETY]; }
  ///
  void set_cube (int x, int y, int z, bool v)
  { game_cube[x+ZONE_SAFETY][y+ZONE_SAFETY][z+ZONE_SAFETY] = v; }

  // Checks to see if a plane was formed.
  void checkForPlane();

  void removePlane(int z);
};

#endif // BLOCKS_H
