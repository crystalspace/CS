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

#include <string.h>

#include "csutil/sysfunc.h"
#include "csutil/csuctransform.h"
#include "csutil/regexp.h"
#include "csutil/profile.h"
#include "csver.h"
#include "csutil/scf.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "csutil/debug.h"
#include "csutil/snprintf.h"
#include "csutil/event.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/box.h"
#include "bugplug.h"
#include "spider.h"
#include "shadow.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/string.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/dbghelp.h"
#include "iutil/virtclk.h"
#include "iutil/memdebug.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivideo/material.h"
#include "ivaria/conout.h"
#include "ivaria/reporter.h"
#include "ivaria/collider.h"
#include "iutil/object.h"
#include "imesh/object.h"
#include "imesh/terrfunc.h"
#include "imesh/thing.h"
#include "imesh/genmesh.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/viscull.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/region.h"
#include "iengine/material.h"
#include "iengine/rview.h"
#include "cstool/csview.h"
#include "cstool/collider.h"
#include "imap/saver.h"
#include "csqint.h"
#include "csqsqrt.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csBugPlug)


SCF_IMPLEMENT_IBASE (csBugPlug)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iBugPlug)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBugPlug::BugPlug)
  SCF_IMPLEMENTS_INTERFACE (iBugPlug)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBugPlug::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

void csBugPlug::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.bugplug", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

csBugPlug::csBugPlug (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiBugPlug);
  object_reg = 0;
  mappings = 0;
  visculler = 0;
  process_next_key = false;
  process_next_mouse = false;
  edit_mode = false;
  initialized = false;
  spider = new csSpider ();
  shadow = new csShadow ();
  spider_hunting = false;
  spider_args = 0;
  scfiEventHandler = 0;

  do_fps = true;
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

  captureFormat = 0;

  do_shadow_debug = false;
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
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
  delete[] captureFormat;
  delete[] spider_args;

  delete spider;
  delete shadow;

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiBugPlug);
  SCF_DESTRUCT_IBASE ();
}

bool csBugPlug::Initialize (iObjectRegistry *object_reg)
{
  csBugPlug::object_reg = object_reg;

  csRef<iKeyboardDriver> currentKbd = 
    CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (currentKbd == 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    return false;
  }
  keyComposer = currentKbd->CreateKeyComposer ();

  if (!scfiEventHandler)
  {
    scfiEventHandler = new EventHandler (this);
  }
  csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler,
    	CSMASK_Nothing | CSMASK_Keyboard |
  	CSMASK_MouseUp | CSMASK_MouseDown | CSMASK_MouseMove);
  return true;
}

void csBugPlug::SetupPlugin ()
{
  if (initialized) return;

  if (!Engine) Engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  if (!G3D) G3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

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

  if (!VFS) VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No VFS!");
    return;
  }

  if (!vc) vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (!vc)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No virtual clock!");
    return;
  }

  if (!Conout) Conout = CS_QUERY_REGISTRY (object_reg, iConsoleOutput);

  config.AddConfig (object_reg, "/config/bugplug.cfg");

  ReadKeyBindings (config->GetStr ("Bugplug.Keybindings", 
    "/config/bugplug.key"));

  captureFormat = csStrNew (config->GetStr ("Bugplug.Capture.FilenameFormat",
    "/this/cryst000.png"));
  // since this string is passed to format later,
  // replace all '%' with '%%'
  {
    char* newstr = new char[strlen(captureFormat)*2 + 1];
    memset (newstr, 0, strlen(captureFormat)*2 + 1);

    char* pos = captureFormat;
    while (pos)
    {
      char* percent = strchr (pos, '%');
      if (percent)
      {
	strncat (newstr, pos, percent-pos);
	strcat (newstr, "%%");
	pos = percent + 1;
      }
      else
      {
	strcat (newstr, pos);
	pos = 0;
      }
    }
    delete[] captureFormat;
    captureFormat = newstr;
  }
  // scan for the rightmost string of digits
  // and create an appropriate format string
  {
    captureFormatNumberMax = 1;
    int captureFormatNumberDigits = 0;

    char* end = strrchr (captureFormat, 0);
    if (end != captureFormat)
    {
      do
      {
	end--;
      }
      while ((end >= captureFormat) && (!isdigit (*end)));
      if (end >= captureFormat)
      {
	do
	{
	  captureFormatNumberMax *= 10;
	  captureFormatNumberDigits++; 
	  end--;
	}
	while ((end >= captureFormat) && (isdigit (*end)));
	
	char nameForm [6];
	cs_snprintf (nameForm, 6, "%%0%dd", captureFormatNumberDigits);

	size_t newlen = strlen(captureFormat)+strlen(nameForm)-
	  captureFormatNumberDigits+1;
	char* newCapForm = new char[newlen];
	memset (newCapForm, 0, newlen);
	strncpy (newCapForm, captureFormat, end-captureFormat+1);
	strcat (newCapForm, nameForm);
	strcat (newCapForm, end+captureFormatNumberDigits+1);
	delete[] captureFormat;
	captureFormat = newCapForm;
      }
    }
  }

  captureMIME = config->GetStr ("Bugplug.Capture.Image.MIME",
    "image/png");

  captureOptions = config->GetStr ("Bugplug.Capture.Image.Options");

  initialized = true;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "BugPlug loaded...");

  do_clear = false;
}

void csBugPlug::UnleashSpider (int cmd, const char* args)
{
  if (Engine)
  {
    spider->ClearCamera ();
    if (spider->WeaveWeb (Engine))
    {
      spider_command = cmd;
      spider_hunting = true;
      spider_timeout = 20;
      delete[] spider_args;
      spider_args = csStrNew (args);
    }
    else
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY,
	"Spider could not weave its web (No sectors)!");
    }
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Spider could not weave its web (No engine)!");
  }
}

void csBugPlug::SwitchCuller (iSector* sector, const char* culler)
{
  Report (CS_REPORTER_SEVERITY_NOTIFY,
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
    if (NoError == matcher.Match (mesh->QueryObject ()->GetName ()))
    {
      cnt++;
      AddSelectedMesh (mesh);
    }
  }
  if (cnt > 0)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
        "Selecting %d mesh(es).", cnt);
    bool bbox, rad;
    shadow->GetShowOptions (bbox, rad);

    if (bbox || rad || show_polymesh != BUGPLUG_POLYMESH_NO)
      shadow->AddToEngine (Engine);
    else
      shadow->RemoveFromEngine (Engine);
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Couldn't find matching meshes for pattern '%s'.", meshname);
  }
}

void csBugPlug::MoveSelectedMeshes (const csVector3& offset)
{
  size_t i;
  size_t count = selected_meshes.Length ();
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
  size_t count = selected_meshes.Length ();
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
  size_t count = selected_meshes.Length ();
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
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Bugplug is currently now tracking a visibility culler!");
    return;
  }
  csRef<iDebugHelper> dbghelp (SCF_QUERY_INTERFACE (visculler, iDebugHelper));
  if (!dbghelp)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "This visibility culler does not support iDebugHelper!");
    return;
  }
  if (dbghelp->DebugCommand (cmd))
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Viscul command '%s' performed.", cmd);
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Viscul command '%s' not supported!", cmd);
  }
}

void csBugPlug::VisculView (iCamera* camera)
{
  if (visculler)
  {
    visculler = 0;
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Disabled visculler graphical dumping");
    return;
  }

  // If we are not tracking a visculler yet we try to find one in current
  // sector.
  iSector* sector = camera->GetSector ();
  visculler = sector->GetVisibilityCuller ();
  if (!visculler)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Bugplug found no visibility culler in this sector!");
    return;
  }
  Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Bugplug is now tracking a visibility culler");
}

void csBugPlug::HideSpider (iCamera* camera)
{
  spider_hunting = false;
  spider->UnweaveWeb (Engine);
  if (camera)
  {
    char buf[80];
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Spider caught a camera!");
    switch (spider_command)
    {
      case DEBUGCMD_DUMPCAM:
	Dump (camera);
	break;
      case DEBUGCMD_FOV:
	{
          int fov = camera->GetFOV ();
	  sprintf (buf, "%d", fov);
	  EnterEditMode (spider_command, "Enter new fov value:", buf);
	}
	break;
      case DEBUGCMD_FOVANGLE:
	{
          float fov = camera->GetFOVAngle ();
	  sprintf (buf, "%g", fov);
	  EnterEditMode (spider_command, "Enter new fov angle:", buf);
	}
	break;
      case DEBUGCMD_SCRSHOT:
        CaptureScreen ();
	break;
      case DEBUGCMD_SAVEMAP:
        SaveMap ();
	break;
      case DEBUGCMD_DEBUGSECTOR:
	SwitchDebugSector (camera->GetTransform ());
        break;
      case DEBUGCMD_MOUSE1:
        MouseButton3 (camera); //@@@ Temp hack to make bugplug cross platform.
	break;
      case DEBUGCMD_MOUSE2:
        MouseButton2 (camera);
	break;
      case DEBUGCMD_MOUSE3:
        MouseButton3 (camera);
	break;
      case DEBUGCMD_VISCULVIEW:
        VisculView (camera);
        break;
      case DEBUGCMD_SWITCHCULLER:
        SwitchCuller (camera->GetSector (), spider_args);
        break;
      case DEBUGCMD_SELECTMESH:
        SelectMesh (camera->GetSector (), spider_args);
        break;
      case DEBUGCMD_ONESECTOR:
        OneSector (camera);
        break;
    }
  }
}

void csBugPlug::ToggleG3DState (G3D_RENDERSTATEOPTION op, const char* name)
{
  if (!G3D) return;
  bool v;
  v = G3D->GetRenderState (op);
  v = !v;
  if (G3D->SetRenderState (op, v))
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, "BugPlug %s %s.",
	v ? "enabled" : "disabled", name);
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, "%s not supported for this renderer!",
    	name);
  }
}

void csBugPlug::MouseButton1 (iCamera*)
{
}

void csBugPlug::MouseButton2 (iCamera* camera)
{
  csVector3 v;
  // Setup perspective vertex, invert mouse Y axis.
  csVector2 p (mouse_x, camera->GetShiftY() * 2 - mouse_y);

  camera->InvPerspective (p, 100, v);
  csVector3 end = camera->GetTransform ().This2Other (v);

  iSector* sector = camera->GetSector ();
  CS_ASSERT (sector != 0);
  csVector3 origin = camera->GetTransform ().GetO2TTranslation ();
  csVector3 isect;

  csRef<iCollideSystem> cdsys = CS_QUERY_REGISTRY (object_reg, iCollideSystem);
  csIntersectingTriangle closest_tri;
  iMeshWrapper* sel;
  float sqdist = csColliderHelper::TraceBeam (cdsys, sector,
	origin, end, true,
	closest_tri, isect, &sel);

  if (sqdist >= 0 && sel)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Hit a mesh '%s' at distance %g!",
	sel->QueryObject ()->GetName (), csQsqrt (sqdist));
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"No mesh hit!");
  }
}

void csBugPlug::MouseButton3 (iCamera* camera)
{
  csVector3 v;
  // Setup perspective vertex, invert mouse Y axis.
  csVector2 p (mouse_x, camera->GetShiftY() * 2 - mouse_y);

  camera->InvPerspective (p, 100, v);
  csVector3 end = camera->GetTransform ().This2Other (v);

  iSector* sector = camera->GetSector ();
  CS_ASSERT (sector != 0);
  csVector3 origin = camera->GetTransform ().GetO2TTranslation ();
  csVector3 isect;

  int polyidx = -1;
  iMeshWrapper* sel = sector->HitBeam (origin, end, isect, &polyidx);
  const char* poly_name = 0;
  if (polyidx != -1)
  {
    csRef<iThingState> ts = SCF_QUERY_INTERFACE (sel->GetMeshObject (),
    	iThingState);
    if (ts)
    {
      poly_name = ts->GetFactory ()->GetPolygonName (polyidx);
      Dump (ts->GetFactory (), polyidx);
    }
  }
  else
  {
    poly_name = 0;
  }

  csVector3 vw = isect;
  v = camera->GetTransform ().Other2This (vw);
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "LMB down : c:(%f,%f,%f) w:(%f,%f,%f) p:'%s'",
    v.x, v.y, v.z, vw.x, vw.y, vw.z, poly_name ? poly_name : "<none>");

  if (sel)
  {
    selected_meshes.Empty ();
    AddSelectedMesh (sel);
    const char* n = sel->QueryObject ()->GetName ();
    Report (CS_REPORTER_SEVERITY_NOTIFY, "BugPlug found mesh '%s'!",
      	n ? n : "<noname>");
    bool bbox, rad;
    shadow->GetShowOptions (bbox, rad);

    if (bbox || rad || show_polymesh != BUGPLUG_POLYMESH_NO)
      shadow->AddToEngine (Engine);
    else
      shadow->RemoveFromEngine (Engine);
  }
}

bool csBugPlug::EatMouse (iEvent& event)
{
  SetupPlugin ();
  if (!process_next_mouse && !debug_view.show) return false;

  bool down = (event.Type == csevMouseDown);
  bool up = (event.Type == csevMouseUp);
  int button = event.Mouse.Button;

  mouse_x = event.Mouse.x;
  mouse_y = event.Mouse.y;

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
      UnleashSpider (DEBUGCMD_MOUSE1+button-1);
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

void csBugPlug::CaptureScreen ()
{
  int i = 0;
  char name[CS_MAXPATHLEN];

  bool exists = false;
  do
  {
    cs_snprintf (name, CS_MAXPATHLEN, captureFormat, i);
    if (exists = VFS->Exists (name)) i++;
  }
  while ((i < captureFormatNumberMax) && (exists));

  if (i >= captureFormatNumberMax)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Too many screenshot files in current directory");
    return;
  }

  csRef<iImage> img (csPtr<iImage> (G2D->ScreenShot ()));
  if (!img)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"The 2D graphics driver does not support screen shots");
    return;
  }
  csRef<iImageIO> imageio (CS_QUERY_REGISTRY (object_reg, iImageIO));
  if (imageio)
  {
    csRef<iDataBuffer> db (imageio->Save (img, captureMIME, 
      captureOptions));
    if (db)
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Screenshot: %s", name);
      if (!VFS->WriteFile (name, (const char*)db->GetData (),
      		db->GetSize ()))
      {
        Report (CS_REPORTER_SEVERITY_NOTIFY,
		"There was an error while writing screen shot");
      }
    }
    else
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY, 
	      "Could not encode screen shot");
    }
  }
}

void csBugPlug::ListLoadedPlugins ()
{
  csRef<iPluginManager> plugmgr = CS_QUERY_REGISTRY (object_reg, 
    iPluginManager);
  csRef<iPluginIterator> plugiter (plugmgr->GetPlugins ());

  csSet<const char*, csConstCharHashKeyHandler> printedPlugins;
  Report (CS_REPORTER_SEVERITY_NOTIFY, 
    "Loaded plugins:");
  while (plugiter->HasNext())
  {
    csRef<iFactory> plugFact = SCF_QUERY_INTERFACE (plugiter->Next (),
      iFactory);
    if (plugFact.IsValid())
    {
      const char* libname = plugFact->QueryModuleName();
      if ((libname != 0) && (!printedPlugins.In (libname)))
      {
	printedPlugins.AddNoTest (libname);
	Report (CS_REPORTER_SEVERITY_NOTIFY, 
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
  char* args;
  int cmd = GetCommandCode (key, shift, alt, ctrl, args);
  if (down)
  {
    // First we check if it is the 'debug enter' key.
    if (cmd == DEBUGCMD_DEBUGENTER)
    {
      process_next_key = !process_next_key;
      if (process_next_key)
      {
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Press debug key...");
      }
      else
      {
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Back to normal key processing.");
      }
      return true;
    }
    if (cmd == DEBUGCMD_MOUSEENTER)
    {
      process_next_mouse = !process_next_mouse;
      if (process_next_mouse)
      {
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Click on screen...");
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
    switch (cmd)
    {
      case DEBUGCMD_UNKNOWN:
        return true;
      case DEBUGCMD_QUIT:
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Nah nah! I will NOT quit!");
        break;
      case DEBUGCMD_STATUS:
        Report (CS_REPORTER_SEVERITY_NOTIFY,
		"I'm running smoothly, thank you...");
        break;
      case DEBUGCMD_DEBUGGRAPH:
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Debug graph dumped!");
	csDebuggingGraph::Dump (object_reg);
        break;
      case DEBUGCMD_ENGINECMD:
	{
	  csRef<iDebugHelper> dbghelp (
	  	SCF_QUERY_INTERFACE (Engine, iDebugHelper));
	  if (dbghelp)
	  {
	    if (dbghelp->DebugCommand (args))
	    {
              Report (CS_REPORTER_SEVERITY_NOTIFY,
	        "Engine command '%s' performed.", args);
	    }
	    else
	    {
              Report (CS_REPORTER_SEVERITY_NOTIFY,
	        "Engine command '%s' not supported!", args);
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
	  	SCF_QUERY_INTERFACE (Engine, iDebugHelper));
	  if (dbghelp)
	  {
	    if (dbghelp->GetSupportedTests () & CS_DBGHELP_STATETEST)
	    {
	      csRef<iString> rc (dbghelp->StateTest ());
	      if (rc)
	      {
                Report (CS_REPORTER_SEVERITY_NOTIFY,
	          "Engine StateTest() failed:");
                Report (CS_REPORTER_SEVERITY_DEBUG,
	          "Engine StateTest() failed:");
                Report (CS_REPORTER_SEVERITY_DEBUG,
		  rc->GetData ());
	      }
	      else
	      {
                Report (CS_REPORTER_SEVERITY_NOTIFY,
	          "Engine StateTest() succeeded!");
	      }
	    }
	    else
	    {
              Report (CS_REPORTER_SEVERITY_NOTIFY,
	        "Engine doesn't support StateTest()!");
	    }
	  }
	}
        break;
      case DEBUGCMD_HELP:
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Sorry, cannot help you yet.");
        break;
      case DEBUGCMD_DUMPENG:
        if (Engine)
	{
          Report (CS_REPORTER_SEVERITY_NOTIFY,
		"Dumping entire engine contents to debug.txt.");
	  Dump (Engine);
	  Report (CS_REPORTER_SEVERITY_DEBUG, "");
	}
        break;
      case DEBUGCMD_DUMPSEC:
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Not implemented yet.");
        break;
      case DEBUGCMD_CLEAR:
        do_clear = !do_clear;
        Report (CS_REPORTER_SEVERITY_NOTIFY, "BugPlug %s screen clearing.",
	  	do_clear ? "enabled" : "disabled");
        break;
      case DEBUGCMD_EDGES:
        ToggleG3DState (G3DRENDERSTATE_EDGES, "edge drawing");
	{
	  if (Engine && (Engine->GetBeginDrawFlags () & CSDRAW_CLEARSCREEN))
	    break;
	  bool v;
	  v = G3D->GetRenderState (G3DRENDERSTATE_EDGES);
	  if (v && !do_clear) do_clear = true;
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
          Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "BugPlug cleared the texture cache.");
	}*/
        break;
      case DEBUGCMD_CACHEDUMP:
        //if (G3D) G3D->DumpCache ();
        break;
      case DEBUGCMD_MIPMAP:
        {
	  if (!G3D) break;
	  char* choices[6] = { "on", "off", "1", "2", "3", 0 };
	  long v = G3D->GetRenderState (G3DRENDERSTATE_MIPMAPENABLE);
	  v = (v+1)%5;
	  G3D->SetRenderState (G3DRENDERSTATE_MIPMAPENABLE, v);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "BugPlug set mipmap to '%s'",
	  	choices[v]);
  	}
	break;
      case DEBUGCMD_INTER:
	{
	  if (!G3D) break;
	  char* choices[5] = { "smart", "step32", "step16", "step8", 0 };
	  long v = G3D->GetRenderState (G3DRENDERSTATE_INTERPOLATIONSTEP);
	  v = (v+1)%4;
	  G3D->SetRenderState (G3DRENDERSTATE_INTERPOLATIONSTEP, v);
	  Report (CS_REPORTER_SEVERITY_NOTIFY, "BugPlug set interpolation to '%s'",
	  	choices[v]);
	}
	break;
      case DEBUGCMD_GAMMA:
        {
	  if (!G3D) break;
	  float val = G2D->GetGamma ();
          char buf[100];
	  sprintf (buf, "%g", val);
          EnterEditMode (cmd, "Enter new gamma:", buf);
	}
        break;
      case DEBUGCMD_SELECTMESH:
        EnterEditMode (cmd, "Enter mesh name regexp pattern:", "");
        break;
      case DEBUGCMD_DBLBUFF:
        {
	  bool state = G2D->GetDoubleBufferState ();
	  state = !state;
	  if (!G2D->DoubleBuffer (state))
	  {
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"Double buffer not supported in current video mode!");
	  }
	  else
	  {
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug %s double buffering.",
		state ? "enabled" : "disabled");
	  }
	}
        break;
      case DEBUGCMD_TERRVIS:
        if (Engine)
	{
	  int enable_disable = -1;
	  int i, j;
	  for (i = 0 ; i < Engine->GetSectors ()->GetCount () ; i++)
	  {
	    iSector* sector = Engine->GetSectors ()->Get (i);
	    iMeshList* ml = sector->GetMeshes ();
	    for (j = 0 ; j < ml->GetCount () ; j++)
	    {
	      iMeshWrapper* terr = ml->Get (j);
	      csRef<iTerrFuncState> st (
	      	SCF_QUERY_INTERFACE (terr->GetMeshObject (),
	      	iTerrFuncState));
	      if (st)
	      {
	        if (enable_disable == -1)
		{
		  enable_disable = (int) (!st->IsVisTestingEnabled ());
		}
		st->SetVisTesting ((bool) enable_disable);
	      }
	    }
	  }
	  if (enable_disable == -1)
	  {
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "BugPlug found no terrains to work with!");
	  }
	  else
	  {
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "BugPlug %s terrain visibility checking!",
	      enable_disable ? "enabled" : "disabled");
	  }
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug has no engine to work on!");
	}
        break;
      case DEBUGCMD_MESHBASEMESH:
	{
	  if (show_polymesh == BUGPLUG_POLYMESH_BASE)
	  {
	    show_polymesh = BUGPLUG_POLYMESH_NO;
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug disabled showing BASE polygonmesh.");
	  }
	  else
	  {
	    show_polymesh = BUGPLUG_POLYMESH_BASE;
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug is showing BASE polygonmesh.");
	  }
	}
        break;
      case DEBUGCMD_MESHSHADMESH:
	{
	  if (show_polymesh == BUGPLUG_POLYMESH_SHAD)
	  {
	    show_polymesh = BUGPLUG_POLYMESH_NO;
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug disabled showing SHAD polygonmesh.");
	  }
	  else
	  {
	    show_polymesh = BUGPLUG_POLYMESH_SHAD;
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug is showing SHAD polygonmesh.");
	  }
	}
        break;
      case DEBUGCMD_MESHVISMESH:
	{
	  if (show_polymesh == BUGPLUG_POLYMESH_VIS)
	  {
	    show_polymesh = BUGPLUG_POLYMESH_NO;
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug disabled showing VIS polygonmesh.");
	  }
	  else
	  {
	    show_polymesh = BUGPLUG_POLYMESH_VIS;
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug is showing VIS polygonmesh.");
	  }
	}
        break;
      case DEBUGCMD_MESHCDMESH:
	{
	  if (show_polymesh == BUGPLUG_POLYMESH_CD)
	  {
	    show_polymesh = BUGPLUG_POLYMESH_NO;
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug disabled showing CD polygonmesh.");
	  }
	  else
	  {
	    show_polymesh = BUGPLUG_POLYMESH_CD;
	    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug is showing CD polygonmesh.");
	  }
	}
        break;
      case DEBUGCMD_MESHBBOX:
	{
	  bool bbox, rad;
	  shadow->GetShowOptions (bbox, rad);
          bbox = !bbox;
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug %s bounding box display.",
		bbox ? "enabled" : "disabled");
	  shadow->SetShowOptions (bbox, rad);
	  if ((bbox || rad || show_polymesh != BUGPLUG_POLYMESH_NO)
			  && HasSelectedMeshes ())
	    shadow->AddToEngine (Engine);
	  else
	    shadow->RemoveFromEngine (Engine);
	}
        break;
      case DEBUGCMD_MESHRAD:
        {
	  bool bbox, rad;
	  shadow->GetShowOptions (bbox, rad);
          rad = !rad;
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug %s bounding sphere display.",
		rad ? "enabled" : "disabled");
	  shadow->SetShowOptions (bbox, rad);
	  if ((bbox || rad || show_polymesh != BUGPLUG_POLYMESH_NO)
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
      case DEBUGCMD_ONESECTOR:
      case DEBUGCMD_VISCULVIEW:
      case DEBUGCMD_DUMPCAM:
      case DEBUGCMD_FOV:
      case DEBUGCMD_FOVANGLE:
      case DEBUGCMD_DEBUGSECTOR:
      case DEBUGCMD_SAVEMAP:
      case DEBUGCMD_SCRSHOT:
        // Set spider on a hunt.
	UnleashSpider (cmd, args);
        break;
      case DEBUGCMD_DS_LEFT:
        if (debug_sector.show && debug_sector.clear)
	{
	  debug_sector.view->GetCamera ()->Move (csVector3 (-1, 0, 0), false);
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
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
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
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
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
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
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
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
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
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
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
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
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
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
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"Debug sector is not active now!");
	}
        break;
      case DEBUGCMD_FPS:
        do_fps = !do_fps;
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug %s fps display.",
		do_fps ? "enabled" : "disabled");
	fps_frame_count = 0;
	fps_tottime = 0;
	fps_cur = -1;
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
	  for (j = 0 ; j < selected_meshes.Length () ; j++)
	  {
	    if (selected_meshes[j])
	      selected_meshes[j]->GetFlags ().Set (CS_ENTITY_INVISIBLE);
	  }
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"There are no selected meshes to hide!");
	}
        break;
      case DEBUGCMD_UNDOHIDE:
        if (HasSelectedMeshes ())
	{
	  size_t j;
	  for (j = 0 ; j < selected_meshes.Length () ; j++)
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
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"BugPlug %s counting.",
		counter_freeze ? "disabled" : "enabled");
	break;
      case DEBUGCMD_MEMORYDUMP:
#       ifdef CS_MEMORY_TRACKER
	  {
	    csRef<iMemoryTracker> mtr = CS_QUERY_REGISTRY_TAG_INTERFACE (
	    	object_reg, "crystalspace.utilities.memorytracker",
		iMemoryTracker);
	    if (!mtr)
	    {
	      Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"Memory tracker interface is missing!");
	    }
	    else
	    {
	      mtr->Dump (false);
	      Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"Memory dump sent to stdout!");
	    }
	  }
#       else
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"Memory tracker not enabled at compile time!...");
#       endif
        break;
      case DEBUGCMD_UNPREPARE:
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	    	"Unprepare all things...");
	{
	  int i;
	  iMeshList* ml = Engine->GetMeshes ();
	  for (i = 0 ; i < ml->GetCount () ; i++)
	  {
	    iMeshWrapper* m = ml->Get (i);
	    csRef<iThingState> th = SCF_QUERY_INTERFACE (m->GetMeshObject (),
	    	iThingState);
	    if (th)
	    {
	      th->Unprepare ();
	    }
	  }
	}
        break;
      case DEBUGCMD_COLORSECTORS:
	Report (CS_REPORTER_SEVERITY_NOTIFY,
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
      case DEBUGCMD_PROFDUMP:
        CS_PROFDUMP(object_reg);
        break;
      case DEBUGCMD_PROFRESET:
        CS_PROFRESET(object_reg);
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
	  csRef<iShaderManager> shmgr ( CS_QUERY_REGISTRY(object_reg, iShaderManager));
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
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "BugPlug %s shadow debugging.",
	    do_shadow_debug ? "enabled" : "disabled");*/
        break;
      case DEBUGCMD_LISTPLUGINS:
	ListLoadedPlugins();
	break;
    }
    process_next_key = false;
  }
  return true;
}

bool csBugPlug::HandleStartFrame (iEvent& /*event*/)
{
  SetupPlugin ();
  if (!G3D) return false;

  if (shadow) shadow->ClearCamera ();
  
  if (do_clear)
  {
    G3D->BeginDraw (CSDRAW_2DGRAPHICS | CSDRAW_CLEARZBUFFER);
    int bgcolor_clear = G2D->FindRGB (0, 255, 255);
    G2D->Clear (bgcolor_clear);
  }
  return false;
}

void GfxWrite (iGraphics2D* g2d, iFont* font,
	int x, int y, int fg, int bg, char *str, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, str);
  vsprintf (buf, str, arg);
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

bool csBugPlug::HandleEndFrame (iEvent& /*event*/)
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
    csRef<iDebugHelper> dbghelp (SCF_QUERY_INTERFACE (visculler, iDebugHelper));
    if (dbghelp)
      dbghelp->Dump (G3D);
  }

  if (debug_sector.show)
  {
    G3D->BeginDraw (CSDRAW_3DGRAPHICS |
    	CSDRAW_CLEARZBUFFER | (debug_sector.clear ? CSDRAW_CLEARSCREEN : 0));
    iCamera* camera = spider->GetCamera ();
    if (camera)
      debug_sector.view->GetCamera ()->SetTransform (camera->GetTransform ());
    debug_sector.view->Draw ();
  }

  if (debug_view.show)
  {
    G3D->BeginDraw (CSDRAW_2DGRAPHICS |
    	(debug_view.clear ? CSDRAW_CLEARSCREEN : 0));
    if (debug_view.object)
      debug_view.object->Render (G3D, &scfiBugPlug);
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

  if (HasSelectedMeshes () && shadow && shadow->GetCamera () &&
		  !debug_view.show && !debug_sector.show)
  {
    size_t k;
    iCamera* cam = shadow->GetCamera ();
    csTransform tr_w2c = cam->GetTransform ();
    float fov = G3D->GetPerspectiveAspect ();
    bool do_bbox, do_rad;
    shadow->GetShowOptions (do_bbox, do_rad);
    G3D->BeginDraw (CSDRAW_2DGRAPHICS);
    for (k = 0 ; k < selected_meshes.Length () ; k++)
    {
      if (!selected_meshes[k]) continue;
      iMovable* mov = selected_meshes[k]->GetMovable ();
      csReversibleTransform tr_o2c = tr_w2c / mov->GetFullTransform ();
      if (do_bbox)
      {
        int bbox_color = G3D->GetDriver2D ()->FindRGB (0, 255, 255);
        csBox3 bbox;
        selected_meshes[k]->GetMeshObject ()->GetObjectModel ()->
    	  GetObjectBoundingBox (bbox);
        csVector3 vxyz = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_xyz);
        csVector3 vXyz = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_Xyz);
        csVector3 vxYz = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_xYz);
        csVector3 vxyZ = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_xyZ);
        csVector3 vXYz = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_XYz);
        csVector3 vXyZ = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_XyZ);
        csVector3 vxYZ = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_xYZ);
        csVector3 vXYZ = tr_o2c * bbox.GetCorner (CS_BOX_CORNER_XYZ);
        G3D->DrawLine (vxyz, vXyz, fov, bbox_color);
        G3D->DrawLine (vXyz, vXYz, fov, bbox_color);
        G3D->DrawLine (vXYz, vxYz, fov, bbox_color);
        G3D->DrawLine (vxYz, vxyz, fov, bbox_color);
        G3D->DrawLine (vxyZ, vXyZ, fov, bbox_color);
        G3D->DrawLine (vXyZ, vXYZ, fov, bbox_color);
        G3D->DrawLine (vXYZ, vxYZ, fov, bbox_color);
        G3D->DrawLine (vxYZ, vxyZ, fov, bbox_color);
        G3D->DrawLine (vxyz, vxyZ, fov, bbox_color);
        G3D->DrawLine (vxYz, vxYZ, fov, bbox_color);
        G3D->DrawLine (vXyz, vXyZ, fov, bbox_color);
        G3D->DrawLine (vXYz, vXYZ, fov, bbox_color);
      }
      if (do_rad)
      {
        int rad_color = G3D->GetDriver2D ()->FindRGB (0, 255, 0);
        csVector3 radius, r, center;
        selected_meshes[k]->GetMeshObject ()->GetObjectModel ()
		->GetRadius (radius,center);
        csVector3 trans_o = tr_o2c * center;
        r.Set (radius.x, 0, 0);
        G3D->DrawLine (trans_o-r, trans_o+r, fov, rad_color);
        r.Set (0, radius.y, 0);
        G3D->DrawLine (trans_o-r, trans_o+r, fov, rad_color);
        r.Set (0, 0, radius.z);
        G3D->DrawLine (trans_o-r, trans_o+r, fov, rad_color);
      }
      if (show_polymesh != BUGPLUG_POLYMESH_NO)
      {
        iPolygonMesh* pm = 0;
        switch (show_polymesh)
        {
	  case BUGPLUG_POLYMESH_CD:
	    pm = selected_meshes[k]->GetMeshObject ()->GetObjectModel ()->
		  GetPolygonMeshColldet ();
	    break;
	  case BUGPLUG_POLYMESH_VIS:
	    pm = selected_meshes[k]->GetMeshObject ()->GetObjectModel ()->
		  GetPolygonMeshViscull ();
	    break;
	  case BUGPLUG_POLYMESH_SHAD:
	    pm = selected_meshes[k]->GetMeshObject ()->GetObjectModel ()->
		  GetPolygonMeshShadows ();
	    break;
	  case BUGPLUG_POLYMESH_BASE:
	    pm = selected_meshes[k]->GetMeshObject ()->GetObjectModel ()->
		  GetPolygonMeshBase ();
	    break;
        }
        if (pm)
        {
          int pm_color = G3D->GetDriver2D ()->FindRGB (255, 255, 128);
	  int vtcount = pm->GetVertexCount ();
	  csVector3* vt = pm->GetVertices ();
	  int pocount = pm->GetPolygonCount ();
	  csMeshedPolygon* po = pm->GetPolygons ();
	  csVector3* vtt = new csVector3[vtcount];
	  int i;
	  for (i = 0 ; i < vtcount ; i++)
	    vtt[i] = tr_o2c * vt[i];
	  for (i = 0 ; i < pocount ; i++)
	  {
	    csMeshedPolygon& pol = po[i];
	    int j, j1;
	    j1 = pol.num_vertices - 1;
	    for (j = 0 ; j < pol.num_vertices ; j++)
	    {
              G3D->DrawLine (vtt[pol.vertices[j]], vtt[pol.vertices[j1]],
	      	fov, pm_color);
	      j1 = j;
	    }
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
      char* msg;
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
      fnt->GetDimensions (dispString, cursor_w, cursor_h);
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
    G3D->FinishDraw ();
  }

  ShowCounters ();

  if (spider_hunting)
  {
    iCamera* camera = spider->GetCamera ();
    if (camera)
    {
      HideSpider (camera);
    }
    else
    {
      spider_timeout--;
      if (spider_timeout < 0)
      {
	HideSpider (0);
        Report (CS_REPORTER_SEVERITY_NOTIFY,
		"Spider could not catch a camera!");
      }
    }
  }

  return false;
}

bool csBugPlug::HandleSystemOpen (iEvent* event)
{
  return false;
}

bool csBugPlug::HandleSystemClose (iEvent* event)
{
  fnt = 0;
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
  int i;
  float f;
  switch (edit_command)
  {
    case DEBUGCMD_GAMMA:
      csScanStr (edit_string, "%f", &f);
      G2D->SetGamma (f);
      break;
    case DEBUGCMD_FOV:
      csScanStr (edit_string, "%d", &i);
      spider->GetCamera ()->SetFOV (i, G3D->GetWidth ());
      break;
    case DEBUGCMD_FOVANGLE:
      csScanStr (edit_string, "%f", &f);
      spider->GetCamera ()->SetFOVAngle (f, G3D->GetWidth ());
      break;
    case DEBUGCMD_SELECTMESH:
      UnleashSpider (edit_command, edit_string);
      break;
  }
}

void csBugPlug::DebugCmd (const char* cmd)
{
  char* cmdstr = csStrNew (cmd);
  char* params;
  char* space = strchr (cmdstr, ' ');
  if (space == 0)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "debugcmd syntax: <plugin> <command>");
  }
  else
  {
    params = space + 1;
    *space = 0;

    csRef<iBase> comp;
    comp = CS_QUERY_REGISTRY_TAG(object_reg, cmdstr);

    if (comp == 0)
    {
      csRef<iPluginManager> plugmgr = 
	CS_QUERY_REGISTRY (object_reg, iPluginManager);
      CS_ASSERT (plugmgr);
      csRef<iBase> comp =
	CS_QUERY_PLUGIN_CLASS (plugmgr, cmdstr, iBase);
    }

    if (!comp)
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY,
	"Could not load plugin '%s' for debug command execution.",
	cmdstr);
    }
    else
    {
      csRef<iDebugHelper> dbghelp = 
	SCF_QUERY_INTERFACE (comp, iDebugHelper);
      if (!dbghelp)
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Plugin '%s' doesn't support debug command execution.",
	  cmdstr);
      }
      else
      {
	bool res = dbghelp->DebugCommand (params);
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Debug command execution %s.",
	  res ? "successful" : "failed");
      }
    }
  }
  delete[] cmdstr;
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

int csBugPlug::GetCommandCode (const char* cmdstr, char* args)
{
  char cmd[256];
  char const* spc = strchr (cmdstr, ' ');
  if (spc)
  {
    strncpy (cmd, cmdstr, spc - cmdstr);
    cmd[spc - cmdstr] = 0;
    strcpy (args, spc + 1);
  }
  else
  {
    strcpy(cmd, cmdstr);
    args[0] = 0;
  }

  if (!strcmp (cmd, "debugenter"))	return DEBUGCMD_DEBUGENTER;
  if (!strcmp (cmd, "mouseenter"))	return DEBUGCMD_MOUSEENTER;
  if (!strcmp (cmd, "quit"))		return DEBUGCMD_QUIT;
  if (!strcmp (cmd, "status"))		return DEBUGCMD_STATUS;
  if (!strcmp (cmd, "help"))		return DEBUGCMD_HELP;

  if (!strcmp (cmd, "dumpeng"))		return DEBUGCMD_DUMPENG;
  if (!strcmp (cmd, "dumpsec"))		return DEBUGCMD_DUMPSEC;
  if (!strcmp (cmd, "edges"))		return DEBUGCMD_EDGES;
  if (!strcmp (cmd, "clear"))		return DEBUGCMD_CLEAR;
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
  if (!strcmp (cmd, "mipmap"))		return DEBUGCMD_MIPMAP;
  if (!strcmp (cmd, "inter"))		return DEBUGCMD_INTER;
  if (!strcmp (cmd, "gamma"))		return DEBUGCMD_GAMMA;
  if (!strcmp (cmd, "dblbuff"))		return DEBUGCMD_DBLBUFF;
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
  if (!strcmp (cmd, "unprepare"))	return DEBUGCMD_UNPREPARE;
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
  if (!strcmp (cmd, "profdump"))	return DEBUGCMD_PROFDUMP;
  if (!strcmp (cmd, "profreset"))	return DEBUGCMD_PROFRESET;

  return DEBUGCMD_UNKNOWN;
}

int csBugPlug::GetCommandCode (int key, bool shift, bool alt, bool ctrl,
	char*& args)
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

  char args[512];
  int cmdcode = GetCommandCode (cmdstring, args);
  // Check if valid command name.
  if (cmdcode == DEBUGCMD_UNKNOWN) return;

  // Check if key isn't already defined.
  char* args2;
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
  if (args[0])
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
  Report (CS_REPORTER_SEVERITY_DEBUG, "    Sector '%s' (%08lx)",
  	sn ? sn : "?", sector);
  Report (CS_REPORTER_SEVERITY_DEBUG, "    %d meshes, %d lights",
  	sector->GetMeshes ()->GetCount (),
	sector->GetLights ()->GetCount ());
  int i;
  for (i = 0 ; i < sector->GetMeshes ()->GetCount () ; i++)
  {
    iMeshWrapper* mesh = sector->GetMeshes ()->Get (i);
    const char* n = mesh->QueryObject ()->GetName ();
    Report (CS_REPORTER_SEVERITY_DEBUG, "        Mesh '%s' (%08lx)",
    	n ? n : "?", mesh);
  }
}

void csBugPlug::Dump (int indent, iMeshWrapper* mesh)
{
  const char* mn = mesh->QueryObject ()->GetName ();
  Report (CS_REPORTER_SEVERITY_DEBUG, "%*s    Mesh wrapper '%s' (%08lx)", 
    indent, "", mn ? mn : "?", mesh);
  iMeshObject* obj = mesh->GetMeshObject ();
  if (!obj)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG, "%*s        Mesh object missing!",
      indent, "");
  }
  else
  {
    csRef<iFactory> fact (SCF_QUERY_INTERFACE (obj, iFactory));
    if (fact)
      Report (CS_REPORTER_SEVERITY_DEBUG, "%*s        Plugin '%s'",
  	  indent, "",
          fact->QueryDescription () ? fact->QueryDescription () : "0");
    csBox3 bbox;
    obj->GetObjectModel ()->GetObjectBoundingBox (bbox);
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
  for (int i=0; i<mesh->GetChildren ()->GetCount (); ++i)
  {
    Dump (indent+4, mesh->GetChildren ()->Get (i));
  }
}

void csBugPlug::Dump (iMeshFactoryWrapper* meshfact)
{
  const char* mn = meshfact->QueryObject ()->GetName ();
  Report (CS_REPORTER_SEVERITY_DEBUG, "        Mesh factory wrapper '%s' (%08lx)",
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
  Report (CS_REPORTER_SEVERITY_DEBUG,
  	"Camera: %s (mirror=%d, fov=%d, fovangle=%g,",
  	sn, c->IsMirrored (), c->GetFOV (), c->GetFOVAngle ());
  Report (CS_REPORTER_SEVERITY_DEBUG, "    shiftx=%g shifty=%g camnr=%d)",
  	c->GetShiftX (), c->GetShiftY (), c->GetCameraNumber ());
  if (far_plane)
    Report (CS_REPORTER_SEVERITY_DEBUG, "    far_plane=(%g,%g,%g,%g)",
    	far_plane->A (), far_plane->B (), far_plane->C (), far_plane->D ());
  csReversibleTransform& trans = c->GetTransform ();
  Dump (4, trans.GetO2TTranslation (), "Camera vector");
  Dump (4, trans.GetO2T (), "Camera matrix");
}

void csBugPlug::Dump (iThingFactoryState* fact, int polyidx)
{
  const char* poly_name = fact->GetPolygonName (polyidx);
  if (!poly_name) poly_name = "<noname>";
  Report (CS_REPORTER_SEVERITY_DEBUG, "Polygon '%s'",
  	poly_name);
  int nv = fact->GetPolygonVertexCount (polyidx);
  int i;
  int* idx = fact->GetPolygonVertexIndices (polyidx);
  char buf[256];
  sprintf (buf, "  Vertices: ");
  for (i = 0 ; i < nv ; i++)
    sprintf (buf+strlen (buf), "%d ", idx[i]);
  Report (CS_REPORTER_SEVERITY_DEBUG, buf);
}

bool csBugPlug::HandleEvent (iEvent& event)
{
  if (event.Type == csevKeyboard)
  {
    return EatKey (event);
  }
  else if (event.Type == csevMouseDown)
  {
    return EatMouse (event);
  }
  else if (event.Type == csevMouseUp)
  {
    return EatMouse (event);
  }
  else if (event.Type == csevMouseMove)
  {
    return EatMouse (event);
  }
  else if (event.Type == csevBroadcast)
  {
    if (event.Command.Code == cscmdPreProcess)
    {
      return HandleStartFrame (event);
    }
    if (event.Command.Code == cscmdPostProcess)
    {
      return HandleEndFrame (event);
    }
    if (event.Command.Code == cscmdSystemOpen)
    {
      return HandleSystemOpen (&event);
    }
    if (event.Command.Code == cscmdSystemClose)
    {
      return HandleSystemClose (&event);
    }
  }

  return false;
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
  iRegion* db_region = Engine->CreateRegion ("__BugPlug_region__");
  db_region->DeleteAll ();

  iRegionList* reglist = Engine->GetRegions ();
  reglist->Remove (db_region);

  delete debug_sector.view;

  debug_sector.sector = 0;
  debug_sector.view = 0;
}

void csBugPlug::SetupDebugSector ()
{
  CleanDebugSector ();
  if (!Engine)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, "There is no engine!");
    return;
  }

  iRegion* db_region = Engine->CreateRegion ("__BugPlug_region__");
  debug_sector.sector = Engine->CreateSector ("__BugPlug_sector__");
  db_region->QueryObject ()->ObjAdd (debug_sector.sector->QueryObject ());

  debug_sector.view = new csView (Engine, G3D);
  int w3d = G3D->GetWidth ();
  int h3d = G3D->GetHeight ();
  debug_sector.view->SetRectangle (0, 0, w3d, h3d);
  debug_sector.view->GetCamera ()->SetSector (debug_sector.sector);
}

iMaterialWrapper* csBugPlug::FindColor (float r, float g, float b)
{
  // Assumes the current region is the debug region.
  char name[100];
  sprintf (name, "mat%d,%d,%d\n", int (r*255), int (g*255), int (b*255));
  iMaterialWrapper* mw = Engine->FindMaterial (name);
  if (mw) return mw;
  // Create a new material.
  csRef<iMaterial> mat (Engine->CreateBaseMaterial (0, 0, 0, 0));
  mat->SetFlatColor (csRGBcolor (int (r*255), int (g*255), int (b*255)));
  mw = Engine->GetMaterialList ()->NewMaterial (mat, name);
  mw->Register (G3D->GetTextureManager ());
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
  	SCF_QUERY_INTERFACE (mf->GetMeshObjectFactory (),
  	iGeneralFactoryState));
  CS_ASSERT (gfs != 0);
  gfs->SetMaterialWrapper (mat);
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
  csRef<iGeneralMeshState> gms (SCF_QUERY_INTERFACE (mw->GetMeshObject (),
  	iGeneralMeshState));
  CS_ASSERT (gms != 0);
  gms->SetLighting (false);
  gms->SetColor (csColor (0, 0, 0));
  gms->SetManualColors (true);
  gms->SetMixMode (mixmode);

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
  	SCF_QUERY_INTERFACE (mf->GetMeshObjectFactory (),
  	iGeneralFactoryState));
  CS_ASSERT (gfs != 0);
  gfs->SetMaterialWrapper (mat);
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
  csRef<iGeneralMeshState> gms (SCF_QUERY_INTERFACE (mw->GetMeshObject (),
  	iGeneralMeshState));
  CS_ASSERT (gms != 0);
  gms->SetLighting (false);
  gms->SetColor (csColor (0, 0, 0));
  gms->SetManualColors (true);
  gms->SetMixMode (mixmode);

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
  	SCF_QUERY_INTERFACE (mf->GetMeshObjectFactory (),
  	iGeneralFactoryState));
  CS_ASSERT (gfs != 0);
  gfs->SetMaterialWrapper (mat);
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
  csRef<iGeneralMeshState> gms (SCF_QUERY_INTERFACE (mw->GetMeshObject (),
  	iGeneralMeshState));
  CS_ASSERT (gms != 0);
  gms->SetLighting (false);
  gms->SetColor (csColor (0, 0, 0));
  gms->SetManualColors (true);
  gms->SetMixMode (mixmode);

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
    Report (CS_REPORTER_SEVERITY_NOTIFY, "There is no debug sector!");
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
  for (i = 0 ; i < counters.Length () ; i++)
    if (!strcmp (counters[i]->countername, countername))
      return i;
  return -1;
}

void csBugPlug::FullResetCounters ()
{
  size_t i;
  for (i = 0 ; i < counters.Length () ; i++)
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
  if (counters.Length () == 0) return;

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
  for (i = 0 ; i < counters.Length () ; i++)
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

void csBugPlug::SaveMap ()
{
  int i = 0;
  char name[CS_MAXPATHLEN];

  bool exists = false;
  do
  {
    cs_snprintf (name, CS_MAXPATHLEN, "/this/world%d.xml", i);
    if (exists = VFS->Exists (name)) i++;
  }
  while ((i < captureFormatNumberMax) && (exists));

  if (i >= captureFormatNumberMax)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Too many screenshot files in current directory");
    return;
  }

  csRef<iPluginManager> plugin_mgr = 
    CS_QUERY_REGISTRY (object_reg, iPluginManager);
  csRef<iSaver> saver = 
    CS_QUERY_PLUGIN_CLASS(plugin_mgr, "crystalspace.level.saver", iSaver);
  if (!saver) 
    saver = CS_LOAD_PLUGIN(plugin_mgr, "crystalspace.level.saver", iSaver);
  if (saver)
    saver->SaveMapFile(name);

}

//---------------------------------------------------------------------------

