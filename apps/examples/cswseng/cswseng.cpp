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

#define CS_SYSDEF_PROVIDE_PATH
#include "cssysdef.h"
#include "cssys/system.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cscolor.h"
#include "csutil/cmdline.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivaria/conout.h"
#include "isys/event.h"
#include "iutil/strvec.h"
#include "cswseng.h"
#include "iengine/camera.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/campos.h"
#include "iengine/mesh.h"
#include "imesh/object.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "iutil/objreg.h"
#include "isys/plugin.h"
#include "ivaria/reporter.h"

#define SET_BIT(var,mask,state) \
  var = (var & ~(mask)) | ((state) ? (mask) : 0);

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global system driver
SysSystemDriver *System;


ceEngineView::ceEngineView (csComponent *iParent, iEngine *Engine,
  	iSector *Start, const csVector3& start_pos, iGraphics3D *G3D)
  	: csComponent (iParent)
{
  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (Engine, G3D);
  view->GetCamera ()->SetSector (Start);
  view->GetCamera ()->GetTransform ().SetOrigin (start_pos);

  motion = 0;
  SetState (CSS_SELECTABLE, true);
  if (parent)
    parent->SendCommand (cscmdWindowSetClient, (void *)this);
}

ceEngineView::~ceEngineView ()
{
  delete view;
  ceCswsEngineApp* lapp = (ceCswsEngineApp*)app;
  lapp->engine_views.Delete (lapp->engine_views.Find (this));
}

bool ceEngineView::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  bool rc = csComponent::SetRect (xmin, ymin, xmax, ymax);

  parent->LocalToGlobal (xmin, ymin);
  parent->LocalToGlobal (xmax, ymax);
  // Engine uses the upside down coordinate system
  ymin = app->bound.Height () - ymin;
  ymax = app->bound.Height () - ymax;
  view->SetRectangle (xmin, ymax, xmax - xmin, ymin - ymax);
  view->GetCamera ()->SetPerspectiveCenter (
  	(xmin + xmax) / 2, (ymin + ymax) / 2);

  return rc;
}

bool ceEngineView::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
      if (Event.Command.Code == cscmdPreProcess)
      {
        csTicks elapsed_time, current_time;
        System->GetElapsedTime (elapsed_time, current_time);

        // Now rotate the camera according to keyboard state
        float speed = (elapsed_time / 1000.) * (0.03 * 20);

        if (motion & 0x00000001)
          view->GetCamera ()->Move (VEC_FORWARD * 4.0f * speed);
        if (motion & 0x00000002)
          view->GetCamera ()->Move (VEC_BACKWARD * 4.0f * speed);
        if (motion & 0x00000004)
          view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_LEFT, speed);
        if (motion & 0x00000008)
          view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed);
        if (motion & 0x00000010)
          view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_UP, speed);
        if (motion & 0x00000020)
          view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_DOWN, speed);

        // Invalidate this view so that it gets updated
	// We invalidate all the time so that animations get updated.
        Invalidate ();
      }
      break;
    case csevKeyDown:
    case csevKeyUp:
      switch (Event.Key.Code)
      {
        case CSKEY_UP:
          SET_BIT (motion, 0x00000001, Event.Type == csevKeyDown);
          break;
        case CSKEY_DOWN:
          SET_BIT (motion, 0x00000002, Event.Type == csevKeyDown);
          break;
        case CSKEY_LEFT:
          SET_BIT (motion, 0x00000004, Event.Type == csevKeyDown);
          break;
        case CSKEY_RIGHT:
          SET_BIT (motion, 0x00000008, Event.Type == csevKeyDown);
          break;
        case CSKEY_PGUP:
          SET_BIT (motion, 0x00000010, Event.Type == csevKeyDown);
          break;
        case CSKEY_PGDN:
          SET_BIT (motion, 0x00000020, Event.Type == csevKeyDown);
          break;
      }
      break;
  }
  return csComponent::HandleEvent (Event);
}

void ceEngineView::SetState (int mask, bool enable)
{
  if (!enable && (mask == CSS_FOCUSED))
    motion = 0;
  csComponent::SetState (mask, enable);
}

// private structure for internal communication with do_invalidate
struct inv_struct
{
  csRect rect;
  csComponent *stop_at;
};

static bool do_invalidate (csComponent *child, void *param)
{
  inv_struct *is = (inv_struct *)param;

  if (child == is->stop_at)
    return true;

  if (child->bound.Intersects (is->rect))
  {
    csRect rel (is->rect);
    rel.xmin -= child->bound.xmin;
    rel.ymin -= child->bound.ymin;
    rel.xmax -= child->bound.xmin;
    rel.ymax -= child->bound.ymin;
    child->Invalidate (rel, true);
  }

  return false;
}

void ceEngineView::Draw ()
{
  // Now its our time... Tell the engine to display 3D graphics
  app->pplBeginDraw (((ceCswsEngineApp *)app)->engine->GetBeginDrawFlags ()
  	| CSDRAW_3DGRAPHICS);
  view->Draw ();

  // Ok, now invalidate all the windows that are above us
  inv_struct is;
  is.stop_at = parent;
  is.rect.Set (bound);
  parent->LocalToGlobal (is.rect.xmin, is.rect.ymin);
  parent->LocalToGlobal (is.rect.xmax, is.rect.ymax);
  app->ForEach (do_invalidate, &is, true);

  // Also tell the graphics pipeline to update this rectangle
  app->pplInvalidate (is.rect);
}

//-----------------------------------------------------------------------------

bool ceControlWindow::HandleEvent (iEvent& Event)
{
  ceCswsEngineApp* ceapp = (ceCswsEngineApp*)app;
  if (Event.Type == csevCommand)
    switch (Event.Command.Code)
    {
      case cecmdQuit:
        app->SendCommand (cscmdQuit);
        break;
      case cecmdNewView:
        {
  	  csWindow *w = new csWindow (app, "3D view",
    	    CSWS_DEFAULTVALUE & ~ CSWS_MENUBAR);
  	  ceapp->engine_views.Push (new ceEngineView (w, ceapp->engine,
	  	ceapp->start_sector, ceapp->start_pos, ceapp->pG3D));
  	  w->SetRect (0, 0, 200, 200);
	  w->Select ();
	}
        break;
      case cecmdLoad:
        {
          csWindow* d = csFileDialog (app, "Load a level");
          if (d)
          {
	    app->StartModal (d, NULL);
          }
        }
        return true;
    }
  return csWindow::HandleEvent (Event);
}

/*---------------------------------------------------------------------*
 * ceCswsEngineApp
 *---------------------------------------------------------------------*/

ceCswsEngineApp::ceCswsEngineApp (iSystem *iSys, csSkin &skin)
	: csApp (iSys, skin)
{
  engine = NULL;
  pG3D = NULL;
  VFS = NULL;
  start_pos.Set (0, 0, 0);
  LevelLoader = NULL;
}

ceCswsEngineApp::~ceCswsEngineApp ()
{
  if (pG3D) pG3D->DecRef ();
  if (VFS) VFS->DecRef ();
  if (LevelLoader) LevelLoader->DecRef ();
  if (engine) engine->DecRef ();
}

void ceCswsEngineApp::SetupDefaultWorld ()
{
  // First disable the lighting cache.
  engine->DeleteAll ();
  engine->SetLightingCacheMode (0);

  iMaterialWrapper* tm = engine->FindMaterial ("stone");
  if (!tm)
  {
    LevelLoader->LoadTexture ("stone", "/lib/std/stone4.gif");
    tm = engine->FindMaterial ("stone");
  }

  iSector* room = engine->CreateSector ("room");
  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  iThingState* walls_state = SCF_QUERY_INTERFACE (walls->GetMeshObject (),
  	iThingState);
  start_sector = room;
  iPolygon3D* p;
  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, -1, 5));
  p->CreateVertex (csVector3 (5, -1, 5));
  p->CreateVertex (csVector3 (5, -1, -5));
  p->CreateVertex (csVector3 (-5, -1, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (5, 20, 5));
  p->CreateVertex (csVector3 (-5, 20, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 20, 5));
  p->CreateVertex (csVector3 (5, 20, 5));
  p->CreateVertex (csVector3 (5, -1, 5));
  p->CreateVertex (csVector3 (-5, -1, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (5, 20, 5));
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (5, -1, -5));
  p->CreateVertex (csVector3 (5, -1, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (-5, 20, 5));
  p->CreateVertex (csVector3 (-5, -1, 5));
  p->CreateVertex (csVector3 (-5, -1, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (-5, -1, -5));
  p->CreateVertex (csVector3 (5, -1, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);
  walls_state->DecRef ();

  iStatLight* light;
  light = engine->CreateLight (NULL, csVector3(-3, 5, 0), 10,
  	csColor(1, 0, 0), false);
  room->AddLight (light);
  light = engine->CreateLight (NULL, csVector3(3, 5, 0), 10,
  	csColor(0, 0, 1), false);
  room->AddLight (light);
  light = engine->CreateLight (NULL, csVector3(0, 5, -3), 10,
  	csColor(0, 1, 0), false);
  room->AddLight (light);
}

bool ceCswsEngineApp::Initialize ()
{
  if (!csApp::Initialize ())
    return false;

  iObjectRegistry* object_reg = System->GetObjectRegistry ();
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  // Find the pointer to engine plugin
  engine = CS_QUERY_PLUGIN (plugin_mgr, iEngine);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswseng", "No iEngine plugin!");
    abort ();
  }

  LevelLoader = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_LVLLOADER, iLoader);
  if (!LevelLoader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswseng", "No iLoader plugin!");
    abort ();
  }
  
  pG3D = CS_QUERY_PLUGIN (plugin_mgr, iGraphics3D);
  // Disable double buffering since it kills performance
  pG3D->GetDriver2D ()->DoubleBuffer (false);
  iTextureManager* txtmgr = pG3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // Initialize the texture manager
  txtmgr->ResetPalette ();
  
  // Allocate a uniformly distributed in R,G,B space palette for console
  // The console will crash on some platforms if this isn't initialize properly
  int r,g,b;
  for (r = 0; r < 8; r++)
    for (g = 0; g < 8; g++)
      for (b = 0; b < 4; b++)
	txtmgr->ReserveColor (r * 32, g * 32, b * 64);
  txtmgr->SetPalette ();

  SetupDefaultWorld ();

  // Change to other directory before doing Prepare()
  // because otherwise precalc_info file will be written into MazeD.zip
  // The /tmp dir is fine for this.
  VFS = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_VFS, iVFS);
  VFS->ChDir ("/tmp");

  // Now prepare the engine
  engine->Prepare ();

  txtmgr->SetPalette ();

  Console = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_CONSOLE, iConsoleOutput);
  if (Console)
  {
    // Tell the console to shut up so that iSystem::Printf() won't clobber CSWS
    Console->SetVisible (false);
    Console->AutoUpdate (false);
  }

  //------------------------------- ok, now initialize the CSWS application ---

  // Initialize the engine window ...
  csWindow *w = new csWindow (this, "3D View",
    CSWS_DEFAULTVALUE & ~(CSWS_BUTCLOSE | CSWS_MENUBAR));
  engine_views.Push (new ceEngineView (w, engine, start_sector,
  	csVector3 (0, 5, 0), pG3D));
  w->SetRect (bound.Width () / 2, 0, bound.Width (), bound.Height () / 2);

  w = new ceControlWindow (this, "", CSWS_DEFAULTVALUE & ~CSWS_MENUBAR);
  w->SetRect (100, 50, 100+120, 50+100);
  csComponent* d = new csDialog (w);
  csButton* but;
  but = new csButton (d, cecmdLoad);
  but->SetText ("Load Level");
  but->SetPos (5, 5); but->SetSize (100, 20);

  but = new csButton (d, cecmdNewView);
  but->SetText ("New View");
  but->SetPos (5, 29); but->SetSize (100, 20);

  but = new csButton (d, cecmdQuit);
  but->SetText ("~Quit");
  but->SetPos (5, 49); but->SetSize (100, 20);

  return true;
}

bool ceCswsEngineApp::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevKeyDown:
      switch (Event.Key.Code)
      {
        case 'q':
        {
          ShutDown ();
          return true;
        }
      }
      break;
    case csevCommand:
      if (Event.Command.Code == cscmdStopModal)
      {
	csComponent* d = GetTopModalComponent ();
	int rc = (int)Event.Command.Info;
	if (rc == cscmdCancel) { delete d; return true; }

        if (GetTopModalUserdata ())
	{
          iMessageBoxData* mbd = SCF_QUERY_INTERFACE (GetTopModalUserdata (),
		iMessageBoxData);
	  if (mbd)
	  {
	    mbd->DecRef ();
	    delete d;
	    return true;
	  }
	}

	// We had a file open dialog since that's the only thing that
	// can cause modality in our app (except for a message box
	// which is handled above).
        char filename[MAXPATHLEN+1];
        csQueryFileDialog ((csWindow*)d, filename, sizeof (filename));
        delete d;
	LoadNewMap (filename);
	return true;
      }
      break;
  }
  return csApp::HandleEvent (Event);
}

void ceCswsEngineApp::LoadNewMap (const char* filename)
{
  // Enable the lighting cache again.
  engine->SetLightingCacheMode (CS_ENGINE_CACHE_READ);

  // If the selected file is already a 'world' file then
  // we use that.
  char copy_filename[MAXPATHLEN+1];
  strcpy (copy_filename, filename);
  int len = strlen (copy_filename);
  if (!strcmp (copy_filename+len-5, "world"))
  {
    copy_filename[len-5] = 0;
  }

  VFS->Unmount ("/tmp/levtool", NULL);
  VFS->Mount ("/tmp/levtool", copy_filename);
  VFS->ChDir ("/tmp/levtool");

  // Load the map from the file.
  if (!LevelLoader->LoadMapFile ("world"))
  {
    csMessageBox (this, "ERROR!", "Loading of map failed!", NULL);
    SetupDefaultWorld ();
    VFS->ChDir ("/tmp");
  }

  engine->Prepare ();

  // Look for the start sector in this map.
  iCameraPosition *cp = engine->FindCameraPosition ("Start");
  const char* room_name;
  start_pos.Set (0, 0, 0);
  if (cp)
  {
    room_name = cp->GetSector ();
    start_pos = cp->GetPosition ();
  }
  else
    room_name = "room";
  start_sector = engine->FindSector (room_name);

  // Update the engine views.
  int i;
  for (i = 0 ; i < engine_views.Length () ; i++)
  {
    ceEngineView* eview = (ceEngineView*)engine_views[i];
    if (cp) cp->Load (eview->GetView ()->GetCamera (), engine);
    else eview->GetView ()->GetCamera ()->GetTransform ().SetOrigin (start_pos);
    eview->GetView ()->GetCamera ()->SetSector (start_sector);
  }
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
CSWS_SKIN_DECLARE_DEFAULT (DefaultSkin);

int main (int argc, char* argv[])
{
  SysSystemDriver Sys;
  srand (time (NULL));
  System = &Sys;

  iObjectRegistry* object_reg = System->GetObjectRegistry ();
  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);
  cmdline->AddOption ("mode", "800x600");
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.engine.3d:Engine");
  System->RequestPlugin ("crystalspace.level.loader:LevelLoader");

  if (!Sys.Initialize (argc, argv, NULL))
  {
    printf ("System not initialized !\n");
    return -1;
  }    
  csInitializeApplication (&Sys);

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  iGraphics3D* g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  iNativeWindow* nw = g3d->GetDriver2D ()->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Example: CSWS And Engine");

  if (!Sys.Open ())
  {
    printf ("Could not open system !\n");
    return -1;
  }
  // Create our main class.
  ceCswsEngineApp theApp (&Sys, DefaultSkin);

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (theApp.Initialize ())
    Sys.Loop ();
  else
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswseng", "Error initializing system!");

  return 0;
}
