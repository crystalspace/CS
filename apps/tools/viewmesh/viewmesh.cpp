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

/* ViewMesh: tool for displaying mesh objects (3d sprites) */

#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "csutil/cscolor.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "viewmesh.h"
#include "iutil/eventq.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "imesh/object.h"
#include "imesh/sprite3d.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "igraphic/imageio.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "iutil/vfs.h"

CS_IMPLEMENT_APPLICATION

#define VIEWMESH_COMMAND_LOADMESH 77701

//-----------------------------------------------------------------------------

ViewMesh::ViewMesh (iObjectRegistry *object_reg, csSkin &Skin)
    : csApp (object_reg, Skin)
{
  SetBackgroundStyle(csabsNothing);
  view = NULL;
  engine = NULL;
  loader = NULL;
  g3d = NULL;
  menu = NULL;
  dialog = NULL;
}

ViewMesh::~ViewMesh ()
{
  if (view) view->DecRef ();
  if (engine) engine->DecRef ();
  if (loader) loader->DecRef();
  if (g3d) g3d->DecRef ();
  if (menu) delete menu;
  if (dialog) delete dialog;
}

bool ViewMesh::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    // display next fram
    Draw();
  }
    
  if (csApp::HandleEvent(ev))
    return true;
    
  switch(ev.Type)
  {
    case csevKeyDown:
      if (ev.Key.Code == CSKEY_ESC)
      {
	iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
	if (q)
	{
	  q->GetEventOutlet()->Broadcast
	    (cscmdQuit);
	  q->DecRef ();
	}
	return true;
      }
      break;
    case csevMouseDown:
    case csevMouseDoubleClick:
      if (menu) {
	  delete menu;
	  menu=NULL;
	  return true;
      }
      // Create a menu
      menu = new csMenu (this, csmfs3D, 0);
      (void)new csMenuItem(menu);
      (void)new csMenuItem(menu,"Load Mesh", VIEWMESH_COMMAND_LOADMESH);
      (void)new csMenuItem (menu, "~Quit", cscmdQuit);
      menu->SetPos(ev.Mouse.x,ev.Mouse.y);
      return true;

    case csevCommand:
      switch(ev.Command.Code)
      {
	case VIEWMESH_COMMAND_LOADMESH:
	{
	  if (menu)
	  {
	    delete menu;
	    menu=NULL;
	  }
      	  if (dialog)
	    delete dialog;
	  dialog= csFileDialog (this, "Select Mesh Object", "/this/", "Open", true);
	  StartModal (dialog, NULL);
	  return true;
	}
	case cscmdStopModal:
	{
	  char filename[1024];
	  csQueryFileDialog (dialog, filename, sizeof(filename));
	  delete dialog;
	  dialog = NULL;

	  if (!LoadSprite(filename,1))
	  {
	    Printf (CS_REPORTER_SEVERITY_ERROR, "couldn't load mesh");
	  }
	}
	default:
	  break;
      }
      break;
  }
 
  return false; 
}

bool ViewMesh::LoadSprite(const char *filename,float scale)
{
  iMeshFactoryWrapper *imeshfact = loader->LoadMeshObjectFactory (filename);

  if (!imeshfact)
    return false;

  iMeshWrapper *sprite = engine->CreateMeshWrapper(
      imeshfact, "MySprite", room,
      csVector3 (0, 10, 0));
  csMatrix3 m; m.Identity(); m *= scale;
  sprite->GetMovable()->GetTransform();
  sprite->GetMovable()->UpdateMove();
  iSprite3DState *spstate = SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
      iSprite3DState);
  if (spstate)
  {
    spstate->SetAction("default");
    spstate->DecRef();
  }
  imeshfact->DecRef();
  sprite->DeferUpdateLighting (CS_NLIGHT_DYNAMIC|CS_NLIGHT_STATIC, 10);

  return true;
}

void ViewMesh::Draw()
{
  // First get elapsed time from the system driver.
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();
  
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  if (!dialog && !menu)
  {
    iCamera* c = view->GetCamera();
    if (GetKeyState (CSKEY_RIGHT))
      c->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed);
    if (GetKeyState (CSKEY_LEFT))
      c->GetTransform ().RotateThis (VEC_ROT_LEFT, speed);
    if (GetKeyState (CSKEY_PGUP))
      c->GetTransform ().RotateThis (VEC_TILT_UP, speed);
    if (GetKeyState (CSKEY_PGDN))
      c->GetTransform ().RotateThis (VEC_TILT_DOWN, speed);
    if (GetKeyState (CSKEY_UP))
      c->Move (VEC_FORWARD * 4 * speed);
    if (GetKeyState (CSKEY_DOWN))
      c->Move (VEC_BACKWARD * 4 * speed);
  }
    
  csApp::Draw();
  pplBeginDraw(CSDRAW_3DGRAPHICS);
  view->Draw();
  pplInvalidate(bound);
  if (menu) {
    menu->Invalidate(true);
  }
  if (dialog) {
    dialog->Invalidate(true);
  }
}

#define VM_QUERYPLUGIN(var, intf, str)				\
  var = CS_QUERY_REGISTRY (object_reg, intf);			\
  if (!var)							\
  {								\
    Printf (CS_REPORTER_SEVERITY_ERROR, "No " str " plugin!");	\
    return false;						\
  }

bool ViewMesh::Initialize ()
{
  if (!csApp::Initialize())
    return false;

  // Query for plugins
  // Find the pointer to engine plugin
  VM_QUERYPLUGIN (engine, iEngine, "iEngine");

  VM_QUERYPLUGIN (loader, iLoader, "iLoader");
  VM_QUERYPLUGIN (g3d, iGraphics3D, "iGraphics3D");

  // Open the main system. This will open all the previously loaded plug-ins.
  iGraphics2D* g2d = g3d->GetDriver2D ();
  iNativeWindow* nw = g2d->GetNativeWindow ();
  if (nw)
    nw->SetTitle ("View Mesh");

  // Setup the texture manager
  iTextureManager* txtmgr = g3d->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // Initialize the texture manager
  txtmgr->ResetPalette ();

  Printf (CS_REPORTER_SEVERITY_NOTIFY,
    "View Mesh version 0.1.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Printf (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    Printf (CS_REPORTER_SEVERITY_ERROR, "Error loading 'stone4' texture!");
    return false;
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  iThingState* walls_state = SCF_QUERY_INTERFACE (walls->GetMeshObject (),
  	iThingState);
  iPolygon3D* p;
  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 0, 5));
  p->CreateVertex (csVector3 (5, 0, 5));
  p->CreateVertex (csVector3 (5, 0, -5));
  p->CreateVertex (csVector3 (-5, 0, -5));
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
  p->CreateVertex (csVector3 (5, 0, 5));
  p->CreateVertex (csVector3 (-5, 0, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (5, 20, 5));
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (5, 0, -5));
  p->CreateVertex (csVector3 (5, 0, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (-5, 20, 5));
  p->CreateVertex (csVector3 (-5, 0, 5));
  p->CreateVertex (csVector3 (-5, 0, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (-5, 0, -5));
  p->CreateVertex (csVector3 (5, 0, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  walls_state->DecRef ();
  walls->DecRef ();

  iStatLight* light;
  iLightList* ll = room->GetLights ();
  light = engine->CreateLight (NULL, csVector3 (-3, 10, 0), 10,
  	csColor (.8, .8, .8), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  light = engine->CreateLight (NULL, csVector3 (3, 10,  0), 10,
  	csColor (.8, .8, .8), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  light = engine->CreateLight (NULL, csVector3 (0, 10, -3), 10,
  	csColor (.8, .8, .8), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  light = engine->CreateLight (NULL, csVector3 (0, 10,  3), 10,
  	csColor (.8, .8, .8), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();

  engine->Prepare ();
  Printf (CS_REPORTER_SEVERITY_NOTIFY, "Created.");

  view = new csView (engine, g3d);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 10, -4));
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  txtmgr->SetPalette ();

  iCommandLineParser *cmdline;
  VM_QUERYPLUGIN (cmdline, iCommandLineParser, "iCommandLineParser");

  const char* meshfilename = cmdline->GetName (0);
  const char* texturefilename = cmdline->GetName (1);
  const char* texturename = cmdline->GetName (2);
  const char* scaleTxt = cmdline->GetName (3);
  cmdline->DecRef ();
  float scale = 1;
  if (scaleTxt != NULL)
  {
    sscanf (scaleTxt, "%f", &scale);
  }

  // Load a texture for our sprite.
  if (texturefilename && texturename)
  {
    iTextureWrapper* txt = loader->LoadTexture (texturename,
  	  texturefilename);
    if (txt == NULL)
    {
      Printf (CS_REPORTER_SEVERITY_ERROR, "Error loading texture '%s'!",
      	texturefilename);
      return false;
    }
    txt->Register (txtmgr);
    txt->GetTextureHandle()->Prepare ();
    iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName (
    	texturename);
    mat->Register (txtmgr);
    mat->GetMaterialHandle ()->Prepare ();
  }

  // Load a sprite template from disk.
  if (meshfilename && !LoadSprite(meshfilename,scale))
  {
    Printf (CS_REPORTER_SEVERITY_ERROR,
    	"Error loading mesh object factory '%s'!", meshfilename);
    return false;
  }

  return true;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/

// define a skin for csws
CSWS_SKIN_DECLARE_DEFAULT (DefaultSkin);

int main (int argc, char* argv[])
{ 
  srand (time (NULL));

  iObjectRegistry *object_reg = csInitializer::CreateEnvironment();
  if (!object_reg)
    return 1;

  if (!csInitializer::SetupConfigManager (object_reg, NULL))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	"crystalspace.application.viewmesh", "Couldn't load config file!");
    return 1;
  }

  csInitializer::SetupCommandLineParser (object_reg, argc, argv);
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
	"crystalspace.application.viewmesh", "Couldn't find plugins!\n"
	"Is your CRYSTAL environment var properly set?");
    return 1;
  }
  
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help(object_reg);
    return 0;
  }

  if (!csInitializer::OpenApplication(object_reg))
  {
    return 1;
  }

  // Create our main class.
  ViewMesh *app = new ViewMesh (object_reg, DefaultSkin);

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!app->Initialize ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, 
	"crystalspace.application.viewmesh", "Error initializing system!");
    csInitializer::DestroyApplication(object_reg);
    return 1;
  }

  // Main loop.
  csDefaultRunLoop(object_reg);

  // Cleanup.
  delete app;
  csInitializer::DestroyApplication(object_reg);

  return 0;
}

