/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "csutil/inifile.h"
#include "apps/perftest/perftest.h"
#include "csgfxldr/csimage.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "iconsole.h"

//------------------------------ We need the VFS plugin -----

REGISTER_STATIC_LIBRARY (vfs)

//-----------------------------------------------------------------------------

void SetupPolygonDPFX (iGraphics3D* /*g3d*/, G3DPolygonDPFX& poly,
	float x1, float y1, float x2, float y2)
{
  poly.num = 4;
  poly.vertices[0].sx = x1;
  poly.vertices[0].sy = y2;
  poly.vertices[0].z = 4.;
  poly.vertices[0].u = 0;
  poly.vertices[0].v = 0;
  poly.vertices[0].r = 1;
  poly.vertices[0].g = 0;
  poly.vertices[0].b = 0;
  poly.vertices[1].sx = x2;
  poly.vertices[1].sy = y2;
  poly.vertices[1].z = 4.;
  poly.vertices[1].u = 1;
  poly.vertices[1].v = 0;
  poly.vertices[1].r = 0;
  poly.vertices[1].g = 1;
  poly.vertices[1].b = 0;
  poly.vertices[2].sx = x2;
  poly.vertices[2].sy = y1;
  poly.vertices[2].z = 4.;
  poly.vertices[2].u = 1;
  poly.vertices[2].v = 1;
  poly.vertices[2].r = 0;
  poly.vertices[2].g = 0;
  poly.vertices[2].b = 1;
  poly.vertices[3].sx = x1;
  poly.vertices[3].sy = y1;
  poly.vertices[3].z = 4.;
  poly.vertices[3].u = 0;
  poly.vertices[3].v = 1;
  poly.vertices[3].r = 1;
  poly.vertices[3].g = 1;
  poly.vertices[3].b = 0;
  poly.use_fog = false;
  poly.inv_aspect = 1./400.;
  poly.txt_handle = NULL;
  poly.flat_color_r = 255;
  poly.flat_color_g = 255;
  poly.flat_color_b = 255;
}

//-----------------------------------------------------------------------------

void SinglePolygonTester::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  SetupPolygonDPFX (g3d, poly, 10, 10, g3d->GetWidth ()-10, g3d->GetHeight ()-10);
  poly.txt_handle = perftest->GetTexture (0);
}

void SinglePolygonTester::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  g3d->StartPolygonFX (poly.txt_handle, CS_FX_COPY|CS_FX_GOURAUD);
  g3d->DrawPolygonFX (poly);
  g3d->FinishPolygonFX ();
}

Tester* SinglePolygonTester::NextTester ()
{
  return new SinglePolygonTesterFlat ();
}

//-----------------------------------------------------------------------------

void SinglePolygonTesterFlat::Setup (iGraphics3D* g3d, PerfTest* /*perftest*/)
{
  draw = 0;
  SetupPolygonDPFX (g3d, poly, 10, 10, g3d->GetWidth ()-10, g3d->GetHeight ()-10);
}

void SinglePolygonTesterFlat::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  g3d->StartPolygonFX (NULL, CS_FX_COPY|CS_FX_GOURAUD);
  g3d->DrawPolygonFX (poly);
  g3d->FinishPolygonFX ();
}

Tester* SinglePolygonTesterFlat::NextTester ()
{
  return new SinglePolygonTesterAlpha ();
}

//-----------------------------------------------------------------------------

void SinglePolygonTesterAlpha::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  SetupPolygonDPFX (g3d, poly, 10, 10, g3d->GetWidth ()-10, g3d->GetHeight ()-10);
  poly.txt_handle = perftest->GetTexture (0);
}

void SinglePolygonTesterAlpha::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  g3d->StartPolygonFX (poly.txt_handle, CS_FX_SETALPHA(.5) |CS_FX_GOURAUD);
  g3d->DrawPolygonFX (poly);
  g3d->FinishPolygonFX ();
}

Tester* SinglePolygonTesterAlpha::NextTester ()
{
  return new MultiPolygonTester ();
}

//-----------------------------------------------------------------------------

void MultiPolygonTester::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  int x, y;
  int w = g3d->GetWidth ()-20;
  int h = g3d->GetHeight ()-20;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
    {
      SetupPolygonDPFX (g3d, poly[x][y], 10+x*w/NUM_MULTIPOLTEST,
      	10+y*h/NUM_MULTIPOLTEST,
      	10+(x+1)*w/NUM_MULTIPOLTEST, 10+(y+1)*h/NUM_MULTIPOLTEST);
      poly[x][y].txt_handle = perftest->GetTexture (0);
    }
}

void MultiPolygonTester::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  g3d->StartPolygonFX (poly[0][0].txt_handle, CS_FX_COPY|CS_FX_GOURAUD);
  int x, y;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
    {
      g3d->DrawPolygonFX (poly[x][y]);
    }
  g3d->FinishPolygonFX ();
}

Tester* MultiPolygonTester::NextTester ()
{
  return new MultiPolygon2Tester ();
}

//-----------------------------------------------------------------------------

void MultiPolygon2Tester::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  int x, y;
  int w = g3d->GetWidth ()-20;
  int h = g3d->GetHeight ()-20;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
    {
      SetupPolygonDPFX (g3d, poly[x][y], 10+x*w/NUM_MULTIPOLTEST,
      	10+y*h/NUM_MULTIPOLTEST,
      	10+(x+1)*w/NUM_MULTIPOLTEST, 10+(y+1)*h/NUM_MULTIPOLTEST);
      poly[x][y].txt_handle = perftest->GetTexture (0);
    }
}

void MultiPolygon2Tester::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  int x, y;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
    {
      g3d->StartPolygonFX (poly[x][y].txt_handle, CS_FX_COPY|CS_FX_GOURAUD);
      g3d->DrawPolygonFX (poly[x][y]);
      g3d->FinishPolygonFX ();
    }
}

Tester* MultiPolygon2Tester::NextTester ()
{
  return new MultiTexture1Tester ();
}

//-----------------------------------------------------------------------------

void MultiTexture1Tester::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  int x, y;
  int w = g3d->GetWidth ()-20;
  int h = g3d->GetHeight ()-20;
  int i = 0;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
    {
      SetupPolygonDPFX (g3d, poly[x][y], 10+x*w/NUM_MULTIPOLTEST,
      	10+y*h/NUM_MULTIPOLTEST,
      	10+(x+1)*w/NUM_MULTIPOLTEST, 10+(y+1)*h/NUM_MULTIPOLTEST);
      poly[x][y].txt_handle = perftest->GetTexture (i%4);
      i++;
    }
}

void MultiTexture1Tester::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  int x, y;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
    {
      g3d->StartPolygonFX (poly[x][y].txt_handle, CS_FX_COPY);
      g3d->DrawPolygonFX (poly[x][y]);
      g3d->FinishPolygonFX ();
    }
}

Tester* MultiTexture1Tester::NextTester ()
{
  return new MultiTexture2Tester ();
}

//-----------------------------------------------------------------------------

void MultiTexture2Tester::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  int x, y;
  int w = g3d->GetWidth ()-20;
  int h = g3d->GetHeight ()-20;
  int i = 0;
  int div = (NUM_MULTIPOLTEST*NUM_MULTIPOLTEST)/4;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
    {
      SetupPolygonDPFX (g3d, poly[x][y], 10+x*w/NUM_MULTIPOLTEST,
        10+y*h/NUM_MULTIPOLTEST,
      	10+(x+1)*w/NUM_MULTIPOLTEST, 10+(y+1)*h/NUM_MULTIPOLTEST);
      poly[x][y].txt_handle = perftest->GetTexture (i/div);
      i++;
    }
}

void MultiTexture2Tester::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  iTextureHandle* prev = NULL;
  int x, y;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
    {
      if (poly[x][y].txt_handle != prev)
      {
        if (prev != NULL) g3d->FinishPolygonFX ();
        g3d->StartPolygonFX (poly[x][y].txt_handle, CS_FX_COPY);
	prev = poly[x][y].txt_handle;
      }
      g3d->DrawPolygonFX (poly[x][y]);
    }
  g3d->FinishPolygonFX ();
}

Tester* MultiTexture2Tester::NextTester ()
{
  return new MeshTester ();
}

//-----------------------------------------------------------------------------

void MeshTester::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  mesh.num_vertices = (NUM_MULTIPOLTEST+1)*(NUM_MULTIPOLTEST/2+1);
  mesh.num_vertices_pool = 1;
  mesh.num_textures = 1;
  mesh.num_triangles = NUM_MULTIPOLTEST*NUM_MULTIPOLTEST;
  mesh.triangles = new csTriangle [mesh.num_triangles];
  mesh.use_vertex_color = false;
  mesh.do_clip = false;
  mesh.do_fog = false;
  mesh.do_mirror = false;
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.vertex_mode = G3DTriangleMesh::VM_VIEWSPACE;
  mesh.fxmode = CS_FX_COPY;
  mesh.morph_factor = 0;
  mesh.vertices[0] = new csVector3 [mesh.num_vertices];
  mesh.texels[0][0] = new csVector2 [mesh.num_vertices];
  mesh.txt_handle[0] = perftest->GetTexture (0);
  int i;
  int x, y;
  float w = (g3d->GetWidth ()-20)/2;
  float h = (g3d->GetHeight ()-20);
  i = 0;
  for (y = 0 ; y <= NUM_MULTIPOLTEST/2 ; y++)
    for (x = 0 ; x <= NUM_MULTIPOLTEST ; x++)
    {
      mesh.vertices[0][i].Set (
      	w * float (x-NUM_MULTIPOLTEST/2) / float (NUM_MULTIPOLTEST/2),
      	h * float (y-NUM_MULTIPOLTEST/2) / float (NUM_MULTIPOLTEST/2) + h/2,
	1.);
      mesh.texels[0][0][i].Set (
        float (x) / float (NUM_MULTIPOLTEST),
        float (y) / float (NUM_MULTIPOLTEST/2)
        );
      i++;
    }
  i = 0;
  for (y = 0 ; y < NUM_MULTIPOLTEST/2 ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
    {
      mesh.triangles[i].c = y*(NUM_MULTIPOLTEST+1) + x;
      mesh.triangles[i].b = y*(NUM_MULTIPOLTEST+1) + x + 1;
      mesh.triangles[i].a = (y+1)*(NUM_MULTIPOLTEST+1) + x;
      i++;
      mesh.triangles[i].c = y*(NUM_MULTIPOLTEST+1) + x + 1;
      mesh.triangles[i].b = (y+1)*(NUM_MULTIPOLTEST+1) + x + 1;
      mesh.triangles[i].a = (y+1)*(NUM_MULTIPOLTEST+1) + x;
      i++;
    }
}

void MeshTester::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  //g3d->SetObjectToCamera ();
  g3d->SetPerspectiveAspect (1);//g3d->GetHeight ());
  g3d->DrawTriangleMesh (mesh);
}

Tester* MeshTester::NextTester ()
{
  delete [] mesh.triangles;
  delete [] mesh.vertices[0];
  delete [] mesh.texels[0][0];
  return NULL;
}

//-----------------------------------------------------------------------------

PerfTest::PerfTest ()
{
}

PerfTest::~PerfTest ()
{
}

void cleanup ()
{
  System->console_out ("Cleaning up...\n");
  delete System;
}

iTextureHandle* PerfTest::LoadTexture (char* file)
{
  iTextureManager* txtmgr = G3D->GetTextureManager ();
  size_t size;
  char* buf;
  iImage* image;
  buf = VFS->ReadFile (file, size);
  if (!buf || !size)
  {
    Printf (MSG_FATAL_ERROR, "Error loading texture '%s'!\n", file);
    exit (-1);
  }
  image = csImageLoader::Load ((UByte*)buf, size,
  	txtmgr->GetTextureFormat ());
  delete [] buf;
  if (!image) exit (-1);
  iTextureHandle* texture = txtmgr->RegisterTexture (image, CS_TEXTURE_3D);
  if (!texture) exit (-1);
  image->DecRef ();
  return texture;
}

bool PerfTest::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("Crystal Space 3D Performance Tester"))
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

  // Initialize textures.
  texture[0] = LoadTexture ("/lib/std/stone4.gif");
  texture[1] = LoadTexture ("/lib/std/mystone2.gif");
  texture[2] = LoadTexture ("/lib/std/andrew_marble4.gif");
  texture[3] = LoadTexture ("/lib/std/andrew_wood.gif");
  txtmgr->PrepareTextures ();

  // Allocate a uniformly distributed in R,G,B space palette for console
  // The console will crash on some platforms if this isn't initialize properly
  int r,g,b;
  for (r = 0; r < 8; r++)
    for (g = 0; g < 8; g++)
      for (b = 0; b < 4; b++)
	txtmgr->ReserveColor (r * 32, g * 32, b * 64);
  txtmgr->SetPalette ();

  // Initialize the console
  if (Console != NULL)
  {
    // Setup console colors
    Console->CacheColors (txtmgr);
    ConsoleReady = true;
    // Don't let messages before this one appear
    Console->Clear ();
  }

  // Some commercials...
  Printf (MSG_INITIALIZATION,
    "Crystal Space 3D Performance Tester 0.1.\n");

  txtmgr->SetPalette ();

  // Update the console with the new palette
  if (System->Console != NULL)
    System->Console->CacheColors (txtmgr);

  current_tester = new SinglePolygonTester ();
  needs_setup = true;

  return true;
}

static time_t last_time;

void PerfTest::NextFrame (time_t elapsed_time, time_t current_time)
{
  superclass::NextFrame (elapsed_time, current_time);

  // Tell 3D driver we're going to display 3D things.
  if (!G3D->BeginDraw (CSDRAW_3DGRAPHICS)) return;

  // Setup if needed.
  if (needs_setup)
  {
    current_tester->Setup (G3D, this);
    last_time = current_time;
  }

  // Do the test frame.
  if (current_tester)
  {
    current_tester->Draw (G3D);
    if (current_time-last_time >= 10000)
    {
      Printf (MSG_INITIALIZATION, "%f FPS\n",
      	current_tester->GetCount ()/10.);
      Tester* next_tester = current_tester->NextTester ();
      delete current_tester;
      current_tester = next_tester;
      if (current_tester)
      {
        needs_setup = true;
	current_tester->Setup (G3D, this);
	last_time = current_time;
      }
    }
  }

  // Start drawing 2D graphics.
  if (needs_setup)
  {
    if (!G3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
    Console->Clear ();
    char desc[255];
    current_tester->Description (desc);
    Printf (MSG_INITIALIZATION, desc);
    needs_setup = false;
  }

  // Drawing code ends here.
  G3D->FinishDraw ();
  // Print the final output.
  G3D->Print (NULL);
}

bool PerfTest::HandleEvent (csEvent &Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if ((Event.Type == csevKeyDown) && (Event.Key.Code == CSKEY_ESC))
  {
    StartShutdown ();
    return true;
  }
  else if ((Event.Type == csevKeyDown) && (Event.Key.Code == ' '))
  {
    last_time -= 10000;
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
  System = new PerfTest ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.console.stdout:Console");

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
