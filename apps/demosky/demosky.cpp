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
*/

#include "cssysdef.h"
#include "cssys/system.h"
#include "apps/demosky/demosky.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/csview.h"
#include "csengine/camera.h"
#include "csengine/light.h"
#include "csengine/polygon.h"
#include "csengine/meshobj.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "csfx/proctex.h"
#include "csfx/prsky.h"
#include "csparser/csloader.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/conout.h"
#include "ivideo/fontserv.h"
#include "imesh/sprite2d.h"
#include "imesh/object.h"
#include "iengine/mesh.h"

//------------------------------------------------- We need the 3D engine -----

REGISTER_STATIC_LIBRARY (engine)
REGISTER_STATIC_LIBRARY (lvlload)

//-----------------------------------------------------------------------------

Simple::Simple ()
{
  view = NULL;
  engine = NULL;
  sky = NULL;
  sky_f = NULL;
  sky_b = NULL;
  sky_l = NULL;
  sky_r = NULL;
  sky_u = NULL;
  sky_d = NULL;
  flock = NULL;
  LevelLoader = NULL;
}

Simple::~Simple ()
{
  delete view;
  if(font) font->DecRef();
  if (LevelLoader) LevelLoader->DecRef();

  delete sky;
  delete sky_f;
  delete sky_b;
  delete sky_l;
  delete sky_r;
  delete sky_u;
  delete sky_d;

  //delete flock;

  System->console_out ("Cleaning up...\n");
  delete System;
}

void Simple::SetTexSpace(csProcSkyTexture *skytex, iPolygon3D *poly, 
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
  skytex->SetTextureSpace(texorig, texu-texorig, texv-texorig);
}


bool Simple::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Find the pointer to engine plugin
  iEngine *Engine = QUERY_PLUGIN (this, iEngine);
  if (!Engine)
  {
    CsPrintf (MSG_FATAL_ERROR, "No iEngine plugin!\n");
    abort ();
  }
  engine = Engine->GetCsEngine ();
  Engine->DecRef ();

  LevelLoader = QUERY_PLUGIN_ID (this, CS_FUNCID_LVLLOADER, iLoaderNew);
  if (!LevelLoader)
  {
    CsPrintf (MSG_FATAL_ERROR, "No iLoaderNew plugin!\n");
    abort ();
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("Crystal Space Procedural Sky Demo"))
  {
    Printf (MSG_FATAL_ERROR, "Error opening system!\n");
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

  font = G2D->GetFontServer()->LoadFont(CSFONT_LARGE);

  // Some commercials...
  Printf (MSG_INITIALIZATION, "Crystal Space Procedural Sky Demo.\n");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->EnableLightingCache (false);

  // Create our world.
  Printf (MSG_INITIALIZATION, "Creating world!...\n");

  //csLoader::LoadTexture (engine, "stone", "/lib/std/stone4.gif");
  //csMaterialWrapper* tm = engine->GetMaterials ()->FindByName ("stone");

  sky = new csProcSky();
  sky->SetAnimated(false);
  sky_f = new csProcSkyTexture(sky);
  csMaterialWrapper* matf = sky_f->Initialize(this, engine, txtmgr, "sky_f");
  sky_b = new csProcSkyTexture(sky);
  csMaterialWrapper* matb = sky_b->Initialize(this, engine, txtmgr, "sky_b");
  sky_l = new csProcSkyTexture(sky);
  csMaterialWrapper* matl = sky_l->Initialize(this, engine, txtmgr, "sky_l");
  sky_r = new csProcSkyTexture(sky);
  csMaterialWrapper* matr = sky_r->Initialize(this, engine, txtmgr, "sky_r");
  sky_u = new csProcSkyTexture(sky);
  csMaterialWrapper* matu = sky_u->Initialize(this, engine, txtmgr, "sky_u");
  sky_d = new csProcSkyTexture(sky);
  csMaterialWrapper* matd = sky_d->Initialize(this, engine, txtmgr, "sky_d");
  iMaterialWrapper* imatf = QUERY_INTERFACE (matf, iMaterialWrapper); imatf->DecRef ();
  iMaterialWrapper* imatb = QUERY_INTERFACE (matb, iMaterialWrapper); imatb->DecRef ();
  iMaterialWrapper* imatl = QUERY_INTERFACE (matl, iMaterialWrapper); imatl->DecRef ();
  iMaterialWrapper* imatr = QUERY_INTERFACE (matr, iMaterialWrapper); imatr->DecRef ();
  iMaterialWrapper* imatu = QUERY_INTERFACE (matu, iMaterialWrapper); imatu->DecRef ();
  iMaterialWrapper* imatd = QUERY_INTERFACE (matd, iMaterialWrapper); imatd->DecRef ();

  room = engine->CreateCsSector ("room");
  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  iThingState* walls_state = QUERY_INTERFACE (walls->GetMeshObject (),
  	iThingState);
  iPolygon3D* p;
  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatd);
  float size = 500.0; /// size of the skybox -- around 0,0,0 for now.
  float simi = size; //*255./256.; /// sizeminor
  p->CreateVertex (csVector3 (-size, -simi, size));
  p->CreateVertex (csVector3 (size, -simi, size));
  p->CreateVertex (csVector3 (size, -simi, -size));
  p->CreateVertex (csVector3 (-size, -simi, -size));
  //sky_d->SetTextureSpace(p->GetVertex (0), p->GetVertex (1)-p->GetVertex (0), p->GetVertex (3)-p->GetVertex (0));
  SetTexSpace (sky_d, p, 256, p->GetVertex  (0), p->GetVertex  (1), 2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatu);
  p->CreateVertex (csVector3 (-size, simi, -size));
  p->CreateVertex (csVector3 (size, simi, -size));
  p->CreateVertex (csVector3 (size, simi, size));
  p->CreateVertex (csVector3 (-size, simi, size));
  //sky_u->SetTextureSpace(p->GetVertex (0), p->GetVertex (1)-p->GetVertex (0), p->GetVertex (3)-p->GetVertex (0));
  SetTexSpace (sky_u, p, 256, p->GetVertex  (0), p->GetVertex  (1), 2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatf);
  p->CreateVertex (csVector3 (-size, size, simi));
  p->CreateVertex (csVector3 (size, size, simi));
  p->CreateVertex (csVector3 (size, -size, simi));
  p->CreateVertex (csVector3 (-size, -size, simi));
  //sky_f->SetTextureSpace(p->GetVertex (0), p->GetVertex (1)-p->GetVertex (0), p->GetVertex (3)-p->GetVertex (0));
  SetTexSpace (sky_f, p, 256, p->GetVertex  (0), p->GetVertex  (1), 2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatr);
  p->CreateVertex (csVector3 (simi, size, size));
  p->CreateVertex (csVector3 (simi, size, -size));
  p->CreateVertex (csVector3 (simi, -size, -size));
  p->CreateVertex (csVector3 (simi, -size, size));
  //sky_r->SetTextureSpace(p->GetVertex (0), p->GetVertex (1)-p->GetVertex (0), p->GetVertex (3)-p->GetVertex (0));
  SetTexSpace (sky_r, p, 256, p->GetVertex  (0), p->GetVertex  (1), 2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatl);
  p->CreateVertex (csVector3 (-simi, size, -size));
  p->CreateVertex (csVector3 (-simi, size, size));
  p->CreateVertex (csVector3 (-simi, -size, size));
  p->CreateVertex (csVector3 (-simi, -size, -size));
  //sky_l->SetTextureSpace(p->GetVertex (0), p->GetVertex (1)-p->GetVertex (0), p->GetVertex (3)-p->GetVertex (0));
  SetTexSpace (sky_l, p, 256, p->GetVertex  (0), p->GetVertex  (1), 2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatb);
  p->CreateVertex (csVector3 (size, size, -simi));
  p->CreateVertex (csVector3 (-size, size, -simi));
  p->CreateVertex (csVector3 (-size, -size, -simi));
  p->CreateVertex (csVector3 (size, -size, -simi));
  //sky_b->SetTextureSpace(p->GetVertex (0), p->GetVertex (1)-p->GetVertex (0), p->GetVertex (3)-p->GetVertex (0));
  SetTexSpace (sky_b, p, 256, p->GetVertex  (0), p->GetVertex  (1), 2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);
  walls_state->DecRef ();

  csLoader::LoadTexture (engine, "seagull", "/lib/std/seagull.gif");
  csMaterialWrapper *sg = engine->GetMaterials ()->FindByName("seagull");
  flock = new Flock(engine, 10, QUERY_INTERFACE(sg, iMaterialWrapper), 
    QUERY_INTERFACE(room, iSector));

  engine->Prepare ();

  Printf (MSG_INITIALIZATION, "--------------------------------------\n");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (engine, G3D);
  view->SetSector (room);
  view->GetCamera ()->SetPosition (csVector3 (0, 0, 0));
  view->SetRectangle (0, 0, FrameWidth, FrameHeight);

  txtmgr->SetPalette ();

  return true;
}

void Simple::NextFrame ()
{
  SysSystemDriver::NextFrame ();
  cs_time elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);

  flock->Update(elapsed_time);

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.) * (0.03 * 20);

  if (GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->Rotate (VEC_ROT_RIGHT, speed);
  if (GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->Rotate (VEC_ROT_LEFT, speed);
  if (GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->Rotate (VEC_TILT_UP, speed);
  if (GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->Rotate (VEC_TILT_DOWN, speed);
  if (GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (VEC_FORWARD * 4.0f * speed);
  if (GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (VEC_BACKWARD * 4.0f * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!G3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  view->Draw ();

  // Start drawing 2D graphics.
  if (!G3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  const char *text = "Press 't' to toggle animation. Escape quits."
    " Arrow keys/pgup/pgdown to move.";
  int txtx = 10;
  int txty = G2D->GetHeight() - 20;
  G2D->Write(font, txtx+1, txty+1, G3D->GetTextureManager()->FindRGB(80,80,80), 
    -1, text);
  G2D->Write(font, txtx, txty, G3D->GetTextureManager()->FindRGB(255,255,255),
    -1, text);

  // Drawing code ends here.
  G3D->FinishDraw ();
  // Print the final output.
  G3D->Print (NULL);
}

bool Simple::HandleEvent (iEvent &Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if ((Event.Type == csevKeyDown) && (Event.Key.Code == 't'))
  {
    /// toggle animation
    sky->SetAnimated( !sky->GetAnimated(), System->GetTime());
    return true;
  }

  if ((Event.Type == csevKeyDown) && (Event.Key.Code == CSKEY_ESC))
  {
    Shutdown = true;
    return true;
  }

  return false;
}

//--- Flock -----------------------
Flock::Flock(csEngine *engine, int num, iMaterialWrapper *mat, iSector *sector)
{
  printf("Creating flock of %d birds\n", num);
  nr = num;
  spr = new iMeshWrapper* [nr];
  speed = new csVector3[nr];
  accel = new csVector3[nr];
  int i;
  iMeshFactoryWrapper *fact = engine->CreateMeshFactory(
    "crystalspace.mesh.object.sprite.2d", "BirdFactory");
  iSprite2DFactoryState *state = QUERY_INTERFACE(fact->GetMeshObjectFactory(),
    iSprite2DFactoryState);
  state->SetMaterialWrapper(mat);
  state->SetLighting(false);

  csVector3 startpos(20,10,20);
  csVector3 pos;

  focus = startpos;
  foc_speed.Set(-5, 0, +5);
  foc_accel.Set(0,0,0);

  for(i=0; i<nr; i++)
  {
    pos = startpos;
    speed[i].Set(0,0,0);
    speed[i].x = (float(rand()+1.)/float(RAND_MAX))*3. - 1.5;
    speed[i].y = (float(rand()+1.)/float(RAND_MAX))*1. - 0.5;
    speed[i].z = (float(rand()+1.)/float(RAND_MAX))*3. - 1.5;
    speed[i] += foc_speed*1.0;
    accel[i].Set(0,0,0);
    pos.x += (float(rand()+1.)/float(RAND_MAX))*20. ;
    pos.z -= (float(rand()+1.)/float(RAND_MAX))*20. ;
    spr[i] = engine->CreateMeshObject(fact, "Bird", sector, pos);

    iSprite2DState *sprstate = QUERY_INTERFACE(spr[i]->GetMeshObject(), 
      iSprite2DState);
    sprstate->GetVertices().SetLimit(4);
    sprstate->GetVertices().SetLength(4);
    sprstate->GetVertices()[0].color_init.Set(1.0,1.0,1.0);
    sprstate->GetVertices()[1].color_init.Set(1.0,1.0,1.0);
    sprstate->GetVertices()[2].color_init.Set(1.0,1.0,1.0);
    sprstate->GetVertices()[3].color_init.Set(1.0,1.0,1.0);
    //sprstate->CreateRegularVertices(4, true);
    float sz = 1.0;
    sprstate->GetVertices()[0].pos.Set(-sz, sz);
    sprstate->GetVertices()[0].u = 0.2;
    sprstate->GetVertices()[0].v = 0;
    sprstate->GetVertices()[1].pos.Set(+sz, sz);
    sprstate->GetVertices()[1].u = 0.8;
    sprstate->GetVertices()[1].v = 0;
    sprstate->GetVertices()[2].pos.Set(+sz, -sz);
    sprstate->GetVertices()[2].u = 0.8;
    sprstate->GetVertices()[2].v = 1.0;
    sprstate->GetVertices()[3].pos.Set(-sz, -sz);
    sprstate->GetVertices()[3].u = 0.2;
    sprstate->GetVertices()[3].v = 1.0;
    sprstate->DecRef();

  }
//  state->DecRef();
}


Flock::~Flock()
{
  int i;
  for(i=0; i<nr; i++)
    spr[i]->DecRef();
  delete[] spr;
}


static void Clamp( float &val, float max)
{
  if(val>max) val=max;
  else if(val<-max) val=-max;
}

void Flock::Update(cs_time elapsed)
{
  float dt = float(elapsed)*0.001; /// delta t in seconds
  /// move focus
  /// physics
  int i;
  csVector3 avg(0,0,0);
  for(i=0; i<nr; i++)
    avg += spr[i]->GetMovable()->GetPosition();
  avg /= nr;

  foc_accel = (-avg)*0.1 - focus*0.1;
  foc_accel.y = 0;
  foc_speed += foc_accel * dt;
  focus += foc_speed * dt;

  /// move each bird -- going towards focus
  for(i=0; i<nr; i++)
  {
    /// aim to focus
    csVector3 want = focus - spr[i]->GetMovable()->GetPosition();
    accel[i] += want * 0.1;
    float maxaccel = 2.5;
    if(accel[i].SquaredNorm() > maxaccel)
    {
      Clamp(accel[i].x, maxaccel);
      Clamp(accel[i].y, maxaccel/2.0);
      Clamp(accel[i].z, maxaccel);
    }
    /// physics
    speed[i] += accel[i] * dt;
    float maxspeed = 10.0;
    if(accel[i].SquaredNorm() > maxspeed)
    {
      Clamp(speed[i].x, maxspeed);
      Clamp(speed[i].y, maxspeed/2.0);
      Clamp(speed[i].z, maxspeed);
    }
    float perturb = 0.1;
    speed[i].x += (float(rand()+1.)/float(RAND_MAX))*perturb - perturb *0.5;
    speed[i].y += (float(rand()+1.)/float(RAND_MAX))*perturb - perturb *0.5;
    speed[i].z += (float(rand()+1.)/float(RAND_MAX))*perturb - perturb *0.5;
    speed[i].z *= 1.0 + (float(rand()+1.)/float(RAND_MAX))*0.2 - 0.1;
    csVector3 move = speed[i] * dt;
    spr[i]->GetMovable()->MovePosition(move);
    spr[i]->GetMovable()->UpdateMove();
  }
}


/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class.
  System = new Simple ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.image.loader:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.engine.core:Engine");
  System->RequestPlugin ("crystalspace.console.output.standard:Console.Output");
  System->RequestPlugin ("crystalspace.level.loader:LevelLoader");

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Printf (MSG_FATAL_ERROR, "Error initializing system!\n");
    exit (1);
  }

  // Main loop.
  System->Loop ();

  return 0;
}


