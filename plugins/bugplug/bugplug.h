/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_BUGPLUG_H__
#define __CS_BUGPLUG_H__

#include "isys/plugin.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/csvector.h"
#include "ivideo/graph3d.h"

struct iSystem;
struct iEngine;
struct iGraphics3D;
struct iGraphics2D;
struct iTextureManager;
struct iConsoleOutput;
struct iVFS;
struct iFile;
struct iSector;
struct iMeshWrapper;
struct iMeshFactoryWrapper;
class csMatrix3;
class csVector3;
class csVector2;
class csPlane3;
class csBox2;
class csBox3;

//--------------------------------------------------------------------------
// Command codes.
//--------------------------------------------------------------------------
#define DEBUGCMD_UNKNOWN	-1	// Error, unknown command

// Commands controlling the plugin itself.
#define DEBUGCMD_DEBUGENTER	0	// Next key will be ours
#define DEBUGCMD_QUIT		1	// Unload
#define DEBUGCMD_STATUS		2	// Request status
#define DEBUGCMD_HELP  		3	// Show help

// Commands for debugging.
#define DEBUGCMD_DUMPENG	1000	// Dump structure of world
#define DEBUGCMD_DUMPSEC	1001	// Dump structure of current sector
#define DEBUGCMD_EDGES		1002	// Enable edge drawing
#define DEBUGCMD_CLEAR		1003	// Clear screen every frame
#define DEBUGCMD_CACHEDUMP	1004	// Dump texture cache
#define DEBUGCMD_CACHECLEAR	1005	// Clear texture cache
#define DEBUGCMD_TEXTURE	1006	// Enable texture mapping
#define DEBUGCMD_BILINEAR	1007	// Enable bi-linear filtering
#define DEBUGCMD_TRILINEAR	1008	// Enable tri-linear filtering
#define DEBUGCMD_LIGHTING	1009	// Enable lighting
#define DEBUGCMD_GOURAUD	1010	// Enable gouraud
#define DEBUGCMD_ILACE		1011	// Enable interlacing
#define DEBUGCMD_MMX		1012	// Enable MMX
#define DEBUGCMD_TRANSP		1013	// Enable transparent mode
#define DEBUGCMD_MIPMAP		1014	// Set mipmapping mode
#define DEBUGCMD_INTER		1015	// Set interpolation mode

/**
 * For key mappings.
 */
struct csKeyMap
{
  csKeyMap* next, * prev;
  int key;
  bool shift, alt, ctrl;
  int cmd;	// One of DEBUGCMD_...
};

//--------------------------------------------------------------------------

/**
 * Debugger plugin. Loading this plugin is sufficient to get debugging
 * functionality in your application.
 */
class csBugPlug : iPlugIn
{
private:
  iSystem *System;
  iEngine *Engine;
  iGraphics3D* G3D;
  iGraphics2D* G2D;
  iConsoleOutput* Conout;
  iVFS* VFS;
  bool initialized;

  //------------------------------------------------------------------
  // Specific debugging options.
  //------------------------------------------------------------------

  /// For 'clear' command.
  bool do_clear;
  /// Dump the contents of the engine.
  void Dump (iEngine* engine);
  /// Dump the contents of a sector.
  void Dump (iSector* sector);
  /// Dump the contents of a mesh object.
  void Dump (iMeshWrapper* mesh);
  /// Dump the contents of a mesh factory.
  void Dump (iMeshFactoryWrapper* meshfact);
  /// Dump various structures.
  void Dump (int indent, const csMatrix3& m, char const* name);
  void Dump (int indent, const csVector3& v, char const* name);
  void Dump (int indent, const csVector2& v, char const* name);
  void Dump (int indent, const csPlane3& p);
  void Dump (int indent, const csBox2& b);
  void Dump (int indent, const csBox3& b);

  /// Toggle a G3D boolean option.
  void ToggleG3DState (G3D_RENDERSTATEOPTION op, const char* name);

  //------------------------------------------------------------------

  csKeyMap* mappings;
  /// If true we will eat next key and process it.
  bool process_next_key;

  /// Eat a key and process it.
  bool EatKey (iEvent& event);
  /// Start of frame.
  bool HandleStartFrame (iEvent& event);
  /// End of frame.
  bool HandleEndFrame (iEvent& event);

  /// Given a command string, return a command code.
  int GetCommandCode (const char* cmd);
  /// Given a keyname, parse it and return key code + modifier status.
  int GetKeyCode (const char* keystring, bool& shift, bool& alt,
	bool& ctrl);

  /// Given a keycode, and shift/alt/ctrl status return a command code.
  int GetCommandCode (int key, bool shift, bool alt, bool ctrl);
  /// Add a new key binding for a command.
  void AddCommand (const char* keystring, const char* cmdstring);

  /// Read one line from a file.
  bool ReadLine (iFile* file, char* buf, int nbytes);
  /// Read a file and process all key bindings in it.
  void ReadKeyBindings (const char* filename);

  /// Setup this plugin.
  void SetupPlugin ();

public:
  DECLARE_IBASE;

  csBugPlug (iBase *iParent);
  virtual ~csBugPlug ();
  ///
  virtual bool Initialize (iSystem *system);
  /// This is set to receive the once per frame nothing  event
  virtual bool HandleEvent (iEvent &event);
};

#endif // __CS_BUGPLUG_H__
