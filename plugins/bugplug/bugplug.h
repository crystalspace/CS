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
struct iCamera;
struct iPolygon3D;
struct iObjectRegistry;
struct iPluginManager;

class csMatrix3;
class csVector3;
class csVector2;
class csPlane3;
class csBox2;
class csBox3;

class csSpider;
class csShadow;

//--------------------------------------------------------------------------
// Command codes.
//--------------------------------------------------------------------------
#define DEBUGCMD_UNKNOWN	-1	// Error, unknown command

// Commands controlling the plugin itself.
#define DEBUGCMD_DEBUGENTER	0	// Next key will be ours
#define DEBUGCMD_MOUSEENTER	1	// Next mouse click will be ours
#define DEBUGCMD_QUIT		2	// Unload
#define DEBUGCMD_STATUS		3	// Request status
#define DEBUGCMD_HELP  		4	// Show help
#define DEBUGCMD_MOUSE1 	5	// Mouse button 1
#define DEBUGCMD_MOUSE2 	6	// Mouse button 2
#define DEBUGCMD_MOUSE3 	7	// Mouse button 3

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
#define DEBUGCMD_GAMMA		1016	// Set gamma
#define DEBUGCMD_DBLBUFF	1017	// Set double buffering (G2D)
#define DEBUGCMD_DUMPCAM	1018	// Dump the camera
#define DEBUGCMD_FOV		1019	// Set fov
#define DEBUGCMD_FOVANGLE	1020	// Set fov in angles
#define DEBUGCMD_TERRVIS	1021	// Enable/disable terrain visibility
#define DEBUGCMD_MESHBBOX	1022	// Show BBOX of selected mesh
#define DEBUGCMD_MESHRAD	1023	// How RADIUS of selected mesh

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
class csBugPlug : public iPlugin
{
private:
  iSystem *System;
  iObjectRegistry* object_reg;
  iPluginManager* plugin_mgr;
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

  /// Dump various structures.
  void Dump (iEngine* engine);
  void Dump (iSector* sector);
  void Dump (iMeshWrapper* mesh);
  void Dump (iMeshFactoryWrapper* meshfact);
  void Dump (iCamera* c);
  void Dump (iPolygon3D* p);
  void Dump (int indent, const csMatrix3& m, char const* name);
  void Dump (int indent, const csVector3& v, char const* name);
  void Dump (int indent, const csVector2& v, char const* name);
  void Dump (int indent, const csPlane3& p);
  void Dump (int indent, const csBox2& b);
  void Dump (int indent, const csBox3& b);

  /// Toggle a G3D boolean option.
  void ToggleG3DState (G3D_RENDERSTATEOPTION op, const char* name);

  /// The selected mesh.
  iMeshWrapper* selected_mesh;

  /// Shadow!
  csShadow* shadow;

  /// Spider!
  csSpider* spider;
  /// If true then spider is hunting.
  bool spider_hunting;
  /**
   * Timeout. Every frame this value is decreased.
   * If it reaches 0 and no camera has been found we stop hunting.
   */
  int spider_timeout;
  /// Command to execute when spider found a camera.
  int spider_command;
  /// Mouse x and y for the command (if a selection command).
  int mouse_x, mouse_y;
  /// Send the Spider on a hunt.
  void UnleashSpider (int cmd);
  /**
   * The Spider has done its job. Send him back to his hiding place.
   * Also perform the Spider command on the camera we found (only if
   * camera != NULL).
   */
  void HideSpider (iCamera* camera);

  //------------------------------------------------------------------

  csKeyMap* mappings;
  /// If true we will eat next key and process it.
  bool process_next_key;
  /// If true we will eat next mouse click and process it.
  bool process_next_mouse;

  /// If true we are in edit mode.
  bool edit_mode;
  /// The cursor position.
  int edit_cursor;
  /// String we are editing.
  char edit_string[81];
  /// Message string:
  char msg_string[81];
  /// Command to perform after finishing edit mode.
  int edit_command;

  /// Eat a key and process it.
  bool EatKey (iEvent& event);
  /// Eat a mouse click and process it.
  bool EatMouse (iEvent& event);
  /// Start of frame.
  bool HandleStartFrame (iEvent& event);
  /// End of frame.
  bool HandleEndFrame (iEvent& event);

  /// Execute mouse button one.
  void MouseButton1 (iCamera* camera);
  /// Execute mouse button two.
  void MouseButton2 (iCamera* camera);
  /// Execute mouse button three.
  void MouseButton3 (iCamera* camera);

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

  /// Enter edit mode.
  void EnterEditMode (int cmd, const char* msg, const char* def = NULL);
  /// Process a command after finishing edit mode.
  void ExitEditMode ();

public:
  SCF_DECLARE_IBASE;

  csBugPlug (iBase *iParent);
  virtual ~csBugPlug ();
  ///
  virtual bool Initialize (iSystem *system);
  /// This is set to receive the once per frame nothing  event
  virtual bool HandleEvent (iEvent &event);
};

#endif // __CS_BUGPLUG_H__
