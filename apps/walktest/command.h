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

#ifndef COMMAND_H
#define COMMAND_H

#include "ivideo/igraph3d.h"
#include "isys/ivfs.h"

class csSystemDriver;
class csEngine;
class csCamera;
struct iConsole;
struct iSystem;

/**
 * This class represents a command that can be send thru the console
 * or by pressing a key from within the engine.
 * It is a static class which contains only static data.
 */
class csCommandProcessor
{
private:
  /// The engine that this command interpreter is using.
  static csEngine* engine;
  /// The corresponding camera object for this command interpreter.
  static csCamera* camera;
  /// The iGraphics3D interface to use.
  static iGraphics3D* g3d;
  /// The iSystem interface to use.
  static iSystem* system;
  /// The console.
  static iConsole* console;

  /**
   * If this variable is non-NULL there is a running script from
   * which commands are read and executed (one command every frame).
   */
  static iFile* script;

public:
  /// Call this first.
  static void Initialize (csEngine* engine, csCamera* camera, iGraphics3D* g3d, iConsole* console, iSystem* system);

  /// This method can be set as a console input callback
  static void perform_callback (void *, const char *command);

  /// Perform the command and return true if it was a valid command.
  static bool perform (const char* cmd, const char* arg = NULL);

  /**
   * Perform the command line (split in command and args),
   * perform the command and return true if it was a valid command.
   */
  static bool perform_line (const char* line);

  /// Virtual override of csSimpleCommand.
  virtual bool PerformLine (const char* line);

  /// Start a script of commands from a file.
  static bool start_script (const char* scr);

  /**
   * Check if a script is running and if so, return one command from the
   * script. This routine will return false if there is no script, or
   * if the last command from the script has been read. The script will
   * be closed automatically in the last case.
   */
  static bool get_script_line (char* buf, int max_size);

  /// Change boolean variable value
  static void change_boolean (const char* arg, bool* value, const char* what);
  /// Change boolean variable value specifically for G3D.
  static bool change_boolean_gfx3d (const char* arg, G3D_RENDERSTATEOPTION op, const char* what);
  /// Change value to one of listed in choices[]
  static void change_choice (const char* arg, int* value, const char* what, const char* const* choices, int num);
  /// Change float variable value
  static bool change_float (const char* arg, float* value, const char* what, float min, float max);
  /// Change int variable value
  static bool change_int (const char* arg, int* value, const char* what, int min, int max);
  /// Change long variable value
  static bool change_long (const char* arg, long* value, const char* what, long min, long max);

  /**
   * Additional command handler. If not NULL, this handler will be called
   * BEFORE internal handler, giving your handler a chance to override
   * default behaviour. Should return TRUE if command has been recognized.
   */
  typedef bool (*CmdHandler) (const char* cmd, const char* arg);
  static CmdHandler ExtraHandler;
};

#endif
