/*
    Copyright (C) 1998-2005 by Jorrit Tyberghein

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


    walktest - the main Crystal Space test application, used for
    demonstrating and testing several features of the engine. (Note
    that walktest code should not be used as an example in most cases :-/)
*/
#include <stdarg.h>

// Some external projects use gcc's -ansi and -pedantic options. We need to
// ensure that CS's public headers are usable when these options are enabled,
// Unfortunately, we can not enable these options globally since CS uses some
// features (such as `long long') which are not available in the present
// language standard. As a compromise, we enable these restrictions on an
// individual module basis (see Jamfile) so as to ensure that the CS headers
// are tested against these options on a regular basis. We purposely #include
// "crystalspace.h" rather than including only the headers we need so that we
// test as many headers for standards conformance as possible.
#include "crystalspace.h"

#include "walktest.h"
#include "infmaze.h"
#include "command.h"

#if defined(CS_PLATFORM_DOS) || defined(CS_PLATFORM_WIN32)
#  include <io.h>
#elif defined(CS_PLATFORM_UNIX)
#  include <unistd.h>
#endif

WalkTest *Sys;

#define Gfx3D Sys->myG3D
#define Gfx2D Sys->myG2D

CS_IMPLEMENT_APPLICATION

bool WalkTest::move_3d = false;

WalkTest::WalkTest () :
  pos (0, 0, 0), velocity (0, 0, 0)
{
  extern bool CommandHandler (const char *cmd, const char *arg);
  csCommandProcessor::ExtraHandler = CommandHandler;
  auto_script = 0;
  view = 0;
  infinite_maze = 0;
  wMissile_boom = 0;
  wMissile_whoosh = 0;
  cslogo = 0;
  anim_sky = 0;
  anim_dirlight = 0;

  do_edges = false;
  do_show_coord = false;
  busy_perf_test = false;
  do_show_z = false;
  do_show_palette = false;
  do_infinite = false;
  do_freelook = false;
  do_logo = true;
  player_spawned = false;
  inverse_mouse = false;
  selected_light = 0;
  selected_polygon = 0;
  move_forward = false;
  cfg_recording = -1;
  recorded_cmd = 0;
  recorded_arg = 0;
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

  velocity.Set (0, 0, 0);
  desired_velocity.Set (0, 0, 0);
  angle_velocity.Set (0, 0, 0);
  desired_angle_velocity.Set (0, 0, 0);

  bgcolor_txtmap = 255;
  bgcolor_map = 0;

  SmallConsole = false;

  vc = 0;
  plugin_mgr = 0;

  debug_box1.Set (csVector3 (-1, -1, -1), csVector3 (1, 1, 1));
  debug_box2.Set (csVector3 (2, 2, 2), csVector3 (3, 3, 3));
  do_show_debug_boxes = false;
  
  split = -1;			// Not split

  canvas_exposed = true;

  first_map = last_map = 0;
  num_maps = 0;
  cache_map = 0;
}

WalkTest::~WalkTest ()
{
  delete [] auto_script;
  delete infinite_maze;
  delete cslogo;

  while (first_map)
  {
    csMapToLoad* map = first_map->next_map;
    delete[] first_map->map_dir;
    delete first_map;
    first_map = map;
  }
}

void WalkTest::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.system", msg, arg);
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

void WalkTest::SetDefaults ()
{
  csRef<iConfigManager> Config (CS_QUERY_REGISTRY (object_reg, iConfigManager));
  bool do_cd = Config->GetBool ("Walktest.Settings.Colldet", true);
  collider_actor.SetCD (do_cd);
  do_logo = Config->GetBool ("Walktest.Settings.DrawLogo", true);

  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));

  const char *val;
  if (!(val = cmdline->GetName ()))
    val = Config->GetStr ("Walktest.Settings.WorldFile");

  int idx = 0;
  cache_map = 0;
  while (val != 0)
  {
    num_maps++;
    idx++;
    csMapToLoad* map = new csMapToLoad;

    // If the given value starts with "cache:" the given location will not
    // be used as a map file (no loading of 'world' there) but instead the
    // lightmap cache will be placed there. This cache:xxx.zip can
    // only occur once (subsequence cache: entries will be ignored).
    if (cache_map == 0 && strlen (val) > 7 && !strncmp ("cache:", val, 6))
    {
      cache_map = map;
      val += 6;
    }

    map->map_dir = csStrNew (val);
    map->next_map = 0;
    if (last_map)
      last_map->next_map = map;
    else
      first_map = map;
    last_map = map;
    val = cmdline->GetName (idx);
  }

  if (cmdline->GetOption ("infinite"))
    do_infinite = true;

  extern bool do_bots;
  if (cmdline->GetOption ("bots"))
    do_bots = true;

  if (cmdline->GetOption ("colldet"))
  {
    collider_actor.SetCD (true);
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Enabled collision detection system.");
  }
  else if (cmdline->GetOption ("nocolldet"))
  {
    collider_actor.SetCD (false);
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Disabled collision detection system.");
  }

  if ((val = cmdline->GetOption ("exec")))
  {
    delete [] auto_script;
    auto_script = csStrNew (val);
  }

  if (cmdline->GetOption ("logo"))
  {
    do_logo = true;
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Logo enabled.");
  }
  else if (cmdline->GetOption ("nologo"))
  {
    do_logo = false;
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Logo disabled.");
  }

  bool doSave = Config->GetBool ("Walktest.Settings.EnableEngineSaving", 
    false);
  doSave = cmdline->GetBoolOption ("saveable", doSave);
  Report (CS_REPORTER_SEVERITY_NOTIFY, "World saving %s.", 
    doSave ? "enabled" : "disabled");
}

void WalkTest::Help ()
{
  csRef<iConfigManager> cfg (CS_QUERY_REGISTRY (object_reg, iConfigManager));
  csPrintf ("Options for WalkTest:\n");
  csPrintf ("  -exec=<script>     execute given script at startup\n");
  csPrintf ("  -[no]colldet       collision detection system (default '%scolldet')\n", collider_actor.HasCD () ? "" : "no");
  csPrintf ("  -[no]logo          draw logo (default '%slogo')\n", do_logo ? "" : "no");
  csPrintf ("  -regions           load every map in a separate region (default off)\n");
  csPrintf ("  -dupes             check for duplicate objects in multiple maps (default off)\n");
  csPrintf ("  -noprecache        after loading don't precache to speed up rendering\n");
  csPrintf ("  -infinite          special infinite level generation (ignores map file!)\n");
  csPrintf ("  -bots              allow random generation of bots\n");
  csPrintf ("  -[no]saveable      enable/disable engine 'saveable' flag\n");
  csPrintf ("  <path>             load map from VFS <path> (default '%s')\n",
        cfg->GetStr ("Walktest.Settings.WorldFile", "world"));
}

//-----------------------------------------------------------------------------
extern bool CommandHandler (const char *cmd, const char *arg);

void WalkTest::SetupFrame ()
{
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  if (!myConsole || !myConsole->GetVisible ())
  {
    // Emit recorded commands directly to the CommandHandler
    if (cfg_playrecording > 0 &&
	recording.Length () > 0)
    {
      csRecordedCamera* reccam = (csRecordedCamera*)recording[
      	cfg_playrecording];
      if (reccam->cmd)
	CommandHandler(reccam->cmd, reccam->arg);
    }
  }

  MoveSystems (elapsed_time, current_time);
  PrepareFrame (elapsed_time, current_time);
  if (canvas_exposed)
    DrawFrame (elapsed_time, current_time);
  else
    csSleep(150); // lower cpu usage

  // Execute one line from the script.
  if (!busy_perf_test)
  {
    char buf[256];
    if (csCommandProcessor::get_script_line (buf, 255)) csCommandProcessor::perform_line (buf);
  }
}

void WalkTest::FinishFrame ()
{
  // Drawing code ends here
  Gfx3D->FinishDraw ();
  // Print the output.
  Gfx3D->Print (0);
}


void WalkTest::MoveSystems (csTicks elapsed_time, csTicks current_time)
{
  // First move the sky.
  if (anim_sky)
  {
    iMovable* move = anim_sky->GetMovable ();
    switch (anim_sky_rot)
    {
      case 0:
	{
          csXRotMatrix3 mat (anim_sky_speed * TWO_PI
	  	* (float)elapsed_time/1000.);
          move->Transform (mat);
	  break;
	}
      case 1:
	{
          csYRotMatrix3 mat (anim_sky_speed * TWO_PI
	  	* (float)elapsed_time/1000.);
          move->Transform (mat);
	  break;
	}
      case 2:
	{
          csZRotMatrix3 mat (anim_sky_speed * TWO_PI
	  	* (float)elapsed_time/1000.);
          move->Transform (mat);
	  break;
	}
    }
    move->UpdateMove ();
  }

  // Update all busy entities.
  // We first push all entities in a vector so that NextFrame() can safely
  // remove it self from the busy_entities list (or add other entities).
  size_t i;
  busy_vector.DeleteAll ();
  csWalkEntity* wentity;
  for (i = 0 ; i < busy_entities.Length () ; i++)
  {
    wentity = busy_entities[i];
    busy_vector.Push (wentity);
  }
  for (i = 0 ; i < busy_vector.Length () ; i++)
  {
    wentity = busy_vector[i];
    wentity->NextFrame (elapsed_time);
  }

  // Record the first time this routine is called.
  extern bool do_bots;
  if (do_bots)
  {
    static long first_time = -1;
    static csTicks next_bot_at;
    if (first_time == -1)
    {
      first_time = current_time;
      next_bot_at = current_time+1000*10;
    }
    if (current_time > next_bot_at)
    {
      add_bot (2, view->GetCamera ()->GetSector (),
               view->GetCamera ()->GetTransform ().GetOrigin (), 0);
      next_bot_at = current_time+1000*10;
    }
  }
  if (!myConsole || !myConsole->GetVisible ())
  {
    InterpolateMovement ();

    if (mySound)
    {
      iSoundListener *sndListener = mySound->GetListener();
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

  move_bots (elapsed_time);

  if (move_forward)
    Step (1);
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
  if (cfg_debug_check_frustum)
  {
    // @@@
  }
  if (do_show_debug_boxes)
  {
    extern void DrawDebugBoxes (iCamera* cam, bool do3d);
    DrawDebugBoxes (view->GetCamera (), false);
  }
}

void WalkTest::DrawFrameExtraDebug ()
{
}

void WalkTest::DrawFrameDebug3D ()
{
  if (do_show_debug_boxes)
  {
    extern void DrawDebugBoxes (iCamera* cam, bool do3d);
    DrawDebugBoxes (view->GetCamera (), true);
  }
}

void WalkTest::GfxWrite (int x, int y, int fg, int bg, char *str, ...)
{
  va_list arg;
  csString buf;

  va_start (arg, str);
  buf.FormatV (str, arg);
  va_end (arg);

  myG2D->Write (Font, x, y, fg, bg, buf);
}

void WalkTest::DrawFrameConsole ()
{
  if (myConsole)
    myConsole->Draw2D (0);

  if (!myConsole || !myConsole->GetVisible ())
  {
    int fw, fh;
    Font->GetMaxSize (fw, fh);

    if (do_show_coord)
    {
      csString buffer;
      buffer.Format ("%2.2f,%2.2f,%2.2f: %s",
      view->GetCamera ()->GetTransform ().GetO2TTranslation ().x,
      view->GetCamera ()->GetTransform ().GetO2TTranslation ().y,
      view->GetCamera ()->GetTransform ().GetO2TTranslation ().z,
      view->GetCamera ()->GetSector()->QueryObject ()->GetName ());

      int buffWidth, buffHeight;
      Font->GetDimensions( buffer, buffWidth, buffHeight );

      GfxWrite ( FRAME_WIDTH - buffWidth - 1, 
                 FRAME_HEIGHT - fh - 3, 0, -1, 
                 "%s", buffer.GetData());

      GfxWrite (FRAME_WIDTH - buffWidth, 
                FRAME_HEIGHT - fh - 2, fgcolor_stats, -1, 
                "%s", buffer.GetData());
    }
  }
}

void WalkTest::DrawFullScreenFX2D (csTicks /*elapsed_time*/,
	csTicks current_time)
{
  if (do_fs_inter)
  {
    csfxInterference (Gfx2D, fs_inter_amount, fs_inter_anim,
    	fs_inter_length);
    fs_inter_anim = fmod (fabs (float (current_time)/3000.0), 1.);
  }
}

void WalkTest::DrawFullScreenFX3D (csTicks /*elapsed_time*/,
	csTicks current_time)
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
    csfxFadeTo (Gfx3D, fs_fadetxt_txt, fs_fadetxt_fade);
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

SCF_IMPLEMENT_IBASE_EXT (WalkDataObject)
  SCF_IMPLEMENTS_INTERFACE (WalkDataObject)
SCF_IMPLEMENT_IBASE_EXT_END

void WalkTest::DrawFrame3D (int drawflags, csTicks /*current_time*/)
{
  // Tell Gfx3D we're going to display 3D things
  if (!Gfx3D->BeginDraw (Engine->GetBeginDrawFlags () | drawflags
  	| CSDRAW_3DGRAPHICS))
    return;

  // Apply lighting BEFORE the very first frame
  size_t i;
  for (i = 0 ; i < dynamic_lights.Length () ; i++)
  {
    iLight* dyn = dynamic_lights[i];
    extern bool HandleDynLight (iLight*, iEngine*);
    csRef<WalkDataObject> dao (
    	CS_GET_CHILD_OBJECT (dyn->QueryObject (), WalkDataObject));
    if (dao)
      if (HandleDynLight (dyn, Engine))
      {
        dynamic_lights.DeleteIndex (i);
	i--;
      }
  }

  //------------
  // Here comes the main call to the engine. view->Draw() actually
  // takes the current camera and starts rendering.
  //------------
  if (!do_covtree_dump)
  {
    if (split == -1)
      view->Draw ();
    else 
    {	
      views[0]->Draw();
      views[1]->Draw();
    }
  }

  // no need to clear screen anymore
  drawflags = 0;

  // Display the 3D parts of the console
  if (myConsole)
    myConsole->Draw3D (0);
}


void WalkTest::DrawFrame2D (void)
{
  if (do_logo && cslogo)
  {
    /*
     * This now uses a new logo designed by Micah Dowty <micah@navi.picogui.org>
     * to look better on various backgrounds than the usual logo, which only looks
     * good on black.
     * The logo was drawn as an SVG image using Sodipodi, so it should be usable in
     * many other contexts if desired.
     */

    // Margin to the edge of the screen, as a fraction of screen width
    const float marginFraction = 0.01f;
    const unsigned margin = (unsigned) (FRAME_WIDTH * marginFraction);

#if 1 /**** Scalable logo ****/

    // Width of the logo, as a fraction of screen width
    const float widthFraction = 0.3f;

    // Scale the logo width to a fraction of the screen, keeping its aspect ratio
    const unsigned w = (unsigned) (FRAME_WIDTH * widthFraction);
    const unsigned h = w * cslogo->Height() / cslogo->Width();

    // Stick it in the top-right corner, with some margin
    cslogo->DrawScaled (Gfx3D, FRAME_WIDTH - w - margin, margin, w, h);

#else /**** Fixed-size logo ****/

    const unsigned w = cslogo->Width();
    const unsigned h = cslogo->Height();

    cslogo->Draw (Gfx3D, FRAME_WIDTH - w - margin, margin);

#endif
  }

  // White-board for debugging purposes.
  if (do_covtree_dump)
    DrawFrameExtraDebug ();
}

void WalkTest::DrawFrame (csTicks elapsed_time, csTicks current_time)
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
      reccam->cmd = recorded_cmd;
      reccam->arg = recorded_arg;
      recorded_cmd = recorded_arg = 0;
      recording.Push (reccam);
    }
    if (cfg_playrecording >= 0 && recording.Length () > 0)
    {
      csRecordedCamera* reccam = (csRecordedCamera*)recording[cfg_playrecording];
      cfg_playrecording++;
      record_frame_count++;
      if ((size_t)cfg_playrecording >= recording.Length ())
      {
	csTicks t1 = record_start_time;
	csTicks t2 = csGetTicks ();
	int num = record_frame_count;
	Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "End demo: %f secs to render %d frames: %f fps",
	  (float) (t2 - t1) / 1000.,
	  num, float (num) * 1000. / (float) (t2 - t1));
	csPrintf (
	  "End demo: %f secs to render %d frames: %f fps\n",
	  (float) (t2 - t1) / 1000.,
	  num, float (num) * 1000. / (float) (t2 - t1));
	fflush (stdout);
	if (cfg_playloop)
	  cfg_playrecording = 0;
	else
	  cfg_playrecording = -1;

        record_start_time = csGetTicks ();
        record_frame_count = 0;
      }
      iCamera* c = view->GetCamera ();
      if (reccam->sector)
        c->SetSector (reccam->sector);
      c->SetMirrored (reccam->mirror);
      c->GetTransform ().SetO2T (reccam->mat);
      c->GetTransform ().SetOrigin (reccam->vec);
    }
  }


  (void)elapsed_time; (void)current_time;

  // Not used since we need WHITE background not black.
  int drawflags = 0;

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

  DrawFrameDebug ();
  DrawFrameConsole ();

  // If console is not active we draw a few additional things.
  if (!myConsole
   || !myConsole->GetVisible ())
    DrawFrame2D ();
}

int cnt = 1;

void WalkTest::PrepareFrame (csTicks elapsed_time, csTicks current_time)
{
  if (!player_spawned)
  {
    CreateColliders ();
    player_spawned=true;
  }

  int shift, ctrl;
  float speed = 1;

  ctrl = kbd->GetKeyState (CSKEY_CTRL);
  shift = kbd->GetKeyState (CSKEY_SHIFT);
  if (ctrl)
    speed = .5;
  if (shift)
    speed = 2;

  float delta = float (elapsed_time) / 1000.0f;
  collider_actor.Move (delta, speed, velocity, angle_velocity);
}

void perf_test (int num)
{
  Sys->busy_perf_test = true;
  csTicks t1, t2, t;
  Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Performance test busy...");
  t = t1 = csGetTicks ();
  int i;
  for (i = 0 ; i < num ; i++)
  {
    if (Sys->cfg_playrecording >= 0 && Sys->recording.Length () > 0)
    {
      Sys->record_frame_count++;
    }
    Sys->DrawFrame (csGetTicks ()-t, csGetTicks ());
    Sys->FinishFrame ();
    t = csGetTicks ();
  }
  t2 = csGetTicks ();
  Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    "%f secs to render %d frames: %f fps", (float) (t2 - t1) / 1000., num,
    float (num) * 1000. / (float) (t2 - t1));
  csPrintf (
    "%f secs to render %d frames: %f fps\n", (float) (t2 - t1) / 1000., num,
    float (num) * 1000. / (float) (t2 - t1));
  fflush (stdout);
  cnt = 1;
  Sys->busy_perf_test = false;
}

/*---------------------------------------------
 * Our main event loop.
 *---------------------------------------------*/

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  free_keymap ();
  Sys->EndEngine ();
  iObjectRegistry* object_reg = Sys->object_reg;
  delete Sys; Sys = 0;
  csInitializer::DestroyApplication (object_reg);
}

void start_console ()
{
}

void WalkTest::EndEngine ()
{
  //  delete view; view = 0;
}

void WalkTest::InitCollDet (iEngine* engine, iRegion* region)
{
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Computing OBBs ...");
  csColliderHelper::InitializeCollisionWrappers (collide_system,
    	engine, region);
}

void WalkTest::LoadLibraryData (iRegion* region)
{
  // Load the "standard" library
  if (!LevelLoader->LoadLibraryFile ("/lib/std/library", region))
  {
    Cleanup ();
    exit (0);
  }
}

void WalkTest::Inititalize2DTextures ()
{
  // Find the Crystal Space logo and set the renderer Flag to for_2d, to allow
  // the use in the 2D part.
  iTextureWrapper *texh = Engine->GetTextureList ()->FindByName ("cslogo2");
  if (texh)
    texh->SetFlags (CS_TEXTURE_2D);
}


void WalkTest::Create2DSprites ()
{
  iTextureWrapper *texh;
  iTextureHandle* phTex;

  // Create a 2D sprite for the Logo.
  texh = Engine->GetTextureList ()->FindByName ("cslogo2");
  if (texh)
  {
    phTex = texh->GetTextureHandle();
    if (phTex)
    {
      cslogo = new csSimplePixmap (phTex);
    }
  }
}

static bool WalkEventHandler (iEvent& ev)
{

  if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdProcess)
  {
    Sys->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdFinalProcess)
  {
    Sys->FinishFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdCommandLineHelp)
  {
    Sys->Help ();
    return true;
  }
  else
  {
    return Sys ? Sys->WalkHandleEvent (ev) : false;
  }
}

bool WalkTest::SetMapDir (const char* map_dir)
{
  csStringArray paths;
  paths.Push ("/lev/");
  if (!myVFS->ChDirAuto (map_dir, &paths, 0, "world"))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error setting directory '%s'!",
    	map_dir);
    return false;
  }
  return true;
}

#if 0

#include "csutil/thread.h"
#include "csutil/sysfunc.h"
#include "iutil/document.h"

class MyThread : public csRunnable
{
private:
  int ref;

public:
  csRef<iDocument> doc;
  csRef<iDataBuffer> buf;

public:
  MyThread () : ref (1) { }
  virtual ~MyThread () { }
  virtual void Run ()
  {
    csSleep (2000);
    csPrintf ("================ START PARSING!\n"); fflush (stdout);
    const char* error = doc->Parse (buf, true);
    if (error != 0)
    {
      csPrintf ("Document system error for file '%s'!", error);
    }
    csPrintf ("================ END PARSING!\n"); fflush (stdout);
  }
  virtual void IncRef () { ref++; }
  virtual void DecRef () { ref--; if (ref <= 0) delete this; }
};

static csRef<csThread> thread1;
static csRef<csThread> thread2;
static csRef<csThread> thread3;

#endif

bool WalkTest::Initialize (int argc, const char* const argv[],
	const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

#if defined(CS_PLATFORM_WIN32)
  cswinMinidumpWriter::SetCrashMinidumpObjectReg (Sys->object_reg);
#endif

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Failed to initialize config!");
    return false;
  }

  csRef<iConfigManager> cfg (CS_QUERY_REGISTRY (object_reg, iConfigManager));
#if defined(CS_PLATFORM_WIN32)
  const bool mdumpDefault =
#if defined(CS_COMPILER_MSVC)
    true;
#else
    false;
#endif
  if (!cfg->GetBool ("Walktest.Win32.Minidumps", mdumpDefault))
  {
    cswinMinidumpWriter::DisableCrashMinidumps ();
  }
  else
  {
    cswinMinidumpWriter::EnableCrashMinidumps ();
  }
#endif

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_REPORTER,
  	CS_REQUEST_REPORTERLISTENER,
  	CS_REQUEST_END
	))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Failed to initialize!");
    return false;
  }

  SetDefaults ();

  if (!csInitializer::SetupEventHandler (object_reg, WalkEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Failed to initialize!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    csInitializer::DestroyApplication (object_reg);
    exit (0);
  }

  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    return false;
  }

  myG2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (!myG2D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics2D plugin!");
    return false;
  }

  myVFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!myVFS)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iVFS plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    return false;
  }

  myConsole = CS_QUERY_REGISTRY (object_reg, iConsoleOutput);
  mySound = CS_QUERY_REGISTRY (object_reg, iSoundRender);

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Crystal Space version %s (%s).", CS_VERSION, CS_RELEASE_DATE);
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Created by Jorrit Tyberghein and others...");

  // Get all collision detection and movement config file parameters.
  cfg_jumpspeed = cfg->GetFloat ("Walktest.CollDet.JumpSpeed", 0.08f);
  cfg_walk_accelerate = cfg->GetFloat ("Walktest.CollDet.WalkAccelerate", 0.007f);
  cfg_walk_maxspeed = cfg->GetFloat ("Walktest.CollDet.WalkMaxSpeed", 0.1f);
  cfg_walk_maxspeed_mult = cfg->GetFloat ("Walktest.CollDet.WalkMaxSpeedMult", 10.0f);
  cfg_walk_maxspeed_multreal = 1.0f;
  cfg_walk_brake = cfg->GetFloat ("Walktest.CollDet.WalkBrake", 0.014f);
  cfg_rotate_accelerate = cfg->GetFloat ("Walktest.CollDet.RotateAccelerate", 0.005f);
  cfg_rotate_maxspeed = cfg->GetFloat ("Walktest.CollDet.RotateMaxSpeed", 0.03f);
  cfg_rotate_brake = cfg->GetFloat ("Walktest.CollDet.RotateBrake", 0.015f);
  cfg_look_accelerate = cfg->GetFloat ("Walktest.CollDet.LookAccelerate", 0.028f);
  cfg_body_height = cfg->GetFloat ("Walktest.CollDet.BodyHeight", 1.4f);
  cfg_body_width = cfg->GetFloat ("Walktest.CollDet.BodyWidth", 0.5f);
  cfg_body_depth = cfg->GetFloat ("Walktest.CollDet.BodyDepth", 0.5f);
  cfg_eye_offset = cfg->GetFloat ("Walktest.CollDet.EyeOffset", -0.7f);
  cfg_legs_width = cfg->GetFloat ("Walktest.CollDet.LegsWidth", 0.4f);
  cfg_legs_depth = cfg->GetFloat ("Walktest.CollDet.LegsDepth", 0.4f);
  cfg_legs_offset = cfg->GetFloat ("Walktest.CollDet.LegsOffset", -1.1f);

  // Load the default font name
  const char * cfg_font = cfg->GetStr("Walktest.Font.Default", CSFONT_LARGE);

  #ifdef CS_DEBUG
    // enable all kinds of useful FPU exceptions on a x86
    // note that we can't do it above since at least on OS/2 each dynamic
    // library on loading/initialization resets the control word to default
    csControl87 (0x33, 0x3f);
  #else
    // this will disable exceptions on DJGPP (for the non-debug version)
    csControl87 (0x3f, 0x3f);
  #endif

  // Start the engine
  iNativeWindow* nw = Gfx2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Standard Test Application");
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    return false;
  }

  FrameWidth = Gfx2D->GetWidth ();
  FrameHeight = Gfx2D->GetHeight ();

  // Find the font we'll use
  Font = Gfx2D->GetFontServer ()->LoadFont (cfg_font);
  if (!Font)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Couldn't load font '%s', using standard one",
      cfg_font);
    Font = Gfx2D->GetFontServer ()->LoadFont (CSFONT_COURIER);
  }

  // Open the startup console
  start_console ();

  // Find the engine plugin and query the csEngine object from it...
  Engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!Engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    return false;
  }
  Engine->SetSaveableFlag (doSave);

  // Find the level loader plugin
  LevelLoader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!LevelLoader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No level loader plugin!");
    return false;
  }

  // Find the model converter plugin
  ModelConverter = CS_QUERY_REGISTRY (object_reg, iModelConverter);

  // Find the model crossbuilder plugin
  CrossBuilder = CS_QUERY_REGISTRY (object_reg, iCrossBuilder);

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  views[0] = csPtr<iView> (new csView (Engine, Gfx3D));
  views[1] = csPtr<iView> (new csView (Engine, Gfx3D));
  view = views[0];

  // Get the collide system plugin.
  const char* p = cfg->GetStr ("Walktest.Settings.CollDetPlugin",
  	"crystalspace.collisiondetection.opcode");
  collide_system = CS_LOAD_PLUGIN (plugin_mgr, p, iCollideSystem);
  if (!collide_system)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No Collision Detection plugin found!");
    return false;
  }
  object_reg->Register (collide_system, "iCollideSystem");

  // Initialize the command processor with the engine and camera.
  csCommandProcessor::Initialize (Engine, view->GetCamera (),
    Gfx3D, myConsole, object_reg);

  if (do_infinite)
  {
    // The infinite maze.

    if (!myVFS->ChDir ("/tmp"))
    {
      Report (CS_REPORTER_SEVERITY_ERROR,
      	"Temporary directory /tmp not mounted on VFS!");
      return false;
    }

    Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating initial room!...");
    Engine->SetLightingCacheMode (0);

    // Unfortunately the current movement system does not allow the user to
    // move around the maze unless collision detection is enabled, even
    // though collision detection does not really make sense in this context.
    // Hopefully the movement system will be fixed some day so that the user
    // can move around even with collision detection disabled.
    collider_actor.SetCD (true);

    // Load two textures that are used in the maze.
    if (!LevelLoader->LoadTexture ("txt", "/lib/std/stone4.gif"))
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Failed to load /lib/std/stone4.gif");
      return false;
    }
    if (!LevelLoader->LoadTexture ("txt2", "/lib/std/mystone2.gif"))
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Failed to load /lib/std/mystone2.gif");
      return false;
    }

    if (do_infinite)
    {
      iSector* room;
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
      view->GetCamera ()->SetSector(room);
    }

    // Prepare the engine. This will calculate all lighting and
    // prepare the lightmaps for the 3D rasterizer.
    csTextProgressMeter* meter = new csTextProgressMeter (myConsole);
    Engine->Prepare (meter);
    delete meter;
  }
  else
  {
    // Load from a map file.
    if (num_maps == 1)
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading map '%s'.",
      	first_map->map_dir);
    else if (num_maps == 2 && cache_map != 0)
    {
      if (cache_map != first_map)
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading map '%s'.",
		first_map->map_dir);
      else
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading map '%s'.",
		first_map->next_map->map_dir);
    }
    else if (num_maps > 1)
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading multiple maps '%s', ...",
      	first_map->map_dir);

    // Check if we have to load every separate map in a separate region.
    csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (object_reg,
    	iCommandLineParser);
    bool do_regions = false;
    if (cmdline->GetOption ("regions"))
      do_regions = true;
    bool do_dupes = false;
    if (cmdline->GetOption ("dupes"))
      do_dupes = true;
      
    if ((!do_regions) && cache_map != 0)
    {
      // Then we set the current directory right.
      if (!SetMapDir (cache_map->map_dir))
	return false;
      // Set the cache manager based on current VFS dir.
      Engine->SetVFSCacheManager ();
    }

    // Check the map and mount it if required.
    int i;
    csMapToLoad* map = first_map;
    Engine->DeleteAll ();
    csTicks start_time = csGetTicks ();
    for (i = 0 ; i < num_maps ; i++, map = map->next_map)
    {
      if (map == cache_map)
      {
	continue;
      }
      if (!SetMapDir (map->map_dir))
	return false;

      // Load the map from the file.
      if (num_maps > 1)
        Report (CS_REPORTER_SEVERITY_NOTIFY, "  Loading map '%s'",
		map->map_dir);
      iRegion* region = 0;
      if (do_regions)
      {
        region = Engine->CreateRegion (map->map_dir);
      }
      if (!LevelLoader->LoadMapFile ("world", false, region, !do_regions,
      		do_dupes))
      {
        Report (CS_REPORTER_SEVERITY_ERROR, "Failing to load map!");
        return false;
      }
      if (do_regions)
      {
        // Set the cache manager based on current VFS dir.
        Engine->SetVFSCacheManager ();
        region->Prepare ();
      }
    }

    iRegion* region = 0;
    if (do_regions)
      region = Engine->CreateRegion ("libdata");
    LoadLibraryData (region);
    if (do_regions)
    {
      region->Prepare ();
    }
    Inititalize2DTextures ();
    ParseKeyCmds ();

    // Prepare the engine. This will calculate all lighting and
    // prepare the lightmaps for the 3D rasterizer.
    if (!do_regions)
    {
      csTextProgressMeter* meter = new csTextProgressMeter (myConsole);
      Engine->Prepare (meter);
      delete meter;
    }

    if (!cmdline->GetOption ("noprecache"))
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Precaching all things...");
      Engine->PrecacheDraw ();
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Precaching finished...");
    }

    csTicks stop_time = csGetTicks ();
    csPrintf ("Total level load time: %g seconds\n",
    	float (stop_time-start_time) / 1000.0); fflush (stdout);

    Create2DSprites ();

    // Look for the start sector in this map.
    bool camok = false;
    if (!camok && Engine->GetCameraPositions ()->GetCount () > 0)
    {
      iCameraPosition *cp = Engine->GetCameraPositions ()->Get (0);
      if (cp->Load(views[0]->GetCamera (), Engine) &&
	  cp->Load(views[1]->GetCamera (), Engine))
	camok = true;
    }
    if (!camok) 
    {
      iSector* room = Engine->GetSectors ()->FindByName ("room");
      if (room)
      {
	views[0]->GetCamera ()->SetSector (room);
	views[1]->GetCamera ()->SetSector (room);
	camok = true;
      }
    }

    if (!camok)
    {
      Report (CS_REPORTER_SEVERITY_ERROR,
        "Map does not contain a valid starting point!\n"
        "Try adding a room called 'room' or a START keyword");
      return false;
    }
  }

  // Initialize collision detection system (even if disabled so
  // that we can enable it later).
  InitCollDet (Engine, 0);

  // Load a few sounds.
  if (mySound)
  {
    csRef<iSoundWrapper> w (CS_GET_NAMED_CHILD_OBJECT (
    	Engine->QueryObject (), iSoundWrapper, "boom.wav"));
    wMissile_boom = w ? w->GetSound () : 0;
    w = CS_GET_NAMED_CHILD_OBJECT (Engine->QueryObject (),
					iSoundWrapper, "whoosh.wav");
    wMissile_whoosh = w ? w->GetSound () : 0;
  }

  Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"--------------------------------------");
  if (myConsole)
  {
    myConsole->SetVisible (false);
    myConsole->AutoUpdate (false);
    ConsoleInput = CS_QUERY_REGISTRY (object_reg, iConsoleInput);
    if (ConsoleInput)
    {
      ConsoleInput->Bind (myConsole);
      ConsoleInput->SetPrompt ("cs# ");
      csCommandProcessor::PerformCallback* cb =
      	new csCommandProcessor::PerformCallback ();
      ConsoleInput->SetExecuteCallback (cb);
      cb->DecRef ();
    }

    // Set console to center of screen, if supported
    int DeltaX = myG2D->GetWidth () / 10;
    int DeltaY = myG2D->GetHeight () / 10;
    SmallConsole = myConsole->PerformExtension ("SetPos", DeltaX, DeltaY,
      myG2D->GetWidth () - DeltaX * 2, myG2D->GetHeight () - DeltaY * 2);
  }

  // Wait one second before starting.
  csTicks t = csGetTicks ()+1000;
  while (csGetTicks () < t) ;

  // Allocate the palette as calculated by the texture manager.
  bgcolor_txtmap = myG2D->FindRGB (128, 128, 128);
  bgcolor_map = 0;
  fgcolor_stats = myG2D->FindRGB (255, 255, 255);

  // Reinit console object for 3D engine use.
  if (myConsole) myConsole->Clear ();

  // We use the width and height from the 3D renderer because this
  // can be different from the frame size (rendering in less res than
  // real window for example).
  int w3d = Gfx3D->GetWidth ();
  int h3d = Gfx3D->GetHeight ();
  // clear all backbuffers to black
  // and Zbuffer, mainly to make better zbufdumps
  Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS | CSDRAW_CLEARZBUFFER);
  myG2D->ClearAll (myG2D->FindRGB(0,0,0));
  Gfx3D->FinishDraw ();
#ifdef CS_DEBUG
  view->SetRectangle (2, 2, w3d - 4, h3d - 4);
  myG2D->SetClipRect (2, 2, w3d - 2, h3d - 2);
#else
  view->SetRectangle (0, 0, w3d, h3d);
#endif

#if 0
{
  csRef<iDocumentSystem> xml (
      CS_QUERY_REGISTRY (object_reg, iDocumentSystem));

  MyThread* t1 = new MyThread ();
  MyThread* t2 = new MyThread ();
  t1->doc = xml->CreateDocument ();
  t2->doc = xml->CreateDocument ();
  myVFS->Mount ("/lev/hydlaa", "hydlaa.zip");
  myVFS->ChDir ("/lev/hydlaa");
  t1->buf = myVFS->ReadFile ("/lev/hydlaa/world");
  t2->buf = t1->buf;

  thread1 = csThread::Create (t1);
  thread1->Start ();
  //thread2 = csThread::Create (t2);
  //thread2->Start ();
}
#endif
  return true;
}

// moved this out of main() to make it easier for app developer
// to override
static void CreateSystem(void)
{
  // Create the system driver object
  Sys = new WalkTest ();
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
#if defined(CS_PLATFORM_WIN32) && defined(CS_COMPILER_MSVC)
  cswinMinidumpWriter::EnableCrashMinidumps ();
#endif
  // Initialize the random number generator
  srand (time (0));

  CreateSystem();

  // Initialize the main system. This will load all needed plugins
  // (3D, 2D, network, sound, ..., engine) and initialize them.
  if (!Sys->Initialize (argc, argv, "/config/walktest.cfg"))
  {
    Sys->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    Cleanup();
    exit (-1);
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
  csDefaultRunLoop(Sys->object_reg);

  Cleanup ();

  return 1;
}

