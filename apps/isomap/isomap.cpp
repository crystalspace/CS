/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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


    isomap - An example on how to load a map into the iso engine
*/

// Based heavily on (or cut n paste from) isotest.cpp

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"

#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"

#include "igraphic/imageio.h"
#include "imesh/object.h"
#include "imesh/sprite2d.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"

#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "ivaria/iso.h"
#include "ivaria/isoldr.h"
#include "csutil/event.h"

#include "iengine/material.h"

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

#include "isomap.h"

CS_IMPLEMENT_APPLICATION

// The global system driver
IsoMap1 *System;

IsoMap1::IsoMap1 ()
{
  mouse = 0;
  lastclick.Set(0,0,0);
  walking = false;
}

IsoMap1::~IsoMap1 ()
{
}

void IsoMap1::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (System->object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.application.isotest", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  iObjectRegistry* object_reg = System->object_reg;
  delete System; System = 0;
  csInitializer::DestroyApplication (object_reg);
}


/// call during 2d phase
static void DebugZBufShow(iGraphics3D *g3d, iGraphics2D *g2d,
  iTextureManager* /*txtmgr*/)
{
  int w = g3d->GetWidth();
  int h = g3d->GetHeight();
  for(int y=0; y<h; y++)
    for(int x=0; x<w; x++)
    {
      float zbuf = g3d->GetZBuffValue(x,y);
      uint32 zint = (uint32)zbuf;
      g2d->DrawPixel(x, y, zint);
    }
}

struct PlayerGridChange : public iGridChangeCallback
{
  IsoMap1* app;
  SCF_DECLARE_IBASE;
  PlayerGridChange () { SCF_CONSTRUCT_IBASE (0); }
  virtual ~PlayerGridChange() { SCF_DESTRUCT_IBASE(); }
  virtual void GridChange (iIsoSprite* spr);
};

SCF_IMPLEMENT_IBASE (PlayerGridChange)
  SCF_IMPLEMENTS_INTERFACE (iGridChangeCallback)
SCF_IMPLEMENT_IBASE_END

/// helper grid change callback for the player sprite - to move the light
void PlayerGridChange::GridChange (iIsoSprite *sprite)
{
  if(app->GetLight())
    app->GetLight()->SetGrid(sprite->GetGrid());
}

static bool IsoEventHandler (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    System->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    System->FinishFrame ();
    return true;
  }
  else
  {
    return System ? System->HandleEvent (ev) : false;
  }
}

bool IsoMap1::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize app!");
    return false;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
  	CS_REQUEST_OPENGL3D,
	  CS_REQUEST_PLUGIN("crystalspace.engine.iso", iIsoEngine),
	  CS_REQUEST_PLUGIN("crystalspace.iso.loader", iIsoLoader),
	  CS_REQUEST_FONTSERVER,
	  CS_REQUEST_IMAGELOADER,
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, IsoEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iIsoEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No IsoEngine plugin!");
    return false;
  }

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

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    return false;
  }

  mouse = CS_QUERY_REGISTRY (object_reg, iMouseDriver);
  if (!mouse)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iMouseDriver!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iIsoLoader);
  if (!loader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,	"No iIsoLoader plugin!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("IsoMap1 Crystal Space Application");
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  csRef<iFontServer> fsvr (CS_QUERY_REGISTRY (object_reg, iFontServer));
  font = fsvr->LoadFont(CSFONT_LARGE);

  // Setup the texture manager
  iTextureManager* txtmgr = myG3D->GetTextureManager ();

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY, 
	  "IsoMap1 Crystal Space Application version 0.1.");

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Loading world...");
  Report (CS_REPORTER_SEVERITY_NOTIFY, "------------------------------");

  // Load map here
  if (!LoadMap())
	  return false;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "World loaded...");
  Report (CS_REPORTER_SEVERITY_NOTIFY, "------------------------------");

  // create our world to play in, and a view on it.
  iIsoGrid *grid = world->FindGrid(startpos);

  // add a dynamic light for the player, above players head.
  light = csPtr<iIsoLight> (engine->CreateLight());
  light->Flags().Set(CSISO_LIGHT_DYNAMIC);
  light->SetPosition(startpos+csVector3(0,5,0));
  light->SetGrid(grid);
  light->SetRadius(5.0);
  light->SetColor(csColor(1.0f, 0.4f, 0.2f));

  // Find a material to use for the player sprite.
  iMaterialList *mats = engine->GetMaterialList();
  iMaterialWrapper *halo = mats->FindByName("halo");

  // add the player sprite to the world
  player = csPtr<iIsoSprite> (engine->CreateFrontSprite(startpos, 1.3f, 2.7f));
  player->SetMaterialWrapper(halo);
  player->SetMixMode(CS_FX_ADD);
  world->AddSprite(player);

  PlayerGridChange* cb = new PlayerGridChange ();
  cb->app = this;
  player->SetGridChangeCallback(cb);
  cb->DecRef ();

  // prepare texture manager
  txtmgr->PrepareTextures ();
  txtmgr->PrepareMaterials ();

  // scroll view to show player position at center of screen
  view->SetScroll(
    startpos, csVector2(myG3D->GetWidth()/2, myG3D->GetHeight()/2));

  return true;
}

bool IsoMap1::LoadMap ()
{

  if (!loader->LoadMapFile ("/lev/isomap/world"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isomap",
    	"Couldn't load world!");
    return false;
  }

  Report(CS_REPORTER_SEVERITY_NOTIFY, "Finished loading world !");

  world = CS_QUERY_REGISTRY (object_reg, iIsoWorld);
  view = CS_QUERY_REGISTRY (object_reg, iIsoView);

  if(!world)
  {
    Report(CS_REPORTER_SEVERITY_NOTIFY, "No world - bye !");
    return false;
  }

  if(!view)
  {
	  Report(CS_REPORTER_SEVERITY_NOTIFY, "No view !! - Making one.");
    view = engine->CreateView(world);
    startpos.Set(5,0,5);
  }
  else
  {
    startpos = view->GetViewScroll();
  }
  
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Map loaded ok!");

  return true;
}



void IsoMap1::SetupFrame ()
{
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  // keep track of fps.
  static bool fpsstarted = false;
  static int numframes = 0;
  static int numtime = 0;
  static float fps = 0.0;
  if(!fpsstarted && (elapsed_time > 0))
  {fpsstarted = true; fps = 1. / (float(elapsed_time)*.001);}
  numtime += elapsed_time;
  numframes ++;
  if(numtime >= 1000)
  {
     fps = float(numframes) / (float(numtime)*.001);
     numtime = 0;
     numframes = 0;
  }

  // move the player and scroll the view to keep the player centered
  // according to keyboard state
  float speed = (elapsed_time / 1000.) * (0.10 * 20);
  csVector3 playermotion(0,0,0);
  if (kbd->GetKeyState (CSKEY_RIGHT)) playermotion += csVector3(0,0,speed);
  if (kbd->GetKeyState (CSKEY_LEFT)) playermotion += csVector3(0,0,-speed);
  if (kbd->GetKeyState (CSKEY_PGUP)) playermotion += csVector3(0,speed,0);
  if (kbd->GetKeyState (CSKEY_PGDN)) playermotion += csVector3(0,-speed,0);
  if (kbd->GetKeyState (CSKEY_UP)) playermotion += csVector3(-speed,0,0);
  if (kbd->GetKeyState (CSKEY_DOWN)) playermotion += csVector3(speed,0,0);
  if(!playermotion.IsZero()) // keyboard stops player movement by mouse
    walking = false;
  if (mouse->GetLastButton(1))
  {
    walking = true;
    int mousex = mouse->GetLastX();
    int mousey = mouse->GetLastY();
    // y is up in camera view
    csVector2 screenpos(mousex, myG3D->GetHeight() - mousey);
    view->S2W(screenpos, lastclick);
  }
  if(walking)
  {
    // if no keyboard cursor use, and there was a click,
    // move towards last click position.
    speed *= 1.412f; // mouse move is as fast as keyboard can be
    playermotion = lastclick - player->GetPosition();
    float playermove = playermotion.Norm(); // distance moved
    if(playermove > speed) playermotion *= speed / playermove;

    // make sure we do not scroll too far
    csBox3 maxbox = player->GetGrid()->GetBox();
    maxbox.SetSize( (maxbox.Max() - maxbox.Min())*1.5 );
    if(!player->GetGrid()->Contains(lastclick)
      && !maxbox.In(view->GetViewScroll()))
      walking = false;
  }
  if(!playermotion.IsZero())
  {
    csVector3 oldpos = player->GetPosition();
    player->MovePosition(playermotion);
    view->MoveScroll(player->GetPosition() - oldpos);
    light->SetPosition(player->GetPosition()+csVector3(0,5,0));
  }

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS
    | CSDRAW_CLEARSCREEN ))
    return;

  view->Draw ();

  // Start drawing 2D graphics.
  if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  if(0) /// debug zbuffer
    DebugZBufShow(myG3D, myG2D, txtmgr);

  char buf[255];
  sprintf(buf, "FPS: %g    loc(%g,%g,%g)", fps, player->GetPosition().x,
    player->GetPosition().y, player->GetPosition().z);
  myG2D->Write(font, 10, myG2D->GetHeight() - 20, myG2D->FindRGB(255,255,255),
    -1, buf);
}

void IsoMap1::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (0);
}


bool IsoMap1::HandleEvent (iEvent &Event)
{
  if ((Event.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_ESC))
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }
  if ((Event.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_TAB))
  {
    static float settings[][5] = {
    // scale should be *displayheight.
    //xscale, yscale, zscale, zskew, xskew
    {1.0f/16.0f, 1.0f/16.0f, 1.0f/16.0f, 1.0f, 1.0f},
    {1.0f/16.0f, 1.0f/16.0f, 1.0f/16.0f, 0.6f, 0.6f},
    {1.0f/16.0f, 1.0f/16.0f, 1.0f/16.0f, 0.5f, 0.5f},
    {1.0f/40.0f, 1.0f/40.0f, 1.0f/40.0f, 0.5f, 0.5f},
    {1.0f/8.0f,  1.0f/8.0f,  1.0f/8.0f,  0.5f, 0.5f},
    };
    static int nrsettings = 5;
    static int setting = 0;
    setting = (setting+1)%nrsettings;
    float h = engine->GetG3D()->GetHeight();
    float ycorrect = 1.0;
    if(setting!=0)
    {
      ycorrect = sqrt(settings[setting][4]*settings[setting][4]+1.)
        + sqrt(settings[setting][3]*settings[setting][3]+1.);
      ycorrect *= .5;
    }
    view->SetAxes( h*settings[setting][0], h*settings[setting][1]*ycorrect,
      h*settings[setting][2], settings[setting][3], settings[setting][4]);
    view->SetScroll(player->GetPosition(),
      csVector2(myG3D->GetWidth()/2, myG3D->GetHeight()/2));
    return true;
  }

  return false;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (0));

  // Create our main class.
  System = new IsoMap1 ();

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, 0))
  {
    System->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    Cleanup ();
    exit (1);
  }

  // Main loop.
  csDefaultRunLoop(System->object_reg);

  // Cleanup.
  Cleanup ();

  return 0;
}
