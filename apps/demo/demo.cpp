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
#include "demo.h"
#include "demoseq.h"
#include "csutil/cscolor.h"
#include "csgeom/path.h"
#include "csfx/csfxscr.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/conout.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/polygon.h"
#include "iengine/thing.h"
#include "iengine/light.h"
#include "iengine/view.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/movable.h"
#include "iengine/halo.h"
#include "imesh/sprite3d.h"
#include "imesh/ball.h"
#include "imesh/surf.h"
#include "imesh/object.h"
#include "imap/reader.h"
#include "igraphic/loader.h"

//-----------------------------------------------------------------------------

Demo::Demo ()
{
  engine = NULL;
  seqmgr = NULL;
}

Demo::~Demo ()
{
  if (engine)
    engine->DecRef ();
  delete seqmgr;
}

void cleanup ()
{
  System->console_out ("Cleaning up...\n");
  delete System;
}

void Demo::LoadFactory (const char* factname, const char* filename,
	const char* classId, iLoaderPlugIn* plug)
{
  iMeshFactoryWrapper* factwrap = engine->CreateMeshFactory (classId,
  	factname);
  iDataBuffer* databuf = System->VFS->ReadFile (filename);
  if (!databuf || !databuf->GetSize ())
  {
    if (databuf) databuf->DecRef ();
    Printf (MSG_FATAL_ERROR, "Could not open file '%s' on VFS!\n", filename);
    exit (0);
  }
  char* buf = **databuf;
  iBase* mof = plug->Parse (buf, engine, factwrap);
  databuf->DecRef ();
  if (!mof)
  {
    Printf (MSG_FATAL_ERROR,
    	"There was an error loading sprite factory from file '%s'!\n",
	filename);
    exit (0);
  }
  factwrap->SetMeshObjectFactory ((iMeshObjectFactory*)mof);
}

void Demo::SetupFactories ()
{
  iMeshFactoryWrapper* fact;
  fact = engine->CreateMeshFactory ("crystalspace.mesh.object.ball",
  	"ball_factory");
  if (!fact)
  {
    Printf (MSG_FATAL_ERROR, "Could not open ball plugin!\n");
    exit (0);
  }

  fact = engine->CreateMeshFactory ("crystalspace.mesh.object.surface",
  	"surf_factory");
  if (!fact)
  {
    Printf (MSG_FATAL_ERROR, "Could not open surface plugin!\n");
    exit (0);
  }

  iLoaderPlugIn* plug;
  plug = LOAD_PLUGIN (this, "crystalspace.mesh.loader.factory.sprite.3d",
  	"MeshLdr", iLoaderPlugIn);
  if (!plug)
  {
    Printf (MSG_FATAL_ERROR, "Could not open sprite 3d factory loader!\n");
    exit (0);
  }
  //LoadFactory ("stars", "/lib/demo/skymesh", "crystalspace.mesh.object.sprite.3d",
	//plug);
  plug->DecRef ();
}

void Demo::LoadMaterial (const char* matname, const char* filename)
{
  iTextureWrapper* txt = engine->CreateTexture (matname, filename,
  	NULL, CS_TEXTURE_3D);
  engine->CreateMaterial (matname, txt);
}

void Demo::SetupMaterials ()
{
  LoadMaterial ("flare_center", "/lib/std/snow.jpg");
  LoadMaterial ("flare_spark", "/lib/std/spark.png");
  LoadMaterial ("stone", "/lib/std/stone4.gif");
  LoadMaterial ("sun", "/lib/std/white.gif");
  LoadMaterial ("jupiter", "/lib/demo/jup0vtt2.jpg");
  LoadMaterial ("saturn", "/lib/demo/Saturn.jpg");
  LoadMaterial ("earth", "/lib/demo/Earth.jpg");
  LoadMaterial ("earthclouds", "/lib/demo/earthclouds.jpg");
  LoadMaterial ("nebula_b", "/lib/demo/nebula_b.png");
  LoadMaterial ("nebula_d", "/lib/demo/nebula_d.png");
  LoadMaterial ("nebula_f", "/lib/demo/nebula_f.png");
  LoadMaterial ("nebula_l", "/lib/demo/nebula_l.png");
  LoadMaterial ("nebula_r", "/lib/demo/nebula_r.png");
  LoadMaterial ("nebula_u", "/lib/demo/nebula_u.png");
  LoadMaterial ("stars", "/lib/demo/stars.png");
}

static void SetTexSpace (iPolygon3D* poly, 
  int size, const csVector3& orig, const csVector3& upt, float ulen, 
  const csVector3& vpt, float vlen)
{
  csVector3 texorig = orig;
  csVector3 texu = upt;
  float texulen = ulen;
  csVector3 texv = vpt;
  float texvlen = vlen;
  /// copied, now adjust
  csVector3 uvector = upt - orig;
  csVector3 vvector = vpt - orig;
  /// to have 1 pixel going over the edges.
  texorig -= uvector / float(size);
  texorig -= vvector / float(size);
  texu += uvector / float(size);
  texv += vvector / float(size);
  texulen += ulen * 2. / float(size);
  texvlen += vlen * 2. / float(size);
  poly->SetTextureSpace (texorig, texu, texulen, texv, texvlen);
}

void Demo::SetupSector ()
{
  room = engine->CreateSector ("room");
  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  walls->GetFlags ().Set (CS_ENTITY_CAMERA);
  walls->SetRenderPriority (engine->GetRenderPriority ("starLevel1"));
  walls->SetZBufMode (CS_ZBUF_NONE);
  iThingState* walls_state = QUERY_INTERFACE (walls->GetMeshObject (),
      iThingState);

  float size = 500.0; /// Size of the skybox -- around 0,0,0 for now.
  iPolygon3D* p;
  p = walls_state->CreatePolygon ("d");
  p->SetMaterial (engine->FindMaterial ("nebula_d"));
  p->CreateVertex (csVector3 (-size, -size, size));
  p->CreateVertex (csVector3 (size, -size, size));
  p->CreateVertex (csVector3 (size, -size, -size));
  p->CreateVertex (csVector3 (-size, -size, -size));
  SetTexSpace (p, 256, p->GetVertex (2), p->GetVertex (3),
  	2.*size, p->GetVertex (1), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ("u");
  p->SetMaterial (engine->FindMaterial ("nebula_u"));
  p->CreateVertex (csVector3 (-size, size, -size));
  p->CreateVertex (csVector3 (size, size, -size));
  p->CreateVertex (csVector3 (size, size, size));
  p->CreateVertex (csVector3 (-size, size, size));
  SetTexSpace (p, 256, p->GetVertex (0), p->GetVertex (1),
  	2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ("f");
  p->SetMaterial (engine->FindMaterial ("nebula_f"));
  p->CreateVertex (csVector3 (-size, size, size));
  p->CreateVertex (csVector3 (size, size, size));
  p->CreateVertex (csVector3 (size, -size, size));
  p->CreateVertex (csVector3 (-size, -size, size));
  SetTexSpace (p, 256, p->GetVertex (0), p->GetVertex (1),
	2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ("r");
  p->SetMaterial (engine->FindMaterial ("nebula_r"));
  p->CreateVertex (csVector3 (size, size, size));
  p->CreateVertex (csVector3 (size, size, -size));
  p->CreateVertex (csVector3 (size, -size, -size));
  p->CreateVertex (csVector3 (size, -size, size));
  SetTexSpace (p, 256, p->GetVertex (0), p->GetVertex (1),
  	2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ("l");
  p->SetMaterial (engine->FindMaterial ("nebula_l"));
  p->CreateVertex (csVector3 (-size, size, -size));
  p->CreateVertex (csVector3 (-size, size, size));
  p->CreateVertex (csVector3 (-size, -size, size));
  p->CreateVertex (csVector3 (-size, -size, -size));
  SetTexSpace (p, 256, p->GetVertex (0), p->GetVertex (1),
  	2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ("b");
  p->SetMaterial (engine->FindMaterial ("nebula_b"));
  p->CreateVertex (csVector3 (size, size, -size));
  p->CreateVertex (csVector3 (-size, size, -size));
  p->CreateVertex (csVector3 (-size, -size, -size));
  p->CreateVertex (csVector3 (size, -size, -size));
  SetTexSpace (p, 256, p->GetVertex (0), p->GetVertex (1),
	2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  walls_state->DecRef ();

  // Create the stars.
  iMeshWrapper* stars;
  iSurfaceState* ss;

  // Create the stars.
  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsF",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();

  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsB",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();
  stars->HardTransform (csTransform (csYRotMatrix3 (M_PI), csVector3 (0)));

  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsR",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();
  stars->HardTransform (csTransform (csYRotMatrix3 (M_PI/2), csVector3 (0)));

  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsL",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();
  stars->HardTransform (csTransform (csYRotMatrix3 (-M_PI/2), csVector3 (0)));

  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsU",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();
  stars->HardTransform (csTransform (csXRotMatrix3 (M_PI/2), csVector3 (0)));

  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsD",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();
  stars->HardTransform (csTransform (csXRotMatrix3 (-M_PI/2), csVector3 (0)));

  iStatLight* light;
  light = engine->CreateLight (csVector3 (-500, 300, 900), 1000000,
  	csColor (1, 1, 1), false);
  iLight* il = QUERY_INTERFACE (light, iLight);
  iFlareHalo* flare = il->CreateFlareHalo ();
  iMaterialWrapper* ifmc = engine->FindMaterial ("flare_center");
  iMaterialWrapper* ifms = engine->FindMaterial ("flare_spark");
  flare->AddComponent (0.0, 0.5, 0.5, CS_FX_ADD, ifmc); // pos, w, h, mixmode
  flare->AddComponent (.01, 1.0, 1.0, CS_FX_ADD, ifms);
  flare->AddComponent (0.3, 0.1, 0.1, CS_FX_ADD, ifms);
  flare->AddComponent (0.6, 0.4, 0.4, CS_FX_ADD, ifms);
  flare->AddComponent (0.8, .05, .05, CS_FX_ADD, ifms);
  flare->AddComponent (1.0, 0.7, 0.7, CS_FX_ADD, ifmc);
  flare->AddComponent (1.3, 0.1, 0.1, CS_FX_ADD, ifms);
  flare->AddComponent (1.5, 0.3, 0.3, CS_FX_ADD, ifms);
  flare->AddComponent (1.8, 0.1, 0.1, CS_FX_ADD, ifms);
  flare->AddComponent (2.0, 0.5, 0.5, CS_FX_ADD, ifmc);
  flare->AddComponent (2.1, .15, .15, CS_FX_ADD, ifms);
  il->DecRef ();
  room->AddLight (light);
}

void Demo::SetupObjects ()
{
  iBallState* bs;

  // Create the sun.
  iMeshWrapper* sun = engine->CreateMeshObject (
  	engine->FindMeshFactory ("ball_factory"), "Sun",
  	NULL, csVector3 (0));
  sun->SetRenderPriority (engine->GetRenderPriority ("object"));
  sun->SetZBufMode (CS_ZBUF_USE);
  bs = QUERY_INTERFACE (sun->GetMeshObject (), iBallState);
  bs->SetRadius (500, 500, 500);
  bs->SetMaterialWrapper (engine->FindMaterial ("sun"));
  bs->SetCylindricalMapping (true);
  bs->SetRimVertices (6);
  bs->SetLighting (false);
  bs->SetColor (csColor (1., 1., 1.));
  bs->DecRef ();

  // Create saturn.
  iMeshWrapper* sat = engine->CreateMeshObject (
  	engine->FindMeshFactory ("ball_factory"), "Saturn",
  	NULL, csVector3 (0));
  sat->SetRenderPriority (engine->GetRenderPriority ("object"));
  sat->SetZBufMode (CS_ZBUF_USE);
  bs = QUERY_INTERFACE (sat->GetMeshObject (), iBallState);
  bs->SetRadius (100, 100, 100);
  bs->SetMaterialWrapper (engine->FindMaterial ("saturn"));
  bs->SetCylindricalMapping (true);
  bs->SetRimVertices (16);
  sat->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  bs->DecRef ();

  // Create jupiter.
  iMeshWrapper* jup = engine->CreateMeshObject (
  	engine->FindMeshFactory ("ball_factory"), "Jupiter",
  	NULL, csVector3 (0));
  jup->SetRenderPriority (engine->GetRenderPriority ("object"));
  jup->SetZBufMode (CS_ZBUF_USE);
  bs = QUERY_INTERFACE (jup->GetMeshObject (), iBallState);
  bs->SetRadius (100, 100, 100);
  bs->SetMaterialWrapper (engine->FindMaterial ("jupiter"));
  bs->SetCylindricalMapping (true);
  bs->SetRimVertices (16);
  jup->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  bs->DecRef ();

  // Create the earth.
  iMeshWrapper* earth = engine->CreateMeshObject (
  	engine->FindMeshFactory ("ball_factory"), "Earth",
  	NULL, csVector3 (0));
  earth->SetRenderPriority (engine->GetRenderPriority ("object"));
  earth->SetZBufMode (CS_ZBUF_USE);
  bs = QUERY_INTERFACE (earth->GetMeshObject (), iBallState);
  bs->SetRadius (50, 50, 50);
  bs->SetMaterialWrapper (engine->FindMaterial ("earth"));
  bs->SetCylindricalMapping (true);
  bs->SetRimVertices (16);
  earth->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  bs->DecRef ();

  // Create the clouds for earth.
  iMeshWrapper* clouds = engine->CreateMeshObject (
  	engine->FindMeshFactory ("ball_factory"), "Clouds",
  	NULL, csVector3 (0));
  clouds->SetRenderPriority (engine->GetRenderPriority ("alpha"));
  clouds->SetZBufMode (CS_ZBUF_TEST);
  bs = QUERY_INTERFACE (clouds->GetMeshObject (), iBallState);
  bs->SetRadius (55, 55, 55);
  bs->SetMaterialWrapper (engine->FindMaterial ("earthclouds"));
  bs->SetCylindricalMapping (true);
  bs->SetRimVertices (16);
  bs->SetMixMode (CS_FX_ADD);
  clouds->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  bs->DecRef ();
}

bool Demo::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Load the engine plugin.
  engine =
    LOAD_PLUGIN (this, "crystalspace.engine.core", CS_FUNCID_ENGINE, iEngine);
  if (!engine)
  {
    Printf (MSG_FATAL_ERROR, "No engine!\n");
    abort ();
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("The Crystal Space Demo."))
  {
    Printf (MSG_FATAL_ERROR, "Error opening system!\n");
    cleanup ();
    exit (1);
  }

  // Setup the texture manager
  iTextureManager* txtmgr = G3D->GetTextureManager ();
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

  // Initialize the console
  if (System->Console != NULL)
    // Don't let messages before this one appear
    System->Console->Clear ();

  // Some commercials...
  Printf (MSG_INITIALIZATION,
    "The Crystal Space Demo.\n");

  // Mount our demo file.
  //if (!VFS->Mount ("/lib/demo", "data/csdemo.zip"))
  //{
    //Printf (MSG_FATAL_ERROR, "Can't load csdemo data!...\n");
    //exit (0);
  //}
  VFS->Mount ("/lib/demo", ".$/csdemo$/");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->EnableLightingCache (false);

  // Create our world.
  Printf (MSG_INITIALIZATION, "Creating world!...\n");

  engine->RegisterRenderPriority ("starLevel1", 1);
  engine->RegisterRenderPriority ("starLevel2", 2);
  engine->RegisterRenderPriority ("object", 3);
  engine->RegisterRenderPriority ("alpha", 4);
  SetupMaterials ();
  SetupFactories ();
  SetupSector ();
  seqmgr = new DemoSequenceManager (this);
  SetupObjects ();
  seqmgr->Setup ("/lib/demo/sequences");

  engine->Prepare ();

  Printf (MSG_INITIALIZATION, "--------------------------------------\n");

  view = engine->CreateView (G3D);
  view->SetSector (room);
  view->GetCamera ()->SetPosition (csVector3 (0, 0, -900));
  view->GetCamera ()->GetTransform ().RotateThis (csVector3 (0, 1, 0), .8);
  view->SetRectangle (0, 0, FrameWidth, FrameHeight);

  txtmgr->SetPalette ();

  return true;
}

static csCubicSpline* spline_debug = NULL;
static float spline_debug_dim[11] = { 200, 200, 200, 200, 250, 200, 200, 200, 200, 200, 200 };

static csNamedPath* spline_map = NULL;
static csVector2 spline_topleft;
static csVector2 spline_botright;
static int spline_map_selpoint = 0;
static bool spline_map_edit_forward = false;

void Demo::NextFrame ()
{
  superclass::NextFrame ();
  cs_time elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);
 
  // Now rotate the camera according to keyboard state
  csReversibleTransform& camtrans = view->GetCamera ()->GetTransform ();
  if (!spline_map)
  {
    float speed = (elapsed_time / 1000.) * (0.03 * 20);
    if (GetKeyState (CSKEY_RIGHT))
      camtrans.RotateThis (VEC_ROT_RIGHT, speed);
    if (GetKeyState (CSKEY_LEFT))
      camtrans.RotateThis (VEC_ROT_LEFT, speed);
    if (GetKeyState (CSKEY_PGUP))
      camtrans.RotateThis (VEC_TILT_UP, speed);
    if (GetKeyState (CSKEY_PGDN))
      camtrans.RotateThis (VEC_TILT_DOWN, speed);
    if (GetKeyState (CSKEY_UP))
      view->GetCamera ()->Move (VEC_FORWARD * 400.0f * speed);
    if (GetKeyState (CSKEY_DOWN))
      view->GetCamera ()->Move (VEC_BACKWARD * 400.0f * speed);

    seqmgr->ControlPaths (view->GetCamera (), current_time);
  }
  else if (spline_map_edit_forward)
  {
    float r = spline_map->GetTimeValue (spline_map_selpoint);
    spline_map->Calculate (r);
    csVector3 pos, up, forward;
    spline_map->GetInterpolatedPosition (pos);
    spline_map->GetInterpolatedUp (up);
    spline_map->GetInterpolatedForward (forward);
    view->GetCamera ()->SetPosition (pos);
    view->GetCamera ()->GetTransform ().LookAt (forward.Unit (), up.Unit ());
  }

  // Tell 3D driver we're going to display 3D things.
  if (!G3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS
  	| CSDRAW_CLEARZBUFFER))
    return;

  if (!spline_debug && (!spline_map || spline_map_edit_forward))
  {
    view->Draw ();
    seqmgr->Draw3DEffects (G3D, current_time);
    if (spline_map_edit_forward) csfxFadeToColor (G3D, .3, csColor (0, 0, 1));
  }

  // Start drawing 2D graphics.
  if (!System->G3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  if (spline_debug)
  {
    G2D->Clear (0);
    int x;
    for (x = 0 ; x < 640 ; x++)
    {
      spline_debug->Calculate (float (x)/640.);
      int y = int (spline_debug->GetInterpolatedDimension (0));
      G2D->DrawPixel (x, y, 0xffffff);
    }
  }
  else if (spline_map && !spline_map_edit_forward)
  {
    G2D->Clear (0);
    G2D->DrawLine (0, 0, 600, 0, 0xff4444);
    G2D->DrawLine (0, 600, 600, 600, 0xff4444);
    G2D->DrawLine (0, 0, 0, 600, 0xff4444);
    G2D->DrawLine (600, 0, 600, 600, 0xff4444);
    float r;
    csVector3 p;
    for (r = 0 ; r <= 1 ; r += .001)
    {
      spline_map->Calculate (r);
      spline_map->GetInterpolatedPosition (p);
      int x = (p.x-spline_topleft.x)*600 / (spline_botright.x-spline_topleft.x);
      int y = (p.z-spline_topleft.y)*600 / (spline_botright.y-spline_topleft.y);
      if (x > 0 && x < 600 && y > 0 && y < 600)
        G2D->DrawPixel (x, y, 0xffffff);
    }
    float* px, * py, * pz;
    px = spline_map->GetDimensionValues (0);
    py = spline_map->GetDimensionValues (1);
    pz = spline_map->GetDimensionValues (2);
    int i;
    for (i = 0 ; i < spline_map->GetNumPoints () ; i++)
    {
      int col = 0xff0000;
      if (spline_map_selpoint == i) col = 0x00ff00;
      int x = (px[i]-spline_topleft.x)*600/(spline_botright.x-spline_topleft.x);
      int y = (pz[i]-spline_topleft.y)*600/(spline_botright.y-spline_topleft.y);
      if (x > 0 && x < 600 && y > 0 && y < 600)
      {
        G2D->DrawPixel (x, y, col);
        G2D->DrawPixel (x-1, y, col);
        G2D->DrawPixel (x+1, y, col);
        G2D->DrawPixel (x, y-1, col);
        G2D->DrawPixel (x, y+1, col);
      }
    }
  }
  else
  {
    seqmgr->Draw2DEffects (G2D, current_time);
  }

  // Drawing code ends here.
  G3D->FinishDraw ();
  // Print the final output.
  G3D->Print (NULL);
}

bool Demo::HandleEvent (iEvent &Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if (Event.Type == csevKeyDown)
  {
    cs_time elapsed_time, current_time;
    GetElapsedTime (elapsed_time, current_time);
    bool shift = (Event.Key.Modifiers & CSMASK_SHIFT) != 0;
    bool alt = (Event.Key.Modifiers & CSMASK_ALT) != 0;
    bool ctrl = (Event.Key.Modifiers & CSMASK_CTRL) != 0;

    if (spline_map && spline_map_edit_forward)
    {
      //==============================
      // Handle keys in path_edit_forward mode.
      //==============================
      float dx = spline_botright.x - spline_topleft.x;
      float speed;
      if (shift) speed = dx/20.;
      else if (ctrl) speed = dx/600.;
      else speed = dx/100.;
      if (Event.Key.Code == CSKEY_UP)
      {
        csVector3 v;
	spline_map->GetPositionVector (spline_map_selpoint, v);
        v.y += speed;
	spline_map->SetPositionVector (spline_map_selpoint, v);
        return true;
      }
      if (Event.Key.Code == CSKEY_DOWN)
      {
        csVector3 v;
	spline_map->GetPositionVector (spline_map_selpoint, v);
        v.y -= speed;
	spline_map->SetPositionVector (spline_map_selpoint, v);
        return true;
      }
      if (Event.Key.Code == CSKEY_LEFT)
      {
        csVector3 up, forward;
	spline_map->GetUpVector (spline_map_selpoint, up);
	spline_map->GetForwardVector (spline_map_selpoint, forward);
	csReversibleTransform trans = view->GetCamera ()->GetTransform ();
        trans.LookAt (forward.Unit (), up.Unit ());
	trans.RotateThis (csVector3 (0, 0, 1), -.1);
	spline_map->SetUpVector (spline_map_selpoint,
		trans.This2Other (csVector3 (0, 1, 0)) - trans.GetOrigin ());
        return true;
      }
      if (Event.Key.Code == CSKEY_RIGHT)
      {
        csVector3 up, forward;
	spline_map->GetUpVector (spline_map_selpoint, up);
	spline_map->GetForwardVector (spline_map_selpoint, forward);
	csReversibleTransform trans = view->GetCamera ()->GetTransform ();
        trans.LookAt (forward.Unit (), up.Unit ());
	trans.RotateThis (csVector3 (0, 0, 1), .1);
	spline_map->SetUpVector (spline_map_selpoint,
		trans.This2Other (csVector3 (0, 1, 0)) - trans.GetOrigin ());
        return true;
      }
      switch (Event.Key.Char)
      {
        case 'c':
          spline_map_edit_forward = false;
	  return true;
      }
    }
    else if (spline_map)
    {
      //==============================
      // Handle keys in path editing mode.
      //==============================
      float dx = spline_botright.x - spline_topleft.x;
      float speed;
      if (shift) speed = dx/20.;
      else if (ctrl) speed = dx/600.;
      else speed = dx/100.;
      if (Event.Key.Code == CSKEY_UP)
      {
        csVector3 v;
	spline_map->GetPositionVector (spline_map_selpoint, v);
        v.z += speed;
	spline_map->SetPositionVector (spline_map_selpoint, v);
        return true;
      }
      if (Event.Key.Code == CSKEY_DOWN)
      {
        csVector3 v;
	spline_map->GetPositionVector (spline_map_selpoint, v);
        v.z -= speed;
	spline_map->SetPositionVector (spline_map_selpoint, v);
        return true;
      }
      if (Event.Key.Code == CSKEY_LEFT)
      {
        csVector3 v;
	spline_map->GetPositionVector (spline_map_selpoint, v);
        v.x -= speed;
	spline_map->SetPositionVector (spline_map_selpoint, v);
        return true;
      }
      if (Event.Key.Code == CSKEY_RIGHT)
      {
        csVector3 v;
	spline_map->GetPositionVector (spline_map_selpoint, v);
        v.x += speed;
	spline_map->SetPositionVector (spline_map_selpoint, v);
        return true;
      }
      switch (Event.Key.Char)
      {
	case 'm':
          //seqmgr->TimeWarp (seqmgr->GetCameraTimePoint (
		  //spline_map->GetTimeValue (spline_map_selpoint))-current_time);
          spline_map = NULL;
          return true;
	case 's':
	  {
	    char buf[200];
	    strcpy (buf, "seq_"); strcat (buf, spline_map->GetName ());
	    strcat (buf, ".txt");
	    FILE* fp = fopen (buf, "w");
	    if (fp)
	    {
	      int i, num = spline_map->GetNumPoints ();
	      fprintf (fp, "  PATH '%s' (\n", spline_map->GetName ());
	      fprintf (fp, "    NUM (%d)\n", num);
	      float* t = spline_map->GetTimeValues ();
	      fprintf (fp, "    TIMES (%g", t[0]);
	      for (i = 1 ; i < num ; i++)
	        fprintf (fp, ",%g", t[i]);
	      fprintf (fp, ")\n");
	      fprintf (fp, "    POS (\n");
	      for (i = 0 ; i < num ; i++)
	      {
	        csVector3 v;
		spline_map->GetPositionVector (i, v);
	        fprintf (fp, "      V (%g,%g,%g)\n", v.x, v.y, v.z);
	      }
	      fprintf (fp, "    )\n");
	      fprintf (fp, "    FORWARD (\n");
	      for (i = 0 ; i < num ; i++)
	      {
	        csVector3 v;
		spline_map->GetForwardVector (i, v);
	        fprintf (fp, "      V (%g,%g,%g)\n", v.x, v.y, v.z);
	      }
	      fprintf (fp, "    )\n");
	      fprintf (fp, "    UP (\n");
	      for (i = 0 ; i < num ; i++)
	      {
	        csVector3 v;
		spline_map->GetUpVector (i, v);
	        fprintf (fp, "      V (%g,%g,%g)\n", v.x, v.y, v.z);
	      }
	      fprintf (fp, "    )\n");
	      fprintf (fp, "  )\n");
	      fclose (fp);
	    }
	  }
	  break;
        case 'i':
	  spline_map->InsertPoint (spline_map_selpoint);
	  spline_map_selpoint++;
	  if (spline_map_selpoint == spline_map->GetNumPoints ()-1)
	  {
	    csVector3 v;
	    spline_map->GetPositionVector (spline_map_selpoint-1, v);
	    spline_map->SetPositionVector (spline_map_selpoint, v);
	    spline_map->GetUpVector (spline_map_selpoint-1, v);
	    spline_map->SetUpVector (spline_map_selpoint, v);
	    spline_map->GetForwardVector (spline_map_selpoint-1, v);
	    spline_map->SetForwardVector (spline_map_selpoint, v);
	    spline_map->SetTimeValue (spline_map_selpoint,
	    	spline_map->GetTimeValue (spline_map_selpoint-1));
	  }
	  else
	  {
	    csVector3 v1, v2, v;
	    spline_map->GetPositionVector (spline_map_selpoint-1, v1);
	    spline_map->GetPositionVector (spline_map_selpoint+1, v2);
	    spline_map->SetPositionVector (spline_map_selpoint, (v1+v2)/2.);
	    spline_map->GetUpVector (spline_map_selpoint-1, v1);
	    spline_map->GetUpVector (spline_map_selpoint+1, v2);
	    spline_map->SetUpVector (spline_map_selpoint, (v1+v2)/2.);
	    spline_map->GetForwardVector (spline_map_selpoint-1, v1);
	    spline_map->GetForwardVector (spline_map_selpoint+1, v2);
	    spline_map->SetForwardVector (spline_map_selpoint, (v1+v2)/2.);
	    spline_map->SetTimeValue (spline_map_selpoint,
	    	(spline_map->GetTimeValue (spline_map_selpoint-1)+
		 spline_map->GetTimeValue (spline_map_selpoint+1))/2.);
	  }
          break;
        case 'd':
	  spline_map->RemovePoint (spline_map_selpoint);
	  if (spline_map_selpoint >= spline_map->GetNumPoints ())
	    spline_map_selpoint--;
	  break;
	case ',':
	  if (spline_map_selpoint > 0
	  	&& spline_map_selpoint < spline_map->GetNumPoints ()-1)
	  {
	    float t = spline_map->GetTimeValue (spline_map_selpoint);
	    float t1 = spline_map->GetTimeValue (spline_map_selpoint-1);
	    float t2 = spline_map->GetTimeValue (spline_map_selpoint+1);
	    float dt = (t2-t1);
	    if (shift) dt /= 5.;
	    else if (ctrl) dt /= 500.;
	    else dt /= 50.;
	    t -= dt;
	    if (t < t1) t = t1;
	    spline_map->SetTimeValue (spline_map_selpoint, t);
	  }
	  break;
	case '.':
	  if (spline_map_selpoint > 0
	  	&& spline_map_selpoint < spline_map->GetNumPoints ()-1)
	  {
	    float t = spline_map->GetTimeValue (spline_map_selpoint);
	    float t1 = spline_map->GetTimeValue (spline_map_selpoint-1);
	    float t2 = spline_map->GetTimeValue (spline_map_selpoint+1);
	    float dt = (t2-t1);
	    if (shift) dt /= 5.;
	    else if (ctrl) dt /= 500.;
	    else dt /= 50.;
	    t += dt;
	    if (t > t2) t = t2;
	    spline_map->SetTimeValue (spline_map_selpoint, t);
	  }
	  break;
	case '/':
	  if (spline_map_selpoint > 0
	  	&& spline_map_selpoint < spline_map->GetNumPoints ()-1)
	  {
	    float t1 = spline_map->GetTimeValue (spline_map_selpoint-1);
	    float t2 = spline_map->GetTimeValue (spline_map_selpoint+1);
	    spline_map->SetTimeValue (spline_map_selpoint, (t1+t2)/2.);
	  }
	  break;
        case '>':
          spline_map_selpoint++;
	  if (spline_map_selpoint >= spline_map->GetNumPoints ())
	    spline_map_selpoint = 0;
	  break;
        case '<':
          spline_map_selpoint--;
	  if (spline_map_selpoint < 0)
	    spline_map_selpoint = spline_map->GetNumPoints ()-1;
	  break;
        case 'c':
	  spline_map_edit_forward = true;
	  break;
      }
    }
    else
    {
      //==============================
      // Handle keys in demo mode.
      //==============================
      if (Event.Key.Code == CSKEY_ESC)
      {
        Shutdown = true;
        return true;
      }
      switch (Event.Key.Char)
      {
        case 'p':
          if (!spline_debug)
          {
            if (seqmgr->IsSuspended ()) seqmgr->Resume ();
            else seqmgr->Suspend ();
          }
	  break;
        case '.':
          seqmgr->TimeWarp (100);
	  break;
        case ',':
          seqmgr->TimeWarp (-100);
	  break;
        case '>':
          seqmgr->TimeWarp (3000);
	  break;
        case '<':
          seqmgr->TimeWarp (-3000);
	  break;
        case 's':
          if (spline_debug) { delete spline_debug; spline_debug = NULL; }
          else
          {
            spline_map = NULL;
            spline_debug = new csCubicSpline (1, 11);
	    float time[11] = { 0, .1, .2, .3, .4, .5, .6, .7, .8, .9, 1 };
	    spline_debug->SetTimeValues (time);
	    spline_debug->SetDimensionValues (0, spline_debug_dim);
          }
          if (spline_debug) seqmgr->Suspend ();
	  break;
        case 'm':
          delete spline_debug; spline_debug = NULL;
          spline_map = seqmgr->GetCameraPath (); //@@@FindPath ("ourShip");
	  if (spline_map)
	  {
	    spline_topleft.Set (-1000, 1000);
	    spline_botright.Set (1000, -1000);
	    spline_map->Calculate (seqmgr->GetCameraIndex (current_time));
	    spline_map_selpoint = spline_map->GetCurrentIndex ();
            seqmgr->Suspend ();
	  }
	  break;
      }
    }
  }
  else if (Event.Type == csevMouseDown)
  {
    if (Event.Mouse.Button == 1)
    {
      csVector2 p (Event.Mouse.x, G2D->GetHeight ()-Event.Mouse.y);
      csVector3 v;
      view->GetCamera ()->InvPerspective (p, 1, v);
      csVector3 vw = view->GetCamera ()->GetTransform ().This2Other (v);
      if (spline_map && spline_map_edit_forward)
      {
        vw -= view->GetCamera ()->GetTransform ().GetOrigin ();
        spline_map->SetForwardVector (spline_map_selpoint, vw);
	csVector3 up;
	spline_map->GetUpVector (spline_map_selpoint, up);
	up = up % vw;
	up = - (up % vw);
	spline_map->SetUpVector (spline_map_selpoint, up);
      }
    }
  }

  return false;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class.
  System = new Demo ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.console.output.simple:Console.Output");

  // Initialize the main system. This will load all needed plug-ins
  // and initialize them.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Printf (MSG_FATAL_ERROR, "Error initializing system!\n");
    cleanup ();
    exit (1);
  }
 
  // Main loop.
  System->Loop ();

  // Cleanup.
  cleanup ();

  return 0;
}
