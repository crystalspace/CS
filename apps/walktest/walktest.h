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

#ifndef MAIN_H
#define MAIN_H

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "cssys/common/sysdriv.h"

class Polygon3D;
class WalkTest;
class LanguageLayer;
class csView;
class InfiniteMaze;
class csSoundBuffer;
class csWorld;

///
struct csKeyMap
{
  csKeyMap* next, * prev;
  int key, shift, alt, ctrl;
  char* cmd;
};

///
class WalkTest : public SysSystemDriver
{
public:
  /// The world file to load
  static char world_file[100];
  /// A script to execute at startup.
  char* auto_script;

  /**
   * If this flag is true we move in 3D (old system). Otherwise we move more like
   * common 3D games do.
   */
  static bool move_3d;

  /// The world.
  csWorld* world;

  /// For scripting.
  LanguageLayer* layer;

  /// The view on the world.
  csView* view;

  /// Our infinite maze object if used.
  InfiniteMaze* infinite_maze;

  /// Some sounds.
  csSoundBuffer* wMissile_boom;
  csSoundBuffer* wMissile_whoosh;

  /// Some flags.
  bool do_fps;
  bool do_stats;
  bool do_clear;
  bool do_show_coord;
  bool busy_perf_test;
  bool do_show_z;
  bool do_infinite;
  bool do_cd;
  bool do_freelook;

  /// Timing.
  float timeFPS;

public:
  ///
  WalkTest ();

  ///
  ~WalkTest ();

  ///
  virtual void NextFrame (long elapsed_time, long current_time);
  ///
  void PrepareFrame (long elapsed_time, long current_time);
  ///
  void DrawFrame (long elapsed_time, long current_time);

  /// Override ParseArg to handle additional arguments
  virtual bool ParseArg (int argc, char* argv[], int& i);
  /// Override Help to show additional arguments help
  virtual void Help ();
  /// Override DemoWrite for nice demo messaging
  virtual void DemoWrite (const char* msg);

private:
  ///
  static void handle_key_forward (float speed, bool shift, bool alt, bool ctrl);
  ///
  static void handle_key_backwards (float speed, bool shift, bool alt, bool ctrl);
  ///
  static void handle_key_left (float speed, bool shift, bool alt, bool ctrl);
  ///
  static void handle_key_right (float speed, bool shift, bool alt, bool ctrl);
  ///
  static void handle_key_pgup (float, bool shift, bool alt, bool ctrl);
  ///
  static void handle_key_pgdn (float, bool shift, bool alt, bool ctrl);
  ///
  static void eatkeypress (int key, bool shift, bool alt, bool ctrl);
};

extern csVector2 coord_check_vector;
extern WalkTest* Sys;

#define FRAME_WIDTH Sys->FrameWidth
#define FRAME_HEIGHT Sys->FrameHeight

extern void perf_test ();
extern void CaptureScreen ();
extern void free_keymap ();

#endif // MAIN_H
