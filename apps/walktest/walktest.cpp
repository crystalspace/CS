/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#define SYSDEF_PATH
#define SYSDEF_ACCESS
#include "sysdef.h"
#include "walktest/walktest.h"
#include "walktest/infmaze.h"
#include "walktest/hugeroom.h"
#include "version.h"
#include "qint.h"
#include "cssys/system.h"
#include "apps/support/command.h"
#include "cstools/simpcons.h"
#include "csparser/csloader.h"
#include "csgeom/frustrum.h"
#include "csengine/dumper.h"
#include "csengine/campos.h"
#include "csengine/csview.h"
#include "csengine/stats.h"
#include "csengine/light.h"
#include "csengine/dynlight.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "csengine/wirefrm.h"
#include "csengine/polytext.h"
#include "csengine/polyset.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/covtree.h"
#include "csengine/solidbsp.h"
#include "csengine/keyval.h"
#include "csscript/csscript.h"
#include "csscript/intscri.h"
#include "csengine/cspixmap.h"
#include "csengine/terrain.h"
#include "csengine/cssprite.h"
#include "csparser/impexp.h"
#include "csutil/inifile.h"
#include "csutil/csrect.h"
#include "csobject/dataobj.h"
#include "csobject/csobject.h"  
#include "csobject/objiter.h"
#include "cssfxldr/common/snddata.h"
#include "csgfxldr/pngsave.h"
#include "csparser/snddatao.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "isndsrc.h"
#include "isndlstn.h"
#include "isndbuf.h"
#include "isndrdr.h"
#include "iimage.h"

#if defined(OS_DOS) || defined(OS_WIN32) || defined (OS_OS2)
#  include <io.h>
#elif defined(OS_UNIX)
#  include <unistd.h>
#endif

#include "debug/fpu80x86.h"	// for debugging numerical instabilities

WalkTest *Sys;
converter *ImportExport;

#define Gfx3D System->G3D
#define Gfx2D System->G2D


//------------------------------ We need the VFS plugin and the 3D engine -----

REGISTER_STATIC_LIBRARY (vfs)
REGISTER_STATIC_LIBRARY (engine)

//-----------------------------------------------------------------------------

char WalkTest::world_dir [100];
bool WalkTest::move_3d = false;

WalkTest::WalkTest () :
  SysSystemDriver (), pos (0, 0, 0), velocity (0, 0, 0)
{
  extern bool CommandHandler (const char *cmd, const char *arg);
  Command::ExtraHandler = CommandHandler;
  auto_script = NULL;
  layer = NULL;
  view = NULL;
  infinite_maze = NULL;
  huge_room = NULL;
  wMissile_boom = NULL;
  wMissile_whoosh = NULL;
  cslogo = NULL;
  world = NULL;

  wf = NULL;
  map_mode = MAP_OFF;
  do_fps = true;
  do_stats = false;
  do_clear = false;
  do_edges = false;
  do_light_frust = false;
  do_show_coord = false;
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

  velocity.Set (0, 0, 0);
  angle.Set (0, 0, 0);
  angle_velocity.Set (0, 0, 0);

//pl=new PhysicsLibrary;

  timeFPS = 0.0;
}

WalkTest::~WalkTest ()
{
  if (World)
    World->DecRef ();
  CHK (delete wf);
  CHK (delete [] auto_script);
  CHK (delete layer);
  CHK (delete view);
  CHK (delete infinite_maze);
  CHK (delete huge_room);
  CHK (delete cslogo);
  CHK (delete body);
  CHK (delete legs);
}

void WalkTest::SetSystemDefaults (csIniFile *Config)
{
  superclass::SetSystemDefaults (Config);
  do_fps = Config->GetYesNo ("WalkTest", "FPS", true);
  do_stats = Config->GetYesNo ("WalkTest", "STATS", false);
  do_cd = Config->GetYesNo ("WalkTest", "COLLDET", true);

  const char *val;
  if (!(val = GetNameCL ()))
    val = Config->GetStr ("World", "WORLDFILE", "world");
  sprintf (world_dir, "/lev/%s", val);

  if (GetOptionCL ("clear"))
  {
    do_clear = true;
    Sys->Printf (MSG_INITIALIZATION, "Screen will be cleared every frame.\n");
  }
  else if (GetOptionCL ("noclear"))
  {
    do_clear = false;
    Sys->Printf (MSG_INITIALIZATION, "Screen will not be cleared every frame.\n");
  }

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
    CHK (delete [] auto_script);
    CHK (auto_script = strnew (val));
  }
}

void WalkTest::Help ()
{
  SysSystemDriver::Help ();
  Sys->Printf (MSG_STDOUT, "  -exec=<script>     execute given script at startup\n");
  Sys->Printf (MSG_STDOUT, "  -[no]clear         clear display every frame (default '%sclear')\n", do_clear ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -[no]stats         statistics (default '%sstats')\n", do_stats ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -[no]fps           frame rate printing (default '%sfps')\n", do_fps ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -[no]colldet       collision detection system (default '%scolldet')\n", do_cd ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -infinite          special infinite level generation (ignores world file!)\n");
  Sys->Printf (MSG_STDOUT, "  -huge              special huge level generation (ignores world file!)\n");
  Sys->Printf (MSG_STDOUT, "  -bots              allow random generation of bots\n");
  Sys->Printf (MSG_STDOUT, "  <path>             load world from VFS <path> (default '%s')\n", Config->GetStr ("World", "WORLDFILE", "world"));
}

//-----------------------------------------------------------------------------

void WalkTest::NextFrame (time_t elapsed_time, time_t current_time)
{
  // The following will fetch all events from queue and handle them
  SysSystemDriver::NextFrame (elapsed_time, current_time);

  Sys->world->UpdateParticleSystems(elapsed_time);

  // Record the first time this routine is called.
  extern bool do_bots;
  if (do_bots)
  {
    static long first_time = -1;
    static time_t next_bot_at;
    if (first_time == -1)
    {
      first_time = current_time;
      next_bot_at = current_time+1000*10;
    }
    if (current_time > next_bot_at)
    {
      extern void add_bot (float size, csSector* where, csVector3 const& pos,
	float dyn_radius);
      add_bot (2, view->GetCamera ()->GetSector (),
        view->GetCamera ()->GetOrigin (), 0);
      next_bot_at = current_time+1000*10;
    }
  }
  if (!System->Console->IsActive ())
  {
    int alt,shift,ctrl;
    float speed = 1;

    alt = GetKeyState (CSKEY_ALT);
    ctrl = GetKeyState (CSKEY_CTRL);
    shift = GetKeyState (CSKEY_SHIFT);
    if (ctrl) speed = .5;
    if (shift) speed = 2;

    /// Act as usual...
    strafe (0,1); look (0,1); step (0,1); rotate (0,1);

    if (Sys->Sound)
    {
      iSoundListener *sndListener = Sys->Sound->GetListener();
      if(sndListener)
      {
        // take position/direction from view->GetCamera ()
        csVector3 v = view->GetCamera ()->GetOrigin ();
        csMatrix3 m = view->GetCamera ()->GetC2W();
        csVector3 f = m.Col3();
        csVector3 t = m.Col2();
        sndListener->SetPosition(v.x, v.y, v.z);
        sndListener->SetDirection(f.x,f.y,f.z,t.x,t.y,t.z);
        //sndListener->SetDirection(...);
      }
    }
  }

  extern void move_bots (time_t);
  move_bots (elapsed_time);

  if (move_forward) step (1, 0);

  PrepareFrame (elapsed_time, current_time);
  DrawFrame (elapsed_time, current_time);

  // Execute one line from the script.
  if (!busy_perf_test)
  {
    char buf[256];
    if (Command::get_script_line (buf, 255)) Command::perform_line (buf);
  }
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
  extern void draw_edges (csRenderView*, int, void*);
  if (do_edges)
  {
    view->GetWorld ()->DrawFunc (view->GetCamera (), view->GetClipper (),
    	draw_edges);
  }
  if (selected_polygon || selected_light)
    view->GetWorld ()->DrawFunc (view->GetCamera (), view->GetClipper (),
	draw_edges, (void*)1);
  if (do_light_frust && selected_light)
  {
    extern void show_frustum (csFrustumView*, int, void*);
    ((csStatLight*)selected_light)->LightingFunc (show_frustum);
  }
  if (cfg_draw_octree)
  {
    extern void DrawOctreeBoxes (int);
    DrawOctreeBoxes (cfg_draw_octree-1);
  }
}

void WalkTest::DrawFrameExtraDebug ()
{
  csCoverageMaskTree* covtree = Sys->world->GetCovtree ();
  csSolidBsp* solidbsp = Sys->world->GetSolidBsp ();
  if (solidbsp)
  {
#   if 1
    Gfx2D->Clear (0);
    solidbsp->GfxDump (Gfx2D, Gfx3D->GetTextureManager (), covtree_level);
    extern csPolygon2D debug_poly2d;
    debug_poly2d.Draw (Gfx2D, Gfx3D->GetTextureManager ()->FindRGB (0, 255, 0));
#   elif 0
    Gfx2D->Clear (0);
    solidbsp->MakeEmpty ();
    csCamera* c = view->GetCamera ();
    csPolygon2D poly1, poly2, poly3, poly4;
    poly1.AddPerspective (c->Other2This (csVector3 (-1.6, 1, 5)));
    poly1.AddPerspective (c->Other2This (csVector3 (1, 1.6, 5)));
    poly1.AddPerspective (c->Other2This (csVector3 (1, -1, 5)));
    poly1.AddPerspective (c->Other2This (csVector3 (-1, -1.3, 5)));
    //solidbsp->InsertPolygon (poly1.GetVertices (), poly1.GetNumVertices ());
    poly2.AddPerspective (c->Other2This (csVector3 (-1.5, .5, 6)));
    poly2.AddPerspective (c->Other2This (csVector3 (.5, .5, 6)));
    poly2.AddPerspective (c->Other2This (csVector3 (.5, -1.5, 6)));
    poly2.AddPerspective (c->Other2This (csVector3 (-1.5, -1.5, 6)));
    printf ("T2:%d ", solidbsp->TestPolygon (poly2.GetVertices (),
	poly2.GetNumVertices ()));
    //printf ("P2:%d ", solidbsp->InsertPolygon (poly2.GetVertices (),
	//poly2.GetNumVertices ()));
    poly3.AddPerspective (c->Other2This (csVector3 (-.5, .15, 7)));
    poly3.AddPerspective (c->Other2This (csVector3 (1.5, .15, 7)));
    poly3.AddPerspective (c->Other2This (csVector3 (1.5, -.5, 7)));
    printf ("T3:%d ", solidbsp->TestPolygon (poly3.GetVertices (),
	poly3.GetNumVertices ()));
    printf ("P3:%d ", solidbsp->InsertPolygon (poly3.GetVertices (),
	poly3.GetNumVertices ()));
    poly4.AddPerspective (c->Other2This (csVector3 (1.5, -.5, 7)));
    poly4.AddPerspective (c->Other2This (csVector3 (1.5, .15, 7)));
    poly4.AddPerspective (c->Other2This (csVector3 (2.5, .15, 7)));
    printf ("T4:%d ", solidbsp->TestPolygon (poly4.GetVertices (),
	poly4.GetNumVertices ()));
    printf ("P4:%d\n", solidbsp->InsertPolygon (poly4.GetVertices (),
	poly4.GetNumVertices ()));
    //poly1.Draw (Gfx2D, Gfx3D->GetTextureManager ()->FindRGB (0, 100, 100));
    //poly2.Draw (Gfx2D, Gfx3D->GetTextureManager ()->FindRGB (100, 0, 100));
    poly3.Draw (Gfx2D, Gfx3D->GetTextureManager ()->FindRGB (100, 100, 0));
    poly4.Draw (Gfx2D, Gfx3D->GetTextureManager ()->FindRGB (100, 100, 100));
    solidbsp->GfxDump (Gfx2D, Gfx3D->GetTextureManager (), covtree_level);
#   endif
  }
  else if (covtree)
  {
    Gfx2D->Clear (0);
#   if 0
    covtree->GfxDump (Gfx2D, covtree_level);
#   else
    //covtree->MakeInvalid ();
    covtree->MakeEmpty ();
    csCamera* c = view->GetCamera ();
    csPolygon2D poly1, poly2, poly3;
    poly1.AddPerspective (c->Other2This (csVector3 (-1.6, 1, 5)));
    poly1.AddPerspective (c->Other2This (csVector3 (1, 1.6, 5)));
    poly1.AddPerspective (c->Other2This (csVector3 (1, -1, 5)));
    poly1.AddPerspective (c->Other2This (csVector3 (-1, -1.3, 5)));
    covtree->InsertPolygon (poly1.GetVertices (),
	poly1.GetNumVertices (), poly1.GetBoundingBox ());
    poly2.AddPerspective (c->Other2This (csVector3 (-1.5, .5, 6)));
    poly2.AddPerspective (c->Other2This (csVector3 (.5, .5, 6)));
    poly2.AddPerspective (c->Other2This (csVector3 (.5, -1.5, 6)));
    poly2.AddPerspective (c->Other2This (csVector3 (-1.5, -1.5, 6)));
    printf ("T2:%d ", covtree->TestPolygon (poly2.GetVertices (),
	poly2.GetNumVertices (), poly2.GetBoundingBox ()));
    printf ("P2:%d ", covtree->InsertPolygon (poly2.GetVertices (),
	poly2.GetNumVertices (), poly2.GetBoundingBox ()));
    poly3.AddPerspective (c->Other2This (csVector3 (-.5, .15, 7)));
    poly3.AddPerspective (c->Other2This (csVector3 (1.5, .15, 7)));
    poly3.AddPerspective (c->Other2This (csVector3 (1.5, -.5, 7)));
    printf ("T3:%d ", covtree->TestPolygon (poly3.GetVertices (),
	poly3.GetNumVertices (), poly3.GetBoundingBox ()));
    printf ("P3:%d\n", covtree->InsertPolygon (poly3.GetVertices (),
	poly3.GetNumVertices (), poly3.GetBoundingBox ()));
    covtree->GfxDump (Gfx2D, covtree_level);
    poly1.Draw (Gfx2D, 0x0303);
    poly2.Draw (Gfx2D, 0x07e0);
    poly3.Draw (Gfx2D, 0x008f);
    //covtree->TestConsistency ();
#   endif
  }
}

void WalkTest::DrawFrameConsole ()
{
  csSimpleConsole* scon = (csSimpleConsole*)System->Console;
  scon->Print (NULL);

  if (!scon->IsActive ())
  {
    if (do_fps)
    {
      GfxWrite(11, FRAME_HEIGHT-11, 0, -1, "FPS=%f", timeFPS);
      GfxWrite(10, FRAME_HEIGHT-10, scon->get_fg (), -1, "FPS=%f", timeFPS);
    }
    if (do_stats)
    {
      char buffer[50];
      sprintf (buffer, "pc=%d pd=%d po=%d pa=%d pr=%d",
      	Stats::polygons_considered, Stats::polygons_drawn,
	Stats::portals_drawn, Stats::polygons_accepted,
	Stats::polygons_rejected);
      GfxWrite(FRAME_WIDTH-30*8-1, FRAME_HEIGHT-11, 0, -1, "%s", buffer);
      GfxWrite(FRAME_WIDTH-30*8, FRAME_HEIGHT-10, scon->get_fg (), -1,
      	"%s", buffer);
    }
    else if (do_show_coord)
    {
      char buffer[100];
      sprintf (buffer, "%2.2f,%2.2f,%2.2f: %s",
        view->GetCamera ()->GetW2CTranslation ().x,
	view->GetCamera ()->GetW2CTranslation ().y,
        view->GetCamera ()->GetW2CTranslation ().z,
	view->GetCamera ()->GetSector()->GetName ());
      Gfx2D->Write(FRAME_WIDTH-24*8-1, FRAME_HEIGHT-11, 0, -1, buffer);
      Gfx2D->Write(FRAME_WIDTH-24*8, FRAME_HEIGHT-10, scon->get_fg (),
      	-1, buffer);
    }
  }
}

void WalkTest::DrawFrame (time_t elapsed_time, time_t current_time)
{
  (void)elapsed_time; (void)current_time;

  //not used since we need WHITE background not black
  int drawflags = do_clear ? CSDRAW_CLEARZBUFFER : 0;
  if (do_clear || map_mode == MAP_ON)
  {
    if (!Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS))
      return;
    Gfx2D->Clear (map_mode == MAP_ON ? 0 : 255);
  }

  if (!System->Console->IsActive ()
   || ((csSimpleConsole*)(System->Console))->IsTransparent ())
  {
    // Tell Gfx3D we're going to display 3D things
    if (!Gfx3D->BeginDraw (drawflags | CSDRAW_3DGRAPHICS))
      return;

    // Advance sprite frames
    Sys->world->AdvanceSpriteFrames (current_time);

    // Apply lighting BEFORE the very first frame
    csDynLight* dyn = Sys->world->GetFirstDynLight ();
    while (dyn)
    {
      extern void HandleDynLight (csDynLight*);
      csDynLight* dn = dyn->GetNext ();
      if (dyn->GetChild (csDataObject::Type)) HandleDynLight (dyn);
      dyn = dn;
    }
    // Apply lighting to all sprites
    light_statics ();

    //------------
    // Here comes the main call to the engine. view->Draw() actually
    // takes the current camera and starts rendering.
    //------------
    if (map_mode != MAP_ON && !do_covtree_dump)
      view->Draw ();

    // no need to clear screen anymore
    drawflags = 0;
  }

  // Start drawing 2D graphics
  if (!Gfx3D->BeginDraw (drawflags | CSDRAW_2DGRAPHICS))
    return;

  if (map_mode != MAP_OFF)
  {
    wf->GetWireframe ()->Clear ();
    extern void draw_map (csRenderView*, int, void*);
    view->GetWorld ()->DrawFunc (view->GetCamera (),
    	view->GetClipper (), draw_map);
    wf->GetWireframe ()->Draw (Gfx3D, wf->GetCamera ());
  }
  else
    DrawFrameDebug ();

  DrawFrameConsole ();

  // If console is not active we draw a few additional things.
  csSimpleConsole* scon = (csSimpleConsole*)System->Console;
  if (!scon->IsActive ())
  {
    if (cslogo)
    {
      unsigned w = cslogo->Width()  * FRAME_WIDTH  / 640;
      unsigned h = cslogo->Height() * FRAME_HEIGHT / 480;
      cslogo->Draw (Gfx3D, FRAME_WIDTH - 2 - (w * 151) / 256 , 2, w, h);
    }

    // White-board for debugging purposes.
    if (do_covtree_dump)
      DrawFrameExtraDebug ();
  }

  // Drawing code ends here
  Gfx3D->FinishDraw ();
  // Print the output.
  Gfx3D->Print (NULL);
}

int cnt = 1;
time_t time0 = (time_t)-1;

void WalkTest::PrepareFrame (time_t elapsed_time, time_t current_time)
{
  static time_t prev_time = 0;
  if (prev_time == 0) prev_time = Time () - 10;

  // If the time interval is too big, limit to something reasonable
  // This will help a little for software OpenGL :-)
  elapsed_time = current_time - prev_time;
  if (elapsed_time > 250)
    prev_time = current_time - (elapsed_time = 250);

  CLights::LightIdle (); // SJI

  if (do_cd)
  {
    extern void DoGravity (csVector3& pos, csVector3& vel);
    if (!player_spawned)
    {
      CreateColliders ();
      player_spawned=true;
    }

    for (; elapsed_time >= 25; elapsed_time -= 25, prev_time += 25)
    {
      if (move_3d)
      {
        // If we are moving in 3d then don't do any camera correction.
      }
      else
      {
        view->GetCamera ()->SetT2O (csMatrix3 ());
        view->GetCamera ()->RotateWorld (csVector3 (0,1,0), angle.y);
        if (!do_gravity)
          view->GetCamera ()->Rotate (csVector3 (1,0,0), angle.x);
      }

      csVector3 vel = view->GetCamera ()->GetT2O ()*velocity;

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
        view->GetCamera ()->Rotate (csVector3 (1,0,0), angle.x);

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

  if (cnt <= 0)
  {
    time_t time1 = SysGetTime ();
    if (time0 != (time_t)-1)
    {
      if (time1 != time0)
        timeFPS = 10000.0f / (float)(time1 - time0);
    }
    cnt = 10;
    time0 = SysGetTime ();
  }
  cnt--;

  layer->step_run ();
}

void perf_test (int num)
{
  Sys->busy_perf_test = true;
  time_t t1, t2, t;
  Sys->Printf (MSG_CONSOLE, "Performance test busy...\n");
  t = t1 = SysGetTime ();
  int i;
  for (i = 0 ; i < num ; i++)
  {
    Sys->layer->step_run ();
    Sys->DrawFrame (SysGetTime ()-t, SysGetTime ());
    t = SysGetTime ();
  }
  t2 = SysGetTime ();
  Sys->Printf (MSG_CONSOLE, "%f secs to render %d frames: %f fps\n",
        (float)(t2-t1)/1000., num, 100000./(float)(t2-t1));
  Sys->Printf (MSG_DEBUG_0, "%f secs to render %d frames: %f fps\n",
        (float)(t2-t1)/1000., num, 100000./(float)(t2-t1));
  cnt = 1;
  time0 = (time_t)-1;
  Sys->busy_perf_test = false;
}

void CaptureScreen ()
{
  int i = 0;
  char name [20];
  do
  {
    sprintf (name, "cryst%03d.png", i++);
  } while (i < 1000 && (access (name, 0) == 0));
  if (i >= 1000)
  {
    System->Printf (MSG_CONSOLE, "Too many screenshot files in current directory\n");
    return;
  }

  iImage *img = System->G2D->ScreenShot ();
  if (!img)
  {
    Sys->Printf (MSG_CONSOLE, "The 2D graphics driver does not support screen shots\n");
    return;
  }

  Sys->Printf (MSG_CONSOLE, "Screenshot: %s\n", name);
  if (!csSavePNG (name, img))
    Sys->Printf (MSG_CONSOLE, "There was an error while writing screen shot\n");
  img->DecRef ();
}

/*---------------------------------------------
 * Our main event loop.
 *---------------------------------------------*/

/*
 * Do a large debug dump just before the program
 * exits. This function can be installed as a last signal
 * handler if the program crashes (for systems that support
 * this).
 */
void debug_dump ()
{
  if (Sys->VFS)
    SaveCamera (Sys->VFS, "/this/coord.bug");
  Sys->Printf (MSG_DEBUG_0, "Camera saved in coord.bug\n");
  Dumper::dump (Sys->view->GetCamera ());
  Sys->Printf (MSG_DEBUG_0, "Camera dumped in debug.txt\n");
  Dumper::dump (Sys->world);
  Sys->Printf (MSG_DEBUG_0, "World dumped in debug.txt\n");
}

/*
 * A sample script which just prints a message on screen.
 */
bool do_message_script (IntRunScript* sc, char* data)
{
  sc->get_layer ()->message (data);
  return true;
}

//---------------------------------------------------------------------------


void cleanup ()
{
  Sys->console_out ("Cleaning up...\n");
  free_keymap ();
  Sys->EndWorld ();
  CHK (delete Sys); Sys = NULL;
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

  txtmgr->SetPalette ();

  ((csSimpleConsole *)System->Console)->SetupColors (txtmgr);
  ((csSimpleConsole *)System->Console)->SetMaxLines (1000);       // Some arbitrary high value.
  ((csSimpleConsole *)System->Console)->SetTransparent (0);

  System->ConsoleReady = true;
}

void WalkTest::EndWorld ()
{
  CHK (delete view); view = NULL;
}

void WalkTest::InitWorld (csWorld* world, csCamera* /*camera*/)
{
  Sys->Printf (MSG_INITIALIZATION, "Computing OBBs ...\n");

  int sn = world->sectors.Length ();
  while (sn > 0)
  {
    sn--;
    csSector* sp = (csSector*)world->sectors[sn];
    // Initialize the sector itself.
    CHK((void)new csRAPIDCollider(*sp, sp));
    // Initialize the things in this sector.
    csThing* tp = sp->GetFirstThing ();
    while (tp)
    {
      CHK((void)new csRAPIDCollider(*tp, tp));
      tp = (csThing*)(tp->GetNext ());
    }
  }
  // Initialize all sprites for collision detection.
  // @@@ This routine ignores 2D sprites for the moment.
  csSprite3D* spp;
  int i;
  for (i = 0 ; i < world->sprites.Length () ; i++)
  {
    csSprite* sp = (csSprite*)world->sprites[i];
    if (sp->GetType () != csSprite3D::Type) continue;
    spp = (csSprite3D*)sp;

    // TODO: Should create beings for these.
    CHK((void)new csRAPIDCollider(*spp, spp));
  }

  // Create a player object that follows the camera around.
//  player = csBeing::PlayerSpawn("Player");

//  init = true;
//  Sys->Printf (MSG_INITIALIZATION, "DONE\n");
}


bool WalkTest::Initialize (int argc, const char* const argv[], const char *iConfigName)
{
  if (!SysSystemDriver::Initialize (argc, argv, iConfigName))
    return false;

  // Get all collision detection and movement config file parameters.
  cfg_jumpspeed = Config->GetFloat ("CD", "JUMPSPEED", 0.08);
  cfg_walk_accelerate = Config->GetFloat ("CD", "WALKACCELERATE", 0.007);
  cfg_walk_maxspeed = Config->GetFloat ("CD", "WALKMAXSPEED", 0.1);
  cfg_walk_brake = Config->GetFloat ("CD", "WALKBRAKE", 0.014);
  cfg_rotate_accelerate = Config->GetFloat ("CD", "ROTATEACCELERATE", 0.005);
  cfg_rotate_maxspeed = Config->GetFloat ("CD", "ROTATEMAXSPEED", 0.03);
  cfg_rotate_brake = Config->GetFloat ("CD", "ROTATEBRAKE", 0.015);
  cfg_look_accelerate = Config->GetFloat ("CD", "LOOKACCELERATE", 0.028);
  cfg_body_height = Config->GetFloat ("CD", "BODYHEIGHT", 1.4);
  cfg_body_width = Config->GetFloat ("CD", "BODYWIDTH", 0.5);
  cfg_body_depth = Config->GetFloat ("CD", "BODYDEPTH", 0.5);
  cfg_eye_offset = Config->GetFloat ("CD", "EYEOFFSET", -0.7);
  cfg_legs_width = Config->GetFloat ("CD", "LEGSWIDTH", 0.4);
  cfg_legs_depth = Config->GetFloat ("CD", "LEGSDEPTH", 0.4);
  cfg_legs_offset = Config->GetFloat ("CD", "LEGSOFFSET", -1.1);

  //--- create the converter class for testing
  CHK (ImportExport = new converter());
  // process import/export files from config and print log for testing
  ImportExport->ProcessConfig (Config);
  // free memory - delete this if you want to use the data in the buffer
  CHK (delete ImportExport);
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

  // Create console object for text and commands.
  CHK (System->Console = new csSimpleConsole (Config, Command::SharedInstance ()));

  // Open the startup console
  start_console ();

  // Some commercials...
  Printf (MSG_INITIALIZATION, "Crystal Space version %s (%s).\n", VERSION, RELEASE_DATE);
  Printf (MSG_INITIALIZATION, "Created by Jorrit Tyberghein and others...\n\n");

  // Set texture manager mode to verbose (useful for debugging)
  iTextureManager* txtmgr = Gfx3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // Find the world plugin and query the csWorld object from it...
  World = QUERY_PLUGIN (Sys, iWorld);
  if (!World)
  {
    Printf (MSG_FATAL_ERROR, "No iWorld plugin!\n");
    return false;
  }
  world = World->GetCsWorld ();

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  CHK (view = new csView (world, Gfx3D));

  // Initialize the command processor with the world and camera.
  Command::Initialize (world, view->GetCamera (), Gfx3D, System->Console, System);

  // Create the language layer needed for scripting.
  // Also register a small C script so that it can be used
  // by levels. large.zip uses this script.
  CHK (layer = new LanguageLayer (world, view->GetCamera ()));
  int_script_reg.reg ("message", &do_message_script);

  // Now we have two choices. Either we create an infinite
  // maze (random). This happens when the '-infinite' commandline
  // option is given. Otherwise we load the given world.
  csSector* room;

  if (do_infinite || do_huge)
  {
    // The infinite maze.

    if (!VFS->ChDir ("/tmp"))
    {
      Printf (MSG_FATAL_ERROR, "Temporary directory /tmp not mounted on VFS!\n");
      return false;
    }

    Printf (MSG_INITIALIZATION, "Creating initial room!...\n");
    world->EnableLightingCache (false);

    // Unfortunately the current movement system does not allow the user to
    // move around the maze unless collision detection is enabled, even
    // though collision detection does not really make sense in this context.
    // Hopefully the movement system will be fixed some day so that the user
    // can move around even with collision detection disabled.
    do_cd = true;

    // Load two textures that are used in the maze.
    csLoader::LoadTexture (world, "txt", "/lib/std/stone4.gif");
    csLoader::LoadTexture (world, "txt2", "/lib/std/mystone2.gif");

    if (do_infinite)
    {
      // Create the initial (non-random) part of the maze.
      CHK (infinite_maze = new InfiniteMaze ());
      room = infinite_maze->create_six_room (world, 0, 0, 0)->sector;
      infinite_maze->create_six_room (world, 0, 0, 1);
      infinite_maze->create_six_room (world, 0, 0, 2);
      infinite_maze->create_six_room (world, 1, 0, 2);
      infinite_maze->create_six_room (world, 0, 1, 2);
      infinite_maze->create_six_room (world, 1, 1, 2);
      infinite_maze->create_six_room (world, 0, 0, 3);
      infinite_maze->create_six_room (world, 0, 0, 4);
      infinite_maze->create_six_room (world, -1, 0, 4);
      infinite_maze->create_six_room (world, -2, 0, 4);
      infinite_maze->create_six_room (world, 0, -1, 3);
      infinite_maze->create_six_room (world, 0, -2, 3);
      infinite_maze->create_six_room (world, 0, 1, 3);
      infinite_maze->create_six_room (world, 0, 2, 3);
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
      CHK (huge_room = new HugeRoom ());
      room = huge_room->create_huge_world (world);
    }

    // Prepare the world. This will calculate all lighting and
    // prepare the lightmaps for the 3D rasterizer.
    world->Prepare ();
  }
  else
  {
    // Load from a world file.
    Printf (MSG_INITIALIZATION, "Loading world '%s'...\n", world_dir);

    // Check the world and mount it if required
    char tmp [100];
    sprintf (tmp, "%s/", world_dir);
    if (!VFS->Exists (world_dir))
    {
      char *name = strrchr (world_dir, '/');
      if (name)
      {
        name++;
        //sprintf (tmp, "$.$/data$/%s.zip, $.$/%s.zip, $(..)$/data$/%s.zip",
        //  name, name, name);
	    const char *valfiletype = "";
	    valfiletype = Config->GetStr ("World", "WORLDZIPTYPE" "");
	    if(strcmp (valfiletype, "") ==0)
		{
	      valfiletype = "zip";
		}
        sprintf (tmp, "$.$/data$/%s.%s, $.$/%s.%s, $(..)$/data$/%s.%s",
           name, valfiletype, name, valfiletype, name, valfiletype );
        VFS->Mount (world_dir, tmp);
      }
    }

    if (!VFS->ChDir (world_dir))
    {
      Printf (MSG_FATAL_ERROR, "The directory on VFS for world file does not exist!\n");
      return false;
    }

    // Load the world from the file.
    if (!csLoader::LoadWorldFile (world, layer, "world"))
    {
      Printf (MSG_FATAL_ERROR, "Loading of world failed!\n");
      return false;
    }

    // Load the "standard" library
    csLoader::LoadLibraryFile (world, "/lib/std/library");

    // Find the Crystal Space logo and set the renderer Flag to for_2d, to allow
    // the use in the 2D part.
    csTextureList *texlist = world->GetTextures ();
    csTextureHandle *texh = texlist->FindByName ("cslogo.gif");
    if (texh) texh->flags = CS_TEXTURE_2D;

    // Prepare the world. This will calculate all lighting and
    // prepare the lightmaps for the 3D rasterizer.
    world->Prepare ();

    // Create a 2D sprite for the Logo.
    if (texh)
    {
      int w, h;
      iTextureHandle* phTex = texh->GetTextureHandle();
      phTex->GetMipMapDimensions (0, w, h);
      CHK (cslogo = new csPixmap (phTex, 0, 0, w, h));
    }

    // Look for the start sector in this world.
    csCameraPosition *cp = (csCameraPosition *)world->camera_positions.FindByName ("Start");
    const char *room_name;
    if (cp)
    {
      room_name = cp->Sector;
      if (!cp->Load (*view->GetCamera (), world))
        room_name = "room";
    }
    else
      room_name = "room";

    room = (csSector *)world->sectors.FindByName (room_name);
    if (!room)
    {
      Printf (MSG_FATAL_ERROR,  "World file does not contain a room called '%s'"
        " which is used\nas a starting point!\n", room_name);
      return false;
    }
  }

  // Initialize collision detection system (even if disabled so that we can enable it later).
  InitWorld (world, view->GetCamera ());

  // Create a wireframe object which will be used for debugging.
  CHK (wf = new csWireFrameCam (txtmgr));

  // Load a few sounds.
#ifdef DO_SOUND
  //csSoundData* w = csSoundDataObject::GetSound(*world, "tada.wav");
  //if (w && Sound) Sound->PlayEphemeral (w);

  wMissile_boom = csSoundDataObject::GetSound(*world, "boom.wav");
  wMissile_whoosh = csSoundDataObject::GetSound(*world, "whoosh.wav");
  //For 2D (nonmoveable) background sound, no control, loop
  csObjIterator sobj = world->GetIterator (csSoundDataObject::Type);
  while (!sobj.IsNull ())
  {
    //sounds (other than standard) are from a zip specified in world file 
    //SOUNDS section and specified in vfs.cfg in lib/sounds
    csSoundData* wSoundData = ((csSoundDataObject&)(*sobj)).GetSound(); 
    if (wSoundData && Sound) 
    {
      //don't play now if loaded for missile
      if ( wSoundData == csSoundDataObject::GetSound(*world, "tada.wav") ||
	   wSoundData == csSoundDataObject::GetSound(*world, "boom.wav") ||
	   wSoundData == csSoundDataObject::GetSound(*world, "whoosh.wav"))
      {
        ///++sobj;
      }
      else
      {
        Sound->PlayEphemeral (wSoundData, true);
        ///++sobj;
      }
    }
	/// fix of infinite loop if Sound is null
	++sobj;
  }
#endif

  Printf (MSG_INITIALIZATION, "--------------------------------------\n");

  // Wait one second before starting.
  long t = Sys->Time ()+1000;
  while (Sys->Time () < t) ;

  // Allocate the palette as calculated by the texture manager.
  txtmgr->SetPalette ();

  // Reinit console object for 3D engine use.
  ((csSimpleConsole *)System->Console)->SetupColors (txtmgr);
  ((csSimpleConsole *)System->Console)->SetMaxLines ();
  ((csSimpleConsole *)System->Console)->SetTransparent ();
  System->Console->Clear ();

  // Initialize our 3D view.
  view->SetSector (room);
  // We use the width and height from the 3D renderer because this
  // can be different from the frame size (rendering in less res than
  // real window for example).
  int w3d = Gfx3D->GetWidth ();
  int h3d = Gfx3D->GetHeight ();
  view->SetRectangle (2, 2, w3d - 4, h3d - 4);

  return true;
}


/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  // Initialize the random number generator
  srand (time (NULL));

  // Create the system driver object
  CHK (Sys = new WalkTest ());

  // Initialize the main system. This will load all needed plugins
  // (3D, 2D, network, sound, ..., engine) and initialize them.
  if (!Sys->Initialize (argc, argv, "/config/cryst.cfg"))
  {
    Sys->Printf (MSG_FATAL_ERROR, "Error initializing system!\n");
    fatal_exit (-1, false);
  }

  // Start the 'autoexec.cfg' script and fully execute it.
  Command::start_script ("/config/autoexec.cfg");
  char cmd_buf[512];
  while (Command::get_script_line (cmd_buf, 511))
    Command::perform_line (cmd_buf);

  // If there is another script given on the commandline
  // start it but do not execute it yet. This will be done
  // frame by frame.
  if (Sys->auto_script)
    Command::start_script (Sys->auto_script);

  // The main loop.
  Sys->Loop ();

  cleanup ();

  return 1;
}

#if 0
void* operator new (size_t s)
{
  printf ("Alloc with size %d\n", s);
  if (Sys && Sys->do_edges && s > 100)
  {
    int a;
    a=1;
  }
  return (void*)malloc (s);
}

#endif

#if 0

#define DETECT_SIZE 20
#define DETECT     "ABCDabcd01234567890+"
#define DETECTAR   "abcdABCD+09876543210"
#define DETECTFREE "@*@*@*@*@*@*@*@*@*@*"

void* operator new (size_t s)
{
  if (s <= 0) printf ("BAD SIZE in new %d\n", s);
  char* rc = (char*)malloc (s+4+DETECT_SIZE+DETECT_SIZE);
  memcpy (rc, DETECT, DETECT_SIZE);
  memcpy (rc+DETECT_SIZE, &s, 4);
  memcpy (rc+DETECT_SIZE+4+s, DETECT, DETECT_SIZE);
  return (void*)(rc+4+DETECT_SIZE);
}

void* operator new[] (size_t s)
{
  if (s <= 0) printf ("BAD SIZE in new[] %d\n", s);
  char* rc = (char*)malloc (s+4+DETECT_SIZE+DETECT_SIZE);
  memcpy (rc, DETECTAR, DETECT_SIZE);
  memcpy (rc+DETECT_SIZE, &s, 4);
  memcpy (rc+DETECT_SIZE+4+s, DETECTAR, DETECT_SIZE);
  return (void*)(rc+4+DETECT_SIZE);
}

void operator delete (void* p)
{
  if (!p) return;
  char* rc = (char*)p;
  rc -= 4+DETECT_SIZE;
  size_t s;
  memcpy (&s, rc+DETECT_SIZE, 4);
  if (strncmp (rc, DETECT, DETECT_SIZE) != 0) { printf ("operator delete: BAD START!\n"); CRASH; }
  if (strncmp (rc+4+DETECT_SIZE+s, DETECT, DETECT_SIZE) != 0) { printf ("operator delete: BAD END!\n"); CRASH; }
  memcpy (rc, DETECTFREE, DETECT_SIZE);
  memcpy (rc+4+s+DETECT_SIZE, DETECTFREE, DETECT_SIZE);
  free (rc);
}

void operator delete[] (void* p)
{
  if (!p) return;
  char* rc = (char*)p;
  rc -= 4+DETECT_SIZE;
  size_t s;
  memcpy (&s, rc+DETECT_SIZE, 4);
  if (strncmp (rc, DETECTAR, DETECT_SIZE) != 0) { printf ("operator delete[]: BAD START!\n"); CRASH; }
  if (strncmp (rc+4+DETECT_SIZE+s, DETECTAR, DETECT_SIZE) != 0) { printf ("operator delete[]: BAD END!\n"); CRASH; }
  memcpy (rc, DETECTFREE, DETECT_SIZE);
  memcpy (rc+4+s+DETECT_SIZE, DETECTFREE, DETECT_SIZE);
  free (rc);
}

#endif
