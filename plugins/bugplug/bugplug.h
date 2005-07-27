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

#include "csutil/cfgacc.h"
#include "csutil/parray.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/weakref.h"
#include "csutil/weakrefarr.h"
#include "iutil/comp.h"
#include "iutil/csinput.h"
#include "iutil/eventh.h"
#include "iutil/plugin.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "ivaria/bugplug.h"

struct iCamera;
struct iConsoleOutput;
struct iEngine;
struct iFile;
struct iFont;
struct iGraphics2D;
struct iGraphics3D;
struct iMaterialWrapper;
struct iMeshFactoryWrapper;
struct iMeshWrapper;
struct iObjectRegistry;
struct iObjectRegistry;
struct iPluginManager;
struct iRegion;
struct iSector;
struct iTextureManager;
struct iThingFactoryState;
struct iVFS;
struct iVirtualClock;
struct iVisibilityCuller;

class csMatrix3;
class csVector3;
class csVector2;
class csPlane3;
class csBox2;
class csBox3;
class csView;

struct csTriangle;

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
#define DEBUGCMD_MESHRAD	1023	// SHow RADIUS of selected mesh
#define DEBUGCMD_DEBUGGRAPH	1024	// Do a dump of the debug graph
#define DEBUGCMD_ENGINECMD	1025	// General engine DebugCommand() (arg)
#define DEBUGCMD_ENGINESTATE	1026	// Test engine state.
#define DEBUGCMD_VISCULVIEW	1027	// Call viscull->Dump(g3d)
#define DEBUGCMD_VISCULCMD	1028	// Call viscull->DebugCommand()
#define DEBUGCMD_DEBUGSECTOR	1029	// Toggle debug sector
#define DEBUGCMD_DS_FORWARD	1030	// Move forward in debug sector
#define DEBUGCMD_DS_BACKWARD	1031	// Move backward in debug sector
#define DEBUGCMD_DS_TURNLEFT	1032	// Rotate left in debug sector
#define DEBUGCMD_DS_TURNRIGHT	1033	// Rotate right in debug sector
#define DEBUGCMD_DS_UP		1034	// Move up in debug sector
#define DEBUGCMD_DS_DOWN	1035	// Move down in debug sector
#define DEBUGCMD_DS_LEFT	1036	// Move left in debug sector
#define DEBUGCMD_DS_RIGHT	1037	// Move right in debug sector
#define DEBUGCMD_DEBUGVIEW	1038	// Toggle debug view
#define DEBUGCMD_SCRSHOT	1039	// Screenshot
#define DEBUGCMD_FPS		1040	// Toggle fps (default on)
#define DEBUGCMD_HIDESELECTED	1041	// Remove selected obj from sectors.
#define DEBUGCMD_UNDOHIDE	1042	// Undo last hide.
#define DEBUGCMD_COUNTERRESET	1043	// Reset all counters.
#define DEBUGCMD_COUNTERREMOVE	1044	// Remove all counters.
#define DEBUGCMD_COUNTERFREEZE	1045	// Freeze all counters.
#define DEBUGCMD_SHADOWDEBUG	1046	// Toggle shadow debugging
#define DEBUGCMD_DEBUGCMD   	1047	// Send a debug command to a plugin
#define DEBUGCMD_MEMORYDUMP   	1048	// Memory dump
#define DEBUGCMD_UNPREPARE   	1049	// Unprepare all things
#define DEBUGCMD_COLORSECTORS  	1050	// Give all sectors a different color
#define DEBUGCMD_SWITCHCULLER  	1051	// Switch to culler
#define DEBUGCMD_SELECTMESH  	1052	// Select a mesh by name
#define DEBUGCMD_MESHCDMESH	1053	// Show CD polymesh of selected mesh
#define DEBUGCMD_MESHVISMESH	1054	// Show viscul polymesh of selected mesh
#define DEBUGCMD_MESHSHADMESH	1055	// Show shadow polymesh of selected mesh
#define DEBUGCMD_MESHBASEMESH	1056	// Show base polymesh of selected mesh
#define DEBUGCMD_ONESECTOR	1057	// Merge all in one sector
#define DEBUGCMD_MESH_XMIN	1058	// Move mesh
#define DEBUGCMD_MESH_XPLUS	1059	// Move mesh
#define DEBUGCMD_MESH_YMIN	1060	// Move mesh
#define DEBUGCMD_MESH_YPLUS	1061	// Move mesh
#define DEBUGCMD_MESH_ZMIN	1062	// Move mesh
#define DEBUGCMD_MESH_ZPLUS	1063	// Move mesh
#define DEBUGCMD_SAVEMAP	1064	// SaveMap
#define DEBUGCMD_LISTPLUGINS	1065	// List all loaded plugins
#define DEBUGCMD_PROFDUMP	1066	// Dump profile info (CS_PROFSTART).
#define DEBUGCMD_PROFRESET	1067	// Reset profile info (CS_PROFRESET).

// For showing of polygon meshes.
#define BUGPLUG_POLYMESH_NO	0
#define BUGPLUG_POLYMESH_CD	1
#define BUGPLUG_POLYMESH_VIS	2
#define BUGPLUG_POLYMESH_SHAD	3
#define BUGPLUG_POLYMESH_BASE	4

/**
 * For key mappings.
 */
struct csKeyMap
{
  csKeyMap* next, * prev;
  int key;
  bool shift, alt, ctrl;
  int cmd;	// One of DEBUGCMD_...
  char* args;
};

struct csCounterValue
{
  float total;
  int current;
};

/**
 * For counters.
 */
struct csCounter
{
  char* countername;
  bool is_enum;
  // Values: if is_enum == false only value[0] is used.
  csCounterValue values[10];

  ~csCounter ()
  {
    delete[] countername;
  }
};

//--------------------------------------------------------------------------

/**
 * Debugger plugin. Loading this plugin is sufficient to get debugging
 * functionality in your application.
 */
class csBugPlug : public iComponent
{
private:
  iObjectRegistry *object_reg;
  csRef<iEngine> Engine;
  csRef<iGraphics3D> G3D;
  csRef<iGraphics2D> G2D;
  csRef<iConsoleOutput> Conout;
  csRef<iVFS> VFS;
  csRef<iVirtualClock> vc;
  csRef<iFont> fnt;
  bool initialized;
  csConfigAccess config;
  /**
   * Keyboard input composer
   */
  csRef<iKeyComposer> keyComposer;

  //------------------------------------------------------------------
  // Specific debugging options.
  //------------------------------------------------------------------

  // All the counters.
  csPDelArray<csCounter> counters;
  // Frame counter for the counters.
  int counter_frames;
  // If true the counters are frozen but still displayed.
  bool counter_freeze;

  // Show polygon mesh for current selected mesh.
  int show_polymesh;

  // For fps
  bool do_fps;
  int fps_frame_count;
  int fps_tottime;
  float fps_cur;

  // For 'clear' command.
  bool do_clear;

  // Dump various structures.
  void Dump (iEngine* engine);
  void Dump (iSector* sector);
  void Dump (int indent, iMeshWrapper* mesh);
  void Dump (iMeshFactoryWrapper* meshfact);
  void Dump (iCamera* c);
  void Dump (iThingFactoryState* fact, int polyidx);
  void Dump (int indent, const csMatrix3& m, char const* name);
  void Dump (int indent, const csVector3& v, char const* name);
  void Dump (int indent, const csVector2& v, char const* name);
  void Dump (int indent, const csPlane3& p);
  void Dump (int indent, const csBox2& b);
  void Dump (int indent, const csBox3& b);

  void Report (int severity, const char* msg, ...);

  // Toggle a G3D boolean option.
  bool do_shadow_debug;
  csRef<iShader> standardShadowShader;
  csRef<iShader> debugShadowShader;
  void ToggleG3DState (G3D_RENDERSTATEOPTION op, const char* name);

  // The selected mesh.
  csWeakRefArray<iMeshWrapper> selected_meshes;
  void AddSelectedMesh (iMeshWrapper* m);
  void RemoveSelectedMesh (iMeshWrapper* m);
  bool HasSelectedMeshes () const { return selected_meshes.Length () > 0; }
  void MoveSelectedMeshes (const csVector3& offset);

  // Shadow!
  csShadow* shadow;

  // Spider!
  csSpider* spider;
  // If true then spider is hunting.
  bool spider_hunting;
  /*
   * Timeout. Every frame this value is decreased.
   * If it reaches 0 and no camera has been found we stop hunting.
   */
  int spider_timeout;
  // Command to execute when spider found a camera.
  int spider_command;
  // Arguments for spider command.
  char* spider_args;
  // Mouse x and y for the command (if a selection command).
  int mouse_x, mouse_y;
  // Send the Spider on a hunt.
  void UnleashSpider (int cmd, const char* args = 0);
  /*
   * The Spider has done its job. Send him back to his hiding place.
   * Also perform the Spider command on the camera we found (only if
   * camera != 0).
   */
  void HideSpider (iCamera* camera);

  /// MIME type of the screenshot image to save
  const char* captureMIME;
  /// image saver options for the screenshot file
  const char* captureOptions;
  /// format of the screenshot filename (e.g. "/tmp/cryst%03d.png")
  char* captureFormat;
  int captureFormatNumberMax;
  /**
   * Make a screenshot.
   */
  void CaptureScreen ();

  /// Current visibility culler we are monitoring.
  csWeakRef<iVisibilityCuller> visculler;

  /// The Debug Sector.
  struct
  {
    iSector* sector;
    csView* view;
    bool show;
    bool clear;
  } debug_sector;

  /// The Debug View.
  struct dbLine { int i1, i2; };
  struct
  {
    bool show;
    bool clear;
    int num_points;
    int max_points;
    csVector2* points;
    int num_lines;
    int max_lines;
    dbLine* lines;
    int num_boxes;
    int max_boxes;
    dbLine* boxes;
    iBugPlugRenderObject* object;
    int drag_point;	// Or -1 if not dragging a point.
  } debug_view;

  /// Set viscull view.
  void VisculView (iCamera* camera);
  /// Call viscull command.
  void VisculCmd (const char* cmd);
  /// Switch culler for current sector.
  void SwitchCuller (iSector* sector, const char* culler);
  /// Select the named mesh if possible.
  void SelectMesh (iSector* sector, const char* meshname);

  void ListLoadedPlugins ();

  //------------------------------------------------------------------

  csKeyMap* mappings;
  /// If true we will eat next key and process it.
  bool process_next_key;
  /// If true we will eat next mouse click and process it.
  bool process_next_mouse;

  /// If true we are in edit mode.
  bool edit_mode;
  /// The cursor position.
  size_t edit_cursor;
  /// String we are editing.
  csString edit_string;
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
  /// Open system.
  bool HandleSystemOpen (iEvent* event);
  /// Close system.
  bool HandleSystemClose (iEvent* event);

  /// Execute mouse button one.
  void MouseButton1 (iCamera* camera);
  /// Execute mouse button two.
  void MouseButton2 (iCamera* camera);
  /// Execute mouse button three.
  void MouseButton3 (iCamera* camera);

  /**
   * Given a command string, return a command code.
   * Optionally fill 'args' with arguments after the command.
   */
  int GetCommandCode (const char* cmd, char* args);
  /// Given a keyname, parse it and return key code + modifier status.
  int GetKeyCode (const char* keystring, bool& shift, bool& alt,
	bool& ctrl);

  /// Given a keycode, and shift/alt/ctrl status return a command code.
  int GetCommandCode (int key, bool shift, bool alt, bool ctrl,
  	char*& args);
  /// Add a new key binding for a command.
  void AddCommand (const char* keystring, const char* cmdstring);

  /// Read one line from a file.
  bool ReadLine (iFile* file, char* buf, int nbytes);
  /// Read a file and process all key bindings in it.
  void ReadKeyBindings (const char* filename);

  /// Setup this plugin.
  void SetupPlugin ();

  /// Enter edit mode.
  void EnterEditMode (int cmd, const char* msg, const char* def = 0);
  /// Process a command after finishing edit mode.
  void ExitEditMode ();

  /// Send a debug command to a plugin
  void DebugCmd (const char* cmd);

  void SaveMap ();
public:
  SCF_DECLARE_IBASE;

  csBugPlug (iBase *iParent);
  virtual ~csBugPlug ();
  ///
  virtual bool Initialize (iObjectRegistry *object_reg);
  /// This is set to receive the once per frame nothing  event
  bool HandleEvent (iEvent &event);

  void OneSector (iCamera* camera);

  iMaterialWrapper* FindColor (float r, float g, float b);
  void CleanDebugSector ();
  void SetupDebugSector ();
  void DebugSectorBox (const csBox3& box, float r, float g, float b,
  	const char* name = 0, iMeshObject* mesh = 0,
	uint mixmode = CS_FX_COPY);
  void DebugSectorTriangle (const csVector3& s1, const csVector3& s2,
  	const csVector3& s3, float r, float g, float b,
	uint mixmode = CS_FX_ADD);
  void DebugSectorMesh (
	csVector3* vertices, int vertex_count,
	csTriangle* triangles, int tri_count,
	float r, float g, float b, uint mixmode = CS_FX_COPY);
  void SwitchDebugSector (const csReversibleTransform& trans,
  	bool clear = true);
  bool CheckDebugSector () const { return debug_sector.show; }

  void CleanDebugView ();
  void SetupDebugView ();
  int DebugViewPoint (const csVector2& point);
  void DebugViewLine (int i1, int i2);
  void DebugViewBox (int i1, int i2);
  int DebugViewPointCount () const { return debug_view.num_points; }
  const csVector2& DebugViewGetPoint (int i) const
  {
    return debug_view.points[i];
  }
  int DebugViewLineCount () const { return debug_view.num_lines; }
  void DebugViewGetLine (int i, int& i1, int& i2) const
  {
    i1 = debug_view.lines[i].i1;
    i2 = debug_view.lines[i].i2;
  }
  int DebugViewBoxCount () const { return debug_view.num_boxes; }
  void DebugViewGetBox (int i, int& i1, int& i2) const
  {
    i1 = debug_view.boxes[i].i1;
    i2 = debug_view.boxes[i].i2;
  }
  void DebugViewRenderObject (iBugPlugRenderObject* obj);
  void DebugViewClearScreen (bool cs) { debug_view.clear = cs; }
  void SwitchDebugView (bool clear = true);
  bool CheckDebugView () const { return debug_view.show; }

  int FindCounter (const char* countername);
  void FullResetCounters ();
  void ShowCounters ();

  void AddCounter (const char* countername, int amount = 1);
  void AddCounterEnum (const char* countername, int enumval, int amount = 1);
  void ResetCounter (const char* countername, int value = 0);
  void RemoveCounter (const char* countername);

  struct BugPlug : public iBugPlug
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBugPlug);
    virtual void SetupDebugSector ()
    {
      scfParent->SetupDebugSector ();
    }
    virtual void DebugSectorBox (const csBox3& box, float r, float g, float b,
  	const char* name = 0, iMeshObject* mesh = 0,
	uint mixmode = CS_FX_COPY)
    {
      scfParent->DebugSectorBox (box, r, g, b, name, mesh, mixmode);
    }
    virtual void DebugSectorTriangle (const csVector3& s1, const csVector3& s2,
  	const csVector3& s3, float r, float g, float b,
	uint mixmode = CS_FX_ADD)
    {
      scfParent->DebugSectorTriangle (s1, s2, s3, r, g, b, mixmode);
    }
    virtual void DebugSectorMesh (
	csVector3* vertices, int vertex_count,
	csTriangle* triangles, int tri_count,
	float r, float g, float b, uint mixmode = CS_FX_COPY)
    {
      scfParent->DebugSectorMesh (vertices, vertex_count,
      	triangles, tri_count, r, g, b, mixmode);
    }
    virtual void SwitchDebugSector (const csReversibleTransform& trans,
    	bool clear = true)
    {
      scfParent->SwitchDebugSector (trans, clear);
    }
    virtual bool CheckDebugSector () const
    {
      return scfParent->CheckDebugSector ();
    }
    virtual void SetupDebugView ()
    {
      scfParent->SetupDebugView ();
    }
    virtual int DebugViewPoint (const csVector2& point)
    {
      return scfParent->DebugViewPoint (point);
    }
    virtual void DebugViewLine (int i1, int i2)
    {
      scfParent->DebugViewLine (i1, i2);
    }
    virtual void DebugViewBox (int i1, int i2)
    {
      scfParent->DebugViewBox (i1, i2);
    }
    virtual int DebugViewPointCount () const
    {
      return scfParent->DebugViewPointCount ();
    }
    virtual const csVector2& DebugViewGetPoint (int i) const
    {
      return scfParent->DebugViewGetPoint (i);
    }
    virtual int DebugViewLineCount () const
    {
      return scfParent->DebugViewLineCount ();
    }
    virtual void DebugViewGetLine (int i, int& i1, int& i2) const
    {
      scfParent->DebugViewGetLine (i, i1, i2);
    }
    virtual int DebugViewBoxCount () const
    {
      return scfParent->DebugViewBoxCount ();
    }
    virtual void DebugViewGetBox (int i, int& i1, int& i2) const
    {
      scfParent->DebugViewGetBox (i, i1, i2);
    }
    virtual void DebugViewRenderObject (iBugPlugRenderObject* obj)
    {
      scfParent->DebugViewRenderObject (obj);
    }
    virtual void DebugViewClearScreen (bool cs)
    {
      scfParent->DebugViewClearScreen (cs);
    }
    virtual void SwitchDebugView (bool clear = true)
    {
      scfParent->SwitchDebugView (clear);
    }
    virtual bool CheckDebugView () const
    {
      return scfParent->CheckDebugView ();
    }
    virtual void AddCounter (const char* countername, int amount = 1)
    {
      scfParent->AddCounter (countername, amount);
    }
    virtual void AddCounterEnum (const char* countername, int enumval,
  	int amount = 1)
    {
      scfParent->AddCounterEnum (countername, enumval, amount);
    }
    virtual void ResetCounter (const char* countername, int value = 0)
    {
      scfParent->ResetCounter (countername, value);
    }
    virtual void RemoveCounter (const char* countername)
    {
      scfParent->RemoveCounter (countername);
    }
  } scfiBugPlug;

  // This is not an embedded interface in order to avoid
  // a circular reference between this registered event handler
  // and the parent object.
  class EventHandler : public iEventHandler
  {
  private:
    csBugPlug* parent;
  public:
    EventHandler (csBugPlug* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    SCF_DECLARE_IBASE;
    virtual bool HandleEvent (iEvent& ev)
    {
      return parent->HandleEvent (ev);
    }
  } *scfiEventHandler;
};

#endif // __CS_BUGPLUG_H__
