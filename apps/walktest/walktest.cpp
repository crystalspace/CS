/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#include <stdarg.h>

#define SYSDEF_PATH
#define SYSDEF_ACCESS
#include "cssysdef.h"
#include "walktest.h"
#include "infmaze.h"
#include "hugeroom.h"
#include "command.h"
#include "csgeom/frustum.h"
#include "csengine/campos.h"
#include "csengine/region.h"
#include "csengine/csview.h"
#include "csengine/stats.h"
#include "csengine/light.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "csengine/wirefrm.h"
#include "csengine/polytext.h"
#include "csengine/polytmap.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/cbuffer.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/meshobj.h"
#include "csengine/terrobj.h"
#include "csparser/impexp.h"
#include "csgeom/csrect.h"
#include "csutil/scanstr.h"
#include "csutil/dataobj.h"
#include "csutil/csobject.h"
#include "csparser/snddatao.h"
#include "cssys/system.h"
#include "cstool/cspixmap.h"
#include "cstool/csfxscr.h"
#include "csver.h"
#include "qint.h"
#include "iutil/cfgmgr.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "isound/handle.h"
#include "isound/source.h"
#include "isound/listener.h"
#include "isound/source.h"
#include "isound/renderer.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "ivaria/collider.h"
#include "iengine/motion.h"
#include "ivaria/perfstat.h"
#include "imap/parser.h"

#if defined(OS_DOS) || defined(OS_WIN32) || defined (OS_OS2)
#  include <io.h>
#elif defined(OS_UNIX)
#  include <unistd.h>
#endif

#include "debug/fpu80x86.h"	// for debugging numerical instabilities

WalkTest *Sys;
converter *ImportExport;

#define Gfx3D Sys->myG3D
#define Gfx2D Sys->myG2D

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

REGISTER_STATIC_LIBRARY (engine)
REGISTER_STATIC_LIBRARY (lvlload)

//-----------------------------------------------------------------------------

char WalkTest::map_dir [100];
bool WalkTest::move_3d = false;

WalkTest::WalkTest () :
  SysSystemDriver (), pos (0, 0, 0), velocity (0, 0, 0)
{
  extern bool CommandHandler (const char *cmd, const char *arg);
  csCommandProcessor::ExtraHandler = CommandHandler;
  auto_script = NULL;
  view = NULL;
  infinite_maze = NULL;
  huge_room = NULL;
  wMissile_boom = NULL;
  wMissile_whoosh = NULL;
  cslogo = NULL;
  Engine = NULL;
  LevelLoader = NULL;
  anim_sky = NULL;
  anim_dirlight = NULL;
  anim_dynlight = NULL;
  
  wf = NULL;
  map_mode = MAP_OFF;
  map_projection = WF_ORTHO_PERSP;
  do_fps = true;
  do_stats = false;
  do_edges = false;
  do_show_coord = false;
  do_show_cbuffer = false;
  busy_perf_test = false;
  do_show_z = false;
  do_show_palette = false;
  do_infinite = false;
  do_huge = false;
  do_cd = true;
  do_freelook = false;
  player_spawned = false;
  do_gravity = true;
  inverse_mouse = false;
  selected_light = NULL;
  selected_polygon = NULL;
  move_forward = false;
  cfg_draw_octree = 0;
  cfg_recording = -1;
  recorded_perf_stats_name = NULL;
  recorded_perf_stats = NULL;
  perf_stats = NULL;
  recorded_cmd = NULL;
  recorded_arg = NULL;
  cfg_playrecording = -1;
  cfg_debug_check_frustum = 0;
  do_fs_inter = false;
  do_fs_shadevert = false;
  do_fs_whiteout = false;
  do_fs_blue = false;
  do_fs_red = false;
  do_fs_green = false;
  do_fs_fadetxt = false;
  do_fs_fadecol = false;
  do_fs_fadeout = false;

  plbody = pllegs = NULL;

  velocity.Set (0, 0, 0);
  angle.Set (0, 0, 0);
  angle_velocity.Set (0, 0, 0);

  timeFPS = 0.0;

  bgcolor_txtmap = 255;
  bgcolor_map = 0;

  ConsoleInput = NULL;
  SmallConsole = false;

  myG2D = NULL;
  myG3D = NULL;
  myConsole = NULL;
  myVFS = NULL;
  mySound = NULL;
  myMotionMan = NULL;

  debug_box1.Set (csVector3 (-1, -1, -1), csVector3 (1, 1, 1));
  debug_box2.Set (csVector3 (2, 2, 2), csVector3 (3, 3, 3));
  do_show_debug_boxes = false;
}

WalkTest::~WalkTest ()
{
  DEC_REF (myVFS);
  DEC_REF (myConsole);
  DEC_REF (myG2D);
  DEC_REF (myG3D);
  DEC_REF (mySound);
  DEC_REF (myMotionMan);
  if (ConsoleInput) ConsoleInput->DecRef ();
  if (collide_system) collide_system->DecRef ();
  if (Font) Font->DecRef ();
  delete wf;
  delete [] auto_script;
  delete view;
  delete infinite_maze;
  delete huge_room;
  delete cslogo;
  if (Engine)
  {
    if (plbody) Engine->RemoveMesh (plbody);
    if (pllegs) Engine->RemoveMesh (pllegs);
  }
  delete [] recorded_perf_stats_name;
  if (perf_stats) perf_stats->DecRef ();
  if (Engine) Engine->DecRef ();
  if (LevelLoader) LevelLoader->DecRef();
}

void WalkTest::SetSystemDefaults (iConfigManager *Config)
{
  superclass::SetSystemDefaults (Config);
  do_fps = Config->GetBool ("Walktest.Settings.FPS", true);
  do_stats = Config->GetBool ("Walktest.Settings.Stats", false);
  do_cd = Config->GetBool ("Walktest.Settings.Colldet", true);

  const char *val;
  if (!(val = GetNameCL ()))
    val = Config->GetStr ("Walktest.Settings.WorldFile");

  // if an absolute path is given, copy it. Otherwise prepend "/lev/".
  if (val[0] == '/')
    strcpy (map_dir, val);
  else
    sprintf (map_dir, "/lev/%s", val);
  
  if (GetOptionCL ("stats"))
  {
    do_stats = true;
    Sys->Printf (MSG_INITIALIZATION, "Statistics enabled.\n");
  }
  else if (GetOptionCL ("nostats"))
  {
    do_stats = false;
    Sys->Printf (MSG_INITIALIZATION, "Statistics disabled.\n");
  }

  if (GetOptionCL ("fps"))
  {
    do_fps = true;
    Sys->Printf (MSG_INITIALIZATION, "Frame Per Second enabled.\n");
  }
  else if (GetOptionCL ("nofps"))
  {
    do_fps = false;
    Sys->Printf (MSG_INITIALIZATION, "Frame Per Second disabled.\n");
  }

  if (GetOptionCL ("infinite"))
    do_infinite = true;

  if (GetOptionCL ("huge"))
    do_huge = true;

  extern bool do_bots;
  if (GetOptionCL ("bots"))
    do_bots = true;

  if (GetOptionCL ("colldet"))
  {
    do_cd = true;
    Sys->Printf (MSG_INITIALIZATION, "Enabled collision detection system.\n");
  }
  else if (GetOptionCL ("nocolldet"))
  {
    do_cd = false;
    Sys->Printf (MSG_INITIALIZATION, "Disabled collision detection system.\n");
  }

  if ((val = GetOptionCL ("exec")))
  {
    delete [] auto_script;
    auto_script = csStrNew (val);
  }
}

void WalkTest::Help ()
{
  SysSystemDriver::Help ();
  Sys->Printf (MSG_STDOUT, "  -exec=<script>     execute given script at startup\n");
  Sys->Printf (MSG_STDOUT, "  -[no]stats         statistics (default '%sstats')\n", do_stats ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -[no]fps           frame rate printing (default '%sfps')\n", do_fps ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -[no]colldet       collision detection system (default '%scolldet')\n", do_cd ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -infinite          special infinite level generation (ignores map file!)\n");
  Sys->Printf (MSG_STDOUT, "  -huge              special huge level generation (ignores map file!)\n");
  Sys->Printf (MSG_STDOUT, "  -bots              allow random generation of bots\n");
  Sys->Printf (MSG_STDOUT, "  <path>             load map from VFS <path> (default '%s')\n",
        GetConfig()->GetStr ("Walktest.Settings.WorldFile", "world"));
}

//-----------------------------------------------------------------------------
extern bool CommandHandler (const char *cmd, const char *arg);

void WalkTest::NextFrame ()
{
  // The following will fetch all events from queue and handle them
  SysSystemDriver::NextFrame ();

  cs_time elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);

  if (perf_stats) timeFPS = perf_stats->GetFPS ();
  else timeFPS = 0;

  if (!myConsole || !myConsole->GetVisible ())
  {
    // If the console was turned off last frame no stats have been accumulated
    // as it was paused so we return here and loop again.
    if (perf_stats && perf_stats->Pause (false))
      return;
    // Emit recorded commands directly to the CommandHandler
    if (cfg_playrecording > 0 &&
	recording.Length () > 0)
    {
      csRecordedCamera* reccam = (csRecordedCamera*)recording[cfg_playrecording];
      if (reccam->cmd)
	CommandHandler(reccam->cmd, reccam->arg);
    }
  }
  else
    // The console has been turned on so we pause the stats plugin.
    if (perf_stats) perf_stats->Pause (true);

  // Update the Motion Manager
  if (Sys->myMotionMan)
    Sys->myMotionMan->UpdateAll ();

  MoveSystems (elapsed_time, current_time);
  PrepareFrame (elapsed_time, current_time);
  DrawFrame (elapsed_time, current_time);

  // Execute one line from the script.
  if (!busy_perf_test)
  {
    char buf[256];
    if (csCommandProcessor::get_script_line (buf, 255)) csCommandProcessor::perform_line (buf);
  }
}


void WalkTest::MoveSystems (cs_time elapsed_time, cs_time current_time)
{
  // First move the sky.
  if (anim_sky)
  {
    iMovable* move = anim_sky->GetMovable ();
    switch (anim_sky_rot)
    {
      case 0:
	{
          csXRotMatrix3 mat (anim_sky_speed * 2. * M_PI
	  	* (float)elapsed_time/1000.);
          move->Transform (mat);
	  break;
	}
      case 1:
	{
          csYRotMatrix3 mat (anim_sky_speed * 2. * M_PI
	  	* (float)elapsed_time/1000.);
          move->Transform (mat);
	  break;
	}
      case 2:
	{
          csZRotMatrix3 mat (anim_sky_speed * 2. * M_PI
	  	* (float)elapsed_time/1000.);
          move->Transform (mat);
	  break;
	}
    }
    move->UpdateMove ();
  }
  // Move the directional light if any.
  if (anim_dirlight)
  {
    csVector3 pos = anim_dirlight->GetTerrainObject ()->GetDirLightPosition ();
    csColor col = anim_dirlight->GetTerrainObject ()->GetDirLightColor ();
    csYRotMatrix3 mat (.05* 2. * M_PI * (float)elapsed_time/1000.);
    pos = mat * pos;
    anim_dirlight->GetTerrainObject ()->SetDirLight (pos, col);
  }
  // Animate the psuedo-dynamic light if any.
  if (anim_dynlight)
  {
    float t = fmod (float (current_time), float(2000.)) / 2000.;
    anim_dynlight->SetColor (csColor (t, 0, 1-t));
  }

  // Update all busy entities.
  // We first push all entities in a vector so that NextFrame() can safely
  // remove it self from the busy_entities list (or add other entities).
  int i;
  busy_vector.SetLength (0);
  csWalkEntity* wentity;
  for (i = 0 ; i < busy_entities.Length () ; i++)
  {
    wentity = (csWalkEntity*)busy_entities[i];
    busy_vector.Push (wentity);
  }
  for (i = 0 ; i < busy_vector.Length () ; i++)
  {
    wentity = (csWalkEntity*)busy_vector[i];
    wentity->NextFrame (elapsed_time);
  }

  // Record the first time this routine is called.
  extern bool do_bots;
  if (do_bots)
  {
    static long first_time = -1;
    static cs_time next_bot_at;
    if (first_time == -1)
    {
      first_time = current_time;
      next_bot_at = current_time+1000*10;
    }
    if (current_time > next_bot_at)
    {
      extern void add_bot (float size, iSector* where, csVector3 const& pos,
	                        float dyn_radius);
      add_bot (2, view->GetCamera ()->GetSector (),
               view->GetCamera ()->GetTransform ().GetOrigin (), 0);
      next_bot_at = current_time+1000*10;
    }
  }
  if (!myConsole || !myConsole->GetVisible ())
  {
    int alt,shift,ctrl;
    float speed = 1;

    alt = GetKeyState (CSKEY_ALT);
    ctrl = GetKeyState (CSKEY_CTRL);
    shift = GetKeyState (CSKEY_SHIFT);
    if (ctrl)
      speed = .5;
    if (shift)
      speed = 2;

    /// Act as usual...
    strafe (0,1);
    look (0,1);
    step (0,1);
    rotate (0,1);

    if (Sys->mySound)
    {
      iSoundListener *sndListener = Sys->mySound->GetListener();
      if(sndListener)
      {
        // take position/direction from view->GetCamera ()
        csVector3 v = view->GetCamera ()->GetTransform ().GetOrigin ();
        csMatrix3 m = view->GetCamera ()->GetTransform ().GetT2O();
        csVector3 f = m.Col3();
        csVector3 t = m.Col2();
        sndListener->SetPosition(v);
        sndListener->SetDirection(f,t);
        //sndListener->SetDirection(...);
      }
    }
  }

  extern void move_bots (cs_time);
  move_bots (elapsed_time);

  if (move_forward)
    step (1, 0);
}


//-----------------------------------------------------------------------------

//@@@
int covtree_level = 1;
bool do_covtree_dump = false;

void WalkTest::DrawFrameDebug ()
{
  if (do_show_z)
  {
    extern void DrawZbuffer ();
    DrawZbuffer ();
  }
  if (do_show_palette)
  {
    extern void DrawPalette ();
    DrawPalette ();
  }
  //if (selected_polygon || selected_light)
    //view->GetEngine ()->DrawFunc (view->GetCamera (),
      //view->GetClipper (), draw_edges, (void*)1);
  if (cfg_draw_octree)
  {
    extern void DrawOctreeBoxes (int);
    DrawOctreeBoxes (cfg_draw_octree == -1 ? -1 : cfg_draw_octree-1);
  }
  if (cfg_debug_check_frustum)
  {
    // @@@
  }
  if (do_show_cbuffer)
  {
    csCBuffer* cbuf = view->GetEngine ()->GetCsEngine ()->GetCBuffer ();
    if (cbuf)
    {
      cbuf->GfxDump (Gfx2D, Gfx3D);
    }
  }
  if (do_show_debug_boxes)
  {
    extern void DrawDebugBoxes (csCamera* cam, bool do3d);
    DrawDebugBoxes (view->GetCamera ()->GetPrivateObject (), false);
  }
}

void WalkTest::DrawFrameExtraDebug ()
{
}

void WalkTest::DrawFrameDebug3D ()
{
  if (do_show_debug_boxes)
  {
    extern void DrawDebugBoxes (csCamera* cam, bool do3d);
    DrawDebugBoxes (view->GetCamera ()->GetPrivateObject (), true);
  }
}

void WalkTest::GfxWrite (int x, int y, int fg, int bg, char *str, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);

  myG2D->Write (Font, x, y, fg, bg, buf);
}

void WalkTest::DrawFrameConsole ()
{
  if (myConsole)
    myConsole->Draw2D (NULL);

  if (!myConsole || !myConsole->GetVisible ())
  {
    int fw, fh;
    Font->GetMaxSize (fw, fh);

    if (do_fps)
    {
      GfxWrite (11, FRAME_HEIGHT - fh - 3, 0, -1, "FPS=%.2f", timeFPS);
      GfxWrite (10, FRAME_HEIGHT - fh - 2, fgcolor_stats, -1, "FPS=%.2f", timeFPS);
    }
    if (do_stats)
    {
      char buffer[50];
      sprintf (buffer, "pc=%d pd=%d po=%d pa=%d pr=%d",
      	Stats::polygons_considered, Stats::polygons_drawn,
	Stats::portals_drawn, Stats::polygons_accepted,
	Stats::polygons_rejected);
      GfxWrite(FRAME_WIDTH - 30 * 8 - 1, FRAME_HEIGHT - fh - 3, 0, -1, "%s", buffer);
      GfxWrite(FRAME_WIDTH - 30 * 8, FRAME_HEIGHT - fh - 2, fgcolor_stats, -1,
      	"%s", buffer);
    }
    else if (do_show_coord)
    {
      char buffer[100];
      sprintf (buffer, "%2.2f,%2.2f,%2.2f: %s",
        view->GetCamera ()->GetTransform ().GetO2TTranslation ().x,
	view->GetCamera ()->GetTransform ().GetO2TTranslation ().y,
        view->GetCamera ()->GetTransform ().GetO2TTranslation ().z,
	view->GetCamera ()->GetSector()->QueryObject ()->GetName ());
      GfxWrite (FRAME_WIDTH - 24 * 8 - 1, FRAME_HEIGHT - fh - 3, 0, -1, buffer);
      GfxWrite (FRAME_WIDTH - 24 * 8, FRAME_HEIGHT - fh - 2, fgcolor_stats, -1, buffer);
    }
  }
}

void WalkTest::DrawFullScreenFX2D (cs_time /*elapsed_time*/,
	cs_time current_time)
{
  if (do_fs_inter)
  {
    iTextureManager* txtmgr = Gfx3D->GetTextureManager ();
    csfxInterference (Gfx2D, txtmgr, fs_inter_amount, fs_inter_anim,
    	fs_inter_length);
    fs_inter_anim = fmod (fabs (float (current_time)/3000.0), 1.);
  }
}

void WalkTest::DrawFullScreenFX3D (cs_time /*elapsed_time*/,
	cs_time current_time)
{
  if (do_fs_fadeout)
  {
    csfxFadeOut (Gfx3D, fs_fadeout_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_fadeout_fade = fmod (t3, 1.0f);
    fs_fadeout_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_fadeout_dir) fs_fadeout_fade = 1-fs_fadeout_fade;
  }
  if (do_fs_fadecol)
  {
    csfxFadeToColor (Gfx3D, fs_fadecol_fade, fs_fadecol_color);
    float t3 = fabs (float (current_time)/3000.0);
    fs_fadecol_fade = fmod (t3, 1.0f);
    fs_fadecol_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_fadecol_dir) fs_fadecol_fade = 1-fs_fadecol_fade;
  }
  if (do_fs_fadetxt)
  {
    csfxFadeTo (Gfx3D, fs_fadetxt_mat, fs_fadetxt_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_fadetxt_fade = fmod (t3, 1.0f);
    fs_fadetxt_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_fadetxt_dir) fs_fadetxt_fade = 1-fs_fadetxt_fade;
  }
  if (do_fs_red)
  {
    csfxRedScreen (Gfx3D, fs_red_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_red_fade = fmod (t3, 1.0f);
    fs_red_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_red_dir) fs_red_fade = 1-fs_red_fade;
  }
  if (do_fs_green)
  {
    csfxGreenScreen (Gfx3D, fs_green_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_green_fade = fmod (t3, 1.0f);
    fs_green_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_green_dir) fs_green_fade = 1-fs_green_fade;
  }
  if (do_fs_blue)
  {
    csfxBlueScreen (Gfx3D, fs_blue_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_blue_fade = fmod (t3, 1.0f);
    fs_blue_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_blue_dir) fs_blue_fade = 1-fs_blue_fade;
  }
  if (do_fs_whiteout)
  {
    csfxWhiteOut (Gfx3D, fs_whiteout_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_whiteout_fade = fmod (t3, 1.0f);
    fs_whiteout_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_whiteout_dir) fs_whiteout_fade = 1-fs_whiteout_fade;
  }
  if (do_fs_shadevert)
  {
    csfxShadeVert (Gfx3D, fs_shadevert_topcol, fs_shadevert_botcol,
    	CS_FX_ADD);
  }
}

void WalkTest::DrawFrame3D (int drawflags, cs_time /*current_time*/)
{
  // Tell Gfx3D we're going to display 3D things
  if (!Gfx3D->BeginDraw (Engine->GetBeginDrawFlags () | drawflags
  	| CSDRAW_3DGRAPHICS))
    return;

  // Apply lighting BEFORE the very first frame
  csEngine* engine = (csEngine*)Engine;
  csDynLight* dyn = engine->GetFirstDynLight ();
  while (dyn)
  {
    extern void HandleDynLight (iDynLight*);
    csDynLight* dn = dyn->GetNext ();
    if (GET_CHILD_OBJECT_FAST (dyn, iDataObject))
      HandleDynLight (&(dyn->scfiDynLight));
    dyn = dn;
  }
  // Apply lighting to all meshes
  light_statics ();

  //------------
  // Here comes the main call to the engine. view->Draw() actually
  // takes the current camera and starts rendering.
  //------------
  if (map_mode != MAP_ON && map_mode != MAP_TXT && !do_covtree_dump)
    view->Draw ();

  // no need to clear screen anymore
  drawflags = 0;

  // Display the 3D parts of the console
  if (myConsole)
    myConsole->Draw3D (NULL);
}


void WalkTest::DrawFrame2D (void)
{
  if (cslogo)
  {
    unsigned w = cslogo->Width()  * FRAME_WIDTH  / 640;
    unsigned h = cslogo->Height() * FRAME_HEIGHT / 480;
    cslogo->DrawScaled (Gfx3D, FRAME_WIDTH - 2 - (w * 151) / 256 , 2, w, h);
  }

  // White-board for debugging purposes.
  if (do_covtree_dump)
    DrawFrameExtraDebug ();
}

void WalkTest::DrawFrameMap ()
{
#if 0
//@@@
  if (map_mode == MAP_TXT)
  {
    // Texture mapped map.
    csPolyIt* pi = view->GetEngine ()->NewPolyIterator ();
    csPolygon3D* p;
    csCamera* cam = wf->GetCamera ();
    const csVector3& c = cam->GetOrigin ();
    float scale = 10.;
    csBox2 box (2, 2, FRAME_WIDTH-2, FRAME_HEIGHT-2);
    csClipper* clipper = new csBoxClipper (box);
    csVector2 maxdist (FRAME_WIDTH/(2*scale), FRAME_HEIGHT/(2*scale));
    csPoly2DPool* render_pool = view->GetEngine ()->render_pol2d_pool;
    Gfx3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);
    int i;
    bool in;
    while ((p = pi->Fetch ()) != NULL)
    {
      if (p->GetPortal ()) continue;
      in = false;
      for (i = 0 ; i < p->GetVertexCount () ; i++)
      {
	const csVector3& v = p->Vwor (i);
	if (ABS (v.x - c.x) < maxdist.x &&
	    ABS (v.z - c.z) < maxdist.y &&
	    v.y <= c.y && v.y >= (c.y-5))
	{
	  in = true;
	  break;
	}
      }
      if (in)
      {
        csPolygon2D* clip = (csPolygon2D*)(render_pool->Alloc ());
	clip->MakeEmpty ();
	for (i = 0 ; i < p->GetVertexCount () ; i++)
	{
	  const csVector3& v = p->Vwor (i);
	  csVector2 v2;
	  v2.x = (v-c).x;
	  v2.y = (v-c).z;
	  v2 *= scale;
	  v2.x += FRAME_WIDTH/2;
	  v2.y += FRAME_HEIGHT/2;
	  clip->AddVertex (v2);
	}
	if (clip->ClipAgainst (clipper))
	{
  	  if (p->GetTextureType () != POLYTXT_LIGHTMAP)
  	  {
	    // @@@ Unsupported for now.
	  }
	  else
	  {
    	    static G3DPolygonDP g3dpoly;
	    g3dpoly.num = clip->GetVertexCount ();
    	    g3dpoly.mat_handle = p->GetMaterialHandle ();
    	    g3dpoly.inv_aspect = view->GetCamera ()->GetInvFOV ();
	    for (i = 0 ; i <g3dpoly.num ; i++)
	    {
	      g3dpoly.vertices[i].sx = clip->GetVertex (i)->x;
	      g3dpoly.vertices[i].sy = clip->GetVertex (i)->y;
	    }
	    g3dpoly.alpha = 0;
	    g3dpoly.z_value = p->Vwor (0).y;
	    g3dpoly.poly_texture = p->GetLightMapInfo ()->GetPolyTex ();

	    csPolyTxtPlane* txt_plane = p->GetLightMapInfo ()->GetTxtPlane ();
    	    csMatrix3 m_cam2tex;
    	    csVector3 v_cam2tex;
	    txt_plane->GetTextureSpace (m_cam2tex, v_cam2tex);
	    float s;
	    s = m_cam2tex.m12; m_cam2tex.m12 = m_cam2tex.m13; m_cam2tex.m13 = s;
	    s = m_cam2tex.m22; m_cam2tex.m22 = m_cam2tex.m23; m_cam2tex.m23 = s;
	    s = m_cam2tex.m32; m_cam2tex.m32 = m_cam2tex.m33; m_cam2tex.m33 = s;
	    s = v_cam2tex.y; v_cam2tex.y = v_cam2tex.z; v_cam2tex.z = s;
    	    g3dpoly.plane.m_cam2tex = &m_cam2tex;
    	    g3dpoly.plane.v_cam2tex = &v_cam2tex;

	    csPlane3* plane = p->GetPolyPlane ();
    	    g3dpoly.normal.A () = plane->A ();
    	    g3dpoly.normal.B () = plane->C ();
    	    g3dpoly.normal.C () = plane->B ();
    	    g3dpoly.normal.D () = plane->D ();

	    Gfx3D->DrawPolygon (g3dpoly);
	  }
	}
        render_pool->Free (clip);
      }
    }

    delete pi;
    delete clipper;
  }
  else
  {
    // Wireframe map.
    wf->GetWireframe ()->Clear ();
    extern void draw_map (csRenderView*, int, void*);
    view->GetEngine ()->DrawFunc (view->GetCamera (),
    	view->GetClipper (), draw_map);
    wf->GetWireframe ()->Draw (Gfx3D, wf->GetCamera (), map_projection);
  }
#endif
}

void WalkTest::DrawFrame (cs_time elapsed_time, cs_time current_time)
{
  if (!myConsole || !myConsole->GetVisible ())
  {
    if (cfg_recording >= 0)
    {
      // @@@ Memory leak!
      csRecordedCamera* reccam = new csRecordedCamera ();
      iCamera* c = view->GetCamera ();
      const csMatrix3& m = c->GetTransform ().GetO2T ();
      const csVector3& v = c->GetTransform ().GetOrigin ();
      reccam->mat = m;
      reccam->vec = v;
      reccam->mirror = c->IsMirrored ();
      reccam->sector = c->GetSector ();
      reccam->angle = Sys->angle;
      reccam->cmd = recorded_cmd;
      reccam->arg = recorded_arg;
      recorded_cmd = recorded_arg = NULL;
      recording.Push ((void*)reccam);
    }
    if (cfg_playrecording >= 0 && recording.Length () > 0)
    {
      csRecordedCamera* reccam = (csRecordedCamera*)recording[cfg_playrecording];
      cfg_playrecording++;
      if (cfg_playrecording >= recording.Length ()) 
      {
	if (cfg_playloop)
	  cfg_playrecording = 0;
	else
	{
	  // A performance measuring demo has finished..stop and write to
	  // file
	  cfg_playrecording = -1;
	  if (perf_stats) perf_stats->FinishSubsection ();
	  recorded_perf_stats = NULL;
	  Sys->Printf (MSG_CONSOLE, "Demo '%s' finished\n", 
		       recorded_perf_stats_name);
	}
      }
      iCamera* c = view->GetCamera ();
      Sys->angle = reccam->angle;
      c->SetSector (reccam->sector);
      c->SetMirrored (reccam->mirror);
      c->GetTransform ().SetO2T (reccam->mat);
      c->GetTransform ().SetOrigin (reccam->vec);
    }
  }


  (void)elapsed_time; (void)current_time;

  //not used since we need WHITE background not black
  int drawflags = (map_mode == MAP_TXT) ? CSDRAW_CLEARZBUFFER : 0;
  if (map_mode == MAP_ON || map_mode == MAP_TXT)
  {
    if (!Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS))
      return;
    int col = 0;
    if (map_mode == MAP_ON) col = bgcolor_map;
    else if (map_mode == MAP_TXT) col = bgcolor_txtmap;
    Gfx2D->Clear (col);
  }

  if (!myConsole
   || !myConsole->GetVisible ()
   || SmallConsole
   || myConsole->GetTransparency ())
  {
    DrawFrame3D (drawflags, current_time);
    DrawFrameDebug3D ();
    DrawFullScreenFX3D (elapsed_time, current_time);
  }

  // Start drawing 2D graphics
  if (!Gfx3D->BeginDraw (drawflags | CSDRAW_2DGRAPHICS))
    return;

  if (!myConsole
   || !myConsole->GetVisible ()
   || SmallConsole
   || myConsole->GetTransparency ())
  {
    DrawFullScreenFX2D (elapsed_time, current_time);
  }

  if (map_mode != MAP_OFF)
    DrawFrameMap ();
  else
    DrawFrameDebug ();

  DrawFrameConsole ();

  // If console is not active we draw a few additional things.
  if (!myConsole
   || !myConsole->GetVisible ())
    DrawFrame2D ();

  // Drawing code ends here
  Gfx3D->FinishDraw ();
  // Print the output.
  Gfx3D->Print (NULL);
}

int cnt = 1;
cs_time time0 = (cs_time)-1;

void WalkTest::PrepareFrame (cs_time elapsed_time, cs_time current_time)
{
  static cs_time prev_time = 0;
  if (prev_time == 0) prev_time = Time () - 10;

  // If the time interval is too big, limit to something reasonable
  // This will help a little for software OpenGL :-)
  elapsed_time = current_time - prev_time;
  if (elapsed_time > 250)
    prev_time = current_time - (elapsed_time = 250);

  if (do_cd)
  {
    extern void DoGravity (csVector3& pos, csVector3& vel);
    if (!player_spawned)
    {
      CreateColliders ();
      player_spawned=true;
    }

    for (; elapsed_time >= 10; elapsed_time -= 10, prev_time += 10)
    {
      if (move_3d)
      {
        // If we are moving in 3d then don't do any camera correction.
      }
      else
      {
        view->GetCamera ()->GetTransform ().SetT2O (csMatrix3 ());
        view->GetCamera ()->GetTransform ().RotateOther (csVector3 (0,1,0), angle.y);
        if (!do_gravity)
          view->GetCamera ()->GetTransform ().RotateThis (csVector3 (1,0,0), angle.x);
      }

      csVector3 vel = view->GetCamera ()->GetTransform ().GetT2O ()*velocity;

      static bool check_once = false;
      if (ABS (vel.x) < SMALL_EPSILON && ABS (vel.y) < SMALL_EPSILON && ABS (vel.z) < SMALL_EPSILON)
      {
        // If we don't move we don't have to do the collision detection tests.
	// However, we need to do it once to make sure that we are standing
	// on solid ground. So we first set 'check_once' to true to enable
	// one more test.
	if (check_once == false) { check_once = true; DoGravity (pos, vel); }
      }
      else { check_once = false; DoGravity (pos, vel); }

      if (do_gravity && !move_3d)
        view->GetCamera ()->GetTransform ().RotateThis (csVector3 (1,0,0), angle.x);

      // Apply angle velocity to camera angle
      angle += angle_velocity;
    }
  }

#if 0
  if (do_cd && csBeing::init)
  {
    // TODO ALEX: In future this should depend on whether the whole world
    // or 'active' sectors need to be set up as monsters hunt for player
    // outside of current sector, but this will do for now.

    // Test camera collision.
    // Load camera location into player.
    csBeing::player->sector = view->GetCamera ()->GetSector ();
    csBeing::player->transform = view->GetCamera ();
    collcount = csBeing::player->CollisionDetect ();
    // Load player transformation back into camera.
    view->GetCamera ()->SetW2C (csBeing::player->transform->GetO2T ());
    view->GetCamera ()->SetPosition (csBeing::player->transform->GetO2TTranslation ());
    view->GetCamera ()->SetSector (csBeing::player->sector);

  }
#endif
}

void perf_test (int num)
{
  Sys->busy_perf_test = true;
  cs_time t1, t2, t;
  Sys->Printf (MSG_CONSOLE, "Performance test busy...\n");
  t = t1 = Sys->Time ();
  int i;
  for (i = 0 ; i < num ; i++)
  {
    Sys->DrawFrame (Sys->Time ()-t, Sys->Time ());
    t = Sys->Time ();
  }
  t2 = Sys->Time ();
  Sys->Printf (MSG_CONSOLE, "%f secs to render %d frames: %f fps\n",
        (float)(t2-t1)/1000., num, 100000./(float)(t2-t1));
  Sys->Printf (MSG_DEBUG_0, "%f secs to render %d frames: %f fps\n",
        (float)(t2-t1)/1000., num, 100000./(float)(t2-t1));
  cnt = 1;
  time0 = (cs_time)-1;
  Sys->busy_perf_test = false;
}

void CaptureScreen ()
{
  int i = 0;
  char name [25];
  do
  {
    sprintf (name, "/this/cryst%03d.png", i++);
  } while (i < 1000 && Sys->myVFS->Exists(name));
  if (i >= 1000)
  {
    System->Printf (MSG_CONSOLE, "Too many screenshot files in current directory\n");
    return;
  }

  iImage *img = Gfx2D->ScreenShot ();
  if (!img)
  {
    Sys->Printf (MSG_CONSOLE, "The 2D graphics driver does not support screen shots\n");
    return;
  }
  iImageIO *imageio = QUERY_PLUGIN (Sys, iImageIO);
  if (imageio)
  {
    iDataBuffer *db = imageio->Save (img, "image/png");
    if (db)
    {
       Sys->Printf (MSG_CONSOLE, "Screenshot: %s\n", name);
      if (!Sys->myVFS->WriteFile (name, (const char*)db->GetData (), db->GetSize ()))
        Sys->Printf (MSG_CONSOLE, "There was an error while writing screen shot\n");
      db->DecRef ();
    }
    imageio->DecRef ();
  }
  img->DecRef ();
}

/*---------------------------------------------
 * Our main event loop.
 *---------------------------------------------*/

void cleanup ()
{
  Sys->console_out ("Cleaning up...\n");
  free_keymap ();
  Sys->EndEngine ();
  delete Sys; Sys = NULL;
}

void start_console ()
{
  iTextureManager* txtmgr = Gfx3D->GetTextureManager ();

  // Initialize the texture manager
  txtmgr->ResetPalette ();

  // Allocate a uniformly distributed in R,G,B space palette for console
  int r,g,b;
  for (r = 0; r < 8; r++)
    for (g = 0; g < 8; g++)
      for (b = 0; b < 4; b++)
        txtmgr->ReserveColor (r * 32, g * 32, b * 64);

  txtmgr->PrepareTextures ();
  txtmgr->SetPalette ();
}

void WalkTest::EndEngine ()
{
  delete view; view = NULL;
}

void WalkTest::InitCollDet (iEngine* engine, iRegion* region)
{
  Sys->Printf (MSG_INITIALIZATION, "Computing OBBs ...\n");

  iPolygonMesh* mesh;
  int sn = engine->GetSectorCount ();
  while (sn > 0)
  {
    sn--;
    iSector* sp = engine->GetSector (sn);
    if (region && !region->IsInRegion (sp->QueryObject ())) continue;
    // Initialize the things in this sector.
    int i;
    for (i = 0 ; i < sp->GetMeshCount () ; i++)
    {
      iMeshWrapper* tp = sp->GetMesh (i);
      mesh = QUERY_INTERFACE (tp->GetMeshObject (), iPolygonMesh);
      if (mesh)
      {
        (void)new csColliderWrapper (tp->QueryObject (), collide_system, mesh);
        mesh->DecRef ();
      }
    }
  }
  // Initialize all mesh objects for collision detection.
  int i;
  for (i = 0 ; i < engine->GetMeshObjectCount () ; i++)
  {
    iMeshWrapper* sp = engine->GetMeshObject (i);
    if (region && !region->IsInRegion (sp->QueryObject ())) continue;
    mesh = QUERY_INTERFACE (sp->GetMeshObject (), iPolygonMesh);
    if (mesh)
    {
      (void)new csColliderWrapper (sp->QueryObject (), collide_system, mesh);
      mesh->DecRef ();
    }
  }

  // Create a player object that follows the camera around.
//  player = csBeing::PlayerSpawn("Player");

//  init = true;
//  Sys->Printf (MSG_INITIALIZATION, "DONE\n");
}

void WalkTest::LoadLibraryData(void)
{
  // Load the "standard" library
  LevelLoader->LoadLibraryFile ("/lib/std/library");
}

void WalkTest::Inititalize2DTextures ()
{
  csTextureWrapper *texh;
  csEngine* engine = (csEngine*)Engine;
  csTextureList *texlist = engine->GetTextures ();

  // Find the Crystal Space logo and set the renderer Flag to for_2d, to allow
  // the use in the 2D part.
  texh = texlist->FindByName ("cslogo");
  if (texh)
    texh->SetFlags (CS_TEXTURE_2D);
}


void WalkTest::Create2DSprites(void)
{
  int w, h;
  csTextureWrapper *texh;
  iTextureHandle* phTex;
  csEngine* engine = (csEngine*)Engine;
  csTextureList *texlist = engine->GetTextures ();

  // Create a 2D sprite for the Logo.
  texh = texlist->FindByName ("cslogo");
  if (texh)
  {
    phTex = texh->GetTextureHandle();
    if (phTex)
    {
      phTex->GetMipMapDimensions (0, w, h);
      cslogo = new csSimplePixmap (phTex, 0, 0, w, h);
    }
  }
}


bool WalkTest::Initialize (int argc, const char* const argv[], const char *iConfigName)
{
  if (!SysSystemDriver::Initialize (argc, argv, iConfigName))
  {
    Printf (MSG_FATAL_ERROR, "Failed to initialize SysSystemDriver!\n");
    return false;
  }

  myG3D = QUERY_PLUGIN_ID (this, CS_FUNCID_VIDEO, iGraphics3D);
  if (!myG3D)
  {
    Printf (MSG_FATAL_ERROR, "No iGraphics3D plugin!\n");
    return false;
  }

  myG2D = QUERY_PLUGIN (this, iGraphics2D);
  if (!myG2D)
  {
    Printf (MSG_FATAL_ERROR, "No iGraphics2D plugin!\n");
    return false;
  }

  myVFS = QUERY_PLUGIN_ID (this, CS_FUNCID_VFS, iVFS);
  if (!myVFS)
  {
    Printf (MSG_FATAL_ERROR, "No iVFS plugin!\n");
    return false;
  }

  myConsole = QUERY_PLUGIN_ID (this, CS_FUNCID_CONSOLE, iConsoleOutput);
  mySound = QUERY_PLUGIN_ID (this, CS_FUNCID_SOUND, iSoundRender);
  myMotionMan = QUERY_PLUGIN_ID (this, CS_FUNCID_MOTION, iMotionManager);

  // Some commercials...
  Printf (MSG_INITIALIZATION, "Crystal Space version %s (%s).\n", CS_VERSION, CS_RELEASE_DATE);
  Printf (MSG_INITIALIZATION, "Created by Jorrit Tyberghein and others...\n\n");

  // Get all collision detection and movement config file parameters.
  cfg_jumpspeed = GetConfig()->GetFloat ("Walktest.CollDet.JumpSpeed", 0.08);
  cfg_walk_accelerate = GetConfig()->GetFloat ("Walktest.CollDet.WalkAccelerate", 0.007);
  cfg_walk_maxspeed = GetConfig()->GetFloat ("Walktest.CollDet.WalkMaxSpeed", 0.1);
  cfg_walk_brake = GetConfig()->GetFloat ("Walktest.CollDet.WalkBrake", 0.014);
  cfg_rotate_accelerate = GetConfig()->GetFloat ("Walktest.CollDet.RotateAccelerate", 0.005);
  cfg_rotate_maxspeed = GetConfig()->GetFloat ("Walktest.CollDet.RotateMaxSpeed", 0.03);
  cfg_rotate_brake = GetConfig()->GetFloat ("Walktest.CollDet.RotateBrake", 0.015);
  cfg_look_accelerate = GetConfig()->GetFloat ("Walktest.CollDet.LookAccelerate", 0.028);
  cfg_body_height = GetConfig()->GetFloat ("Walktest.CollDet.BodyHeight", 1.4);
  cfg_body_width = GetConfig()->GetFloat ("Walktest.CollDet.BodyWidth", 0.5);
  cfg_body_depth = GetConfig()->GetFloat ("Walktest.CollDet.BodyDepth", 0.5);
  cfg_eye_offset = GetConfig()->GetFloat ("Walktest.CollDet.EyeOffset", -0.7);
  cfg_legs_width = GetConfig()->GetFloat ("Walktest.CollDet.LegsWidth", 0.4);
  cfg_legs_depth = GetConfig()->GetFloat ("Walktest.CollDet.LegsDepth", 0.4);
  cfg_legs_offset = GetConfig()->GetFloat ("Walktest.CollDet.LegsOffset", -1.1);

  //--- create the converter class for testing
  ImportExport = new converter();
  // process import/export files from config and print log for testing
  ImportExport->ProcessConfig (GetConfig());
  // free memory - delete this if you want to use the data in the buffer
  delete ImportExport;
  //--- end converter test

#ifdef CS_DEBUG
  // enable all kinds of useful FPU exceptions on a x86
  // note that we can't do it above since at least on OS/2 each dynamic
  // library on loading/initialization resets the control word to default
  _control87 (0x33, 0x3f);
#else
  // this will disable exceptions on DJGPP (for the non-debug version)
  _control87 (0x3f, 0x3f);
#endif

  // Start the engine
  if (!Open ("Crystal Space"))
  {
    Printf (MSG_FATAL_ERROR, "Error opening system!\n");
    return false;
  }

  // Find the font we'll use
  Font = Gfx2D->GetFontServer ()->LoadFont (CSFONT_LARGE);

  // Open the startup console
  start_console ();

  // Set texture manager mode to verbose (useful for debugging)
  iTextureManager* txtmgr = Gfx3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // Find the engine plugin and query the csEngine object from it...
  Engine = QUERY_PLUGIN (Sys, iEngine);
  if (!Engine)
  {
    Printf (MSG_FATAL_ERROR, "No iEngine plugin!\n");
    return false;
  }

  // Find the level loader plugin
  LevelLoader = QUERY_PLUGIN_ID (Sys, CS_FUNCID_LVLLOADER, iLoader);
  if (!LevelLoader)
  {
    Printf (MSG_FATAL_ERROR, "No level loader plugin!\n");
    return false;
  }

  // performance statistics module, also takes care of fps
  perf_stats = QUERY_PLUGIN (this, iPerfStats);
  if (!perf_stats)
  {
    Printf (MSG_WARNING, "No iPerfStats plugin: you will have no performance statistics!\n");
  }

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView ((csEngine*)Engine, Gfx3D);

  // Get the collide system plugin.
  const char* p = GetConfig()->GetStr ("Walktest.Settings.CollDetPlugIn",
  	"crystalspace.collisiondetection.rapid");
  collide_system = LOAD_PLUGIN (Sys, p, "CollDet", iCollideSystem);
  if (!collide_system)
  {
    Printf (MSG_FATAL_ERROR, "No Collision Detection plugin found!\n");
    return false;
  }

  // Initialize the command processor with the engine and camera.
  csCommandProcessor::Initialize ((csEngine*)Engine,
    view->GetCamera ()->GetPrivateObject (), Gfx3D,
    Sys->myConsole, System);

  // Now we have two choices. Either we create an infinite
  // maze (random). This happens when the '-infinite' commandline
  // option is given. Otherwise we load the given map.
  iSector* room;

  if (do_infinite || do_huge)
  {
    // The infinite maze.

    if (!myVFS->ChDir ("/tmp"))
    {
      Printf (MSG_FATAL_ERROR, "Temporary directory /tmp not mounted on VFS!\n");
      return false;
    }

    Printf (MSG_INITIALIZATION, "Creating initial room!...\n");
    Engine->EnableLightingCache (false);

    // Unfortunately the current movement system does not allow the user to
    // move around the maze unless collision detection is enabled, even
    // though collision detection does not really make sense in this context.
    // Hopefully the movement system will be fixed some day so that the user
    // can move around even with collision detection disabled.
    do_cd = true;

    // Load two textures that are used in the maze.
    LevelLoader->LoadTexture ("txt", "/lib/std/stone4.gif");
    LevelLoader->LoadTexture ("txt2", "/lib/std/mystone2.gif");

    if (do_infinite)
    {
      // Create the initial (non-random) part of the maze.
      infinite_maze = new InfiniteMaze ();
      room = infinite_maze->create_six_room (Engine, 0, 0, 0)->sector;
      infinite_maze->create_six_room (Engine, 0, 0, 1);
      infinite_maze->create_six_room (Engine, 0, 0, 2);
      infinite_maze->create_six_room (Engine, 1, 0, 2);
      infinite_maze->create_six_room (Engine, 0, 1, 2);
      infinite_maze->create_six_room (Engine, 1, 1, 2);
      infinite_maze->create_six_room (Engine, 0, 0, 3);
      infinite_maze->create_six_room (Engine, 0, 0, 4);
      infinite_maze->create_six_room (Engine, -1, 0, 4);
      infinite_maze->create_six_room (Engine, -2, 0, 4);
      infinite_maze->create_six_room (Engine, 0, -1, 3);
      infinite_maze->create_six_room (Engine, 0, -2, 3);
      infinite_maze->create_six_room (Engine, 0, 1, 3);
      infinite_maze->create_six_room (Engine, 0, 2, 3);
      infinite_maze->connect_infinite (0, 0, 0, 0, 0, 1);
      infinite_maze->connect_infinite (0, 0, 1, 0, 0, 2);
      infinite_maze->connect_infinite (0, 0, 2, 0, 0, 3);
      infinite_maze->connect_infinite (0, 0, 2, 1, 0, 2);
      infinite_maze->connect_infinite (0, 0, 2, 0, 1, 2);
      infinite_maze->connect_infinite (1, 1, 2, 0, 1, 2);
      infinite_maze->connect_infinite (1, 1, 2, 1, 0, 2);
      infinite_maze->connect_infinite (0, 0, 3, 0, 0, 4);
      infinite_maze->connect_infinite (-1, 0, 4, 0, 0, 4);
      infinite_maze->connect_infinite (-2, 0, 4, -1, 0, 4);
      infinite_maze->connect_infinite (0, 0, 3, 0, -1, 3);
      infinite_maze->connect_infinite (0, -1, 3, 0, -2, 3);
      infinite_maze->connect_infinite (0, 0, 3, 0, 1, 3);
      infinite_maze->connect_infinite (0, 1, 3, 0, 2, 3);
      infinite_maze->create_loose_portal (-2, 0, 4, -2, 1, 4);
    }
    else
    {
      // Create the huge world.
      huge_room = new HugeRoom ();
      room = huge_room->create_huge_world (Engine);
    }

    // Prepare the engine. This will calculate all lighting and
    // prepare the lightmaps for the 3D rasterizer.
    Engine->Prepare ();
  }
  else
  {
    // Load from a map file.
    Printf (MSG_INITIALIZATION, "Loading map '%s'...\n", map_dir);

    // Check the map and mount it if required
    char tmp [100];
    sprintf (tmp, "%s/", map_dir);
    if (!myVFS->Exists (map_dir))
    {
      char *name = strrchr (map_dir, '/');
      if (name)
      {
        name++;
        //sprintf (tmp, "$.$/data$/%s.zip, $.$/%s.zip, $(..)$/data$/%s.zip",
        //  name, name, name);
	const char *valfiletype = "";
	valfiletype = GetConfig()->GetStr ("Walktest.Settings.WorldZipType", "");
	if (strcmp (valfiletype, "") ==0)
	{
	  valfiletype = "zip";
	}
        sprintf (tmp, "$.$/data$/%s.%s, $.$/%s.%s, $(..)$/data$/%s.%s",
           name, valfiletype, name, valfiletype, name, valfiletype );
        myVFS->Mount (map_dir, tmp);
      }
    }

    if (!myVFS->ChDir (map_dir))
    {
      Printf (MSG_FATAL_ERROR, "The directory on VFS for map file does not exist!\n");
      return false;
    }

    // Load the map from the file.
    if (!LevelLoader->LoadMapFile ("world"))
    {
      Printf (MSG_FATAL_ERROR, "Loading of map failed!\n");
      return false;
    }

    LoadLibraryData ();
    Inititalize2DTextures ();
    ParseKeyCmds ();

    // Prepare the engine. This will calculate all lighting and
    // prepare the lightmaps for the 3D rasterizer.
    Engine->Prepare ();

    Create2DSprites ();

    // Look for the start sector in this map.
    csEngine* engine = (csEngine*)Engine;
    csCameraPosition *cp = (csCameraPosition *)engine->camera_positions.FindByName ("Start");
    const char *room_name;
    if (cp)
    {
      room_name = cp->Sector;
      if (!cp->Load (view->GetCamera (), engine))
        room_name = "room";
    }
    else
      room_name = "room";

    room = Engine->FindSector (room_name);
    if (!room)
    {
      Printf (MSG_FATAL_ERROR,  "Map does not contain a room called '%s'"
        " which is used\nas a starting point!\n", room_name);
      return false;
    }
  }

  // Initialize collision detection system (even if disabled so that we can enable it later).
  InitCollDet (Engine, NULL);

  // Create a wireframe object which will be used for debugging.
  wf = new csWireFrameCam (txtmgr);

  // Load a few sounds.
  if (mySound)
  {
    iSoundWrapper *w = GET_NAMED_CHILD_OBJECT_FAST (Engine->QueryObject (),
      iSoundWrapper, "boom.wav");
    wMissile_boom = w ? w->GetSound () : NULL;

    w = GET_NAMED_CHILD_OBJECT_FAST (Engine->QueryObject (),
      iSoundWrapper, "whoosh.wav");
    wMissile_whoosh = w ? w->GetSound () : NULL;
   }

  Printf (MSG_INITIALIZATION, "--------------------------------------\n");
  if (myConsole)
  {
    myConsole->SetVisible (false);
    myConsole->AutoUpdate (false);
    ConsoleInput = QUERY_PLUGIN (System, iConsoleInput);
    if (ConsoleInput)
    {
      ConsoleInput->Bind (myConsole);
      ConsoleInput->SetPrompt ("cs# ");
      ConsoleInput->ExecuteCallback (csCommandProcessor::perform_callback, NULL);
    }

    // Set console to center of screen, if supported
    int DeltaX = myG2D->GetWidth () / 10;
    int DeltaY = myG2D->GetHeight () / 10;
    SmallConsole = myConsole->ConsoleExtension ("SetPos", DeltaX, DeltaY,
      myG2D->GetWidth () - DeltaX * 2, myG2D->GetHeight () - DeltaY * 2);
  }

  // Wait one second before starting.
  cs_time t = Sys->Time ()+1000;
  while (Sys->Time () < t) ;

  // Allocate the palette as calculated by the texture manager.
  txtmgr->SetPalette ();
  bgcolor_txtmap = txtmgr->FindRGB (128, 128, 128);
  bgcolor_map = 0;
  fgcolor_stats = txtmgr->FindRGB (255, 255, 255);

  // Reinit console object for 3D engine use.
  if (myConsole) myConsole->Clear ();

  // Initialize our 3D view.
  view->GetCamera ()->SetSector (room);
  // We use the width and height from the 3D renderer because this
  // can be different from the frame size (rendering in less res than
  // real window for example).
  int w3d = Gfx3D->GetWidth ();
  int h3d = Gfx3D->GetHeight ();
  view->SetRectangle (2, 2, w3d - 4, h3d - 4);

  // clear all backbuffers to black
  myG2D->BeginDraw ();
  myG2D->ClearAll (txtmgr->FindRGB(0,0,0));
  myG2D->FinishDraw ();

  return true;
}

#if 1
// moved this out of main() to make it easier for app developer
// to override
void CreateSystem(void)
{
  // Create the system driver object
  Sys = new WalkTest ();
}
#endif

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  // Initialize the random number generator
  srand (time (NULL));

  extern void CreateSystem(void);
  CreateSystem();
  
  // Initialize the main system. This will load all needed plugins
  // (3D, 2D, network, sound, ..., engine) and initialize them.
  if (!Sys->Initialize (argc, argv, "/config/walktest.cfg"))
  {
    Sys->Printf (MSG_FATAL_ERROR, "Error initializing system!\n");
    fatal_exit (-1, false);
  }

  // Start the 'autoexec.cfg' script and fully execute it.
  csCommandProcessor::start_script ("/config/autoexec.cfg");
  char cmd_buf[512];
  while (csCommandProcessor::get_script_line (cmd_buf, 511))
    csCommandProcessor::perform_line (cmd_buf);

  // If there is another script given on the commandline
  // start it but do not execute it yet. This will be done
  // frame by frame.
  if (Sys->auto_script)
    csCommandProcessor::start_script (Sys->auto_script);

  // The main loop.
  Sys->Loop ();

  cleanup ();

  return 1;
}

