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
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cscolor.h"
#include "csutil/cmdline.h"
#include "csutil/cmdhelp.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivaria/conout.h"
#include "iutil/event.h"
#include "iutil/virtclk.h"
#include "cswseng.h"
#include "iengine/camera.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/campos.h"
#include "iengine/mesh.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "igraphic/imageio.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "csutil/event.h"

#define SET_BIT(var,mask,state) \
  var = (var & ~(mask)) | ((state) ? (mask) : 0);

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

ceEngineView::ceEngineView (csComponent *iParent, iEngine *Engine,
  	iSector *Start, const csVector3& start_pos, iGraphics3D *G3D)
  	: csComponent (iParent)
{
  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = csPtr<iView> (new csView (Engine, G3D));
  view->GetCamera ()->SetSector (Start);
  view->GetCamera ()->GetTransform ().SetOrigin (start_pos);

  motion = 0;
  SetState (CSS_SELECTABLE, true);
  if (parent)
    parent->SendCommand (cscmdWindowSetClient, (intptr_t)this);
}

ceEngineView::~ceEngineView ()
{
  ceCswsEngineApp* lapp = (ceCswsEngineApp*)app;
  lapp->engine_views.DeleteIndex (lapp->engine_views.Find (this));
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
	csRef<iVirtualClock> vc (
		CS_QUERY_REGISTRY (app->object_reg, iVirtualClock));
        elapsed_time = vc->GetElapsedTicks ();
	current_time = vc->GetCurrentTicks ();

        // Now rotate the camera according to keyboard state
        float speed = (elapsed_time / 1000.0f) * (0.03f * 20.0f);

        if (motion & 0x00000001)
          view->GetCamera ()->Move (CS_VEC_FORWARD * 4.0f * speed);
        if (motion & 0x00000002)
          view->GetCamera ()->Move (CS_VEC_BACKWARD * 4.0f * speed);
        if (motion & 0x00000004)
          view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
        if (motion & 0x00000008)
          view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
        if (motion & 0x00000010)
          view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
        if (motion & 0x00000020)
          view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);

        // Invalidate this view so that it gets updated
	// We invalidate all the time so that animations get updated.
        Invalidate ();
      }
      break;
    case csevKeyboard:
      switch (csKeyEventHelper::GetCookedCode (&Event))
      {
        case CSKEY_UP:
          SET_BIT (motion, 0x00000001, 
	    csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown);
          break;
        case CSKEY_DOWN:
          SET_BIT (motion, 0x00000002, 
	    csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown);
          break;
        case CSKEY_LEFT:
          SET_BIT (motion, 0x00000004, 
	    csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown);
          break;
        case CSKEY_RIGHT:
          SET_BIT (motion, 0x00000008, 
	    csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown);
          break;
        case CSKEY_PGUP:
          SET_BIT (motion, 0x00000010, 
	    csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown);
          break;
        case CSKEY_PGDN:
          SET_BIT (motion, 0x00000020, 
	    csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown);
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

static bool do_invalidate (csComponent *child, intptr_t param)
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
  app->ForEach (do_invalidate, (intptr_t)&is, true);

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
	    app->StartModal (d, 0);
          }
        }
        return true;
    }
  return csWindow::HandleEvent (Event);
}

/*---------------------------------------------------------------------*
 * ceCswsEngineApp
 *---------------------------------------------------------------------*/

ceCswsEngineApp::ceCswsEngineApp (iObjectRegistry *object_reg, csSkin &skin)
	: csApp (object_reg, skin)
{
  start_pos.Set (0, 0, 0);
}

ceCswsEngineApp::~ceCswsEngineApp ()
{
}

void ceCswsEngineApp::SetupDefaultWorld ()
{
  // First disable the lighting cache.
  engine->DeleteAll ();
  engine->SetLightingCacheMode (0);

  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");
  if (!tm)
  {
    LevelLoader->LoadTexture ("stone", "/lib/std/stone4.gif");
    tm = engine->GetMaterialList ()->FindByName ("stone");
  }

  iSector* room = engine->CreateSector ("room");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> ws =
  	SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  start_sector = room;
  walls_state->AddInsideBox (csVector3 (-5, -1, -5), csVector3 (5, 20, 5));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  iLightList* ll = room->GetLights ();
  csRef<iLight> light;
  light = engine->CreateLight (0, csVector3(-3, 5, 0), 10,
  	csColor(1, 0, 0));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3(3, 5, 0), 10,
  	csColor(0, 0, 1));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3(0, 5, -3), 10,
  	csColor(0, 1, 0));
  ll->Add (light);
}

bool ceCswsEngineApp::Initialize ()
{
  if (!csApp::Initialize ())
    return false;

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswseng", "No iEngine plugin!");
    exit (-1);
  }

  LevelLoader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!LevelLoader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswseng", "No iLoader plugin!");
    exit (-1);
  }

  pG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  // Disable double buffering since it kills performance
  pG3D->GetDriver2D ()->DoubleBuffer (false);

  SetupDefaultWorld ();

  // Change to other directory before doing Prepare()
  // because otherwise precalc_info file will be written into MazeD.zip
  // The /tmp dir is fine for this.
  VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  VFS->ChDir ("/tmp");

  // Now prepare the engine
  engine->Prepare ();

  Console = CS_QUERY_REGISTRY (object_reg, iConsoleOutput);
  if (Console)
  {
    // Tell the console to shut up so that Printf() won't clobber CSWS
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
    case csevKeyboard:
      if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
      {
	switch (csKeyEventHelper::GetCookedCode (&Event))
	{
	  case 'q':
	  {
	    ShutDown ();
	    return true;
	  }
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
          csRef<iMessageBoxData> mbd (
	  	SCF_QUERY_INTERFACE (GetTopModalUserdata (),
		iMessageBoxData));
	  if (mbd)
	  {
	    delete d;
	    return true;
	  }
	}

	// We had a file open dialog since that's the only thing that
	// can cause modality in our app (except for a message box
	// which is handled above).
        char filename[CS_MAXPATHLEN+1];
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
  char copy_filename[CS_MAXPATHLEN+1];
  strcpy (copy_filename, filename);
  int len = strlen (copy_filename);
  if (!strcmp (copy_filename+len-5, "world"))
  {
    copy_filename[len-5] = 0;
  }

  VFS->Unmount ("/tmp/levtool", 0);
  VFS->Mount ("/tmp/levtool", copy_filename);
  VFS->ChDir ("/tmp/levtool");

  // Load the map from the file.
  if (!LevelLoader->LoadMapFile ("world"))
  {
    csMessageBox (this, "ERROR!", "Loading of map failed!", 0);
    SetupDefaultWorld ();
    VFS->ChDir ("/tmp");
  }

  engine->Prepare ();

  // Look for the start sector in this map.
  iCameraPosition *cp = engine->GetCameraPositions ()->FindByName ("Start");
  const char* room_name;
  start_pos.Set (0, 0, 0);
  if (cp)
  {
    room_name = cp->GetSector ();
    start_pos = cp->GetPosition ();
  }
  else
    room_name = "room";
  start_sector = engine->GetSectors ()->FindByName (room_name);

  // Update the engine views.
  size_t i;
  for (i = 0 ; i < engine_views.Length () ; i++)
  {
    ceEngineView* eview = engine_views[i];
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
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;

  srand (time (0));

  if (!csInitializer::SetupConfigManager (object_reg, 0))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswseng",
	"Can't initialize system!");
    return -1;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_SOFTWARE3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswseng",
	"Can't initialize system!");
    return -1;
  }

  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));
  cmdline->AddOption ("mode", "800x600");

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iGraphics3D));
  iNativeWindow* nw = g3d->GetDriver2D ()->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Example: CSWS And Engine");

  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswseng",
	"Can't open system!");
    return -1;
  }

  // Create our main class.
  ceCswsEngineApp *theApp = new ceCswsEngineApp (object_reg, DefaultSkin);

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (theApp->Initialize ())
    csDefaultRunLoop(object_reg);
  else
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswseng", "Error initializing system!");

  delete theApp;
  g3d = 0;
  csInitializer::DestroyApplication (object_reg);

  return 0;
}
