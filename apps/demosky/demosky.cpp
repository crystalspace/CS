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
#include "igraph3d.h"
#include "itxtmgr.h"
#include "iconsole.h"
#include "ifontsrv.h"

//------------------------------------------------- We need the 3D engine -----

REGISTER_STATIC_LIBRARY (engine)

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
}

Simple::~Simple ()
{
  delete view;
  if(font) font->DecRef();

  delete sky;
  delete sky_f;
  delete sky_b;
  delete sky_l;
  delete sky_r;
  delete sky_u;
  delete sky_d;
}

void cleanup ()
{
  System->console_out ("Cleaning up...\n");
  delete System;
}


void Simple::SetTexSpace(csProcSkyTexture *skytex, csPolygon3D *poly, 
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

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("Crystal Space Procedural Sky Demo"))
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

  room = engine->CreateCsSector ("room");
  csThing* walls = engine->CreateSectorWalls (room, "walls");
  csPolygon3D* p;
  p = walls->NewPolygon (matd);
  float size = 500.0; /// size of the skybox -- around 0,0,0 for now.
  float simi = size; //*255./256.; /// sizeminor
  p->AddVertex (-size, -simi, size);
  p->AddVertex (size, -simi, size);
  p->AddVertex (size, -simi, -size);
  p->AddVertex (-size, -simi, -size);
  //sky_d->SetTextureSpace(p->Vobj(0), p->Vobj(1)-p->Vobj(0), p->Vobj(3)-p->Vobj(0));
  SetTexSpace (sky_d, p, 256, p->Vobj (0), p->Vobj (1), 2.*size, p->Vobj(3), 2.*size);
  p->flags.Set(CS_POLY_LIGHTING, 0);

  p = walls->NewPolygon (matu);
  p->AddVertex (-size, simi, -size);
  p->AddVertex (size, simi, -size);
  p->AddVertex (size, simi, size);
  p->AddVertex (-size, simi, size);
  //sky_u->SetTextureSpace(p->Vobj(0), p->Vobj(1)-p->Vobj(0), p->Vobj(3)-p->Vobj(0));
  SetTexSpace (sky_u, p, 256, p->Vobj (0), p->Vobj (1), 2.*size, p->Vobj(3), 2.*size);
  p->flags.Set(CS_POLY_LIGHTING, 0);

  p = walls->NewPolygon (matf);
  p->AddVertex (-size, size, simi);
  p->AddVertex (size, size, simi);
  p->AddVertex (size, -size, simi);
  p->AddVertex (-size, -size, simi);
  //sky_f->SetTextureSpace(p->Vobj(0), p->Vobj(1)-p->Vobj(0), p->Vobj(3)-p->Vobj(0));
  SetTexSpace (sky_f, p, 256, p->Vobj (0), p->Vobj (1), 2.*size, p->Vobj(3), 2.*size);
  p->flags.Set(CS_POLY_LIGHTING, 0);

  p = walls->NewPolygon (matr);
  p->AddVertex (simi, size, size);
  p->AddVertex (simi, size, -size);
  p->AddVertex (simi, -size, -size);
  p->AddVertex (simi, -size, size);
  //sky_r->SetTextureSpace(p->Vobj(0), p->Vobj(1)-p->Vobj(0), p->Vobj(3)-p->Vobj(0));
  SetTexSpace (sky_r, p, 256, p->Vobj (0), p->Vobj (1), 2.*size, p->Vobj(3), 2.*size);
  p->flags.Set(CS_POLY_LIGHTING, 0);

  p = walls->NewPolygon (matl);
  p->AddVertex (-simi, size, -size);
  p->AddVertex (-simi, size, size);
  p->AddVertex (-simi, -size, size);
  p->AddVertex (-simi, -size, -size);
  //sky_l->SetTextureSpace(p->Vobj(0), p->Vobj(1)-p->Vobj(0), p->Vobj(3)-p->Vobj(0));
  SetTexSpace (sky_l, p, 256, p->Vobj (0), p->Vobj (1), 2.*size, p->Vobj(3), 2.*size);
  p->flags.Set(CS_POLY_LIGHTING, 0);

  p = walls->NewPolygon (matb);
  p->AddVertex (size, size, -simi);
  p->AddVertex (-size, size, -simi);
  p->AddVertex (-size, -size, -simi);
  p->AddVertex (size, -size, -simi);
  //sky_b->SetTextureSpace(p->Vobj(0), p->Vobj(1)-p->Vobj(0), p->Vobj(3)-p->Vobj(0));
  SetTexSpace (sky_b, p, 256, p->Vobj (0), p->Vobj (1), 2.*size, p->Vobj(3), 2.*size);
  p->flags.Set(CS_POLY_LIGHTING, 0);

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
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.engine.core:Engine");
  System->RequestPlugin ("crystalspace.console.output.standard:Console");

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
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
