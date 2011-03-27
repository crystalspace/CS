/*
    Copyright (C) 2001-2006 by Jorrit Tyberghein

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

#include "cstool/numberedfilenamehelper.h"
#include "csutil/cfgacc.h"
#include "csutil/parray.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/util.h"
#include "csutil/weakref.h"
#include "csutil/weakrefarr.h"
#include "iengine/engine.h"
#include "iutil/comp.h"
#include "iutil/csinput.h"
#include "iutil/eventh.h"
#include "iutil/plugin.h"
#include "iutil/strset.h"
#include "ivaria/bugplug.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

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
struct iSector;
struct iTextureManager;
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

class csShadow;

CS_PLUGIN_NAMESPACE_BEGIN(BugPlug)
{

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
#define DEBUGCMD_CACHEDUMP	1004	// Dump texture cache
#define DEBUGCMD_CACHECLEAR	1005	// Clear texture cache
#define DEBUGCMD_GAMMA		1016	// Set gamma
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
//#define DEBUGCMD_UNPREPARE   	1049	// Unprepare all things
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
#define DEBUGCMD_PROFTOGGLELOG	1066	// Start/stop profiler logging
#define DEBUGCMD_PROFAUTORESET	1067	// Reset profiler automagically at end of every frame
#define DEBUGCMD_UBERSCREENSHOT 1068    // Create an "uberscreenshot"
#define DEBUGCMD_MESHNORM       1069    // Draw normals of selected mesh
#define DEBUGCMD_TOGGLEFPSTIME  1070    // Toggle between fps and frame time display
#define DEBUGCMD_MESHSKEL       1080    // Draw skeleton of selected mesh
#define DEBUGCMD_PRINTPORTALS   1090    // Print portal info for the current sector
#define DEBUGCMD_PRINTPOSITION  1091    // Print current camera position in CS format

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

class csCameraCatcher : public scfImplementation1<csCameraCatcher,
	iEngineFrameCallback>
{
public:
  iCamera* camera;

public:
  csCameraCatcher () : scfImplementationType (this) { camera = 0; }
  virtual ~csCameraCatcher () { }
  virtual void StartFrame (iEngine* engine, iRenderView* rview)
  {
    camera = rview->GetCamera ();
  }
};

//--------------------------------------------------------------------------

/**
 * Debugger plugin. Loading this plugin is sufficient to get debugging
 * functionality in your application.
 */
class csBugPlug : public scfImplementation3<csBugPlug,
  iBugPlug, iComponent, iEventHandler>
{
private:
  iObjectRegistry *object_reg;
  csRef<iEngine> Engine;
  csRef<iEventHandler> weakEventHandler;
  csRef<iGraphics3D> G3D;
  csRef<iGraphics2D> G2D;
  csRef<iConsoleOutput> Conout;
  csRef<iVFS> VFS;
  csRef<iVirtualClock> vc;
  csRef<iFont> fnt;
  csRef<iStringSet> stringSet;
  csRef<iShaderVarStringSet> stringSetSvName;
  csRef<iStandardReporterListener> stdrep;
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

  // For profiling
  bool do_profiler_reset;
  bool do_profiler_log;

  // Dump various structures.
  void Dump (iEngine* engine);
  void Dump (iSector* sector);
  void Dump (int indent, iMeshWrapper* mesh);
  void Dump (iMeshFactoryWrapper* meshfact);
  void Dump (iCamera* c);
  void Dump (int indent, const csMatrix3& m, char const* name);
  void Dump (int indent, const csVector3& v, char const* name);
  void Dump (int indent, const csVector2& v, char const* name);
  void Dump (int indent, const csPlane3& p);
  void Dump (int indent, const csBox2& b);
  void Dump (int indent, const csBox3& b);

  void Report (int severity, const char* msg, ...);

  bool do_shadow_debug;
  csRef<iShader> standardShadowShader;
  csRef<iShader> debugShadowShader;
  // Report on the result of a G3D state toggling
  void ReportG3DState (bool prevState, bool state, const char* name);

  // The selected mesh.
  csWeakRefArray<iMeshWrapper> selected_meshes;
  void AddSelectedMesh (iMeshWrapper* m);
  void RemoveSelectedMesh (iMeshWrapper* m);
  bool HasSelectedMeshes () const { return selected_meshes.GetSize () > 0; }
  void MoveSelectedMeshes (const csVector3& offset);  

  // Shadow!
  csShadow* shadow;

  // The camera catcher.
  csRef<csCameraCatcher> catcher;
  // Mouse x and y for the command (if a selection command).
  int mouse_x, mouse_y;

  /// MIME type of the screenshot image to save
  const char* captureMIME;
  /// image saver options for the screenshot file
  const char* captureOptions;
  /// Helper for screenshot filename (e.g. "/tmp/cryst%03d.png")
  CS::NumberedFilenameHelper captureFormat;
  
  /**
   * Make a screenshot.
   */
  void CaptureScreen ();
  /**
   * Make an uberscreenshot.
   * \sa CS::UberScreenshotMaker
   */
  void CaptureUberScreen (uint w, uint h);

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

  /// Delayed command (this command will be delayed one frame).
  int delay_command;

  /// Eat a key and process it.
  bool EatKey (iEvent& event);
  /// Eat a mouse click and process it.
  bool EatMouse (iEvent& event);
  /// Start of frame.
  bool HandleStartFrame (iEvent& event);
  /// End of frame.
  bool HandleFrame (iEvent& event);
  /// Open system.
  bool HandleSystemOpen (iEvent* event);
  /// Close system.
  bool HandleSystemClose (iEvent* event);

  /// Execute right mouse button.
  void MouseButtonRight (iCamera* camera);
  /// Execute left mouse button.
  void MouseButtonLeft (iCamera* camera);

  /// Execute a debug command.
  bool ExecCommand (int cmd, const csString& args);

  /**
   * Given a command string, return a command code.
   * Optionally fill 'args' with arguments after the command.
   */
  int GetCommandCode (const char* cmd, csString& args);
  /// Given a keyname, parse it and return key code + modifier status.
  int GetKeyCode (const char* keystring, bool& shift, bool& alt,
	bool& ctrl);

  /// Given a keycode, and shift/alt/ctrl status return a command code.
  int GetCommandCode (int key, bool shift, bool alt, bool ctrl,
  	csString& args);
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

  bool ExecCommand (const char* command);

  CS_EVENTHANDLER_NAMES("crystalspace.bugplug")
  virtual const csHandlerID * GenericPrec(
    csRef<iEventHandlerRegistry>&, 
    csRef<iEventNameRegistry>&,
    csEventID) const;
  virtual const csHandlerID * GenericSucc(
    csRef<iEventHandlerRegistry>&, 
    csRef<iEventNameRegistry>&,
    csEventID) const;
  CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS

  CS_DECLARE_EVENT_SHORTCUTS;

  private:
    bool display_time;

    /**
    * Embedded iEventHandler interface that handles frame events in the
    * logic phase.
    */
    class LogicEventHandler : 
      public scfImplementation1<LogicEventHandler, 
      iEventHandler>
    {
    private:
      csWeakRef<csBugPlug> parent;
    public:
      LogicEventHandler (csBugPlug* parent) :
          scfImplementationType (this), parent (parent) { }
      virtual ~LogicEventHandler () { }
      virtual bool HandleEvent (iEvent& ev)
      {
        if (parent && (ev.Name == parent->Frame))
        {      
          return parent->HandleStartFrame (ev);
        }

        return false;
      }
      CS_EVENTHANDLER_PHASE_LOGIC("crystalspace.bugplug.frame.logic")
    };
    csRef<LogicEventHandler> logicEventHandler;
};

}
CS_PLUGIN_NAMESPACE_END(BugPlug)

#endif // __CS_BUGPLUG_H__
