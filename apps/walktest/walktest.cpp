/*
    Copyright (C) 1998-2006 by Jorrit Tyberghein

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
#include "csver.h"
#include "isndsys.h"

#include "walktest.h"
#include "command.h"
#include "splitview.h"
#include "recorder.h"
#include "missile.h"
#include "lights.h"
#include "animsky.h"

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
  world_file = "world";
  views = 0;
  wMissile_boom = 0;
  wMissile_whoosh = 0;
  cslogo = 0;
  sky = new WalkTestAnimateSky (this);
  fsfx = new WalkTestFullScreenFX (this);

  do_edges = false;
  do_show_coord = false;
  do_object_move = false;
  object_move_speed = 1.0f;
  busy_perf_test = false;
  do_show_z = false;
  do_show_palette = false;
  do_freelook = false;
  do_logo = true;
  player_spawned = false;
  inverse_mouse = false;
  selected_light = 0;
  selected_polygon = 0;
  move_forward = false;

  recorder = new WalkTestRecorder (this);

  cfg_debug_check_frustum = 0;

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

  canvas_exposed = true;

  first_map = last_map = 0;
  num_maps = 0;
  cache_map = 0;
  doSave = false;
  spritesLoaded = false;

  bots = new BotManager (this);
  do_bots = false;
  missiles = new WalkTestMissileLauncher (this);
  lights = new WalkTestLights (this);
}

WalkTest::~WalkTest ()
{
  delete [] auto_script;
  delete cslogo;

  while (first_map)
  {
    csMapToLoad* map = first_map->next_map;
    delete first_map;
    first_map = map;
  }
  delete recorder;
  delete views;
  delete sky;
  delete fsfx;
  delete missiles;
  delete lights;
}

void WalkTest::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV(object_reg, severity, "crystalspace.system", msg, arg);
  va_end (arg);
}

void WalkTest::SetDefaults ()
{
  csRef<iConfigManager> Config (csQueryRegistry<iConfigManager> (object_reg));
  bool do_cd = Config->GetBool ("Walktest.Settings.Colldet", true);
  collider_actor.SetCD (do_cd);
  do_logo = Config->GetBool ("Walktest.Settings.DrawLogo", true);

  csRef<iCommandLineParser> cmdline (
  	csQueryRegistry<iCommandLineParser> (object_reg));

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

    map->map_name = val;
    map->next_map = 0;
    if (last_map)
      last_map->next_map = map;
    else
      first_map = map;
    last_map = map;
    val = cmdline->GetName (idx);
  }

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

  if ((val = cmdline->GetOption ("world")))
  {
    world_file = val;
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

  // Set whether to use multi-threading within the loader.
  if(!cmdline->GetBoolOption("threaded", true) || CS::Platform::GetProcessorCount() == 1)
  {
    csRef<iThreadManager> tman = csQueryRegistry<iThreadManager> (object_reg);
    tman->SetAlwaysRunNow(true);
  }

  doSave = Config->GetBool ("Walktest.Settings.EnableEngineSaving", doSave);
  doSave = cmdline->GetBoolOption ("saveable", doSave);
  Report (CS_REPORTER_SEVERITY_NOTIFY, "World saving %s.", 
    doSave ? "enabled" : "disabled");
}

void WalkTest::Help ()
{
  csCommandLineHelper commandLineHelper;

  csRef<iConfigManager> cfg (csQueryRegistry<iConfigManager> (object_reg));

  // Command line options
  commandLineHelper.AddCommandLineOption ("exec", "Execute given script at startup", csVariant (""));
  commandLineHelper.AddCommandLineOption ("threaded", "Use threaded loading", csVariant (true));
  commandLineHelper.AddCommandLineOption ("colldet", "Enable collision detection system",
					  csVariant (collider_actor.HasCD () ? true : false));
  commandLineHelper.AddCommandLineOption ("logo", "Draw logo", csVariant (do_logo ? true : false));
  commandLineHelper.AddCommandLineOption ("collections", "Load every map in a separate collection",
					  csVariant (false));
  commandLineHelper.AddCommandLineOption ("dupes", "Check for duplicate objects in multiple maps",
					  csVariant (true));
  commandLineHelper.AddCommandLineOption ("noprecache",
					  "After loading don't precache to speed up rendering",
					  csVariant ());
  commandLineHelper.AddCommandLineOption ("bots", "Allow random generation of bots", csVariant ());
  commandLineHelper.AddCommandLineOption ("saveable",
					  csString ().Format ("Enable engine %s flag",
							      CS::Quote::Single ("saveable")),
					  csVariant ());
  commandLineHelper.AddCommandLineOption ("world",
					  csString ().Format ("Use given world file instead of %s",
							      CS::Quote::Single ("world")).GetData (),
					  csVariant (""));

  // Printing help
  commandLineHelper.PrintApplicationHelp
    (object_reg, "walktest", "walktest [OPTIONS] [filename]", csString().Format
     ("This is the quintessential test application for Crystal Space. It allows mainly to load"
      " world files and navigate into them.\n\n"
      "Developers should probably not use this application as a guide for developing their own"
      " programs, as it is not the best example in clean coding.\n\n"
      "More information is available on the usages of walktest in the Crystal Space manual\n\n"
      "If specified, walktest will load the map from the given VFS path instead of the default %s",
      CS::Quote::Single (cfg->GetStr ("Walktest.Settings.WorldFile", "world"))));
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
    recorder->HandleRecordedCommand ();
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
  sky->MoveSky (elapsed_time, current_time);

  // Update all busy entities.
  // We first push all entities in a vector so that NextFrame() can safely
  // remove it self from the busy_entities list (or add other entities).
  size_t i;
  busy_vector.DeleteAll ();
  csWalkEntity* wentity;
  for (i = 0 ; i < busy_entities.GetSize () ; i++)
  {
    wentity = busy_entities[i];
    busy_vector.Push (wentity);
  }
  for (i = 0 ; i < busy_vector.GetSize () ; i++)
  {
    wentity = busy_vector[i];
    wentity->NextFrame (elapsed_time);
  }

  // Record the first time this routine is called.
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
      bots->CreateBot (views->GetCamera ()->GetSector (),
               views->GetCamera ()->GetTransform ().GetOrigin (), 0, false);
      next_bot_at = current_time+1000*10;
    }
  }
  if (!myConsole || !myConsole->GetVisible ())
  {
    InterpolateMovement ();

    if (mySound)
    {
      iSndSysListener *sndListener = mySound->GetListener();
      if(sndListener)
      {
        // take position/direction from views->GetCamera ()
        csVector3 v = views->GetCamera ()->GetTransform ().GetOrigin ();
        csMatrix3 m = views->GetCamera ()->GetTransform ().GetT2O();
        csVector3 f = m.Col3();
        csVector3 t = m.Col2();
        sndListener->SetPosition(v);
        sndListener->SetDirection(f,t);
        //sndListener->SetDirection(...);
      }
    }
  }

  bots->MoveBots (elapsed_time);

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
  //if (do_show_palette)
  //{
    //extern void DrawPalette ();
    //DrawPalette ();
  //}
  if (do_show_debug_boxes)
  {
    extern void DrawDebugBoxes (iCamera* cam, bool do3d);
    DrawDebugBoxes (views->GetCamera (), false);
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
    DrawDebugBoxes (views->GetCamera (), true);
  }
}

void WalkTest::GfxWrite (int x, int y, int fg, int bg, const char *str, ...)
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

    if (do_object_move || closestMesh)
    {
      csString buffer;
      if (closestMesh)
        buffer.Format ("[%s%s]", do_object_move ? "O:" : "",
	    closestMesh->QueryObject ()->GetName ());
      else
        buffer.Format ("[%s]", do_object_move ? "O:" : "");
      GfxWrite ( FRAME_WIDTH / 3, FRAME_HEIGHT - fh - 3, 0, -1, 
                 "%s", buffer.GetData ());
      GfxWrite ( FRAME_WIDTH / 3, FRAME_HEIGHT - fh - 3, fgcolor_stats, -1, 
                 "%s", buffer.GetData ());
    }
    if (do_show_coord)
    {
      csString buffer;
      buffer.Format ("%2.2f,%2.2f,%2.2f: %s",
      views->GetCamera ()->GetTransform ().GetO2TTranslation ().x,
      views->GetCamera ()->GetTransform ().GetO2TTranslation ().y,
      views->GetCamera ()->GetTransform ().GetO2TTranslation ().z,
      views->GetCamera ()->GetSector()->QueryObject ()->GetName ());

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

void WalkTest::DrawFrame3D (int drawflags, csTicks /*current_time*/)
{
  // Tell Gfx3D we're going to display 3D things
  /*
  if (!Gfx3D->BeginDraw (Engine->GetBeginDrawFlags () | drawflags
  	| CSDRAW_3DGRAPHICS))
    return;
    */

  // Apply lighting BEFORE the very first frame
  lights->HandleDynLights ();

  //------------
  // Here comes the main call to the engine. views->Draw() actually
  // takes the current camera and starts rendering.
  //------------
  if (!do_covtree_dump)
  {
    views->Draw ();
  }

  // no need to clear screen anymore
  drawflags = 0;

  // Tell Gfx3D we're going to display 3D things
  if (!Gfx3D->BeginDraw (/*Engine->GetBeginDrawFlags () |*/ drawflags
  	| CSDRAW_3DGRAPHICS))
    return;

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
    recorder->RecordCamera (views->GetCamera ());
    recorder->HandleRecordedCamera (views->GetCamera ());
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
    fsfx->Draw3D (current_time);
  }

  // Start drawing 2D graphics
  if (!Gfx3D->BeginDraw (drawflags | CSDRAW_2DGRAPHICS))
    return;

  if (!myConsole
   || !myConsole->GetVisible ()
   || SmallConsole
   || myConsole->GetTransparency ())
  {
    fsfx->Draw2D (current_time);
  }

  DrawFrameDebug ();
  DrawFrameConsole ();

  // If console is not active we draw a few additional things.
  if (!myConsole
   || !myConsole->GetVisible ())
    DrawFrame2D ();
}

int cnt = 1;

void WalkTest::PrepareFrame (csTicks elapsed_time, csTicks /*current_time*/)
{
  if (!player_spawned)
  {
    CreateColliders ();
    player_spawned=true;
  }

  int shift, ctrl;
  float speed = 1;
  object_move_speed = 0.01f;

  ctrl = kbd->GetKeyState (CSKEY_CTRL);
  shift = kbd->GetKeyState (CSKEY_SHIFT);
  if (ctrl)
  {
    speed = 0.5f;
  }
  if (shift)
  {
    speed = 2;
    object_move_speed = 1.0f;
  }

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
    Sys->recorder->NewFrame ();
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
  if (e3DEventHandler)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q)
      q->RemoveListener (e3DEventHandler);
  }

  if (frameEventHandler)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q)
      q->RemoveListener (frameEventHandler);
  }
}

void WalkTest::InitCollDet (iEngine* engine, iCollection* collection)
{
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Computing OBBs ...");
  csColliderHelper::InitializeCollisionWrappers (collide_system,
    	engine, collection);
}

void WalkTest::LoadLibraryData (iCollection* collection)
{
  csRef<iTextureWrapper> tex = LevelLoader->LoadTexture ("cslogo2", "/lib/std/cslogo2.png",
    CS_TEXTURE_2D, 0, true, true, true, csRef<iCollection>(collection));
  if(!tex.IsValid())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "logo failed to load!\n");
  }

  if(!LevelLoader->LoadLibraryFile ("/lib/std/library", collection))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "std library failed to load!\n");
  }
}

bool WalkTest::Create2DSprites ()
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
      return true;
    }
  }
  return false;
}

static bool WalkEventHandler (iEvent& ev)
{
  if (!Sys)
    return false;

  return Sys->WalkHandleEvent (ev);
}

bool WalkTest::SetMapDir (const char* map_dir, csString& map_file)
{
  const char* fileNameToOpen;
  if (!CS::Utility::SmartChDir (myVFS, map_dir, world_file, &fileNameToOpen))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error setting directory %s!",
	CS::Quote::Single (map_dir));
    return false;
  }

  map_file = fileNameToOpen;
  return true;
}

bool WalkTest::Initialize (int argc, const char* const argv[],
	const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Failed to initialize config!\n"
      " (A common cause is the CRYSTAL environment variable "
      "not being set correctly.)");
    return false;
  }

  csRef<iConfigManager> cfg (csQueryRegistry<iConfigManager> (object_reg));

  SetDefaults ();

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_REPORTER,
  	CS_REQUEST_REPORTERLISTENER,
  	CS_REQUEST_END
	))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Failed to initialize!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, WalkEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Failed to initialize!");
    return false;
  }

  CS_INITIALIZE_FRAME_EVENT_SHORTCUTS (object_reg);
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));

  if (!e3DEventHandler)
  {
    e3DEventHandler.AttachNew (new E3DEventHandler (this));
  }
  if (q != 0)
  {
    csEventID events[2] = { Frame, CS_EVENTLIST_END };
    q->RegisterListener (e3DEventHandler, events);
  }

  if (!frameEventHandler)
  {
    frameEventHandler.AttachNew (new FrameEventHandler (this));
  }
  if (q != 0)
  {
    csEventID events[2] = { Frame, CS_EVENTLIST_END };
    q->RegisterListener (frameEventHandler, events);
  }


  //Must have before help is called
  name_reg = csEventNameRegistry::GetRegistry (object_reg);
  CommandLineHelp = csevCommandLineHelp (name_reg);
  

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    Help ();
    csInitializer::DestroyApplication (object_reg);
    exit (0);
  }

  plugin_mgr = csQueryRegistry<iPluginManager> (object_reg);
  vc = csQueryRegistry<iVirtualClock> (object_reg);

  myG3D = csQueryRegistry<iGraphics3D> (object_reg);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    return false;
  }

  myG2D = csQueryRegistry<iGraphics2D> (object_reg);
  if (!myG2D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics2D plugin!");
    return false;
  }

  myVFS = csQueryRegistry<iVFS> (object_reg);
  if (!myVFS)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iVFS plugin!");
    return false;
  }

  kbd = csQueryRegistry<iKeyboardDriver> (object_reg);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    return false;
  }

  CanvasHidden = csevCanvasHidden (name_reg, myG2D);
  CanvasExposed = csevCanvasExposed (name_reg, myG2D);
  CanvasResize = csevCanvasResize (name_reg, myG2D);

  myConsole = csQueryRegistry<iConsoleOutput> (object_reg);
  mySound = csQueryRegistry<iSndSysRenderer> (object_reg);

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
    //csControl87 (0x33, 0x3f);
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
  if (!Gfx2D->GetFontServer ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No font server available");
    return false;
  }
  Font = Gfx2D->GetFontServer ()->LoadFont (cfg_font);
  if (!Font)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Couldn't load font %s, using standard one",
      CS::Quote::Single (cfg_font));
    Font = Gfx2D->GetFontServer ()->LoadFont (CSFONT_COURIER);
  }
  if (!Font)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Could not load any font");
    return false;
  }

  // Open the startup console
  start_console ();

  tm = csQueryRegistry<iThreadManager>(object_reg);
  if(!tm)
  {
    return false;
  }

  // Find the engine plugin and query the csEngine object from it...
  Engine = csQueryRegistry<iEngine> (object_reg);
  if (!Engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    return false;
  }
  Engine->SetSaveableFlag (doSave);

  // Find the level loader plugin
  LevelLoader = csQueryRegistry<iLoader>(object_reg);
  TLevelLoader = csQueryRegistry<iThreadedLoader>(object_reg);

  if (!LevelLoader || !TLevelLoader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No level loader plugin!");
    return false;
  }

  views = new WalkTestViews (this);

  // Get the collide system plugin.
  const char* p = cfg->GetStr ("Walktest.Settings.CollDetPlugin",
  	"crystalspace.collisiondetection.opcode");
  collide_system = csLoadPlugin<iCollideSystem> (plugin_mgr, p);
  if (!collide_system)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No Collision Detection plugin found!");
    return false;
  }
  object_reg->Register (collide_system, "iCollideSystem");

  // Initialize the command processor with the engine and camera.
  csCommandProcessor::Initialize (Engine, views->GetView ()->GetCamera (),
    Gfx3D, myConsole, object_reg);

  // Load from a map file.
  if (num_maps == 1)
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading map %s.",
      CS::Quote::Single (first_map->map_name.GetData()));
  else if (num_maps == 2 && cache_map != 0)
  {
    if (cache_map != first_map)
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading map %s.",
	      CS::Quote::Single (first_map->map_name.GetData()));
    else
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading map %s.",
	      CS::Quote::Single (first_map->next_map->map_name.GetData()));
  }
  else if (num_maps > 1)
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading multiple maps %s, ...",
      CS::Quote::Single (first_map->map_name.GetData()));

  // Check if we have to load every separate map in a separate collection.
  csRef<iCommandLineParser> cmdline = 
      csQueryRegistry<iCommandLineParser> (object_reg);
  bool do_collections = cmdline->GetBoolOption ("collections");
    
  if ((!do_collections) && cache_map != 0)
  {
    csString cacheMapFN;
    // Then we set the current directory right.
    if (!SetMapDir (cache_map->map_name, cacheMapFN))
      return false;
    // Set the cache manager based on current VFS dir.
    Engine->SetVFSCacheManager ();
  }

  // Check the map and mount it if required.
  int i;
  csMapToLoad* map = first_map;
  csTicks start_time = csGetTicks ();
  for (i = 0 ; i < num_maps ; i++, map = map->next_map)
  {
    if (map == cache_map)
    {
      continue;
    }
    csString map_name;
    if (!SetMapDir (map->map_name, map_name))
      return false;

    // Load the map from the file.
    if (num_maps > 1)
      Report (CS_REPORTER_SEVERITY_NOTIFY, "  Loading map %s",
	      CS::Quote::Single (map->map_name.GetData()));
    iCollection* collection = 0;
    if (do_collections)
    {
      collection = Engine->CreateCollection (map->map_name.GetData());
    }

    csRef<iThreadReturn> ret = TLevelLoader->LoadMapFileWait (myVFS->GetCwd(), map_name, false, collection,
      0, 0, KEEP_ALL, cmdline->GetBoolOption("verbose", false));

    if(!ret->WasSuccessful())
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Failing to load map!");
      return false;
    }

    if (do_collections)
    {
      // Set the cache manager based on current VFS dir.
      Engine->SetVFSCacheManager ();
    }
  }

  iCollection* collection = 0;
  if (do_collections)
    collection = Engine->CreateCollection ("libdata");
  LoadLibraryData (collection);
  ParseKeyCmds ();

  csTicks stop_time = csGetTicks ();
  csPrintf ("\nLevel load time: %g seconds.\n",
    float (stop_time-start_time) / 1000.0); fflush (stdout);

  // Prepare the engine. This will calculate all lighting and
  // prepare the lightmaps for the 3D rasterizer.
  if (!do_collections)
  {
    csTextProgressMeter* meter = new csTextProgressMeter (myConsole);
    Engine->Prepare (meter);
    delete meter;
  }

  if (!cmdline->GetOption ("noprecache"))
  {
    csTicks start = csGetTicks ();
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Precaching all things...\n");
    Engine->PrecacheDraw ();
    Report (CS_REPORTER_SEVERITY_NOTIFY, "\nPrecaching finished... took %g seconds.\n", (csGetTicks()-start)/1000.0f);
  }

  printf("\nTotal load time: %g seconds.\n", (csGetTicks()-start_time)/1000.0f);

  Create2DSprites ();

  // Look for the start sector in this map.
  bool camok = views->SetupViewStart ();
  if (!camok)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Map does not contain a valid starting point!\n"
      "Try adding a room called %s or a START keyword",
      CS::Quote::Single ("room"));
    return false;
  }

  // Initialize collision detection system (even if disabled so
  // that we can enable it later).
  InitCollDet (Engine, 0);

  // Load a few sounds.
  if (mySound)
  {
    csRef<iSndSysManager> mgr = csQueryRegistry<iSndSysManager> (
	object_reg);
    wMissile_boom = mgr->FindSoundByName ("boom.wav");
    wMissile_whoosh = mgr->FindSoundByName ("whoosh.wav");
  }

  Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"--------------------------------------");
  if (myConsole)
  {
    myConsole->SetVisible (false);
    myConsole->AutoUpdate (false);
    ConsoleInput = csQueryRegistry<iConsoleInput> (object_reg);
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
    int DeltaX = myG2D->GetWidth () / 18;
    int DeltaY = myG2D->GetHeight () / 18;
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

  // clear all backbuffers to black
  // and Zbuffer, mainly to make better zbufdumps
  Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS | CSDRAW_CLEARZBUFFER);
  myG2D->ClearAll (myG2D->FindRGB(0,0,0));
  Gfx3D->FinishDraw ();
#ifdef CS_DEBUG
  // We use the width and height from the 3D renderer because this
  // can be different from the frame size (rendering in less res than
  // real window for example).
  int w3d = Gfx3D->GetWidth ();
  int h3d = Gfx3D->GetHeight ();
  myG2D->SetClipRect (2, 2, w3d - 2, h3d - 2);
#endif

#if 0
{
  csRef<iDocumentSystem> xml (
      csQueryRegistry<iDocumentSystem> (object_reg));

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

  // Lastly set the camera to the collider so its transform is
  // updated to the camera's one.
  collider_actor.SetCamera (views->GetCamera (), true);

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
  // Initialize the random number generator
  srand (time (0));

  CreateSystem();

  // Initialize the main system. This will load all needed plugins
  // (3D, 2D, network, sound, ..., engine) and initialize them.
  if (!Sys->Initialize (argc, argv, "/config/walktest.cfg"))
  {
    Sys->Report (CS_REPORTER_SEVERITY_ERROR,
      "Error initializing system!");
    Cleanup();
    return -1;
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

  return 0;
}

