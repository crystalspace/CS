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
#include "cssysdef.h"
#include "csqint.h"
#include "csqsqrt.h"

#include <string.h>
#include <ctype.h>

#define CS_DEPRECATION_SUPPRESS_HACK
#include "csgeom/trimesh.h"
#undef CS_DEPRECATION_SUPPRESS_HACK

#include "csgeom/box.h"
#include "csgeom/plane3.h"
#include "csgeom/tri.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/math3d.h"
#include "csgfx/trianglestream.h"
#include "cstool/collider.h"
#include "cstool/csview.h"
#include "cstool/enginetools.h"
#include "cstool/rbuflock.h"
#include "cstool/uberscreenshot.h"
#include "csutil/cscolor.h"
#include "csutil/csuctransform.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"
#include "csutil/eventhandlers.h"
#include "csutil/flags.h"
#include "csutil/regexp.h"
#include "csutil/scanstr.h"
#include "csutil/set.h"
#include "csutil/scf.h"
#include "csutil/snprintf.h"
#include "csutil/sysfunc.h"

#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/portalcontainer.h"
#include "iengine/portal.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iengine/viscull.h"
#include "iengine/scenenode.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "imap/saver.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "imesh/animesh.h"
#include "imesh/skeleton2.h"
#include "imesh/skeleton2anim.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "iutil/dbghelp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/memdebug.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/string.h"
#include "iutil/vfs.h"
#include "iutil/virtclk.h"
#include "ivaria/collider.h"
#include "ivaria/conout.h"
#include "ivaria/reporter.h"
#include "ivaria/profile.h"
#include "ivaria/stdrep.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"
#include "ivideo/txtmgr.h"

#include "bugplug.h"
#include "shadow.h"




CS_PLUGIN_NAMESPACE_BEGIN(BugPlug)
{

SCF_IMPLEMENT_FACTORY (csBugPlug)

CS_DECLARE_PROFILER

void csBugPlug::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep = csQueryRegistry<iReporter> (object_reg);
  if (rep)
  {
    bool old_stdout = false;
    if (stdrep && severity == CS_REPORTER_SEVERITY_DEBUG)
    {
      old_stdout = stdrep->IsStandardOutput (severity);
      stdrep->SetStandardOutput (severity, true);
    }
    rep->ReportV (severity, "crystalspace.bugplug", msg, arg);
    if (stdrep && severity == CS_REPORTER_SEVERITY_DEBUG)
      stdrep->SetStandardOutput (severity, old_stdout);
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

csBugPlug::csBugPlug (iBase *iParent)
  : scfImplementationType (this, iParent)
{  
  object_reg = 0;
  mappings = 0;
  visculler = 0;
  process_next_key = false;
  process_next_mouse = false;
  edit_mode = false;
  initialized = false;
  catcher.AttachNew (new csCameraCatcher ());
  shadow = new csShadow ();
  weakEventHandler = 0;

  do_fps = true;
  display_time = false;
  fps_frame_count = 0;
  fps_tottime = 0;
  fps_cur = -1;
  counter_frames = 0;
  counter_freeze = false;
  show_polymesh = BUGPLUG_POLYMESH_NO;

  debug_sector.sector = 0;
  debug_sector.view = 0;
  debug_sector.show = false;
  debug_sector.clear = true;

  debug_view.show = false;
  debug_view.clear = true;
  debug_view.num_points = 0;
  debug_view.max_points = 0;
  debug_view.points = 0;
  debug_view.num_lines = 0;
  debug_view.max_lines = 0;
  debug_view.lines = 0;
  debug_view.num_boxes = 0;
  debug_view.max_boxes = 0;
  debug_view.boxes = 0;
  debug_view.object = 0;
  debug_view.drag_point = -1;

  do_shadow_debug = false;
  delay_command = DEBUGCMD_UNKNOWN;

  do_profiler_reset = false;
  do_profiler_log = false;
}

csBugPlug::~csBugPlug ()
{
  CleanDebugSector ();
  CleanDebugView ();

  while (mappings)
  {
    csKeyMap* n = mappings->next;
    delete[] mappings->args;
    delete mappings;
    mappings = n;
  }
  if (weakEventHandler)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q)
      RemoveWeakListener (q, weakEventHandler);
  }

  if (logicEventHandler)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q)
      q->RemoveListener (logicEventHandler);
  }

  delete shadow;

  if (do_profiler_log)
  {
    CS_PROFILER_STOP_LOGGING();
  }
}

bool csBugPlug::Initialize (iObjectRegistry *object_reg)
{
  csBugPlug::object_reg = object_reg;

  csRef<iKeyboardDriver> currentKbd = 
    csQueryRegistry<iKeyboardDriver> (object_reg);
  if (currentKbd == 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    return false;
  }
  keyComposer = currentKbd->CreateKeyComposer ();

  CS_INITIALIZE_EVENT_SHORTCUTS (object_reg);

  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
  if (q != 0)
  {
    csEventID esub[] = { 
      Frame,
      KeyboardEvent,
      MouseEvent,
      SystemOpen,
      SystemClose,
      CS_EVENTLIST_END 
    };
    RegisterWeakListener (q, this, esub, weakEventHandler);
  }

  if (!logicEventHandler)
  {
    logicEventHandler.AttachNew (new LogicEventHandler (this));
  }
  if (q != 0)
  {
    csEventID events[2] = { Frame, CS_EVENTLIST_END };
    q->RegisterListener (logicEventHandler, events);
  }

  stringSet = csQueryRegistryTagInterface<iStringSet> (object_reg,
    "crystalspace.shared.stringset");
  stringSetSvName = csQueryRegistryTagInterface<iShaderVarStringSet> (object_reg,
    "crystalspace.shader.variablenameset");
  return true;
}

void csBugPlug::SetupPlugin ()
{
  if (initialized) return;

  if (!Engine)
  {
    Engine = csQueryRegistry<iEngine> (object_reg);
    if (Engine)
      Engine->AddEngineFrameCallback (catcher);
  }

  if (!G3D) G3D = csQueryRegistry<iGraphics3D> (object_reg);

  if (!G3D)
  {
    initialized = true;
    Report (CS_REPORTER_SEVERITY_ERROR, "No G3D!");
    return;
  }

  if (!G2D)
    G2D = G3D->GetDriver2D ();
  if (!G2D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No G2D!");
    return;
  }
  iFontServer* fntsvr = G2D->GetFontServer ();
  if (fntsvr)
  {
    fnt = fntsvr->LoadFont (CSFONT_COURIER);
    CS_ASSERT (fnt != 0);    
  }

  if (!VFS) VFS = csQueryRegistry<iVFS> (object_reg);
  if (!VFS)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No VFS!");
    return;
  }

  if (!vc) vc = csQueryRegistry<iVirtualClock> (object_reg);
  if (!vc)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No virtual clock!");
    return;
  }

  if (!Conout) Conout = csQueryRegistry<iConsoleOutput> (object_reg);

  if (!stdrep) stdrep = csQueryRegistry<iStandardReporterListener> (
      object_reg);

  config.AddConfig (object_reg, "/config/bugplug.cfg");

  ReadKeyBindings (config->GetStr ("Bugplug.Keybindings", 
    "/config/bugplug.key"));

  captureFormat.SetMask (config->GetStr ("Bugplug.Capture.FilenameFormat",
    "/this/cryst000.png"));
  captureMIME = config->GetStr ("Bugplug.Capture.Image.MIME",
    "image/png");
  captureOptions = config->GetStr ("Bugplug.Capture.Image.Options");
  
  const char* framespeed = config->GetStr ("Bugplug.ShowFrameSpeed", "fps");
  if (framespeed)
  {
    if (strcmp (framespeed, "fps") == 0)
    {
      do_fps = true;
      display_time = false;
    }
    else if (strcmp (framespeed, "time") == 0)
    {
      do_fps = true;
      display_time = true;
    }
    else if (strcmp (framespeed, "off") == 0)
    {
      do_fps = false;
      display_time = false;
    }
    else
    {
      Report (CS_REPORTER_SEVERITY_WARNING,
        "Invalid value '%s' for frame speed display", framespeed);
    }
  }

  initialized = true;

  Report (CS_REPORTER_SEVERITY_DEBUG, "BugPlug loaded...");
}

void csBugPlug::SwitchCuller (iSector* sector, const char* culler)
{
  Report (CS_REPORTER_SEVERITY_DEBUG,
      "Switching to visibility culler '%s'.", culler);
  sector->SetVisibilityCullerPlugin (culler);
}

void csBugPlug::SelectMesh (iSector* sector, const char* meshname)
{
  iMeshList* ml = sector->GetMeshes ();

  selected_meshes.Empty ();
  csRegExpMatcher matcher (meshname);
  int i;
  int cnt = 0;
  for (i = 0 ; i < ml->GetCount () ; i++)
  {
    iMeshWrapper* mesh = ml->Get (i);
    if (csrxNoError == matcher.Match (mesh->QueryObject ()->GetName ()))
    {
      cnt++;
      AddSelectedMesh (mesh);
    }
  }
  if (cnt > 0)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
        "Selecting %d mesh(es).", cnt);
    bool bbox, rad, norm, skel;
    shadow->GetShowOptions (bbox, rad, norm, skel);

    if (bbox || rad || norm || skel || show_polymesh != BUGPLUG_POLYMESH_NO)
      shadow->AddToEngine (Engine);
    else
      shadow->RemoveFromEngine (Engine);
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
      "Couldn't find matching meshes for pattern '%s'.", meshname);
  }
}

void csBugPlug::MoveSelectedMeshes (const csVector3& offset)
{
  size_t i;
  size_t count = selected_meshes.GetSize ();
  for (i = 0 ; i < count ; i++)
  {
    // Assign selected_mesh[i] to temporary variable to avoid an
    // internal MSVC6 error. Luca
    iMeshWrapper* mesh = selected_meshes[i];
    mesh->GetMovable ()->MovePosition (offset);
    mesh->GetMovable ()->UpdateMove ();
  }
}

void csBugPlug::AddSelectedMesh (iMeshWrapper* m)
{
  size_t i;
  size_t count = selected_meshes.GetSize ();
  for (i = 0 ; i < count ; i++)
  {
    // Assign selected_mesh[i] to temporary variable to avoid an
    // internal MSVC6 error. Luca
    iMeshWrapper* mesh = selected_meshes[i];
    if (mesh == m) 
      return;
  }
  selected_meshes.Push (m);
}

void csBugPlug::RemoveSelectedMesh (iMeshWrapper* m)
{
  size_t i;
  size_t count = selected_meshes.GetSize ();
  for (i = 0 ; i < count ; i++)
  {
    // Assign selected_mesh[i] to temporary variable to avoid an
    // internal MSVC6 error. Luca
    iMeshWrapper* mesh = selected_meshes[i];
    if (mesh == m)
    {
      selected_meshes.DeleteIndex (i);
      return;
    }
  }
}

void csBugPlug::VisculCmd (const char* cmd)
{
  if (!visculler)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
      "Bugplug is currently not tracking a visibility culler!");
    return;
  }
  csRef<iDebugHelper> dbghelp (scfQueryInterface<iDebugHelper> (visculler));
  if (!dbghelp)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
      "This visibility culler does not support iDebugHelper!");
    return;
  }
  if (dbghelp->DebugCommand (cmd))
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
      "Viscul command '%s' performed.", cmd);
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
      "Viscul command '%s' not supported!", cmd);
  }
}

void csBugPlug::VisculView (iCamera* camera)
{
  if (visculler)
  {
    visculler = 0;
    Report (CS_REPORTER_SEVERITY_DEBUG,
      "Disabled visculler graphical dumping");
    return;
  }

  // If we are not tracking a visculler yet we try to find one in current
  // sector.
  iSector* sector = camera->GetSector ();
  visculler = sector->GetVisibilityCuller ();
  if (!visculler)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
      "Bugplug found no visibility culler in this sector!");
    return;
  }
  Report (CS_REPORTER_SEVERITY_DEBUG,
      "Bugplug is now tracking a visibility culler");
}

void csBugPlug::ToggleG3DState (G3D_RENDERSTATEOPTION op, const char* name)
{
  if (!G3D) return;
  bool v;
  v = (G3D->GetRenderState (op) != 0);
  v = !v;
  if (G3D->SetRenderState (op, v))
  {
    Report (CS_REPORTER_SEVERITY_DEBUG, "BugPlug %s %s.",
	v ? "enabled" : "disabled", name);
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_DEBUG, "%s not supported for this renderer!",
    	name);
  }
}

void csBugPlug::MouseButtonRight (iCamera* camera)
{
  csRef<iCollideSystem> cdsys = csQueryRegistry<iCollideSystem> (object_reg);
  csScreenTargetResult result = csEngineTools::FindScreenTarget (
      csVector2 (mouse_x, mouse_y), 100.0f, camera, cdsys);
  if (result.mesh)
  {
    float sqdist = csSquaredDist::PointPoint (
	camera->GetTransform ().GetOrigin (), result.isect);
    Report (CS_REPORTER_SEVERITY_DEBUG,
    	"Hit a mesh '%s' at distance %g!",
	result.mesh->QueryObject ()->GetName (), csQsqrt (sqdist));
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
    	"No mesh hit!");
  }
}

void csBugPlug::MouseButtonLeft (iCamera* camera)
{
  csScreenTargetResult result = csEngineTools::FindScreenTarget (
      csVector2 (mouse_x, mouse_y), 100.0f, camera);
  iMeshWrapper* sel = result.mesh;

  csVector3 vw = result.isect;
  csVector3 v = camera->GetTransform ().Other2This (vw);
  Report (CS_REPORTER_SEVERITY_DEBUG,
    "LMB down : c:(%f,%f,%f) w:(%f,%f,%f)", v.x, v.y, v.z, vw.x, vw.y, vw.z);

  if (sel)
  {
    selected_meshes.Empty ();
    AddSelectedMesh (sel);
    const char* n = sel->QueryObject ()->GetName ();
    Report (CS_REPORTER_SEVERITY_DEBUG, "BugPlug found mesh '%s'!",
      	n ? n : "<noname>");
    bool bbox, rad, norm, skel;
    shadow->GetShowOptions (bbox, rad, norm, skel);

    if (bbox || rad || norm || skel || show_polymesh != BUGPLUG_POLYMESH_NO)
      shadow->AddToEngine (Engine);
    else
      shadow->RemoveFromEngine (Engine);
  }
}

bool csBugPlug::EatMouse (iEvent& event)
{
  SetupPlugin ();
  if (!process_next_mouse && !debug_view.show) return false;

  bool down = (CS_IS_MOUSE_EVENT(object_reg, event) && 
	       (csMouseEventHelper::GetEventType(&event)
	       == csMouseEventTypeDown));
  bool up = (CS_IS_MOUSE_EVENT(object_reg, event) &&
	     (csMouseEventHelper::GetEventType(&event) == csMouseEventTypeUp));
  int button = csMouseEventHelper::GetButton(&event);

  mouse_x = csMouseEventHelper::GetX(&event);
  mouse_y = csMouseEventHelper::GetY(&event);

  if (down)
  {
    if (debug_view.show)
    {
      int i;
      debug_view.drag_point = -1;
      for (i = 0 ; i < debug_view.num_points ; i++)
      {
        int x = int (debug_view.points[i].x);
        int y = int (debug_view.points[i].y);
	if (ABS (mouse_x-x) < 4 && ABS ((mouse_y)-y) < 4)
	{
	  debug_view.drag_point = i;
	  break;
	}
      }
    }
    else
    {
      if (catcher->camera)
      {
        switch (button)
	{
	  case csmbLeft: MouseButtonLeft (catcher->camera); break;
	  case csmbRight: MouseButtonRight (catcher->camera); break;
	}
      }
      process_next_mouse = false;
    }
  }
  else if (up)
  {
    debug_view.drag_point = -1;
  }
  else
  {
    if (debug_view.show && debug_view.drag_point != -1)
    {
      debug_view.points[debug_view.drag_point].x = mouse_x;
      debug_view.points[debug_view.drag_point].y = mouse_y;
    }
  }
  return true;
}

bool csBugPlug::ExecCommand (int cmd, const csString& args)
{
  switch (cmd)
  {
    case DEBUGCMD_QUIT:
      Report (CS_REPORTER_SEVERITY_DEBUG, "Nah nah! I will NOT quit!");
      break;
    case DEBUGCMD_STATUS:
      Report (CS_REPORTER_SEVERITY_DEBUG,
		"I'm running smoothly, thank you...");
      break;
    case DEBUGCMD_ENGINECMD:
	{
	  csRef<iDebugHelper> dbghelp (
	  	scfQueryInterface<iDebugHelper> (Engine));
	  if (dbghelp)
	  {
	    if (dbghelp->DebugCommand (args))
	    {
            Report (CS_REPORTER_SEVERITY_DEBUG,
	        "Engine command '%s' performed.", args.GetData());
	    }
	    else
	    {
            Report (CS_REPORTER_SEVERITY_DEBUG,
	        "Engine command '%s' not supported!", args.GetData());
	    }
	  }
	}
      break;
    case DEBUGCMD_VISCULCMD:
      VisculCmd (args);
      break;
    case DEBUGCMD_DEBUGCMD:
	{
	  DebugCmd (args);
	  break;
	}
    case DEBUGCMD_ENGINESTATE:
	{
	  csRef<iDebugHelper> dbghelp (
	  	scfQueryInterface<iDebugHelper> (Engine));
	  if (dbghelp)
	  {
	    if (dbghelp->GetSupportedTests () & CS_DBGHELP_STATETEST)
	    {
	      csRef<iString> rc (dbghelp->StateTest ());
	      if (rc)
	      {
              Report (CS_REPORTER_SEVERITY_DEBUG,
	          "Engine StateTest() failed:");
              Report (CS_REPORTER_SEVERITY_DEBUG,
	          "Engine StateTest() failed:");
              Report (CS_REPORTER_SEVERITY_DEBUG,
		  rc->GetData ());
	      }
	      else
	      {
              Report (CS_REPORTER_SEVERITY_DEBUG,
	          "Engine StateTest() succeeded!");
	      }
	    }
	    else
	    {
            Report (CS_REPORTER_SEVERITY_DEBUG,
	        "Engine doesn't support StateTest()!");
	    }
	  }
	}
      break;
    case DEBUGCMD_HELP:
      Report (CS_REPORTER_SEVERITY_DEBUG, "Sorry, cannot help you yet.");
      break;
    case DEBUGCMD_DUMPENG:
      if (Engine)
      {
        Report (CS_REPORTER_SEVERITY_DEBUG,
		"Dumping entire engine contents to debug.txt.");
	Dump (Engine);
      }
      break;
    case DEBUGCMD_DUMPSEC:
      Report (CS_REPORTER_SEVERITY_DEBUG, "Not implemented yet.");
      break;
    case DEBUGCMD_EDGES:
      ToggleG3DState (G3DRENDERSTATE_EDGES, "edge drawing");
      {
	if (Engine && (Engine->GetBeginDrawFlags () & CSDRAW_CLEARSCREEN))
	  break;
	bool v;
	v = (G3D->GetRenderState (G3DRENDERSTATE_EDGES) != 0);
      }
      break;
    case DEBUGCMD_TEXTURE:
      ToggleG3DState (G3DRENDERSTATE_TEXTUREMAPPINGENABLE, "texture mapping");
      break;
    case DEBUGCMD_BILINEAR:
      ToggleG3DState (G3DRENDERSTATE_BILINEARMAPPINGENABLE,
		"bi-linear filtering");
	break;
    case DEBUGCMD_TRILINEAR:
      ToggleG3DState (G3DRENDERSTATE_TRILINEARMAPPINGENABLE,
		"tri-linear filtering");
	break;
    case DEBUGCMD_LIGHTING:
      ToggleG3DState (G3DRENDERSTATE_LIGHTINGENABLE, "lighting");
	break;
    case DEBUGCMD_GOURAUD:
      ToggleG3DState (G3DRENDERSTATE_GOURAUDENABLE, "gouraud shading");
	break;
    case DEBUGCMD_ILACE:
      ToggleG3DState (G3DRENDERSTATE_INTERLACINGENABLE, "interlaced mode");
	break;
    case DEBUGCMD_MMX:
      ToggleG3DState (G3DRENDERSTATE_MMXENABLE, "mmx mode");
	break;
    case DEBUGCMD_TRANSP:
      ToggleG3DState (G3DRENDERSTATE_TRANSPARENCYENABLE, "transp mode");
      break;
    case DEBUGCMD_CACHECLEAR:
      /*if (G3D)
	{
	  G3D->ClearCache ();
        Report (CS_REPORTER_SEVERITY_DEBUG,
	    "BugPlug cleared the texture cache.");
	}*/
      break;
    case DEBUGCMD_CACHEDUMP:
      //if (G3D) G3D->DumpCache ();
      break;
    case DEBUGCMD_GAMMA:
      {
	  if (!G3D) break;
	  float val = G2D->GetGamma ();
        csString buf;
	  buf.Format ("%g", val);
        EnterEditMode (cmd, "Enter new gamma:", buf);
	}
      break;
    case DEBUGCMD_SELECTMESH:
      EnterEditMode (cmd, "Enter mesh name regexp pattern:", "");
      break;
    case DEBUGCMD_TERRVIS:
	{
	  Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug Terrain Visualization not implemented!");
	}
      break;
    case DEBUGCMD_MESHBASEMESH:
	{
	  if (show_polymesh == BUGPLUG_POLYMESH_BASE)
	  {
	    show_polymesh = BUGPLUG_POLYMESH_NO;
	    Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug disabled showing BASE polygonmesh.");
	  }
	  else
	  {
	    show_polymesh = BUGPLUG_POLYMESH_BASE;
	    Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug is showing BASE polygonmesh.");
	  }
	}
      break;
    case DEBUGCMD_MESHSHADMESH:
	{
	  if (show_polymesh == BUGPLUG_POLYMESH_SHAD)
	  {
	    show_polymesh = BUGPLUG_POLYMESH_NO;
	    Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug disabled showing SHAD polygonmesh.");
	  }
	  else
	  {
	    show_polymesh = BUGPLUG_POLYMESH_SHAD;
	    Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug is showing SHAD polygonmesh.");
	  }
	}
      break;
    case DEBUGCMD_MESHVISMESH:
	{
	  if (show_polymesh == BUGPLUG_POLYMESH_VIS)
	  {
	    show_polymesh = BUGPLUG_POLYMESH_NO;
	    Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug disabled showing VIS polygonmesh.");
	  }
	  else
	  {
	    show_polymesh = BUGPLUG_POLYMESH_VIS;
	    Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug is showing VIS polygonmesh.");
	  }
	}
      break;
    case DEBUGCMD_MESHCDMESH:
	{
	  if (show_polymesh == BUGPLUG_POLYMESH_CD)
	  {
	    show_polymesh = BUGPLUG_POLYMESH_NO;
	    Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug disabled showing CD polygonmesh.");
	  }
	  else
	  {
	    show_polymesh = BUGPLUG_POLYMESH_CD;
	    Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug is showing CD polygonmesh.");
	  }
	}
      break;
    case DEBUGCMD_MESHBBOX:
      {
        bool bbox, rad, norm, skel;
        shadow->GetShowOptions (bbox, rad, norm, skel);
        
        bbox = !bbox;
        Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug %s bounding box display.",
		bbox ? "enabled" : "disabled");
        shadow->SetShowOptions (bbox, rad, norm, skel);
        if ((bbox || rad || norm || skel || show_polymesh != BUGPLUG_POLYMESH_NO)
    	  && HasSelectedMeshes ())
          shadow->AddToEngine (Engine);
        else
          shadow->RemoveFromEngine (Engine);
      }
      break;
    case DEBUGCMD_MESHRAD:
      {
	bool bbox, rad, norm, skel;
	shadow->GetShowOptions (bbox, rad, norm, skel);
        rad = !rad;
	Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug %s bounding sphere display.",
		rad ? "enabled" : "disabled");
	shadow->SetShowOptions (bbox, rad, norm, skel);
	if ((bbox || rad || norm || skel || show_polymesh != BUGPLUG_POLYMESH_NO)
	  && HasSelectedMeshes ())
	  shadow->AddToEngine (Engine);
	else
	  shadow->RemoveFromEngine (Engine);
      }
      break;
    case DEBUGCMD_MESHNORM:
      {
	bool bbox, rad, norm, skel;
	shadow->GetShowOptions (bbox, rad, norm, skel);
        norm = !norm;
	Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug %s normals display.",
		norm ? "enabled" : "disabled");
	shadow->SetShowOptions (bbox, rad, norm, skel);
	if ((bbox || rad || norm || skel || show_polymesh != BUGPLUG_POLYMESH_NO)
	  && HasSelectedMeshes ())
	  shadow->AddToEngine (Engine);
	else
	  shadow->RemoveFromEngine (Engine);
      }
      break;
    case DEBUGCMD_MESHSKEL:
      {
        bool bbox, rad, norm, skel;
        shadow->GetShowOptions (bbox, rad, norm, skel);
        skel = !skel;
        Report (CS_REPORTER_SEVERITY_DEBUG,
          "BugPlug %s skeleton display.",
          norm ? "enabled" : "disabled");
        shadow->SetShowOptions (bbox, rad, norm, skel);
        if ((bbox || rad || norm || skel || show_polymesh != BUGPLUG_POLYMESH_NO)
          && HasSelectedMeshes ())
          shadow->AddToEngine (Engine);
        else
          shadow->RemoveFromEngine (Engine);
      }
      break;
    case DEBUGCMD_DEBUGVIEW:
	SwitchDebugView ();
      break;
    case DEBUGCMD_SWITCHCULLER:
	if (catcher->camera)
        SwitchCuller (catcher->camera->GetSector (), args);
      break;
    case DEBUGCMD_ONESECTOR:
	if (catcher->camera)
        OneSector (catcher->camera);
	break;
    case DEBUGCMD_VISCULVIEW:
      if (catcher->camera)
        VisculView (catcher->camera);
      break;
    case DEBUGCMD_DUMPCAM:
      if (catcher->camera) Dump (catcher->camera);
	break;
    case DEBUGCMD_FOV:
      if (catcher->camera)
	{
	  csRef<iPerspectiveCamera> pcamera =
	    scfQueryInterface<iPerspectiveCamera> (catcher->camera);
	  if (pcamera)
	    {
	      csString buf;
	      float fov = pcamera->GetFOV ();
	      buf.Format ("%f", fov);
	      EnterEditMode (cmd, "Enter new fov value:", buf);
	    }
	}
	break;
    case DEBUGCMD_FOVANGLE:
      if (catcher->camera)
	{
	  csRef<iPerspectiveCamera> pcamera =
	    scfQueryInterface<iPerspectiveCamera> (catcher->camera);
	  if (pcamera)
	    {
	      csString buf;
	      float fov = pcamera->GetFOVAngle ();
	      buf.Format ("%g", fov);
	      EnterEditMode (cmd, "Enter new fov angle:", buf);
	    }
	}
	break;
    case DEBUGCMD_DEBUGSECTOR:
	if (catcher->camera)
	  SwitchDebugSector (catcher->camera->GetTransform ());
      break;
    case DEBUGCMD_SAVEMAP:
      delay_command = DEBUGCMD_SAVEMAP;
	break;
    case DEBUGCMD_SCRSHOT:
      delay_command = DEBUGCMD_SCRSHOT;
      break;
    case DEBUGCMD_DS_LEFT:
      if (debug_sector.show && debug_sector.clear)
	{
	  debug_sector.view->GetCamera ()->Move (csVector3 (-1, 0, 0), false);
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Debug sector is not active now!");
	}
      break;
    case DEBUGCMD_DS_RIGHT:
      if (debug_sector.show && debug_sector.clear)
	{
	  debug_sector.view->GetCamera ()->Move (csVector3 (1, 0, 0), false);
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Debug sector is not active now!");
	}
      break;
    case DEBUGCMD_DS_FORWARD:
      if (debug_sector.show && debug_sector.clear)
	{
	  debug_sector.view->GetCamera ()->Move (csVector3 (0, 0, 1), false);
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Debug sector is not active now!");
	}
      break;
    case DEBUGCMD_DS_BACKWARD:
      if (debug_sector.show && debug_sector.clear)
	{
	  debug_sector.view->GetCamera ()->Move (csVector3 (0, 0, -1), false);
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Debug sector is not active now!");
	}
      break;
    case DEBUGCMD_DS_UP:
      if (debug_sector.show && debug_sector.clear)
	{
	  debug_sector.view->GetCamera ()->Move (csVector3 (0, 1, 0), false);
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Debug sector is not active now!");
	}
      break;
    case DEBUGCMD_DS_DOWN:
      if (debug_sector.show && debug_sector.clear)
	{
	  debug_sector.view->GetCamera ()->Move (csVector3 (0, -1, 0), false);
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Debug sector is not active now!");
	}
      break;
    case DEBUGCMD_DS_TURNLEFT:
      if (debug_sector.show && debug_sector.clear)
	{
	  debug_sector.view->GetCamera ()->GetTransform ().
	  	RotateThis (CS_VEC_ROT_LEFT, 0.2f);
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Debug sector is not active now!");
	}
      break;
    case DEBUGCMD_DS_TURNRIGHT:
      if (debug_sector.show && debug_sector.clear)
	{
	  debug_sector.view->GetCamera ()->GetTransform ().
	  	RotateThis (CS_VEC_ROT_RIGHT, 0.2f);
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Debug sector is not active now!");
	}
      break;
    case DEBUGCMD_FPS:
      do_fps = !do_fps;
	Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug %s fps display.",
		do_fps ? "enabled" : "disabled");
	fps_frame_count = 0;
	fps_tottime = 0;
	fps_cur = -1;
      break;
    case DEBUGCMD_TOGGLEFPSTIME:
      display_time = !display_time;
      Report (CS_REPORTER_SEVERITY_DEBUG, "BugPlug will display %s.", display_time ? "frame time" : "fps");
      break;
    case DEBUGCMD_MESH_XMIN:
      MoveSelectedMeshes (csVector3 (-1, 0, 0));
      break;
    case DEBUGCMD_MESH_XPLUS:
      MoveSelectedMeshes (csVector3 (1, 0, 0));
      break;
    case DEBUGCMD_MESH_YMIN:
      MoveSelectedMeshes (csVector3 (0, -1, 0));
      break;
    case DEBUGCMD_MESH_YPLUS:
      MoveSelectedMeshes (csVector3 (0, 1, 0));
      break;
    case DEBUGCMD_MESH_ZMIN:
      MoveSelectedMeshes (csVector3 (0, 0, -1));
      break;
    case DEBUGCMD_MESH_ZPLUS:
      MoveSelectedMeshes (csVector3 (0, 0, 1));
      break;
    case DEBUGCMD_HIDESELECTED:
      if (HasSelectedMeshes ())
	{
	  size_t j;
	  for (j = 0 ; j < selected_meshes.GetSize () ; j++)
	  {
	    if (selected_meshes[j])
	      selected_meshes[j]->GetFlags ().Set (CS_ENTITY_INVISIBLE);
	  }
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"There are no selected meshes to hide!");
	}
      break;
    case DEBUGCMD_UNDOHIDE:
      if (HasSelectedMeshes ())
	{
	  size_t j;
	  for (j = 0 ; j < selected_meshes.GetSize () ; j++)
	  {
	    if (selected_meshes[j])
	      selected_meshes[j]->GetFlags ().Reset (CS_ENTITY_INVISIBLE);
	  }
	}
      break;
    case DEBUGCMD_COUNTERRESET:
      FullResetCounters ();
	break;
    case DEBUGCMD_COUNTERREMOVE:
      counters.DeleteAll ();
	break;
    case DEBUGCMD_COUNTERFREEZE:
      counter_freeze = !counter_freeze;
	Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"BugPlug %s counting.",
		counter_freeze ? "disabled" : "enabled");
	break;
    case DEBUGCMD_MEMORYDUMP:
	  {
	    csRef<iMemoryTracker> mtr = csQueryRegistryTagInterface<iMemoryTracker> (
	    	object_reg, "crystalspace.utilities.memorytracker");
	    if (!mtr)
	    {
	      Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Memory tracker interface is missing!");
	    }
	    else
	    {
	      mtr->Dump (false);
	      Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Memory dump sent to stdout!");
	    }
	  }
      break;
    case DEBUGCMD_PRINTPORTALS:
        {
          if (catcher->camera)
          {
            iSector* sector = catcher->camera->GetSector();
            printf("Printing portals for sector %s:\n", sector->QueryObject()->GetName());
            csSet<csPtrKey<iMeshWrapper> > portalms = sector->GetPortalMeshes();
            csSet<csPtrKey<iMeshWrapper> >::GlobalIterator itr = portalms.GetIterator();
            while(itr.HasNext())
            {
              iPortalContainer* portalc = itr.Next()->GetPortalContainer();
              if(portalc)
              {
                for(int i=0; i<portalc->GetPortalCount(); ++i)
                {
                  iPortal* portal = portalc->GetPortal(i);
                  printf("Portal name: %s\n", portal->GetName());
                  printf("Portal warp: %s\n", portal->GetWarp().Description().GetData());
                  if(portal->GetSector())
                  {
                    printf("Portal target sector: %s\n", portal->GetSector()->QueryObject()->GetName());
                  }
                  else
                  {
                    printf("Portal target sector: Not resolved\n");
                  }
                  printf("Portal WS plane: %s\n", portal->GetWorldPlane().Description().GetData());
                }
              }
            }
          }
          break;
        }
    case DEBUGCMD_COLORSECTORS:
	Report (CS_REPORTER_SEVERITY_DEBUG,
	    	"Color all sectors...");
	{
	  csColor color_table[14];
	  color_table[0].Set (255, 0, 0);
	  color_table[1].Set (0, 255, 0);
	  color_table[2].Set (0, 0, 255);
	  color_table[3].Set (255, 255, 0);
	  color_table[4].Set (255, 0, 255);
	  color_table[5].Set (0, 255, 255);
	  color_table[6].Set (255, 255, 255);
	  color_table[7].Set (128, 0, 0);
	  color_table[8].Set (0, 128, 0);
	  color_table[9].Set (0, 0, 128);
	  color_table[10].Set (128, 128, 0);
	  color_table[11].Set (128, 0, 128);
	  color_table[12].Set (0, 128, 128);
	  color_table[13].Set (128, 128, 128);
	  int i;
	  iSectorList* sl = Engine->GetSectors ();
	  for (i = 0 ; i < sl->GetCount () ; i++)
	  {
	    iSector* s = sl->Get (i);
	    s->SetDynamicAmbientLight (color_table[i%14]);
	  }
	}
      break;
    case DEBUGCMD_SHADOWDEBUG:
	// swap the default shadow volume material shader to/from a version
	// better visualizing the volume.
	/*csRef<iMaterialWrapper> shadowmat = 
	  Engine->FindMaterial ("shadow extruder");
	if (!standardShadowShader)
	  standardShadowShader = shadowmat->GetMaterial()->GetShader();
	if (!debugShadowShader)
	{
	  csRef<iShaderManager> shmgr ( csQueryRegistry<iShaderManager> (object_reg));
	  if(shmgr)
	  {
	    debugShadowShader = shmgr->CreateShader();
	    if(debugShadowShader)
	    {
	      debugShadowShader->Load (csRef<iDataBuffer> 
		(VFS->ReadFile("/shader/shadowdebug.xml")));
	      if(!debugShadowShader->Prepare())
	      {
		debugShadowShader = 0;
		return true;
	      }
	    }
	    else
	      return true;
	  }
	  else
	    return true;
	}
	do_shadow_debug = !do_shadow_debug;
	if (do_shadow_debug)
	{
	  shadowmat->GetMaterial ()->SetShader(debugShadowShader);
	}
	else
	{
	  shadowmat->GetMaterial ()->SetShader(standardShadowShader);
	}
	Report (CS_REPORTER_SEVERITY_DEBUG,
	    "BugPlug %s shadow debugging.",
	    do_shadow_debug ? "enabled" : "disabled");*/
      break;
    case DEBUGCMD_LISTPLUGINS:
	ListLoadedPlugins();
	break;
    case DEBUGCMD_PROFTOGGLELOG:
      {
        if (do_profiler_log)
        {
          CS_PROFILER_STOP_LOGGING();
        }
        else
        {
          CS_PROFILER_START_LOGGING(0, 0);
        }

        do_profiler_log = !do_profiler_log;
        break;
      }
    case DEBUGCMD_PROFAUTORESET:
      {
        do_profiler_reset = !do_profiler_reset;
        break;
      }
    case DEBUGCMD_UBERSCREENSHOT:
        {
          uint shotW, shotH;
          if (args.IsEmpty() || (sscanf (args, "%u %u", &shotW, &shotH) != 2))
          {
            shotW = 2048;
            shotH = 1536;
	  }
	  CaptureUberScreen (shotW, shotH);
        }
        break;
    default:
        return false;
  }
  return true;
}

void csBugPlug::CaptureScreen ()
{
  csRef<iImage> img (csPtr<iImage> (G2D->ScreenShot ()));
  if (!img)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
    	"The 2D graphics driver does not support screen shots");
    return;
  }
  csRef<iImageIO> imageio (csQueryRegistry<iImageIO> (object_reg));
  if (imageio)
  {
    csRef<iDataBuffer> db (imageio->Save (img, captureMIME, 
      captureOptions));
    if (db)
    {
      csString name = captureFormat.FindNextFilename (VFS);
      if (!VFS->WriteFile (name, (const char*)db->GetData (),
      		db->GetSize ()))
      {
        Report (CS_REPORTER_SEVERITY_DEBUG,
		"There was an error while writing screen shot to %s",
		name.GetData());
      }
      else
        Report (CS_REPORTER_SEVERITY_DEBUG, "Wrote screenshot %s",
	    name.GetData());
    }
    else
    {
      Report (CS_REPORTER_SEVERITY_DEBUG, 
	      "Could not encode screen shot");
    }
  }
}

void csBugPlug::CaptureUberScreen (uint w, uint h)
{
  csString descr; descr.Format ("%ux%u \xC3\xBC" "berscreenshot", w, h);

  if (!catcher->camera)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
    	"Could not take %s: no camera", descr.GetData());
    return;
  }

  csRef<iImage> img;
  {
    CS::UberScreenshotMaker shotMaker (w, h, catcher->camera, Engine, G3D);
    img = shotMaker.Shoot();
  }
  if (!img)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
    	"Could not take %s", descr.GetData());
    return;
  }
  csRef<iImageIO> imageio (csQueryRegistry<iImageIO> (object_reg));
  if (imageio)
  {
    csRef<iDataBuffer> db (imageio->Save (img, captureMIME, 
      captureOptions));
    if (db)
    {
      csString name = captureFormat.FindNextFilename (VFS);
      if (!VFS->WriteFile (name, (const char*)db->GetData (),
      		db->GetSize ()))
      {
        Report (CS_REPORTER_SEVERITY_DEBUG,
		"There was an error while writing %s to %s", descr.GetData(), 
		name.GetData());
      }
      else
	Report (CS_REPORTER_SEVERITY_DEBUG, "Wrote %s %s", descr.GetData(), 
	  name.GetData());
    }
    else
    {
      Report (CS_REPORTER_SEVERITY_DEBUG, 
	      "Could not encode %s", descr.GetData());
    }
  }
}

void csBugPlug::ListLoadedPlugins ()
{
  csRef<iPluginManager> plugmgr =  
    csQueryRegistry<iPluginManager> (object_reg);
  csRef<iPluginIterator> plugiter (plugmgr->GetPluginInstances ());

  csSet<const char*> printedPlugins;
  Report (CS_REPORTER_SEVERITY_DEBUG, 
    "Loaded plugins:");
  while (plugiter->HasNext())
  {
    csRef<iFactory> plugFact = 
      scfQueryInterface<iFactory> (plugiter->Next ());
    if (plugFact.IsValid())
    {
      const char* libname = plugFact->QueryModuleName();
      if ((libname != 0) && (!printedPlugins.In (libname)))
      {
	printedPlugins.AddNoTest (libname);
	Report (CS_REPORTER_SEVERITY_DEBUG, 
	  "  %s", libname);
      }
    }
  }
}

bool csBugPlug::EatKey (iEvent& event)
{
  int type = csKeyEventHelper::GetEventType (&event);
  SetupPlugin ();
  utf32_char key = csKeyEventHelper::GetRawCode (&event);
  bool down = (type == csKeyEventTypeDown);
  csKeyModifiers m;
  csKeyEventHelper::GetModifiers (&event, m);
  bool shift = m.modifiers[csKeyModifierTypeShift] != 0;
  bool alt = m.modifiers[csKeyModifierTypeAlt] != 0;
  bool ctrl = m.modifiers[csKeyModifierTypeCtrl] != 0;

  // If we are in edit mode we do special processing.
  if (edit_mode)
  {
    if (down)
    {
      size_t l = edit_string.Length (); 
      key = csKeyEventHelper::GetCookedCode (&event);
      if (key == CSKEY_ENTER)
      {
        // Exit edit_mode.
	edit_mode = false;
	ExitEditMode ();
      }
      else if (key == CSKEY_BACKSPACE)
      {
        // Backspace.
	if (edit_cursor > 0)
	{
	  int cs = csUnicodeTransform::UTF8Rewind (
	    (utf8_char*)edit_string.GetData () + edit_cursor, edit_cursor);
	  edit_cursor -= cs;
	  edit_string.DeleteAt (edit_cursor, cs);
	}
      }
      else if (key == CSKEY_DEL)
      {
        // Delete.
	int cs = csUnicodeTransform::UTF8Skip (
	  (utf8_char*)edit_string.GetData () + edit_cursor, 
	  l - edit_cursor);
	if (cs > 0)
	{
	  edit_string.DeleteAt (edit_cursor, cs);
	}
      }
      else if (key == CSKEY_HOME)
      {
        edit_cursor = 0;
      }
      else if (key == CSKEY_END)
      {
        edit_cursor = l;
      }
      else if (key == CSKEY_LEFT)
      {
        if (edit_cursor > 0) 
	{
	  int cs = csUnicodeTransform::UTF8Rewind (
	    (utf8_char*)edit_string.GetData () + edit_cursor, edit_cursor);
	  edit_cursor -= cs;
	}
      }
      else if (key == CSKEY_RIGHT)
      {
        if (edit_cursor < l) 
	{
	  int cs = csUnicodeTransform::UTF8Skip (
	    (utf8_char*)edit_string.GetData () + edit_cursor, 
	    l - edit_cursor);
	  edit_cursor += cs;
	}
      }
      else if (key == CSKEY_ESC)
      {
        // Cancel.
	edit_string.Replace ("");
	edit_mode = false;
      }
      else //if (edit_cursor < 79)
      {
	utf32_char composedChs[3];
	int composedNum;
	csKeyEventData eventData;
	csKeyEventHelper::GetEventData (&event, eventData);

	keyComposer->HandleKey (eventData, composedChs, 
	  sizeof (composedChs) / sizeof (utf32_char), &composedNum);
	
        if (composedNum > 0)
	{
	  utf8_char ch[CS_UC_MAX_UTF8_ENCODED*2 + 1];
	  size_t chSize = csUnicodeTransform::UTF32to8 (ch, 
	    sizeof (ch) / sizeof (utf8_char), composedChs,
	    composedNum) - 1;
	  edit_string.Insert (edit_cursor, (char*)ch);
	  edit_cursor += chSize;
	}
      }
    }
    return true;
  }

  // Get command.
  csString args;
  int cmd = GetCommandCode (key, shift, alt, ctrl, args);
  if (down)
  {
    // First we check if it is the 'debug enter' key.
    if (cmd == DEBUGCMD_DEBUGENTER)
    {
      process_next_key = !process_next_key;
      if (process_next_key)
      {
        Report (CS_REPORTER_SEVERITY_DEBUG, "Press debug key...");
      }
      else
      {
        Report (CS_REPORTER_SEVERITY_DEBUG, "Back to normal key processing.");
      }
      return true;
    }
    if (cmd == DEBUGCMD_MOUSEENTER)
    {
      process_next_mouse = !process_next_mouse;
      if (process_next_mouse)
      {
        Report (CS_REPORTER_SEVERITY_DEBUG, "Click on screen...");
      }
      return true;
    }
  }

  // Return false if we are not processing our own keys.
  // If debug_sector.show is true we will process our own keys to...
  if (!process_next_key && !(debug_sector.show && debug_sector.clear)
  	&& !debug_view.show)
    return false;
  if ((debug_sector.show && debug_sector.clear) || debug_view.show)
  {
    process_next_key = false;
  }

  if (down)
  {
    if (ExecCommand (cmd, args))
      process_next_key = false;
  }
  return true;
}

bool csBugPlug::HandleStartFrame (iEvent& /*event*/)
{
  SetupPlugin ();
  if (!G3D) return false;

  if (shadow) shadow->ClearView ();
  
  if (do_profiler_reset)
  {
    CS_PROFILER_RESET();
  }

  return false;
}

static void GfxWrite (iGraphics2D* g2d, iFont* font,
	int x, int y, int fg, int bg, const char *str, ...)
{
  va_list arg;
  csString buf;

  va_start (arg, str);
  buf.FormatV (str, arg);
  va_end (arg);

  g2d->Write (font, x, y, fg, bg, buf);
}

static inline void BugplugBox (iGraphics2D* G2D, 
			       int x, int y, int  w, int h)
{
  int bgcolor = G2D->FindRGB (255, 255, 192);
  G2D->DrawBox (x, y, w, h, bgcolor);
  int bordercolor = G2D->FindRGB (192, 192, 64);
  G2D->DrawLine (x, y, x+w, y, bordercolor);
  G2D->DrawLine (x+w, y, x+w, y+h, bordercolor);
  G2D->DrawLine (x+w, y+h, x, y+h, bordercolor);
  G2D->DrawLine (x, y+h, x, y, bordercolor);
}

static inline void DrawBox3D (iGraphics3D* G3D, 
                              const csBox3& box,
                              const csReversibleTransform& tr,
                              int color)
{
  csVector3 vxyz = tr * box.GetCorner (CS_BOX_CORNER_xyz);
  csVector3 vXyz = tr * box.GetCorner (CS_BOX_CORNER_Xyz);
  csVector3 vxYz = tr * box.GetCorner (CS_BOX_CORNER_xYz);
  csVector3 vxyZ = tr * box.GetCorner (CS_BOX_CORNER_xyZ);
  csVector3 vXYz = tr * box.GetCorner (CS_BOX_CORNER_XYz);
  csVector3 vXyZ = tr * box.GetCorner (CS_BOX_CORNER_XyZ);
  csVector3 vxYZ = tr * box.GetCorner (CS_BOX_CORNER_xYZ);
  csVector3 vXYZ = tr * box.GetCorner (CS_BOX_CORNER_XYZ);
  float fov = G3D->GetPerspectiveAspect ();
  G3D->DrawLine (vxyz, vXyz, fov, color);
  G3D->DrawLine (vXyz, vXYz, fov, color);
  G3D->DrawLine (vXYz, vxYz, fov, color);
  G3D->DrawLine (vxYz, vxyz, fov, color);
  G3D->DrawLine (vxyZ, vXyZ, fov, color);
  G3D->DrawLine (vXyZ, vXYZ, fov, color);
  G3D->DrawLine (vXYZ, vxYZ, fov, color);
  G3D->DrawLine (vxYZ, vxyZ, fov, color);
  G3D->DrawLine (vxyz, vxyZ, fov, color);
  G3D->DrawLine (vxYz, vxYZ, fov, color);
  G3D->DrawLine (vXyz, vXyZ, fov, color);
  G3D->DrawLine (vXYz, vXYZ, fov, color);
}

bool csBugPlug::HandleFrame (iEvent& /*event*/)
{
  SetupPlugin ();
  if (!G3D) return false;

  if (do_fps)
  {
    csTicks elapsed_time = vc->GetElapsedTicks ();
    fps_tottime += elapsed_time;
    fps_frame_count++;
    if (fps_tottime > 500)
    {
      fps_cur = (float (fps_frame_count) * 1000.0) / float (fps_tottime);
      fps_frame_count = 0;
      fps_tottime = 0;
    }
  }

  if (visculler)
  {
    csRef<iDebugHelper> dbghelp (scfQueryInterface<iDebugHelper> (visculler));
    if (dbghelp)
      dbghelp->Dump (G3D);
  }

  if (debug_sector.show)
  {
    G3D->BeginDraw (CSDRAW_3DGRAPHICS |
    	CSDRAW_CLEARZBUFFER | (debug_sector.clear ? CSDRAW_CLEARSCREEN : 0));
    iCamera* camera = catcher->camera;
    if (camera)
      debug_sector.view->GetCamera ()->SetTransform (camera->GetTransform ());
    debug_sector.view->Draw ();
  }

  if (debug_view.show)
  {
    G3D->BeginDraw (CSDRAW_2DGRAPHICS |
    	(debug_view.clear ? CSDRAW_CLEARSCREEN : 0));
    if (debug_view.object)
      debug_view.object->Render (G3D, this);
    int pointcol = G2D->FindRGB (255, 255, 0);
    int linecol = G2D->FindRGB (255, 255, 255);
    int i;
    for (i = 0 ; i < debug_view.num_lines ; i++)
    {
      int i1 = debug_view.lines[i].i1;
      int i2 = debug_view.lines[i].i2;
      G2D->DrawLine (
      	debug_view.points[i1].x, debug_view.points[i1].y,
      	debug_view.points[i2].x, debug_view.points[i2].y, linecol);
    }
    for (i = 0 ; i < debug_view.num_boxes ; i++)
    {
      int i1 = debug_view.boxes[i].i1;
      int i2 = debug_view.boxes[i].i2;
      float x1 = debug_view.points[i1].x;
      float y1 = debug_view.points[i1].y;
      float x2 = debug_view.points[i2].x;
      float y2 = debug_view.points[i2].y;
      G2D->DrawLine (x1, y1, x2, y1, linecol);
      G2D->DrawLine (x2, y1, x2, y2, linecol);
      G2D->DrawLine (x2, y2, x1, y2, linecol);
      G2D->DrawLine (x1, y2, x1, y1, linecol);
    }
    for (i = 0 ; i < debug_view.num_points ; i++)
    {
      float x = debug_view.points[i].x;
      float y = debug_view.points[i].y;
      G2D->DrawLine (x-5, y-5, x+5, y+5, pointcol);
      G2D->DrawLine (x-5, y+5, x+5, y-5, pointcol);
    }
  }

  if (HasSelectedMeshes () && shadow && shadow->GetView () &&
		  !debug_view.show && !debug_sector.show)
  {
    size_t k;
    iRenderView* rview = shadow->GetView();
    iCamera* cam = rview->GetOriginalCamera();
    csTransform tr_w2c = cam->GetTransform ();
    float fov = G3D->GetPerspectiveAspect ();
    bool do_bbox, do_rad, do_norm, do_skel;
    shadow->GetShowOptions (do_bbox, do_rad, do_norm, do_skel);
    G3D->BeginDraw (CSDRAW_2DGRAPHICS);
    for (k = 0 ; k < selected_meshes.GetSize () ; k++)
    {
      if (!selected_meshes[k]) continue;
      iMovable* mov = selected_meshes[k]->GetMovable ();
      csReversibleTransform tr_o2c = tr_w2c / mov->GetFullTransform ();
      csRenderMesh** rmeshes = 0;
      int rmesh_num = 0;
      if (do_bbox)
      {
        int bbox_color;
        if (!rmeshes) rmeshes = 
          selected_meshes[k]->GetMeshObject()->GetRenderMeshes (rmesh_num, rview, 
            mov, ~0/*frustum_mask*/);
        if (rmeshes != 0)
        {
	  bbox_color = G3D->GetDriver2D ()->FindRGB (255, 0, 255);
          for (int n = 0; n < rmesh_num; n++)
          {
            DrawBox3D (G3D, rmeshes[n]->bbox, tr_o2c, bbox_color);
          }
        }
        
        bbox_color = G3D->GetDriver2D ()->FindRGB (0, 255, 255);
        const csBox3& bbox = selected_meshes[k]->GetMeshObject ()
	  ->GetObjectModel ()->GetObjectBoundingBox ();
	DrawBox3D (G3D, bbox, tr_o2c, bbox_color);
      }
      if (do_rad)
      {
        int rad_color = G3D->GetDriver2D ()->FindRGB (0, 255, 0);
	float radius;
        csVector3 r, center;
        selected_meshes[k]->GetMeshObject ()->GetObjectModel ()
		->GetRadius (radius,center);
        csVector3 trans_o = tr_o2c * center;
        r.Set (radius, 0, 0);
        G3D->DrawLine (trans_o-r, trans_o+r, fov, rad_color);
        r.Set (0, radius, 0);
        G3D->DrawLine (trans_o-r, trans_o+r, fov, rad_color);
        r.Set (0, 0, radius);
        G3D->DrawLine (trans_o-r, trans_o+r, fov, rad_color);
      }
      if (do_norm)
      {
        int norm_color = G3D->GetDriver2D ()->FindRGB (0, 0, 255);
        int denorm_color = G3D->GetDriver2D ()->FindRGB (128, 0, 255);
        int tang_color = G3D->GetDriver2D ()->FindRGB (255, 0, 0);
        int bitang_color = G3D->GetDriver2D ()->FindRGB (0, 255, 0);
        
        if (!rmeshes) rmeshes = 
          selected_meshes[k]->GetMeshObject()->GetRenderMeshes (rmesh_num, rview, 
            mov, ~0/*frustum_mask*/);
        if (rmeshes != 0)
        {
          for (int n = 0; n < rmesh_num; n++)
          {
            iRenderBuffer* bufPos = rmeshes[n]->buffers->GetRenderBuffer (
              CS_BUFFER_POSITION);
            iRenderBuffer* bufNorm = rmeshes[n]->buffers->GetRenderBuffer (
              CS_BUFFER_NORMAL);
            iRenderBuffer* bufIndex = rmeshes[n]->buffers->GetRenderBuffer (
              CS_BUFFER_INDEX);
            if (!bufPos || !bufNorm || !bufIndex) continue;
            iRenderBuffer* bufTang = rmeshes[n]->buffers->GetRenderBuffer (
              CS_BUFFER_TANGENT);
            iRenderBuffer* bufBitang = rmeshes[n]->buffers->GetRenderBuffer (
              CS_BUFFER_BINORMAL);

            // @@@ FIXME: Handle other component types.
            csRenderBufferLock<csVector3> positions (bufPos, CS_BUF_LOCK_READ);
            csRenderBufferLock<csVector3> normals (bufNorm, CS_BUF_LOCK_READ);

          #include "csutil/custom_new_disable.h"
            const size_t bufLockSize = sizeof (csRenderBufferLock<csVector3>);
            uint8 tangLockStore[bufLockSize * 2];
            csRenderBufferLock<csVector3>* tangents = 0;
            if (bufTang != 0)
            {
              tangents = (csRenderBufferLock<csVector3>*)tangLockStore;
              new (tangents) csRenderBufferLock<csVector3> (bufTang, 
                CS_BUF_LOCK_READ);
            }
            csRenderBufferLock<csVector3>* bitangents = 0;
            if (bufBitang != 0)
            {
              bitangents = ((csRenderBufferLock<csVector3>*)tangLockStore) + 1;
              new (bitangents) csRenderBufferLock<csVector3> (bufBitang, 
                CS_BUF_LOCK_READ);
            }
          #include "csutil/custom_new_enable.h"

            CS::TriangleIndicesStream<size_t> tris (bufIndex, rmeshes[n]->meshtype,
              rmeshes[n]->indexstart, rmeshes[n]->indexend);
            while (tris.HasNext())
            {
              CS::TriangleT<size_t> tri = tris.Next();
              for (int t = 0; t < 3; t++)
              {
                csVector3 p = tr_o2c.Other2This (positions[tri[t]]);
                csVector3 n = tr_o2c.Other2ThisRelative (normals[tri[t]]);
                int color = 
                  (fabsf (n.Norm() - 1.0f) < EPSILON) ? norm_color : denorm_color;
                // @@@ FIXME: Should perhaps be configurable
                const float normScale = 0.5f; 
                G3D->DrawLine (p, p+n*normScale, fov, color);
                if (tangents != 0)
                {
                  csVector3 tng = tr_o2c.Other2ThisRelative ((*tangents)[tri[t]]);
                  G3D->DrawLine (p, p+tng*normScale, fov, tang_color);
                }
                if (bitangents != 0)
                {
                  csVector3 bit = tr_o2c.Other2ThisRelative ((*bitangents)[tri[t]]);
                  G3D->DrawLine (p, p+bit*normScale, fov, bitang_color);
                }
              }
            }

            if (tangents != 0) tangents->~csRenderBufferLock<csVector3>();
            if (bitangents != 0) bitangents->~csRenderBufferLock<csVector3>();
          }
        }
      }
      if (do_skel)
      {
        int bone_color = G3D->GetDriver2D ()->FindRGB (255, 0, 255);

        csRef<CS::Mesh::iAnimatedMesh> aniMesh = scfQueryInterfaceSafe<CS::Mesh::iAnimatedMesh> (
          selected_meshes[k]->GetMeshObject ());
        if (!aniMesh)
          continue;

	CS::Animation::iSkeleton* skeleton = aniMesh->GetSkeleton ();
        if (!skeleton)
          continue;


        CS::Animation::iSkeletonFactory* fact = skeleton->GetFactory ();

        // Setup the "end" positions of all bones
        const CS::Animation::BoneID lastId = fact->GetTopBoneID ();
        csArray<csVector3> childPos;
        csArray<int> numChild;

        childPos.SetSize (lastId+1, csVector3 (0));
        numChild.SetSize (lastId+1, 0);

        for (CS::Animation::BoneID i = 0; i < lastId+1; ++i)
        {
          if (!fact->HasBone (i))
            continue;

          CS::Animation::BoneID parent = fact->GetBoneParent (i);
          if (parent != CS::Animation::InvalidBoneID)
          {
            csQuaternion q;
            csVector3 v;
            skeleton->GetTransformBoneSpace (i, q, v);

            childPos[parent] += v;
            numChild[parent] += 1;
          }
        }

        // Now draw the bones
        for (CS::Animation::BoneID i = 0; i < lastId+1; ++i)
        {
          if (!fact->HasBone (i))
            continue;

          csQuaternion q;
          csVector3 v;
          skeleton->GetTransformAbsSpace (i, q, v);

          csVector3 gs = tr_o2c * v;
          csVector3 endLocal;

          if (numChild[i] > 0)
          {
            endLocal = childPos[i] / numChild[i];
          }
          else
          {
            endLocal = csVector3 (0,0,1);
          }

          csVector3 endGlobal = v + q.Rotate (endLocal);
          csVector3 ge = tr_o2c * endGlobal;

          G3D->DrawLine (gs, ge, fov, bone_color);
        }
      }
      if (show_polymesh != BUGPLUG_POLYMESH_NO)
      {
	csRef<iTriangleMesh> trimesh;
	iObjectModel* objmodel = selected_meshes[k]->GetMeshObject ()
	  ->GetObjectModel ();
	csStringID base_id = stringSet->Request ("base");
	if (objmodel->IsTriangleDataSet (base_id))
	{
          switch (show_polymesh)
          {
	    case BUGPLUG_POLYMESH_CD:
	      {
		csStringID id = stringSet->Request ("colldet");
	        if (objmodel->IsTriangleDataSet (id))
		  trimesh = objmodel->GetTriangleData (id);
	        else
		  trimesh = objmodel->GetTriangleData (base_id);
	      }
	      break;
	    case BUGPLUG_POLYMESH_VIS:
	      {
		csStringID id = stringSet->Request ("viscull");
	        if (objmodel->IsTriangleDataSet (id))
		  trimesh = objmodel->GetTriangleData (id);
	        else
		  trimesh = objmodel->GetTriangleData (base_id);
	      }
	      break;
	    case BUGPLUG_POLYMESH_SHAD:
	      {
		csStringID id = stringSet->Request ("shadows");
	        if (objmodel->IsTriangleDataSet (id))
		  trimesh = objmodel->GetTriangleData (id);
	        else
		  trimesh = objmodel->GetTriangleData (base_id);
	      }
	      break;
	    case BUGPLUG_POLYMESH_BASE:
	      trimesh = objmodel->GetTriangleData (stringSet->Request (
		    "base"));
	      break;
          }
	}
        if (trimesh)
        {
          int pm_color = G3D->GetDriver2D ()->FindRGB (255, 255, 128);
	  size_t vtcount = trimesh->GetVertexCount ();
	  csVector3* vt = trimesh->GetVertices ();
	  size_t pocount = trimesh->GetTriangleCount ();
	  csTriangle* po = trimesh->GetTriangles ();
	  csVector3* vtt = new csVector3[vtcount];
	  size_t i;
	  for (i = 0 ; i < vtcount ; i++)
	    vtt[i] = tr_o2c * vt[i];
	  for (i = 0 ; i < pocount ; i++)
	  {
	    csTriangle& tri = po[i];
            G3D->DrawLine (vtt[tri.a], vtt[tri.c], fov, pm_color);
            G3D->DrawLine (vtt[tri.b], vtt[tri.a], fov, pm_color);
            G3D->DrawLine (vtt[tri.c], vtt[tri.b], fov, pm_color);
	  }
          delete[] vtt;
        }
      }
    }
  }

  if (process_next_key || process_next_mouse)
  {
    if (fnt)
    {
      G3D->BeginDraw (CSDRAW_2DGRAPHICS);
      int fw, fh;
      fnt->GetMaxSize (fw, fh);
      int sh = G2D->GetHeight ();
      int x = 150;
      int y = sh/2 - (fh+5*2)/2;
      int w = 200;
      int h = fh+5*2;
      BugplugBox (G2D, x, y, w, h);
      int fgcolor = G2D->FindRGB (0, 0, 0);
      const char* msg;
      if (process_next_key) msg = "Press a BugPlug key...";
      else msg = "Click on screen...";
      G2D->Write (fnt, x+5, y+5, fgcolor, -1, msg);
    }
  }

  if (edit_mode)
  {
    if (fnt)
    {
      G3D->BeginDraw (CSDRAW_2DGRAPHICS);
      int fw, fh;
      fnt->GetMaxSize (fw, fh);
      int sw = G2D->GetWidth ();
      int sh = G2D->GetHeight ();
      int x = 10;
      int y = sh/2 - (fh*2+5*3)/2;
      int w = sw-20;
      int h = fh*2+5*3;
      BugplugBox (G2D, x, y, w, h);
      int fgcolor = G2D->FindRGB (0, 0, 0);
      int maxlen = fnt->GetLength (msg_string, w-10);
      if (maxlen < 80) msg_string[maxlen] = 0;
      G2D->Write (fnt, x+5, y+5, fgcolor, -1, msg_string);
      maxlen = fnt->GetLength (edit_string, w-10);
      csString dispString (edit_string);
      dispString.Truncate (maxlen);
      //if (maxlen < 80) edit_string[maxlen] = 0;
      G2D->Write (fnt, x+5, y+5+fh+5, fgcolor, -1, dispString);
      //char cursor[83];
      //strcpy (cursor, edit_string);
      //cursor[edit_cursor] = 0;
      dispString.Truncate (edit_cursor);
      int cursor_w, cursor_h;
      fnt->GetDimensions (dispString.GetDataSafe(), cursor_w, cursor_h);
      G2D->Write (fnt, x+5+cursor_w, y+5+fh+7, fgcolor, -1, "_");
    }
  }

  if (do_fps)
  {
    if (fnt)
    {
      G3D->BeginDraw (CSDRAW_2DGRAPHICS);
      int sh = G2D->GetHeight ();
      int fw, fh;
      fnt->GetMaxSize (fw, fh);
      int fgcolor = G2D->FindRGB (255, 255, 255);

      if (display_time)
      {
        const float mspf = 1000.0f / fps_cur;

        GfxWrite (G2D, fnt, 11, sh - fh - 3, 0, -1, "%.3f msec", mspf);
        GfxWrite (G2D, fnt, 10, sh - fh - 2, fgcolor, -1, "%.3f msec", mspf);
      }
      else
      {
        if (fps_cur < 0.5)
        {
          const float spf = 1.0f/fps_cur;
          GfxWrite (G2D, fnt, 11, sh - fh - 3, 0, -1, "SPF=%.2f", spf);
          GfxWrite (G2D, fnt, 10, sh - fh - 2, fgcolor, -1, "SPF=%.2f", spf);
        }
        else
        {
          GfxWrite (G2D, fnt, 11, sh - fh - 3, 0, -1, "FPS=%.2f", fps_cur);
          GfxWrite (G2D, fnt, 10, sh - fh - 2, fgcolor, -1, "FPS=%.2f", fps_cur);
        }
      }
    }
    G3D->FinishDraw ();
  }

  ShowCounters ();

  if (delay_command != DEBUGCMD_UNKNOWN)
  {
    switch (delay_command)
    {
      case DEBUGCMD_SAVEMAP:
        SaveMap ();
	break;
      case DEBUGCMD_SCRSHOT:
        CaptureScreen ();
        break;
    }
    delay_command = DEBUGCMD_UNKNOWN;
  }

  return false;
}

bool csBugPlug::HandleSystemOpen (iEvent* /*event*/)
{
  return false;
}

bool csBugPlug::HandleSystemClose (iEvent* /*event*/)
{
  fnt = 0;
  shadow->RemoveFromEngine (Engine);
  return false;
}

void csBugPlug::EnterEditMode (int cmd, const char* msg, const char* def)
{
  if (edit_mode) return;
  if (!fnt) return;	// No edit mode if no font server
  edit_mode = true;
  strcpy (msg_string, msg);
  if (def) edit_string.Replace (def);// strcpy (edit_string, def);
  else edit_string.Replace ("");
  edit_cursor = edit_string.Length ();
  edit_command = cmd;
}

void csBugPlug::ExitEditMode ()
{
  if (edit_string.Length () == 0) return;
  float f;
  switch (edit_command)
  {
    case DEBUGCMD_GAMMA:
      csScanStr (edit_string, "%f", &f);
      G2D->SetGamma (f);
      break;
    case DEBUGCMD_FOV:
      csScanStr (edit_string, "%f", &f);
      if (catcher->camera)
	{
	  csRef<iPerspectiveCamera> pcamera =
	    scfQueryInterface<iPerspectiveCamera> (catcher->camera);
	  if (pcamera)
	    pcamera->SetFOV (f, 1.0f);
	}
      break;
    case DEBUGCMD_FOVANGLE:
      csScanStr (edit_string, "%f", &f);
      if (catcher->camera)
	{
	  csRef<iPerspectiveCamera> pcamera =
	    scfQueryInterface<iPerspectiveCamera> (catcher->camera);
	  if (pcamera)
	    pcamera->SetFOVAngle (f, 1.0f);
	}
      break;
    case DEBUGCMD_SELECTMESH:
      if (catcher->camera)
        SelectMesh (catcher->camera->GetSector (), edit_string);
      break;
  }
}

void csBugPlug::DebugCmd (const char* cmd)
{
  CS_ALLOC_STACK_ARRAY(char, cmdstr, strlen (cmd)+1);
  strcpy (cmdstr, cmd);
  char* params;
  char* space = strchr (cmdstr, ' ');
  if (space == 0)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
      "debugcmd syntax: <plugin> <command>");
  }
  else
  {
    params = space + 1;
    *space = 0;

    csRef<iBase> comp;
    comp = csQueryRegistryTag(object_reg, cmdstr);

    if (comp == 0)
    {
      csRef<iPluginManager> plugmgr = 
	csQueryRegistry<iPluginManager> (object_reg);
      CS_ASSERT (plugmgr);
      csRef<iBase> comp =
	CS_QUERY_PLUGIN_CLASS (plugmgr, cmdstr, iBase);
    }

    if (!comp)
    {
      Report (CS_REPORTER_SEVERITY_DEBUG,
	"Could not load plugin '%s' for debug command execution.",
	cmdstr);
    }
    else
    {
      csRef<iDebugHelper> dbghelp = 
	scfQueryInterface<iDebugHelper> (comp);
      if (!dbghelp)
      {
	Report (CS_REPORTER_SEVERITY_DEBUG,
	  "Plugin '%s' doesn't support debug command execution.",
	  cmdstr);
      }
      else
      {
	bool res = dbghelp->DebugCommand (params);
	Report (CS_REPORTER_SEVERITY_DEBUG,
	  "Execution of debug command '%s' on '%s' %s.",
	  params, cmdstr,
	  res ? "successful" : "failed");
      }
    }
  }
}

int csBugPlug::GetKeyCode (const char* keystring, bool& shift, bool& alt,
	bool& ctrl)
{
  shift = alt = ctrl = false;
  char const* dash = strchr (keystring, '-');
  while (dash)
  {
    if (!strncmp (keystring, "shift", int (dash-keystring))) shift = true;
    else if (!strncmp (keystring, "alt", int (dash-keystring))) alt = true;
    else if (!strncmp (keystring, "ctrl", int (dash-keystring))) ctrl = true;
    keystring = dash+1;
    dash = strchr (keystring, '-');
  }

  int keycode = -1;
  if (!strcmp (keystring, "tab")) keycode = CSKEY_TAB;
  else if (!strcmp (keystring, "space")) keycode = ' ';
  else if (!strcmp (keystring, "esc")) keycode = CSKEY_ESC;
  else if (!strcmp (keystring, "enter")) keycode = CSKEY_ENTER;
  else if (!strcmp (keystring, "bs")) keycode = CSKEY_BACKSPACE;
  else if (!strcmp (keystring, "up")) keycode = CSKEY_UP;
  else if (!strcmp (keystring, "down")) keycode = CSKEY_DOWN;
  else if (!strcmp (keystring, "right")) keycode = CSKEY_RIGHT;
  else if (!strcmp (keystring, "left")) keycode = CSKEY_LEFT;
  else if (!strcmp (keystring, "pgup")) keycode = CSKEY_PGUP;
  else if (!strcmp (keystring, "pgdn")) keycode = CSKEY_PGDN;
  else if (!strcmp (keystring, "home")) keycode = CSKEY_HOME;
  else if (!strcmp (keystring, "end")) keycode = CSKEY_END;
  else if (!strcmp (keystring, "ins")) keycode = CSKEY_INS;
  else if (!strcmp (keystring, "del")) keycode = CSKEY_DEL;
  else if (!strcmp (keystring, "f1")) keycode = CSKEY_F1;
  else if (!strcmp (keystring, "f2")) keycode = CSKEY_F2;
  else if (!strcmp (keystring, "f3")) keycode = CSKEY_F3;
  else if (!strcmp (keystring, "f4")) keycode = CSKEY_F4;
  else if (!strcmp (keystring, "f5")) keycode = CSKEY_F5;
  else if (!strcmp (keystring, "f6")) keycode = CSKEY_F6;
  else if (!strcmp (keystring, "f7")) keycode = CSKEY_F7;
  else if (!strcmp (keystring, "f8")) keycode = CSKEY_F8;
  else if (!strcmp (keystring, "f9")) keycode = CSKEY_F9;
  else if (!strcmp (keystring, "f10")) keycode = CSKEY_F10;
  else if (!strcmp (keystring, "f11")) keycode = CSKEY_F11;
  else if (!strcmp (keystring, "f12")) keycode = CSKEY_F12;
  else if (*(keystring+1) != 0) return -1;
  else if ((*keystring >= 'A' && *keystring <= 'Z')
  	|| strchr ("!@#$%^&*()_+", *keystring))
  {
    shift = 1;
    keycode = *keystring;
  }
  else
    keycode = *keystring;

  return keycode;
}

int csBugPlug::GetCommandCode (const char* cmdstr, csString& args)
{
  if ((cmdstr == 0) || (*cmdstr == 0)) return DEBUGCMD_UNKNOWN;

  csString cmd;
  char const* spc = strchr (cmdstr, ' ');
  if (spc)
  {
    cmd.Append (cmdstr, spc - cmdstr);
    args.Replace (spc + 1);
  }
  else
  {
    cmd = cmdstr;
    args.Clear();
  }

  if (!strcmp (cmd, "debugenter"))	return DEBUGCMD_DEBUGENTER;
  if (!strcmp (cmd, "mouseenter"))	return DEBUGCMD_MOUSEENTER;
  if (!strcmp (cmd, "quit"))		return DEBUGCMD_QUIT;
  if (!strcmp (cmd, "status"))		return DEBUGCMD_STATUS;
  if (!strcmp (cmd, "help"))		return DEBUGCMD_HELP;

  if (!strcmp (cmd, "dumpeng"))		return DEBUGCMD_DUMPENG;
  if (!strcmp (cmd, "dumpsec"))		return DEBUGCMD_DUMPSEC;
  if (!strcmp (cmd, "edges"))		return DEBUGCMD_EDGES;
  if (!strcmp (cmd, "cacheclear"))	return DEBUGCMD_CACHECLEAR;
  if (!strcmp (cmd, "cachedump"))	return DEBUGCMD_CACHEDUMP;
  if (!strcmp (cmd, "texture"))		return DEBUGCMD_TEXTURE;
  if (!strcmp (cmd, "bilinear"))	return DEBUGCMD_BILINEAR;
  if (!strcmp (cmd, "trilinear"))	return DEBUGCMD_TRILINEAR;
  if (!strcmp (cmd, "lighting"))	return DEBUGCMD_LIGHTING;
  if (!strcmp (cmd, "gouraud"))		return DEBUGCMD_GOURAUD;
  if (!strcmp (cmd, "ilace"))		return DEBUGCMD_ILACE;
  if (!strcmp (cmd, "mmx"))		return DEBUGCMD_MMX;
  if (!strcmp (cmd, "transp"))		return DEBUGCMD_TRANSP;
  if (!strcmp (cmd, "gamma"))		return DEBUGCMD_GAMMA;
  if (!strcmp (cmd, "dumpcam"))		return DEBUGCMD_DUMPCAM;
  if (!strcmp (cmd, "fov"))		return DEBUGCMD_FOV;
  if (!strcmp (cmd, "fovangle"))	return DEBUGCMD_FOVANGLE;
  if (!strcmp (cmd, "terrvis"))		return DEBUGCMD_TERRVIS;
  if (!strcmp (cmd, "meshbbox"))	return DEBUGCMD_MESHBBOX;
  if (!strcmp (cmd, "meshrad"))		return DEBUGCMD_MESHRAD;
  if (!strcmp (cmd, "meshcd"))		return DEBUGCMD_MESHCDMESH;
  if (!strcmp (cmd, "meshvis"))		return DEBUGCMD_MESHVISMESH;
  if (!strcmp (cmd, "meshshad"))	return DEBUGCMD_MESHSHADMESH;
  if (!strcmp (cmd, "meshbase"))	return DEBUGCMD_MESHBASEMESH;
  if (!strcmp (cmd, "debuggraph"))	return DEBUGCMD_DEBUGGRAPH;
  if (!strcmp (cmd, "enginecmd"))	return DEBUGCMD_ENGINECMD;
  if (!strcmp (cmd, "enginestate"))	return DEBUGCMD_ENGINESTATE;
  if (!strcmp (cmd, "visculview"))	return DEBUGCMD_VISCULVIEW;
  if (!strcmp (cmd, "visculcmd"))	return DEBUGCMD_VISCULCMD;
  if (!strcmp (cmd, "debugsector"))	return DEBUGCMD_DEBUGSECTOR;
  if (!strcmp (cmd, "ds_forward"))	return DEBUGCMD_DS_FORWARD;
  if (!strcmp (cmd, "ds_backward"))	return DEBUGCMD_DS_BACKWARD;
  if (!strcmp (cmd, "ds_up"))		return DEBUGCMD_DS_UP;
  if (!strcmp (cmd, "ds_down"))		return DEBUGCMD_DS_DOWN;
  if (!strcmp (cmd, "ds_turnleft"))	return DEBUGCMD_DS_TURNLEFT;
  if (!strcmp (cmd, "ds_turnright"))	return DEBUGCMD_DS_TURNRIGHT;
  if (!strcmp (cmd, "ds_left"))		return DEBUGCMD_DS_LEFT;
  if (!strcmp (cmd, "ds_right"))	return DEBUGCMD_DS_RIGHT;
  if (!strcmp (cmd, "debugview"))	return DEBUGCMD_DEBUGVIEW;
  if (!strcmp (cmd, "scrshot"))		return DEBUGCMD_SCRSHOT;
  if (!strcmp (cmd, "savemap"))		return DEBUGCMD_SAVEMAP;
  if (!strcmp (cmd, "fps"))		return DEBUGCMD_FPS;
  if (!strcmp (cmd, "hideselected"))	return DEBUGCMD_HIDESELECTED;
  if (!strcmp (cmd, "undohide"))	return DEBUGCMD_UNDOHIDE;
  if (!strcmp (cmd, "counterreset"))	return DEBUGCMD_COUNTERRESET;
  if (!strcmp (cmd, "counterfreeze"))	return DEBUGCMD_COUNTERFREEZE;
  if (!strcmp (cmd, "counterremove"))	return DEBUGCMD_COUNTERREMOVE;
  if (!strcmp (cmd, "shadowdebug"))	return DEBUGCMD_SHADOWDEBUG;
  if (!strcmp (cmd, "debugcmd"))	return DEBUGCMD_DEBUGCMD;
  if (!strcmp (cmd, "memorydump"))	return DEBUGCMD_MEMORYDUMP;
  if (!strcmp (cmd, "colorsectors"))	return DEBUGCMD_COLORSECTORS;
  if (!strcmp (cmd, "switchculler"))	return DEBUGCMD_SWITCHCULLER;
  if (!strcmp (cmd, "selectmesh"))	return DEBUGCMD_SELECTMESH;
  if (!strcmp (cmd, "onesector"))	return DEBUGCMD_ONESECTOR;
  if (!strcmp (cmd, "mesh_xmin"))	return DEBUGCMD_MESH_XMIN;
  if (!strcmp (cmd, "mesh_xplus"))	return DEBUGCMD_MESH_XPLUS;
  if (!strcmp (cmd, "mesh_ymin"))	return DEBUGCMD_MESH_YMIN;
  if (!strcmp (cmd, "mesh_yplus"))	return DEBUGCMD_MESH_YPLUS;
  if (!strcmp (cmd, "mesh_zmin"))	return DEBUGCMD_MESH_ZMIN;
  if (!strcmp (cmd, "mesh_zplus"))	return DEBUGCMD_MESH_ZPLUS;
  if (!strcmp (cmd, "listplugins"))	return DEBUGCMD_LISTPLUGINS;
  if (!strcmp (cmd, "prof_log"))	return DEBUGCMD_PROFTOGGLELOG;
  if (!strcmp (cmd, "prof_autoreset"))	return DEBUGCMD_PROFAUTORESET;
  if (!strcmp (cmd, "uberscreenshot"))	return DEBUGCMD_UBERSCREENSHOT;
  if (!strcmp (cmd, "meshnorm"))	return DEBUGCMD_MESHNORM;
  if (!strcmp (cmd, "toggle_fps_time")) return DEBUGCMD_TOGGLEFPSTIME;
  if (!strcmp (cmd, "meshskel"))        return DEBUGCMD_MESHSKEL;
  if (!strcmp (cmd, "printportals"))    return DEBUGCMD_PRINTPORTALS;

  return DEBUGCMD_UNKNOWN;
}

int csBugPlug::GetCommandCode (int key, bool shift, bool alt, bool ctrl,
	csString& args)
{
  csKeyMap* m = mappings;
  while (m)
  {
    if (m->key == key && m->shift == shift && m->alt == alt && m->ctrl == ctrl)
    {
      args = m->args;
      return m->cmd;
    }
    m = m->next;
  }
  args = 0;
  return DEBUGCMD_UNKNOWN;
}

void csBugPlug::AddCommand (const char* keystring, const char* cmdstring)
{
  bool shift, alt, ctrl;
  int keycode = GetKeyCode (keystring, shift, alt, ctrl);
  // Check if valid key name.
  if (keycode == -1) return;

  csString args;
  int cmdcode = GetCommandCode (cmdstring, args);
  // Check if valid command name.
  if (cmdcode == DEBUGCMD_UNKNOWN) return;

  // Check if key isn't already defined.
  csString args2;
  if (GetCommandCode (keycode, shift, alt, ctrl, args2) != DEBUGCMD_UNKNOWN)
    return;

  // Make new key assignment.
  csKeyMap* map = new csKeyMap ();
  map->key = keycode;
  map->shift = shift;
  map->alt = alt;
  map->ctrl = ctrl;
  map->cmd = cmdcode;
  map->next = mappings;
  if (mappings) mappings->prev = map;
  map->prev = 0;
  if (!args.IsEmpty())
    map->args = csStrNew (args);
  else
    map->args = 0;
  mappings = map;
}

bool csBugPlug::ReadLine (iFile* file, char* buf, int nbytes)
{
  if (!file)
    return false;

  char c = '\n';
  while (c == '\n' || c == '\r')
    if (!file->Read (&c, 1))
      break;

  if (file->AtEOF())
    return false;

  char* p = buf;
  const char* plim = p + nbytes - 1;
  while (p < plim)
  {
    if (c == '\n' || c == '\r')
      break;
    *p++ = c;
    if (!file->Read (&c, 1))
      break;
  }
  *p = '\0';
  return true;
}

void csBugPlug::ReadKeyBindings (const char* filename)
{
  csRef<iFile> f (VFS->Open (filename, VFS_FILE_READ));
  if (f)
  {
    char buf[256];
    while (ReadLine (f, buf, 255))
    {
      buf[255] = 0;
      char* del = strchr (buf, '=');
      if (del)
      {
        *del = 0;
	AddCommand (buf, del+1);
      }
      else
      {
        Report (CS_REPORTER_SEVERITY_WARNING,
    	  "BugPlug hit a badly formed line in '%s'!", filename);
        return;
      }
    }
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
    	"BugPlug could not read '%s'!", filename);
  }
}

void csBugPlug::Dump (iEngine* engine)
{
  Report (CS_REPORTER_SEVERITY_DEBUG,
  	"===========================================");
  iTextureList* txts = engine->GetTextureList ();
  iMaterialList* mats = engine->GetMaterialList ();
  iSectorList* sectors = engine->GetSectors ();
  iMeshList* meshes = engine->GetMeshes ();
  iMeshFactoryList* factories = engine->GetMeshFactories ();
  Report (CS_REPORTER_SEVERITY_DEBUG,
    "%d textures, %d materials, %d sectors, %d mesh factories, %d mesh objects",
    txts->GetCount (),
    mats->GetCount (),
    sectors->GetCount (),
    factories->GetCount (),
    meshes->GetCount ());
  int i;
  for (i = 0 ; i < txts->GetCount () ; i++)
  {
    iTextureWrapper* txt = txts->Get (i);
    Report (CS_REPORTER_SEVERITY_DEBUG, "texture %d '%s'", i,
    	txt->QueryObject ()->GetName ());
  }
  for (i = 0 ; i < mats->GetCount () ; i++)
  {
    iMaterialWrapper* mat = mats->Get (i);
    Report (CS_REPORTER_SEVERITY_DEBUG, "material %d '%s'", i,
    	mat->QueryObject ()->GetName ());
  }
  for (i = 0 ; i < sectors->GetCount () ; i++)
  {
    iSector* sector = sectors->Get (i);
    Dump (sector);
  }
  for (i = 0 ; i < factories->GetCount () ; i++)
  {
    iMeshFactoryWrapper* meshfact = factories->Get (i);
    Dump (meshfact);
  }
  for (i = 0 ; i < meshes->GetCount () ; i++)
  {
    iMeshWrapper* mesh = meshes->Get (i);
    Dump (0, mesh);
  }
  Report (CS_REPORTER_SEVERITY_DEBUG,
  	"===========================================");
}

void csBugPlug::Dump (iSector* sector)
{
  const char* sn = sector->QueryObject ()->GetName ();
  Report (CS_REPORTER_SEVERITY_DEBUG, "    Sector '%s' (%08p)",
  	sn ? sn : "?", sector);
  Report (CS_REPORTER_SEVERITY_DEBUG, "    %d meshes, %d lights",
  	sector->GetMeshes ()->GetCount (),
	sector->GetLights ()->GetCount ());
  int i;
  for (i = 0 ; i < sector->GetMeshes ()->GetCount () ; i++)
  {
    iMeshWrapper* mesh = sector->GetMeshes ()->Get (i);
    const char* n = mesh->QueryObject ()->GetName ();
    Report (CS_REPORTER_SEVERITY_DEBUG, "        Mesh '%s' (%08p)",
    	n ? n : "?", mesh);
  }
}

void csBugPlug::Dump (int indent, iMeshWrapper* mesh)
{
  const char* mn = mesh->QueryObject ()->GetName ();
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*s    Mesh wrapper '%s' (%08p)", 
    indent, "", mn ? mn : "?", mesh);
  iMeshObject* obj = mesh->GetMeshObject ();
  if (!obj)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG, "%*s        Mesh object missing!",
      indent, "");
  }
  else
  {
    csRef<iFactory> fact (scfQueryInterface<iFactory> (obj));
    if (fact)
      Report (CS_REPORTER_SEVERITY_DEBUG, "%*s        Plugin '%s'",
  	  indent, "",
          fact->QueryDescription () ? fact->QueryDescription () : "0");
    const csBox3& bbox = obj->GetObjectModel ()->GetObjectBoundingBox ();
    Report (CS_REPORTER_SEVERITY_DEBUG, "%*s        Object bounding box:",
      indent, "");
    Dump (indent+8, bbox);
  }
  iMovable* movable = mesh->GetMovable ();
  if (!movable)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG, "%*s        Mesh object missing!",
      indent, "");
  }
  else
  {
    csReversibleTransform& trans = movable->GetTransform ();
    Dump (indent+8, trans.GetOrigin (), "Movable origin");
    Dump (indent+8, trans.GetO2T (), "Movable O2T");
    int cnt = movable->GetSectors ()->GetCount ();
    int i;
    for (i = 0 ; i < cnt ; i++)
    {
      iSector* sec = movable->GetSectors ()->Get (i);
      const char* sn = sec->QueryObject ()->GetName ();
      Report (CS_REPORTER_SEVERITY_DEBUG, "%*s        In sector '%s'",
      	indent, "", sn ? sn : "?");
    }
  }
  csRef<iSceneNodeArray> children = mesh->QuerySceneNode ()->GetChildrenArray ();
  for (size_t i=0; i<children->GetSize (); ++i)
  {
    iMeshWrapper* m = children->Get(i)->QueryMesh ();
    if (m)
      Dump (indent+4, m);
  }
}

void csBugPlug::Dump (iMeshFactoryWrapper* meshfact)
{
  const char* mn = meshfact->QueryObject ()->GetName ();
  Report (CS_REPORTER_SEVERITY_DEBUG, "        Mesh factory wrapper '%s' (%08p)",
  	mn ? mn : "?", meshfact);
}

void csBugPlug::Dump (int indent, const csMatrix3& m, char const* name)
{
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*sMatrix '%s':", indent, "", name);
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*s/", indent, "");
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*s| %3.2f %3.2f %3.2f", indent,
  	"", m.m11, m.m12, m.m13);
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*s| %3.2f %3.2f %3.2f", indent,
  	"", m.m21, m.m22, m.m23);
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*s| %3.2f %3.2f %3.2f", indent,
  	"", m.m31, m.m32, m.m33);
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*s\\", indent, "");
}

void csBugPlug::Dump (int indent, const csVector3& v, char const* name)
{
  Report (CS_REPORTER_SEVERITY_DEBUG,
  	"%*sVector '%s': (%f,%f,%f)", indent, "", name, v.x, v.y, v.z);
}

void csBugPlug::Dump (int indent, const csVector2& v, char const* name)
{
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*sVector '%s': (%f,%f)",
  	indent, "", name, v.x, v.y);
}

void csBugPlug::Dump (int indent, const csPlane3& p)
{
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*sA=%2.2f B=%2.2f C=%2.2f D=%2.2f",
            indent, "", p.norm.x, p.norm.y, p.norm.z, p.DD);
}

void csBugPlug::Dump (int indent, const csBox2& b)
{
  char ind[255];
  int i;
  for (i = 0 ; i < indent ; i++) ind[i] = ' ';
  ind[i] = 0;
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*s(%2.2f,%2.2f)-(%2.2f,%2.2f)", indent, "",
  	b.MinX (), b.MinY (), b.MaxX (), b.MaxY ());
}

void csBugPlug::Dump (int indent, const csBox3& b)
{
  char ind[255];
  int i;
  for (i = 0 ; i < indent ; i++) ind[i] = ' ';
  ind[i] = 0;
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*s(%2.2f,%2.2f,%2.2f)-(%2.2f,%2.2f,%2.2f)",
  	indent, "", b.MinX (), b.MinY (), b.MinZ (), b.MaxX (), b.MaxY (), b.MaxZ ());
}

void csBugPlug::Dump (iCamera* c)
{
  const char* sn = c->GetSector ()->QueryObject ()->GetName ();
  if (!sn) sn = "?";
  csPlane3* far_plane = c->GetFarPlane ();
  csRef<iPerspectiveCamera> pcamera =
    scfQueryInterface<iPerspectiveCamera> (catcher->camera);

  if (pcamera)
    {
      Report (CS_REPORTER_SEVERITY_DEBUG,
	      "Camera: %s (mirror=%d, fov=%g, fovangle=%g,",
	      sn, (int)c->IsMirrored (), pcamera->GetFOV (), pcamera->GetFOVAngle ());
      Report (CS_REPORTER_SEVERITY_DEBUG, "    shiftx=%g shifty=%g camnr=%ld)",
	      pcamera->GetShiftX (), pcamera->GetShiftY (), c->GetCameraNumber ());
    }

  else
    {
      Report (CS_REPORTER_SEVERITY_DEBUG,
	      "Camera: %s (mirror=%d, ", sn, (int)c->IsMirrored ());
      Report (CS_REPORTER_SEVERITY_DEBUG, "    camnr=%ld)",
	      c->GetCameraNumber ());
    }

  if (far_plane)
    Report (CS_REPORTER_SEVERITY_DEBUG, "    far_plane=(%g,%g,%g,%g)",
    	far_plane->A (), far_plane->B (), far_plane->C (), far_plane->D ());
  csReversibleTransform& trans = c->GetTransform ();
  Dump (4, trans.GetO2TTranslation (), "Camera vector");
  Dump (4, trans.GetO2T (), "Camera matrix");
}

bool csBugPlug::HandleEvent (iEvent& event)
{
  if (CS_IS_KEYBOARD_EVENT(object_reg, event))
    return EatKey (event);
  else if (CS_IS_MOUSE_EVENT(object_reg, event))
    return EatMouse (event);
  else if (event.Name == Frame)
    return HandleFrame (event);
  else if (event.Name == SystemOpen)
    return HandleSystemOpen (&event);
  else if (event.Name == SystemClose)
    return HandleSystemClose (&event);

  return false;
}


/* We want to handle frame event in the DEBUG phase,
   and input (key/mouse) events before the renderer or printer */

const csHandlerID * csBugPlug::GenericPrec(
	csRef<iEventHandlerRegistry> &handler_reg,
	csRef<iEventNameRegistry> &name_reg,
	csEventID e) const
{
  static csHandlerID Constraints[2]; // TODO : this is not thread-safe/reentrant

  Constraints[0] = FrameSignpost_ConsoleDebug::StaticID (handler_reg);
  Constraints[1] = CS_HANDLERLIST_END;
  
  if (name_reg->IsKindOf(e, csevFrame (name_reg)))
  {
    return Constraints;
  }
  else
  {
    return NULL;
  }
}

const csHandlerID * csBugPlug::GenericSucc(
	csRef<iEventHandlerRegistry> &handler_reg,
	csRef<iEventNameRegistry> &name_reg,
	csEventID e) const
{
  static csHandlerID Constraints[2][3]; // TODO : this is not thread-safe/reentrant
  Constraints[0][0] = handler_reg->GetGenericID("crystalspace.graphics3d");
  Constraints[0][1] = handler_reg->GetGenericID("crystalspace.window");
  Constraints[0][2] = CS_HANDLERLIST_END;

  Constraints[1][0] = FrameSignpost_DebugFrame::StaticID (handler_reg);
  Constraints[1][1] = CS_HANDLERLIST_END;
  
  if (name_reg->IsKindOf(e, csevKeyboardEvent (name_reg)) || 
      name_reg->IsKindOf(e, csevMouseEvent (name_reg)))
  {
    return Constraints[0];
  }
  else if (name_reg->IsKindOf(e, csevFrame (name_reg)))
  {
    return Constraints[1];
  }
  else
  {
    return NULL;
  }
}


//---------------------------------------------------------------------------

void csBugPlug::OneSector (iCamera* camera)
{
  iSector* one = Engine->FindSector ("bugplug_one_sector");
  if (!one)
  {
    int i;
    iSectorList* sl = Engine->GetSectors ();
    one = Engine->CreateSector ("bugplug_one_sector");
    for (i = 0 ; i < sl->GetCount () ; i++)
    {
      iSector* sec = sl->Get (i);
      if (sec != one)
      {
        iMeshList* ml = sec->GetMeshes ();
	int j;
	for (j = 0 ; j < ml->GetCount () ; j++)
	{
	  iMeshWrapper* m = ml->Get (j);
	  if (!m->GetPortalContainer ())
	  {
	    m->GetMovable ()->GetSectors ()->Add (one);
	    m->GetMovable ()->UpdateMove ();
	  }
	}
      }
    }
  }
  camera->SetSector (one);
}

//---------------------------------------------------------------------------

void csBugPlug::CleanDebugSector ()
{
  if (!debug_sector.sector) return;
  Engine->RemoveCollection ("__BugPlug_region__");

  delete debug_sector.view;

  debug_sector.sector = 0;
  debug_sector.view = 0;
}

void csBugPlug::SetupDebugSector ()
{
  CleanDebugSector ();
  if (!Engine)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG, "There is no engine!");
    return;
  }

  iCollection* db_collection = Engine->CreateCollection ("__BugPlug_collection__");
  debug_sector.sector = Engine->CreateSector ("__BugPlug_sector__");
  db_collection->Add (debug_sector.sector->QueryObject ());

  debug_sector.view = new csView (Engine, G3D);
  int w3d = G3D->GetWidth ();
  int h3d = G3D->GetHeight ();
  debug_sector.view->SetRectangle (0, 0, w3d, h3d);
  debug_sector.view->GetCamera ()->SetSector (debug_sector.sector);
}

iMaterialWrapper* csBugPlug::FindColor (float r, float g, float b)
{
  // Assumes the current region is the debug region.
  csString name;
  name.Format ("mat%d,%d,%d\n", int (r*255), int (g*255), int (b*255));
  iMaterialWrapper* mw = Engine->FindMaterial (name);
  if (mw) return mw;
  // Create a new material.
  csRef<iMaterial> mat (Engine->CreateBaseMaterial (0));

  // Attach a new SV to it
  csShaderVariable* var = mat->GetVariableAdd (stringSetSvName->Request (CS_MATERIAL_VARNAME_FLATCOLOR));
  var->SetValue (csColor (r,g,b));

  mw = Engine->GetMaterialList ()->NewMaterial (mat, name);
  return mw;
}

void csBugPlug::DebugSectorBox (const csBox3& box, float r, float g, float b,
  	const char* name, iMeshObject*, uint mixmode)
{
  if (!debug_sector.sector) return;

  iMaterialWrapper* mat = FindColor (r, g, b);
  // Create the box and add it to the engine.
  csVector3 pos = box.GetCenter ();
  csBox3 tbox;
  tbox.Set (box.Min ()-pos, box.Max ()-pos);

  csRef<iMeshFactoryWrapper> mf (Engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", name ? name : "__BugPlug_fact__"));
  csRef<iGeneralFactoryState> gfs (
  	
  	scfQueryInterface<iGeneralFactoryState> (mf->GetMeshObjectFactory ()));
  CS_ASSERT (gfs != 0);
  mf->GetMeshObjectFactory ()->SetMaterialWrapper (mat);
  gfs->GenerateBox (tbox);
  gfs->CalculateNormals ();
  gfs->GetColors ()[0].Set (1, 1, 1);
  gfs->GetColors ()[1].Set (0.5f, 0.8f, 0.5f);
  gfs->GetColors ()[2].Set (0.5f, 0.3f, 0.8f);
  gfs->GetColors ()[3].Set (0.8f, 0.8f, 0.8f);
  gfs->GetColors ()[4].Set (0.9f, 0.4f, 0.4f);
  gfs->GetColors ()[5].Set (0.2f, 0.8f, 0.7f);
  gfs->GetColors ()[6].Set (0.9f, 0.9f, 0.9f);
  gfs->GetColors ()[7].Set (0.6f, 0.6f, 0.6f);
  gfs->GetColors ()[8].Set (1, 1, 1);
  gfs->GetColors ()[9].Set (0.5f, 0.8f, 0.5f);
  gfs->GetColors ()[10].Set (0.5f, 0.3f, 0.8f);
  gfs->GetColors ()[11].Set (0.8f, 0.8f, 0.8f);
  gfs->GetColors ()[12].Set (0.9f, 0.4f, 0.4f);
  gfs->GetColors ()[13].Set (0.2f, 0.8f, 0.7f);
  gfs->GetColors ()[14].Set (0.9f, 0.9f, 0.9f);
  gfs->GetColors ()[15].Set (0.6f, 0.6f, 0.6f);
  gfs->GetColors ()[16].Set (1, 1, 1);
  gfs->GetColors ()[17].Set (0.5f, 0.8f, 0.5f);
  gfs->GetColors ()[18].Set (0.5f, 0.3f, 0.8f);
  gfs->GetColors ()[19].Set (0.8f, 0.8f, 0.8f);
  gfs->GetColors ()[20].Set (0.9f, 0.4f, 0.4f);
  gfs->GetColors ()[21].Set (0.2f, 0.8f, 0.7f);
  gfs->GetColors ()[22].Set (0.9f, 0.9f, 0.9f);
  gfs->GetColors ()[23].Set (0.6f, 0.6f, 0.6f);

  csRef<iMeshWrapper> mw (Engine->CreateMeshWrapper (
  	mf, name ? name : "__BugPlug_mesh__", debug_sector.sector, pos));
  csRef<iGeneralMeshState> gms (
  	scfQueryInterface<iGeneralMeshState> (mw->GetMeshObject ()));
  CS_ASSERT (gms != 0);
  gms->SetManualColors (true);
  mw->GetMeshObject ()->SetColor (csColor (0, 0, 0));
  mw->GetMeshObject ()->SetMixMode (mixmode);

  if (mixmode == CS_FX_COPY)
  {
    mw->SetZBufMode (CS_ZBUF_USE);
    mw->SetRenderPriority (Engine->GetObjectRenderPriority ());
  }
  else
  {
    mw->SetZBufMode (CS_ZBUF_TEST);
    mw->SetRenderPriority (Engine->GetAlphaRenderPriority ());
  }
}

void csBugPlug::DebugSectorTriangle (const csVector3& s1, const csVector3& s2,
  	const csVector3& s3, float r, float g, float b,
	uint mixmode)
{
  if (!debug_sector.sector) return;

  iMaterialWrapper* mat = FindColor (r, g, b);
  // Create the box and add it to the engine.
  csVector3 pos = s1;
  csVector3 ss1 (0);
  csVector3 ss2 = s2-s1;
  csVector3 ss3 = s3-s1;

  csRef<iMeshFactoryWrapper> mf (Engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "__BugPlug_tri__"));
  csRef<iGeneralFactoryState> gfs (
  	
  	scfQueryInterface<iGeneralFactoryState> (mf->GetMeshObjectFactory ()));
  CS_ASSERT (gfs != 0);
  mf->GetMeshObjectFactory ()->SetMaterialWrapper (mat);
  gfs->SetVertexCount (3);
  gfs->GetVertices ()[0] = ss1;
  gfs->GetVertices ()[1] = ss2;
  gfs->GetVertices ()[2] = ss3;
  gfs->GetTexels ()[0].Set (0, 0);
  gfs->GetTexels ()[1].Set (1, 0);
  gfs->GetTexels ()[2].Set (0, 1);
  gfs->SetTriangleCount (2);
  gfs->GetTriangles ()[0].a = 0;
  gfs->GetTriangles ()[0].b = 1;
  gfs->GetTriangles ()[0].c = 2;
  gfs->GetTriangles ()[1].a = 2;
  gfs->GetTriangles ()[1].b = 1;
  gfs->GetTriangles ()[1].c = 0;

  gfs->CalculateNormals ();
  gfs->GetColors ()[0].Set (1, 1, 1);
  gfs->GetColors ()[1].Set (0, 0, 0);
  gfs->GetColors ()[2].Set (0, 0, 0);

  csRef<iMeshWrapper> mw (Engine->CreateMeshWrapper (
  	mf, "__BugPlug_tri__", debug_sector.sector, pos));
  csRef<iGeneralMeshState> gms (
  	scfQueryInterface<iGeneralMeshState> (mw->GetMeshObject ()));
  CS_ASSERT (gms != 0);
  gms->SetManualColors (true);
  mw->GetMeshObject ()->SetColor (csColor (0, 0, 0));
  mw->GetMeshObject ()->SetMixMode (mixmode);

  if (mixmode == CS_FX_COPY)
  {
    mw->SetZBufMode (CS_ZBUF_USE);
    mw->SetRenderPriority (Engine->GetObjectRenderPriority ());
  }
  else
  {
    mw->SetZBufMode (CS_ZBUF_TEST);
    mw->SetRenderPriority (Engine->GetAlphaRenderPriority ());
  }
}

void csBugPlug::DebugSectorMesh (
	csVector3* vertices, int vertex_count,
	csTriangle* triangles, int tri_count,
	float r, float g, float b, uint mixmode)
{
  if (!debug_sector.sector) return;

  iMaterialWrapper* mat = FindColor (r, g, b);
  // Create the box and add it to the engine.
  csVector3 pos = vertices[0];

  csRef<iMeshFactoryWrapper> mf (Engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "__BugPlug_mesh__"));
  csRef<iGeneralFactoryState> gfs (
  	
  	scfQueryInterface<iGeneralFactoryState> (mf->GetMeshObjectFactory ()));
  CS_ASSERT (gfs != 0);
  mf->GetMeshObjectFactory ()->SetMaterialWrapper (mat);
  gfs->SetVertexCount (vertex_count);
  int i;
  for (i = 0 ; i < vertex_count ; i++)
  {
    gfs->GetVertices ()[i] = vertices[i]-pos;
    switch (i%9)
    {
      case 0: gfs->GetTexels ()[i].Set (0, 0); break;
      case 1: gfs->GetTexels ()[i].Set (1, 0); break;
      case 2: gfs->GetTexels ()[i].Set (1, 1); break;
      case 3: gfs->GetTexels ()[i].Set (0, 1); break;
      case 4: gfs->GetTexels ()[i].Set (.5, .5); break;
      case 5: gfs->GetTexels ()[i].Set (0, .5); break;
      case 6: gfs->GetTexels ()[i].Set (1, .5); break;
      case 7: gfs->GetTexels ()[i].Set (.5, 0); break;
      case 8: gfs->GetTexels ()[i].Set (.5, 1); break;
    }
    gfs->GetColors ()[i].Set (1, 1, 1);
  }
  gfs->SetTriangleCount (tri_count);
  for (i = 0 ; i < tri_count ; i++)
  {
    gfs->GetTriangles ()[i] = triangles[i];
  }

  gfs->CalculateNormals ();

  csRef<iMeshWrapper> mw (Engine->CreateMeshWrapper (
  	mf, "__BugPlug_mesh__", debug_sector.sector, pos));
  csRef<iGeneralMeshState> gms (
  	scfQueryInterface<iGeneralMeshState> (mw->GetMeshObject ()));
  CS_ASSERT (gms != 0);
  gms->SetManualColors (true);
  mw->GetMeshObject ()->SetColor (csColor (0, 0, 0));
  mw->GetMeshObject ()->SetMixMode (mixmode);

  if (mixmode == CS_FX_COPY)
  {
    mw->SetZBufMode (CS_ZBUF_USE);
    mw->SetRenderPriority (Engine->GetObjectRenderPriority ());
  }
  else
  {
    mw->SetZBufMode (CS_ZBUF_TEST);
    mw->SetRenderPriority (Engine->GetAlphaRenderPriority ());
  }
}

void csBugPlug::SwitchDebugSector (const csReversibleTransform& trans,
	bool clear)
{
  if (!debug_sector.sector)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG, "There is no debug sector!");
    return;
  }
  debug_sector.show = !debug_sector.show;
  debug_sector.clear = clear;
  if (debug_sector.show)
  {
    debug_sector.view->GetCamera ()->SetTransform (trans);
    debug_view.show = false;
  }
}

//---------------------------------------------------------------------------

void csBugPlug::CleanDebugView ()
{
  delete[] debug_view.lines;
  debug_view.lines = 0;
  debug_view.num_lines = 0;
  debug_view.max_lines = 0;
  delete[] debug_view.boxes;
  debug_view.boxes = 0;
  debug_view.num_boxes = 0;
  debug_view.max_boxes = 0;
  delete[] debug_view.points;
  debug_view.points = 0;
  debug_view.num_points = 0;
  debug_view.max_points = 0;
  if (debug_view.object)
  {
    debug_view.object->DecRef ();
    debug_view.object = 0;
  }
}

void csBugPlug::SetupDebugView ()
{
  CleanDebugView ();
}

int csBugPlug::DebugViewPoint (const csVector2& point)
{
  if (debug_view.num_points >= debug_view.max_points)
  {
    debug_view.max_points += 50;
    csVector2* new_points = new csVector2 [debug_view.max_points];
    if (debug_view.num_points > 0)
    {
      memcpy (new_points, debug_view.points,
      	sizeof (csVector2)*debug_view.num_points);
      delete[] debug_view.points;
    }
    debug_view.points = new_points;
  }
  debug_view.points[debug_view.num_points++] = point;
  return debug_view.num_points-1;
}

void csBugPlug::DebugViewLine (int i1, int i2)
{
  if (debug_view.num_lines >= debug_view.max_lines)
  {
    debug_view.max_lines += 30;
    dbLine* new_lines = new dbLine [debug_view.max_lines];
    if (debug_view.num_lines > 0)
    {
      memcpy (new_lines, debug_view.lines,
      	sizeof (dbLine)*debug_view.num_lines);
      delete[] debug_view.lines;
    }
    debug_view.lines = new_lines;
  }
  debug_view.lines[debug_view.num_lines].i1 = i1;
  debug_view.lines[debug_view.num_lines].i2 = i2;
  debug_view.num_lines++;
}

void csBugPlug::DebugViewBox (int i1, int i2)
{
  if (debug_view.num_boxes >= debug_view.max_boxes)
  {
    debug_view.max_boxes += 30;
    dbLine* new_boxes = new dbLine [debug_view.max_boxes];
    if (debug_view.num_boxes > 0)
    {
      memcpy (new_boxes, debug_view.boxes,
      	sizeof (dbLine)*debug_view.num_boxes);
      delete[] debug_view.boxes;
    }
    debug_view.boxes = new_boxes;
  }
  debug_view.boxes[debug_view.num_boxes].i1 = i1;
  debug_view.boxes[debug_view.num_boxes].i2 = i2;
  debug_view.num_boxes++;
}

void csBugPlug::DebugViewRenderObject (iBugPlugRenderObject* obj)
{
  if (obj) obj->IncRef ();
  if (debug_view.object) debug_view.object->DecRef ();
  debug_view.object = obj;
}

void csBugPlug::SwitchDebugView (bool clear)
{
  debug_view.show = !debug_view.show;
  debug_sector.clear = clear;
  if (debug_view.show)
  {
    debug_sector.show = false;
    debug_view.drag_point = -1;
  }
}

int csBugPlug::FindCounter (const char* countername)
{
  size_t i;
  for (i = 0 ; i < counters.GetSize () ; i++)
    if (!strcmp (counters[i]->countername, countername))
      return (int)i;
  return -1;
}

void csBugPlug::FullResetCounters ()
{
  size_t i;
  for (i = 0 ; i < counters.GetSize () ; i++)
  {
    int j;
    for (j = 0 ; j < 10 ; j++)
    {
      counters[i]->values[j].total = 0;
      counters[i]->values[j].current = 0;
    }
  }
  counter_frames = 0;
}

void csBugPlug::ShowCounters ()
{
  if (counters.GetSize () == 0) return;

  G3D->BeginDraw (CSDRAW_2DGRAPHICS);
  if (!fnt) return;

  int fw, fh;
  fnt->GetMaxSize (fw, fh);
  int sh = G2D->GetHeight ();
  int bgcolor = G2D->FindRGB (255, 255, 255);
  int fgcolor = G2D->FindRGB (0, 0, 0);

  if (!counter_freeze) counter_frames++;
  size_t i;
  int cur_y = 10;
  for (i = 0 ; i < counters.GetSize () ; i++)
  {
    csCounter* c = counters[i];
    int j;
    for (j = 0 ; j < 10 ; j++)
    {
      c->values[j].total += c->values[j].current;
    }
    if (c->is_enum)
    {
      float tottotal = 0;
      float totcurrent = 0;
      for (j = 0 ; j < 10 ; j++)
      {
        tottotal += c->values[j].total;
	totcurrent += float (c->values[j].current);
      }
      if (totcurrent == 0) totcurrent = 1;
      GfxWrite (G2D, fnt, 10, cur_y, fgcolor, bgcolor,
    	"%s: %3.0f %3.0f %3.0f %3.0f %3.0f %3.0f %3.0f %3.0f %3.0f %3.0f / %3.0f %3.0f %3.0f %3.0f %3.0f %3.0f %3.0f %3.0f %3.0f %3.0f",
	c->countername,
	c->values[0].total * 100.0 / tottotal,
	c->values[1].total * 100.0 / tottotal,
	c->values[2].total * 100.0 / tottotal,
	c->values[3].total * 100.0 / tottotal,
	c->values[4].total * 100.0 / tottotal,
	c->values[5].total * 100.0 / tottotal,
	c->values[6].total * 100.0 / tottotal,
	c->values[7].total * 100.0 / tottotal,
	c->values[8].total * 100.0 / tottotal,
	c->values[9].total * 100.0 / tottotal,
	c->values[0].current * 100.0 / totcurrent,
	c->values[1].current * 100.0 / totcurrent,
	c->values[2].current * 100.0 / totcurrent,
	c->values[3].current * 100.0 / totcurrent,
	c->values[4].current * 100.0 / totcurrent,
	c->values[5].current * 100.0 / totcurrent,
	c->values[6].current * 100.0 / totcurrent,
	c->values[7].current * 100.0 / totcurrent,
	c->values[8].current * 100.0 / totcurrent,
	c->values[9].current * 100.0 / totcurrent);
      for (j = 0 ; j < 10 ; j++)
      {
	c->values[j].current = 0;
      }
    }
    else
    {
      GfxWrite (G2D, fnt, 10, cur_y, fgcolor, bgcolor,
    	"%s: last=%d tot=%g avg=%g",
	c->countername,
	c->values[0].current, c->values[0].total,
	c->values[0].total / float (counter_frames));
      c->values[0].current = 0;
    }
    cur_y += fh+4;
    if (cur_y > sh-10) break;
  }
}

void csBugPlug::AddCounter (const char* countername, int amount)
{
  if (counter_freeze) return;
  int i = FindCounter (countername);
  if (i == -1)
  {
    csCounter* c = new csCounter ();
    c->is_enum = false;
    c->countername = csStrNew (countername);
    c->values[0].total = 0;
    c->values[0].current = amount;
    counters.Push (c);
  }
  else
  {
    counters[i]->is_enum = false;
    counters[i]->values[0].current += amount;
  }
}

void csBugPlug::AddCounterEnum (const char* countername, int enumval,
	int amount)
{
  if (counter_freeze) return;
  if (enumval < 0 || enumval > 9) return;
  int i = FindCounter (countername);
  if (i == -1)
  {
    csCounter* c = new csCounter ();
    c->is_enum = true;
    c->countername = csStrNew (countername);
    int j;
    for (j = 0 ; j < 10 ; j++)
    {
      c->values[j].total = 0;
      c->values[j].current = 0;
    }
    c->values[enumval].current = amount;
    counters.Push (c);
  }
  else
  {
    if (!counters[i]->is_enum)
    {
      int j;
      for (j = 0 ; j < 10 ; j++)
      {
        counters[i]->values[j].total = 0;
        counters[i]->values[j].current = 0;
      }
      counters[i]->is_enum = true;
    }
    counters[i]->values[enumval].current += amount;
  }
}

void csBugPlug::ResetCounter (const char* countername, int value)
{
  if (counter_freeze) return;
  int i = FindCounter (countername);
  if (i != -1)
  {
    int j;
    for (j = 0 ; j < 10 ; j++)
    {
      counters[i]->values[j].current = value;
    }
  }
}

void csBugPlug::RemoveCounter (const char* countername)
{
  int i = FindCounter (countername);
  if (i != -1)
    counters.DeleteIndex (i);
}

bool csBugPlug::ExecCommand (const char* command)
{
  csString args;
  int cmd = GetCommandCode (command, args);
  if (cmd == DEBUGCMD_UNKNOWN) return false;
  return ExecCommand (cmd, args);
}

void csBugPlug::SaveMap ()
{
  uint i = 0;
  csString name;

  bool exists = false;
  do
  {
    name.Format ("/this/world%u.xml", i);
    if ((exists = VFS->Exists (name))) i++;
  }
  while ((i > 0) && (exists));

  if ((i == 0) && (exists))
  {
    Report (CS_REPORTER_SEVERITY_DEBUG,
    	"Too many world files in current directory");
    return;
  }

  csRef<iSaver> saver = csLoadPluginCheck<iSaver> (object_reg,
  	"crystalspace.level.saver");
  if (saver)
    saver->SaveMapFile(name);
}

//---------------------------------------------------------------------------

}
CS_PLUGIN_NAMESPACE_END(BugPlug)
