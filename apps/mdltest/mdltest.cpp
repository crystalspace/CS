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
#include "cssys/sysfunc.h"
#include "mdltest.h"
#include "csutil/cscolor.h"
#include "cstool/csview.h"
#include "cstool/mdldata.h"
#include "cstool/mdltool.h"
#include "cstool/initapp.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "imesh/object.h"
#include "imesh/crossbld.h"
#include "imesh/sprite3d.h"
#include "imesh/mdlconv.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "imap/parser.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "iutil/csinput.h"
#include "csutil/csstring.h"
#include "csutil/cmdhelp.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "igraphic/imageio.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global system driver
Simple *System;

void Cleanup ();

void InitializeSprite (iMeshWrapper *SpriteWrapper)
{
  iSprite3DState *sprState = SCF_QUERY_INTERFACE (
  	SpriteWrapper->GetMeshObject (), iSprite3DState);
  sprState->SetBaseColor (csColor (1, 1, 1));
  sprState->SetLighting (false);
  sprState->SetAction ("action");
  sprState->DecRef ();

  int i;
  iMeshList* ml = SpriteWrapper->GetChildren ();
  for (i=0; i<ml->GetCount (); i++)
    InitializeSprite (ml->Get (i));
}

iModelDataVertices *Simple::CreateDefaultModelVertexFrame ()
{
  iModelDataVertices *Vertices = new csModelDataVertices ();

  Vertices->AddNormal (csVector3 (1, 0, 0));
  Vertices->AddNormal (csVector3 (-1, 0, 0));
  Vertices->AddNormal (csVector3 (0, 1, 0));
  Vertices->AddNormal (csVector3 (0, -1, 0));
  Vertices->AddNormal (csVector3 (0, 0, 1));
  Vertices->AddNormal (csVector3 (0, 0, -1));

  Vertices->AddColor (csColor (1, 1, 1));

  Vertices->AddTexel (csVector2 (0, 0));
  Vertices->AddTexel (csVector2 (0, 5));
  Vertices->AddTexel (csVector2 (5, 5));
  Vertices->AddTexel (csVector2 (5, 0));

  return Vertices;
}

void Simple::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.application.mdltest", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

iModelData *Simple::CreateDefaultModel ()
{
  iMaterialWrapper *m1 = engine->GetMaterialList ()
  	->FindByName ("material1");
  iMaterialWrapper *m2 = engine->GetMaterialList ()
  	->FindByName ("material2");
  iMaterialWrapper *m3 = engine->GetMaterialList ()
  	->FindByName ("material3");

  iModelData *Model = new csModelData ();

  iModelDataMaterial *mat = new csModelDataMaterial ();
  mat->SetMaterialWrapper (m1);
  iModelDataMaterial *mat2 = new csModelDataMaterial ();
  mat2->SetMaterialWrapper (m2);
  iModelDataMaterial *mat3 = new csModelDataMaterial ();
  mat3->SetMaterialWrapper (m3);

  iModelDataObject *Object = new csModelDataObject ();
  Model->QueryObject ()->ObjAdd (Object->QueryObject ());
  iModelDataVertices *Vertices = CreateDefaultModelVertexFrame ();
  Object->SetDefaultVertices (Vertices);
  iModelDataAction *Action = new csModelDataAction ();
  Action->QueryObject ()->SetName ("action");
  Object->QueryObject ()->ObjAdd (Action->QueryObject ());

  Action->AddFrame (3, Vertices->QueryObject ());
  Vertices->AddVertex (csVector3 (-3, -3, -3));
  Vertices->AddVertex (csVector3 (-3, -3, +3));
  Vertices->AddVertex (csVector3 (+3, -3, +3));
  Vertices->AddVertex (csVector3 (+3, -3, -3));
  Vertices->AddVertex (csVector3 (-3, +3, -3));
  Vertices->AddVertex (csVector3 (-3, +3, +3));
  Vertices->AddVertex (csVector3 (+3, +3, +3));
  Vertices->AddVertex (csVector3 (+3, +3, -3));

  Vertices = CreateDefaultModelVertexFrame ();
  Action->AddFrame (1, Vertices->QueryObject ());
  Vertices->AddVertex (csVector3 (-3, -7, -3));
  Vertices->AddVertex (csVector3 (-3, -7, +3));
  Vertices->AddVertex (csVector3 (+3, -7, +3));
  Vertices->AddVertex (csVector3 (+3, -7, -3));
  Vertices->AddVertex (csVector3 (-3, +7, -3));
  Vertices->AddVertex (csVector3 (-3, +7, +3));
  Vertices->AddVertex (csVector3 (+3, +7, +3));
  Vertices->AddVertex (csVector3 (+3, +7, -3));

  Vertices = CreateDefaultModelVertexFrame ();
  Action->AddFrame (2, Vertices->QueryObject ());
  Vertices->AddVertex (csVector3 (-7, -3, -3));
  Vertices->AddVertex (csVector3 (-7, -3, +3));
  Vertices->AddVertex (csVector3 (+7, -3, +3));
  Vertices->AddVertex (csVector3 (+7, -3, -3));
  Vertices->AddVertex (csVector3 (-7, +3, -3));
  Vertices->AddVertex (csVector3 (-7, +3, +3));
  Vertices->AddVertex (csVector3 (+7, +3, +3));
  Vertices->AddVertex (csVector3 (+7, +3, -3));

  iModelDataPolygon *Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (0, 3, 0, 0);
  Polygon->AddVertex (1, 3, 0, 1);
  Polygon->AddVertex (2, 3, 0, 2);
  Polygon->AddVertex (3, 3, 0, 3);
  Polygon->SetMaterial (mat);

  Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (1, 1, 0, 0);
  Polygon->AddVertex (0, 1, 0, 1);
  Polygon->AddVertex (4, 1, 0, 2);
  Polygon->AddVertex (5, 1, 0, 3);
  Polygon->SetMaterial (mat2);

  Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (2, 4, 0, 0);
  Polygon->AddVertex (1, 4, 0, 1);
  Polygon->AddVertex (5, 4, 0, 2);
  Polygon->AddVertex (6, 4, 0, 3);
  Polygon->SetMaterial (mat3);

  Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (3, 0, 0, 0);
  Polygon->AddVertex (2, 0, 0, 1);
  Polygon->AddVertex (6, 0, 0, 2);
  Polygon->AddVertex (7, 0, 0, 3);
  Polygon->SetMaterial (mat);

  Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (0, 5, 0, 0);
  Polygon->AddVertex (3, 5, 0, 1);
  Polygon->AddVertex (7, 5, 0, 2);
  Polygon->AddVertex (4, 5, 0, 3);
  Polygon->SetMaterial (mat2);

  Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (7, 2, 0, 0);
  Polygon->AddVertex (6, 2, 0, 1);
  Polygon->AddVertex (5, 2, 0, 2);
  Polygon->AddVertex (4, 2, 0, 3);
  Polygon->SetMaterial (mat3);

  return Model;
}

iMaterialWrapper *Simple::LoadTexture (const char *name, const char *fn)
{
  if (!loader->LoadTexture (name, fn))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error loading texture '%s' !", fn);
    Cleanup ();
    exit (1);
  }
  return engine->GetMaterialList ()->FindByName (name);
}

iModelData *Simple::ImportModel (const char *fn)
{
  iDataBuffer *filebuf = vfs->ReadFile (fn);
  if (!filebuf)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening model file '%s' !", fn);
    Cleanup ();
    exit (1);
  }

  iModelData *mdl = converter->Load (filebuf->GetUint8 (), filebuf->GetSize ());
  filebuf->DecRef ();
  if (!mdl)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Invalid model file: '%s' !", fn);
    Cleanup ();
    exit (1);
  }

  return mdl;
}

//-----------------------------------------------------------------------------

Simple::Simple ()
{
  view = NULL;
  engine = NULL;
  loader = NULL;
  g3d = NULL;
  kbd = NULL;
}

Simple::~Simple ()
{
  if (view) view->DecRef ();
  if (engine) engine->DecRef ();
  if (loader) loader->DecRef();
  if (g3d) g3d->DecRef ();
  if (crossbuilder) crossbuilder->DecRef ();
  if (converter) converter->DecRef ();
  if (vfs) vfs->DecRef ();
  if (kbd) kbd->DecRef ();
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  iObjectRegistry* object_reg = System->object_reg;
  delete System; System = NULL;
  csInitializer::DestroyApplication (object_reg);
}

static bool SimpleEventHandler (iEvent& ev)
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

bool Simple::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment ();
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize app!");
    return false;
  }

  csInitializer::SetupCommandLineParser (object_reg, argc, argv);
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_SOFTWARE3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_PLUGIN("crystalspace.mesh.crossbuilder:CrossBuilder",
		iCrossBuilder),
	CS_REQUEST_PLUGIN("crystalspace.modelconverter.multiplexer:Converter",
		iModelConverter),
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::Initialize (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, SimpleEventHandler))
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

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    abort ();
  }
  engine->IncRef ();

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!loader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!");
    abort ();
  }
  loader->IncRef ();

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!g3d)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    abort ();
  }
  g3d->IncRef ();

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver plugin!");
    abort ();
  }
  kbd->IncRef();

  crossbuilder = CS_QUERY_REGISTRY (object_reg, iCrossBuilder);
  if (!crossbuilder)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iCrossBuilder plugin!");
    abort ();
  }
  crossbuilder->IncRef ();

  converter = CS_QUERY_REGISTRY (object_reg, iModelConverter);
  if (!converter)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iModelConverter plugin!");
    abort ();
  }
  converter->IncRef ();

  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!vfs)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iVFS plugin!");
    abort ();
  }
  vfs->IncRef ();

  iImageIO *imageio = CS_QUERY_REGISTRY (object_reg, iImageIO);
  if (!imageio)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iModelConverter plugin!\n");
    abort ();
  }
  imageio->IncRef ();

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  // Setup the texture manager
  iTextureManager* txtmgr = g3d->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // Initialize the texture manager
  txtmgr->ResetPalette ();

  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "Simple Crystal Space Application version 0.1.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");

  iMaterialWrapper* tm = LoadTexture ("material1", "/lib/std/stone4.gif");
  LoadTexture ("material2", "/lib/std/andrew_wood.gif");
  LoadTexture ("material3", "/lib/std/andrew_marble4.gif");

  room = engine->CreateSector ("room");

  // -------------------------------------------------------------------------

  const char *Filename = cmdline->GetName (0);
  iModelData *Model = Filename ? ImportModel (Filename) : CreateDefaultModel ();

  csModelDataTools::MergeObjects (Model, true);

//  Model->LoadImages (vfs, imageio, g3d->GetTextureManager ()->GetTextureFormat ());
  imageio->DecRef ();
//  Model->RegisterTextures (engine->GetTextureList ());
//  Model->RegisterMaterials (engine->GetMaterialList ());

  iMeshObjectType *ThingType = engine->GetThingType ();
  iMeshObjectFactory *ThingFactory = ThingType->NewFactory ();

  iThingState *fState =
	SCF_QUERY_INTERFACE (ThingFactory, iThingState);
  iModelDataObject *mdo = CS_GET_CHILD_OBJECT (Model->QueryObject (), iModelDataObject);
  crossbuilder->BuildThing (mdo, fState, tm);
  csModelDataTools::SplitObjectsByMaterial (Model);
  iMeshFactoryWrapper *sfWrapper = crossbuilder->BuildSpriteFactoryHierarchy (Model, engine, tm);
  fState->DecRef ();
  mdo->DecRef ();
  Model->DecRef ();

  iMeshObject *ThingObject = ThingFactory->NewInstance ();
  iMeshWrapper *ThingWrapper = engine->CreateMeshWrapper (ThingObject, "thing");
  iMeshWrapper *SpriteWrapper = engine->CreateMeshWrapper (sfWrapper, "sprite");

  // @@@ hardcoded == BAD!
  float rad = 6;

  csTransform tr;
  tr.SetOrigin (csVector3 (-rad, 0, 0));
  ThingWrapper->HardTransform (tr);
  ThingWrapper->GetMovable ()->SetSector (room);
  ThingWrapper->GetMovable ()->UpdateMove ();
  ThingWrapper->GetFlags().Set (CS_ENTITY_CONVEX);
  ThingWrapper->SetZBufMode (CS_ZBUF_USE);
  ThingWrapper->SetRenderPriority (engine->GetWallRenderPriority ());

  SpriteWrapper->GetMovable ()->SetPosition (csVector3 (rad, 0, 0));
  SpriteWrapper->GetMovable ()->SetSector (room);
  SpriteWrapper->GetMovable ()->UpdateMove ();
  SpriteWrapper->GetFlags().Set (CS_ENTITY_CONVEX);
  SpriteWrapper->SetZBufMode (CS_ZBUF_USE);
  SpriteWrapper->SetRenderPriority (engine->GetWallRenderPriority ());

  InitializeSprite (SpriteWrapper);

  // -------------------------------------------------------------------------

  engine->SetAmbientLight (csColor (0.5, 0.5, 0.5));

  engine->Prepare ();
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Created.");

  view = new csView (engine, g3d);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, -3*rad));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  txtmgr->SetPalette ();
  return true;
}

bool Simple::HandleEvent (iEvent& Event)
{
  if (Event.Type == csevKeyDown && Event.Key.Code == CSKEY_ESC)
  {
    iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q) q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }

  return false;
}

void Simple::SetupFrame ()
{
  // First get elapsed time from the system driver.
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();
  
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * 2;

  iCamera* c = view->GetCamera();
  if (kbd->GetKeyState (CSKEY_RIGHT))
    c->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    c->GetTransform ().RotateThis (VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    c->GetTransform ().RotateThis (VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    c->GetTransform ().RotateThis (VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    c->Move (VEC_FORWARD * 4 * speed, false);
  if (kbd->GetKeyState (CSKEY_DOWN))
    c->Move (VEC_BACKWARD * 4 * speed, false);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (
      engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS | CSDRAW_CLEARSCREEN |
        CSDRAW_CLEARZBUFFER))
      return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
}

void Simple::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (NULL);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class.
  System = new Simple ();

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
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
