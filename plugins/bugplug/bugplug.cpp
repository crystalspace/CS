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

#include <string.h>
#define CS_SYSDEF_PROVIDE_PATH
#include "cssysdef.h"
#include "csver.h"
#include "csutil/scf.h"
#include "csutil/scanstr.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/box.h"
#include "bugplug.h"
#include "spider.h"
#include "shadow.h"
#include "isys/system.h"
#include "isys/vfs.h"
#include "isys/event.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivaria/conout.h"
#include "iutil/object.h"
#include "imesh/object.h"
#include "imesh/terrfunc.h"
#include "imesh/thing/polygon.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/mesh.h"
#include "iengine/terrain.h"
#include "iengine/movable.h"
#include "iengine/camera.h"
#include "isys/plugin.h"
#include "iutil/objreg.h"
#include "qint.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csBugPlug)

SCF_EXPORT_CLASS_TABLE (bugplug)
  SCF_EXPORT_CLASS (csBugPlug, "crystalspace.utilities.bugplug",
    "Debugging utility")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csBugPlug)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

#define SysPrintf System->Printf

csBugPlug::csBugPlug (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  Engine = NULL;
  System = NULL;
  G3D = NULL;
  G2D = NULL;
  Conout = NULL;
  VFS = NULL;
  mappings = NULL;
  process_next_key = false;
  process_next_mouse = false;
  edit_mode = false;
  edit_string[0] = 0;
  initialized = false;
  spider = new csSpider ();
  shadow = new csShadow ();
  spider_hunting = false;
  selected_mesh = NULL;
  shadow->SetShadowMesh (selected_mesh);
}

csBugPlug::~csBugPlug ()
{
  if (selected_mesh) selected_mesh->DecRef ();
  if (spider)
  {
    if (Engine) spider->UnweaveWeb (Engine);
    delete spider;
  }
  if (shadow)
  {
    if (Engine) shadow->RemoveFromEngine (Engine);
    delete shadow;
  }
  if (Engine) Engine->DecRef ();
  if (G3D) G3D->DecRef ();
  if (Conout) Conout->DecRef ();
  if (VFS) VFS->DecRef ();
  while (mappings)
  {
    csKeyMap* n = mappings->next;
    delete mappings;
    mappings = n;
  }
}

bool csBugPlug::Initialize (iSystem *system)
{
  System = system;
  object_reg = system->GetObjectRegistry ();
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  if (!System->CallOnEvents (this, CSMASK_Nothing|CSMASK_KeyUp|CSMASK_KeyDown|
  	CSMASK_MouseUp|CSMASK_MouseDown))
    return false;

  return true;
}

void csBugPlug::SetupPlugin ()
{
  if (initialized) return;

  if (!Engine)
    Engine = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_ENGINE, iEngine);
  if (!Engine)
  {
    // No engine. It is possible that we are working with the iso-engine.
  }

  if (!G3D)
    G3D = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_VIDEO, iGraphics3D);
  if (!G3D)
  {
    initialized = true;
    printf ("No G3D!\n");
    return;
  }

  if (!G2D)
    G2D = G3D->GetDriver2D ();
  if (!G2D)
  {
    printf ("No G2D!\n");
    return;
  }

  if (!VFS)
    VFS = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_VFS, iVFS);
  if (!VFS)
  {
    printf ("No VFS!\n");
    return;
  }

  if (!Conout)
    Conout = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_CONSOLE, iConsoleOutput);

  ReadKeyBindings ("/config/bugplug.cfg");

  initialized = true;

  System->Printf (CS_MSG_CONSOLE, "BugPlug loaded...\n");

  do_clear = false;
}

void csBugPlug::UnleashSpider (int cmd)
{
  if (Engine)
  {
    spider->ClearCamera ();
    if (spider->WeaveWeb (Engine))
    {
      spider_command = cmd;
      spider_hunting = true;
      spider_timeout = 20;
    }
    else
    {
      System->Printf (CS_MSG_CONSOLE,
	"Spider could not weave its web (No sectors)!\n");
    }
  }
  else
  {
    System->Printf (CS_MSG_CONSOLE,
      "Spider could not weave its web (No engine)!\n");
  }
}

void csBugPlug::HideSpider (iCamera* camera)
{
  spider_hunting = false;
  spider->UnweaveWeb (Engine);
  if (camera)
  {
    char buf[80];
    System->Printf (CS_MSG_CONSOLE, "Spider caught a camera!\n");
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
      case DEBUGCMD_MOUSE1:
        MouseButton3 (camera); //@@@ Temporary hack to make bugplug cross platform. MHV.
	break;
      case DEBUGCMD_MOUSE2:
        MouseButton2 (camera);
	break;
      case DEBUGCMD_MOUSE3:
        MouseButton3 (camera);
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
    System->Printf (CS_MSG_CONSOLE, "BugPlug %s %s.\n",
	v ? "enabled" : "disabled", name);
  }
  else
  {
    System->Printf (CS_MSG_CONSOLE, "%s not supported for this renderer!\n",
    	name);
  }
}

void csBugPlug::MouseButton1 (iCamera*)
{
}

void csBugPlug::MouseButton2 (iCamera*)
{
}

void csBugPlug::MouseButton3 (iCamera* camera)
{
  csVector3 v;
  // Setup perspective vertex, invert mouse Y axis.
  csVector2 p (mouse_x, camera->GetShiftY() * 2 - mouse_y);

  camera->InvPerspective (p, 1, v);
  csVector3 vw = camera->GetTransform ().This2Other (v);

  iSector* sector = camera->GetSector ();
  csVector3 origin = camera->GetTransform ().GetO2TTranslation ();
  csVector3 isect, end = origin + (vw - origin) * 60;

  sector->HitBeam (origin, end, isect);
  iPolygon3D* poly = NULL;
  iObject* sel = sector->HitBeam (origin, end, isect, &poly);
  const char* poly_name;
  unsigned long poly_id;
  if (poly)
  {
    poly_name = poly->QueryObject ()->GetName ();
    poly_id = poly->GetPolygonID ();
    Dump (poly);
  }
  else
  {
    poly_name = NULL;
    poly_id = 0;
  }

  vw = isect;
  v = camera->GetTransform ().Other2This (vw);
  System->Printf (CS_MSG_CONSOLE,
    "LMB down : cam:(%f,%f,%f) world:(%f,%f,%f) poly:'%s'/%d\n",
    v.x, v.y, v.z, vw.x, vw.y, vw.z, poly_name ? poly_name : "<none>", poly_id);
  System->Printf (CS_MSG_DEBUG_0,
    "LMB down : cam:(%f,%f,%f) world:(%f,%f,%f) poly:'%s'/%d\n",
    v.x, v.y, v.z, vw.x, vw.y, vw.z, poly_name ? poly_name : "<none>", poly_id);

  if (sel)
  {
    iMeshWrapper* mesh = SCF_QUERY_INTERFACE (sel, iMeshWrapper);
    if (mesh)
    {
      // First release the ref to the previous selected_mesh.
      if (selected_mesh) selected_mesh->DecRef ();

      // The SCF_QUERY_INTERFACE increased the reference to this mesh. We
      // don't DecRef() it to make sure it doesn't get deleted while
      // BugPlug still has a reference.
      // But of course we have to make sure that it will actually get
      // deleted when the app/engine wants it deleted. So BugPlug will
      // monitor the ref count of this mesh and decrease its own reference
      // as soon as the ref count reaches one.
      selected_mesh = mesh;
      const char* n = selected_mesh->QueryObject ()->GetName ();
      System->Printf (CS_MSG_CONSOLE, "BugPlug found mesh '%s'!\n",
      	n ? n : "<noname>");
      bool bbox, rad, bm;
      shadow->GetShowOptions (bbox, rad, bm);
      shadow->SetShadowMesh (selected_mesh);

      shadow->SetBeam (origin, end, isect);
      if (bbox || rad || bm)
	shadow->AddToEngine (Engine);
      else
	shadow->RemoveFromEngine (Engine);
    }
  }
}

bool csBugPlug::EatMouse (iEvent& event)
{
  SetupPlugin ();
  if (!process_next_mouse) return false;

  bool down = (event.Type == csevMouseDown);
  int button = event.Mouse.Button;

  if (down)
  {
    mouse_x = event.Mouse.x;
    mouse_y = event.Mouse.y;
    UnleashSpider (DEBUGCMD_MOUSE1+button-1);
    process_next_mouse = false;
  }
  return true;
}

bool csBugPlug::EatKey (iEvent& event)
{
  SetupPlugin ();
  int key = event.Key.Code;
  bool down = (event.Type == csevKeyDown);
  bool shift = (event.Key.Modifiers & CSMASK_SHIFT) != 0;
  bool alt = (event.Key.Modifiers & CSMASK_ALT) != 0;
  bool ctrl = (event.Key.Modifiers & CSMASK_CTRL) != 0;

  // If we are in edit mode we do special processing.
  if (edit_mode)
  {
    if (down)
    {
      int l = strlen (edit_string);
      if (key == '\n')
      {
        // Exit edit_mode.
	edit_mode = false;
	ExitEditMode ();
      }
      else if (key == '\b')
      {
        // Backspace.
	if (edit_cursor > 0)
	{
	  edit_cursor--;
	  edit_string[edit_cursor] = 0;
	}
      }
      else if (key == CSKEY_DEL)
      {
        // Delete.
	int i;
	for (i = edit_cursor ; i < 79 ; i++)
	  edit_string[i] = edit_string[i+1];
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
        if (edit_cursor > 0) edit_cursor--;
      }
      else if (key == CSKEY_RIGHT)
      {
        if (edit_cursor < l) edit_cursor++;
      }
      else if (key == CSKEY_ESC)
      {
        // Cancel.
	edit_string[0] = 0;
	edit_mode = false;
      }
      else if (edit_cursor < 79)
      {
        int i;
	for (i = 78 ; i >= edit_cursor ; i--)
	  edit_string[i+1] = edit_string[i];
        edit_string[edit_cursor] = key;
	edit_cursor++;
      }
    }
    return true;
  }

  // Get command.
  int cmd = GetCommandCode (key, shift, alt, ctrl);
  if (down)
  {
    // First we check if it is the 'debug enter' key.
    if (cmd == DEBUGCMD_DEBUGENTER)
    {
      process_next_key = !process_next_key;
      if (process_next_key)
      {
        System->Printf (CS_MSG_CONSOLE, "Press debug key...\n");
      }
      else
      {
        System->Printf (CS_MSG_CONSOLE, "Back to normal key processing.\n");
      }
      return true;
    }
    if (cmd == DEBUGCMD_MOUSEENTER)
    {
      process_next_mouse = !process_next_mouse;
      if (process_next_mouse)
      {
        System->Printf (CS_MSG_CONSOLE, "Click on screen...\n");
      }
      return true;
    }
  }

  // Return false if we are not processing our own keys.
  if (!process_next_key) return false;
  if (down)
  {
    char buf[100];
    switch (cmd)
    {
      case DEBUGCMD_UNKNOWN:
        return true;
      case DEBUGCMD_QUIT:
        System->Printf (CS_MSG_CONSOLE, "Nah nah! I will NOT quit!\n");
        break;
      case DEBUGCMD_STATUS:
        System->Printf (CS_MSG_CONSOLE, "I'm running smoothly, thank you...\n");
        break;
      case DEBUGCMD_HELP:
        System->Printf (CS_MSG_CONSOLE, "Sorry, cannot help you yet.\n");
        break;
      case DEBUGCMD_DUMPENG:
        if (Engine)
	{
          System->Printf (CS_MSG_CONSOLE,
		"Dumping entire engine contents to debug.txt.\n");
	  Dump (Engine);
	  System->Printf (CS_MSG_DEBUG_0F, "\n");
	}
        break;
      case DEBUGCMD_DUMPSEC:
        System->Printf (CS_MSG_CONSOLE, "Not implemented yet.\n");
        break;
      case DEBUGCMD_CLEAR:
        do_clear = !do_clear;
        System->Printf (CS_MSG_CONSOLE, "BugPlug %s screen clearing.\n",
	  	do_clear ? "enabled" : "disabled");
        break;
      case DEBUGCMD_EDGES:
        ToggleG3DState (G3DRENDERSTATE_EDGES, "edge drawing");
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
        if (G3D)
	{
	  G3D->ClearCache ();
          System->Printf (CS_MSG_CONSOLE,
	    "BugPlug cleared the texture cache.\n");
	}
        break;
      case DEBUGCMD_CACHEDUMP:
        if (G3D) G3D->DumpCache ();
        break;
      case DEBUGCMD_MIPMAP:
        {
	  if (!G3D) break;
	  char* choices[6] = { "on", "off", "1", "2", "3", NULL };
	  long v = G3D->GetRenderState (G3DRENDERSTATE_MIPMAPENABLE);
	  v = (v+1)%5;
	  G3D->SetRenderState (G3DRENDERSTATE_MIPMAPENABLE, v);
	  System->Printf (CS_MSG_CONSOLE, "BugPlug set mipmap to '%s'\n",
	  	choices[v]);
  	}
	break;
      case DEBUGCMD_INTER:
	{
	  if (!G3D) break;
	  char* choices[5] = { "smart", "step32", "step16", "step8", NULL };
	  long v = G3D->GetRenderState (G3DRENDERSTATE_INTERPOLATIONSTEP);
	  v = (v+1)%4;
	  G3D->SetRenderState (G3DRENDERSTATE_INTERPOLATIONSTEP, v);
	  System->Printf (CS_MSG_CONSOLE, "BugPlug set interpolation to '%s'\n",
	  	choices[v]);
	}
	break;
      case DEBUGCMD_GAMMA:
        {
	  if (!G3D) break;
	  float val = G3D->GetRenderState (G3DRENDERSTATE_GAMMACORRECTION)
		/ 65536.;
	  sprintf (buf, "%g", val);
          EnterEditMode (cmd, "Enter new gamma:", buf);
	}
        break;
      case DEBUGCMD_DBLBUFF:
        {
	  bool state = G2D->GetDoubleBufferState ();
	  state = !state;
	  if (!G2D->DoubleBuffer (state))
	  {
	    System->Printf (CS_MSG_CONSOLE,
	    	"Double buffer not supported in current video mode!\n");
	  }
	  else
	  {
	    System->Printf (CS_MSG_CONSOLE,
	    	"BugPlug %s double buffering.\n",
		state ? "enabled" : "disabled");
	  }
	}
        break;
      case DEBUGCMD_TERRVIS:
        if (Engine)
	{
	  int enable_disable = -1;
	  int i, j;
	  for (i = 0 ; i < Engine->GetSectors ()->GetSectorCount () ; i++)
	  {
	    iSector* sector = Engine->GetSectors ()->GetSector (i);
	    for (j = 0 ; j < sector->GetMeshCount () ; j++)
	    {
	      iMeshWrapper* terr = sector->GetMesh (j);
	      iTerrFuncState* st = SCF_QUERY_INTERFACE (terr->GetMeshObject (),
	      	iTerrFuncState);
	      if (st)
	      {
	        if (enable_disable == -1)
		{
		  enable_disable = (int) (!st->IsVisTestingEnabled ());
		}
		st->SetVisTesting ((bool) enable_disable);
	        st->DecRef ();
	      }
	    }
	  }
	  if (enable_disable == -1)
	  {
	    System->Printf (CS_MSG_CONSOLE,
	      "BugPlug found no terrains to work with!\n");
	  }
	  else
	  {
	    System->Printf (CS_MSG_CONSOLE,
	      "BugPlug %s terrain visibility checking!\n",
	      enable_disable ? "enabled" : "disabled");
	  }
	}
	else
	{
	  System->Printf (CS_MSG_CONSOLE,
	    	"BugPlug has no engine to work on!\n");
	}
        break;
      case DEBUGCMD_MESHBBOX:
	{
	  bool bbox, rad, bm;
	  shadow->GetShowOptions (bbox, rad, bm);
          bbox = !bbox;
	  System->Printf (CS_MSG_CONSOLE,
	    	"BugPlug %s bounding box display.\n",
		bbox ? "enabled" : "disabled");
	  shadow->SetShowOptions (bbox, rad, bm);
	  if ((bbox || rad || bm) && selected_mesh)
	    shadow->AddToEngine (Engine);
	  else
	    shadow->RemoveFromEngine (Engine);
	}
        break;
      case DEBUGCMD_MESHRAD:
        {
	  bool bbox, rad, bm;
	  shadow->GetShowOptions (bbox, rad, bm);
          rad = !rad;
	  System->Printf (CS_MSG_CONSOLE,
	    	"BugPlug %s bounding sphere display.\n",
		rad ? "enabled" : "disabled");
	  shadow->SetShowOptions (bbox, rad, bm);
	  if ((bbox || rad || bm) && selected_mesh)
	    shadow->AddToEngine (Engine);
	  else
	    shadow->RemoveFromEngine (Engine);
	}
        break;
      case DEBUGCMD_DUMPCAM:
      case DEBUGCMD_FOV:
      case DEBUGCMD_FOVANGLE:
        // Set spider on a hunt.
	UnleashSpider (cmd);
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

  if (do_clear)
  {
    G3D->BeginDraw (CSDRAW_2DGRAPHICS);
    int bgcolor_clear = G3D->GetTextureManager ()->FindRGB (0, 255, 255);
    G2D->Clear (bgcolor_clear);
  }
  if (selected_mesh)
  {
    // Here we see if we have the last ref count to the selected mesh.
    // In that case we also release it.
    if (selected_mesh->GetRefCount () == 1)
    {
      shadow->SetShadowMesh (NULL);
      shadow->RemoveFromEngine (Engine);
      selected_mesh->DecRef ();
      selected_mesh = NULL;
      System->Printf (CS_MSG_CONSOLE, "Selected mesh is deleted!");
    }
  }
  return false;
}

bool csBugPlug::HandleEndFrame (iEvent& /*event*/)
{
  SetupPlugin ();
  if (!G3D) return false;

  if (edit_mode)
  {
    G3D->BeginDraw (CSDRAW_2DGRAPHICS);
    iFontServer* fntsvr = G2D->GetFontServer ();
    CS_ASSERT (fntsvr != NULL);
    iFont* fnt = fntsvr->GetFont (0);
    if (fnt == NULL)
    {
      fnt = fntsvr->LoadFont (CSFONT_COURIER);
    }
    CS_ASSERT (fnt != NULL);
    int fw, fh;
    fnt->GetMaxSize (fw, fh);
    int sw = G2D->GetWidth ();
    int sh = G2D->GetHeight ();
    int x = 10;
    int y = sh/2 - (fh*2+5*3)/2;
    int w = sw-20;
    int h = fh*2+5*3;
    int bgcolor = G3D->GetTextureManager ()->FindRGB (255, 255, 0);
    G2D->DrawBox (x, y, w, h, bgcolor);
    int fgcolor = G3D->GetTextureManager ()->FindRGB (0, 0, 0);
    int maxlen = fnt->GetLength (msg_string, w-10);
    if (maxlen < 80) msg_string[maxlen] = 0;
    G2D->Write (fnt, x+5, y+5, fgcolor, bgcolor, msg_string);
    maxlen = fnt->GetLength (edit_string, w-10);
    if (maxlen < 80) edit_string[maxlen] = 0;
    G2D->Write (fnt, x+5, y+5+fh+5, fgcolor, bgcolor, edit_string);
    char cursor[83];
    strcpy (cursor, edit_string);
    cursor[edit_cursor] = 0;
    int cursor_w, cursor_h;
    fnt->GetDimensions (cursor, cursor_w, cursor_h);
    G2D->Write (fnt, x+5+cursor_w, y+5+fh+7, fgcolor, -1, "_");
    G3D->FinishDraw ();
    G2D->Print (NULL);
  }
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
	HideSpider (NULL);
        System->Printf (CS_MSG_CONSOLE, "Spider could not catch a camera!\n");
      }
    }
  }
  return false;
}

void csBugPlug::EnterEditMode (int cmd, const char* msg, const char* def)
{
  if (edit_mode) return;
  iFontServer* fntsvr = G2D->GetFontServer ();
  if (!fntsvr) return;	// No edit mode if no font server
  edit_mode = true;
  strcpy (msg_string, msg);
  if (def) strcpy (edit_string, def);
  else edit_string[0] = 0;
  edit_cursor = strlen (edit_string);
  edit_command = cmd;
}

void csBugPlug::ExitEditMode ()
{
  if (edit_string[0] == 0) return;
  int i;
  float f;
  switch (edit_command)
  {
    case DEBUGCMD_GAMMA:
      csScanStr (edit_string, "%f", &f);
      G3D->SetRenderState (G3DRENDERSTATE_GAMMACORRECTION,
      	QRound (f * 65536));
      break;
    case DEBUGCMD_FOV:
      csScanStr (edit_string, "%d", &i);
      spider->GetCamera ()->SetFOV (i, G3D->GetWidth ());
      break;
    case DEBUGCMD_FOVANGLE:
      csScanStr (edit_string, "%f", &f);
      spider->GetCamera ()->SetFOVAngle (f, G3D->GetWidth ());
      break;
  }
}

int csBugPlug::GetKeyCode (const char* keystring, bool& shift, bool& alt,
	bool& ctrl)
{
  shift = alt = ctrl = false;
  char* dash = strchr (keystring, '-');
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

int csBugPlug::GetCommandCode (const char* cmd)
{
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

  return DEBUGCMD_UNKNOWN;
}

int csBugPlug::GetCommandCode (int key, bool shift, bool alt, bool ctrl)
{
  csKeyMap* m = mappings;
  while (m)
  {
    if (m->key == key && m->shift == shift && m->alt == alt && m->ctrl == ctrl)
      return m->cmd;
    m = m->next;
  }
  return DEBUGCMD_UNKNOWN;
}

void csBugPlug::AddCommand (const char* keystring, const char* cmdstring)
{
  bool shift, alt, ctrl;
  int keycode = GetKeyCode (keystring, shift, alt, ctrl);
  // Check if valid key name.
  if (keycode == -1) return;

  int cmdcode = GetCommandCode (cmdstring);
  // Check if valid command name.
  if (cmdcode == DEBUGCMD_UNKNOWN) return;

  // Check if key isn't already defined.
  if (GetCommandCode (keycode, shift, alt, ctrl) != DEBUGCMD_UNKNOWN) return;

  // Make new key assignment.
  csKeyMap* map = new csKeyMap ();
  map->key = keycode;
  map->shift = shift;
  map->alt = alt;
  map->ctrl = ctrl;
  map->cmd = cmdcode;
  map->next = mappings;
  if (mappings) mappings->prev = map;
  map->prev = NULL;
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
  iFile* f = VFS->Open (filename, VFS_FILE_READ);
  if (f)
  {
    char buf[256];
    while (ReadLine (f, buf, 255))
    {
      char* del = strchr (buf, '=');
      if (del)
      {
        *del = 0;
	AddCommand (buf, del+1);
      }
      else
      {
        System->Printf (CS_MSG_WARNING,
    	  "BugPlug hit a badly formed line in '%s'!\n", filename);
	f->DecRef ();
        return;
      }
    }
    f->DecRef ();
  }
  else
  {
    System->Printf (CS_MSG_WARNING,
    	"BugPlug could not read '%s'!\n", filename);
  }
}

void csBugPlug::Dump (iEngine* engine)
{
  System->Printf (CS_MSG_DEBUG_0, "===========================================\n");
  iSectorList* sectors = engine->GetSectors ();
  iMeshList* meshes = engine->GetMeshes ();
  iMeshFactoryList* factories = engine->GetMeshFactories ();
  System->Printf (CS_MSG_DEBUG_0,
    "%d sectors, %d mesh factories, %d mesh objects\n",
    sectors->GetSectorCount (),
    factories->GetMeshFactoryCount (),
    meshes->GetMeshCount ());
  int i;
  for (i = 0 ; i < sectors->GetSectorCount () ; i++)
  {
    iSector* sector = sectors->GetSector (i);
    Dump (sector);
  }
  for (i = 0 ; i < factories->GetMeshFactoryCount () ; i++)
  {
    iMeshFactoryWrapper* meshfact = factories->GetMeshFactory (i);
    Dump (meshfact);
  }
  for (i = 0 ; i < meshes->GetMeshCount () ; i++)
  {
    iMeshWrapper* mesh = meshes->GetMesh (i);
    Dump (mesh);
  }
  System->Printf (CS_MSG_DEBUG_0, "===========================================\n");
}

void csBugPlug::Dump (iSector* sector)
{
  const char* sn = sector->QueryObject ()->GetName ();
  System->Printf (CS_MSG_DEBUG_0, "    Sector '%s' (%08lx)\n",
  	sn ? sn : "?", sector);
  System->Printf (CS_MSG_DEBUG_0, "    %d meshes, %d lights\n",
  	sector->GetMeshCount (), sector->GetLightCount ());
  int i;
  for (i = 0 ; i < sector->GetMeshCount () ; i++)
  {
    iMeshWrapper* mesh = sector->GetMesh (i);
    const char* n = mesh->QueryObject ()->GetName ();
    System->Printf (CS_MSG_DEBUG_0, "        Mesh '%s' (%08lx)\n",
    	n ? n : "?", mesh);
  }
}

void csBugPlug::Dump (iMeshWrapper* mesh)
{
  const char* mn = mesh->QueryObject ()->GetName ();
  System->Printf (CS_MSG_DEBUG_0, "    Mesh wrapper '%s' (%08lx)\n",
  	mn ? mn : "?", mesh);
  iMeshObject* obj = mesh->GetMeshObject ();
  if (!obj)
  {
    System->Printf (CS_MSG_DEBUG_0, "        Mesh object missing!\n");
  }
  else
  {
    iFactory* fact = SCF_QUERY_INTERFACE (obj, iFactory);
    if (fact)
    {
      System->Printf (CS_MSG_DEBUG_0, "        Plugin '%s'\n",
  	  fact->QueryDescription () ? fact->QueryDescription () : "NULL");
      fact->DecRef ();
    }
    csBox3 bbox;
    obj->GetObjectBoundingBox (bbox);
    System->Printf (CS_MSG_DEBUG_0, "        Object bounding box:\n");
    Dump (8, bbox);
  }
  iMovable* movable = mesh->GetMovable ();
  if (!movable)
  {
    System->Printf (CS_MSG_DEBUG_0, "        Mesh object missing!\n");
  }
  else
  {
    csReversibleTransform& trans = movable->GetTransform ();
    Dump (8, trans.GetOrigin (), "Movable origin");
    Dump (8, trans.GetO2T (), "Movable O2T");
    int cnt = movable->GetSectorCount ();
    int i;
    for (i = 0 ; i < cnt ; i++)
    {
      iSector* sec = movable->GetSector (i);
      const char* sn = sec->QueryObject ()->GetName ();
      System->Printf (CS_MSG_DEBUG_0, "        In sector '%s'\n",
      	sn ? sn : "?");
    }
  }
}

void csBugPlug::Dump (iMeshFactoryWrapper* meshfact)
{
  const char* mn = meshfact->QueryObject ()->GetName ();
  System->Printf (CS_MSG_DEBUG_0, "        Mesh factory wrapper '%s' (%08lx)\n",
  	mn ? mn : "?", meshfact);
}

void csBugPlug::Dump (int indent, const csMatrix3& m, char const* name)
{
  char ind[255];
  int i;
  for (i = 0 ; i < indent ; i++) ind[i] = ' ';
  ind[i] = 0;
  System->Printf (CS_MSG_DEBUG_0, "%sMatrix '%s':\n", ind, name);
  System->Printf (CS_MSG_DEBUG_0, "%s/\n", ind);
  System->Printf (CS_MSG_DEBUG_0, "%s| %3.2f %3.2f %3.2f\n",
  	ind, m.m11, m.m12, m.m13);
  System->Printf (CS_MSG_DEBUG_0, "%s| %3.2f %3.2f %3.2f\n",
  	ind, m.m21, m.m22, m.m23);
  System->Printf (CS_MSG_DEBUG_0, "%s| %3.2f %3.2f %3.2f\n",
  	ind, m.m31, m.m32, m.m33);
  System->Printf (CS_MSG_DEBUG_0, "%s\\\n", ind);
}

void csBugPlug::Dump (int indent, const csVector3& v, char const* name)
{
  char ind[255];
  int i;
  for (i = 0 ; i < indent ; i++) ind[i] = ' ';
  ind[i] = 0;
  System->Printf (CS_MSG_DEBUG_0,
  	"%sVector '%s': (%f,%f,%f)\n", ind, name, v.x, v.y, v.z);
}

void csBugPlug::Dump (int indent, const csVector2& v, char const* name)
{
  char ind[255];
  int i;
  for (i = 0 ; i < indent ; i++) ind[i] = ' ';
  ind[i] = 0;
  System->Printf (CS_MSG_DEBUG_0, "%sVector '%s': (%f,%f)\n",
  	ind, name, v.x, v.y);
}

void csBugPlug::Dump (int indent, const csPlane3& p)
{
  char ind[255];
  int i;
  for (i = 0 ; i < indent ; i++) ind[i] = ' ';
  ind[i] = 0;
  System->Printf (CS_MSG_DEBUG_0, "%sA=%2.2f B=%2.2f C=%2.2f D=%2.2f\n",
            ind, p.norm.x, p.norm.y, p.norm.z, p.DD);
}

void csBugPlug::Dump (int indent, const csBox2& b)
{
  char ind[255];
  int i;
  for (i = 0 ; i < indent ; i++) ind[i] = ' ';
  ind[i] = 0;
  System->Printf (CS_MSG_DEBUG_0, "%s(%2.2f,%2.2f)-(%2.2f,%2.2f)\n", ind,
  	b.MinX (), b.MinY (), b.MaxX (), b.MaxY ());
}

void csBugPlug::Dump (int indent, const csBox3& b)
{
  char ind[255];
  int i;
  for (i = 0 ; i < indent ; i++) ind[i] = ' ';
  ind[i] = 0;
  System->Printf (CS_MSG_DEBUG_0, "%s(%2.2f,%2.2f,%2.2f)-(%2.2f,%2.2f,%2.2f)\n",
  	ind, b.MinX (), b.MinY (), b.MinZ (), b.MaxX (), b.MaxY (), b.MaxZ ());
}

void csBugPlug::Dump (iCamera* c)
{
  const char* sn = c->GetSector ()->QueryObject ()->GetName ();
  if (!sn) sn = "?";
  csPlane3 far_plane;
  bool has_far_plane = c->GetFarPlane (far_plane);
  System->Printf (CS_MSG_DEBUG_0,
  	"Camera: %s (mirror=%d, fov=%d, fovangle=%g,\n",
  	sn, c->IsMirrored (), c->GetFOV (), c->GetFOVAngle ());
  System->Printf (CS_MSG_DEBUG_0, "    shiftx=%g shifty=%g camnr=%d)\n",
  	c->GetShiftX (), c->GetShiftY (), c->GetCameraNumber ());
  if (has_far_plane)
    System->Printf (CS_MSG_DEBUG_0, "    far_plane=(%g,%g,%g,%g)\n",
    	far_plane.A (), far_plane.B (), far_plane.C (), far_plane.D ());
  csReversibleTransform& trans = c->GetTransform ();
  Dump (4, trans.GetO2TTranslation (), "Camera vector");
  Dump (4, trans.GetO2T (), "Camera matrix");
}

void csBugPlug::Dump (iPolygon3D* poly)
{
  const char* poly_name = poly->QueryObject ()->GetName ();
  if (!poly_name) poly_name = "<noname>";
  unsigned long poly_id = poly->GetPolygonID ();
  System->Printf (CS_MSG_DEBUG_0, "Polygon '%s' (id=%ld)\n",
  	poly_name, poly_id);
  int nv = poly->GetVertexCount ();
  int i;
  int* idx = poly->GetVertexIndices ();
  System->Printf (CS_MSG_DEBUG_0, "  Vertices: ");
  for (i = 0 ; i < nv ; i++)
    System->Printf (CS_MSG_DEBUG_0, "%d ", idx[i]);
  System->Printf (CS_MSG_DEBUG_0, "\n");
}

bool csBugPlug::HandleEvent (iEvent& event)
{
  if (event.Type == csevKeyDown)
  {
    return EatKey (event);
  }
  else if (event.Type == csevKeyUp)
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
  }

  return false;
}

