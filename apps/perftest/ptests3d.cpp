/*
    Copyright (C) 2000 by Jorrit Tyberghein
    With additions by Samuel Humphreys

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
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iengine/material.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "apps/perftest/ptests3d.h"


void SetupPolygonDPFX (iGraphics3D* /*g3d*/, G3DPolygonDPFX& poly,
	float x1, float y1, float x2, float y2)
{
  poly.num = 4;
  poly.vertices[0].x = x1;
  poly.vertices[0].y = y2;
  poly.z[0] = 4.;
  poly.texels[0].x = 0;
  poly.texels[0].y = 0;
  poly.colors[0].red = 1;
  poly.colors[0].green = 0;
  poly.colors[0].blue = 0;
  poly.vertices[1].x = x2;
  poly.vertices[1].y = y2;
  poly.z[1] = 4.;
  poly.texels[1].x = 1;
  poly.texels[1].y = 0;
  poly.colors[1].red = 0;
  poly.colors[1].green = 1;
  poly.colors[1].blue = 0;
  poly.vertices[2].x = x2;
  poly.vertices[2].y = y1;
  poly.z[2] = 4.;
  poly.texels[2].x = 1;
  poly.texels[2].y = 1;
  poly.colors[2].red = 0;
  poly.colors[2].green = 0;
  poly.colors[2].blue = 1;
  poly.vertices[3].x = x1;
  poly.vertices[3].y = y1;
  poly.z[3] = 4.;
  poly.texels[3].x = 0;
  poly.texels[3].y = 1;
  poly.colors[3].red = 1;
  poly.colors[3].green = 1;
  poly.colors[3].blue = 0;
  poly.use_fog = false;
#ifndef CS_USE_OLD_RENDERER
  poly.tex_handle = 0;
#else
  poly.mat_handle = 0;
#endif
  poly.flat_color_r = 255;
  poly.flat_color_g = 255;
  poly.flat_color_b = 255;
  poly.mixmode = CS_FX_COPY;
}

//-----------------------------------------------------------------------------

void SinglePolygonTester::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  SetupPolygonDPFX (g3d, poly, 10, 10, g3d->GetWidth ()-10, g3d->GetHeight ()-10);
#ifdef CS_USE_OLD_RENDERER
  poly.mat_handle = perftest->GetMaterial (0);
#endif
}

void SinglePolygonTester::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetPerspectiveAspect (400);
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  g3d->DrawPolygonFX (poly);
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
#ifdef CS_USE_OLD_RENDERER
  poly.mat_handle = 0;
#endif
}

void SinglePolygonTesterFlat::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  g3d->DrawPolygonFX (poly);
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
#ifdef CS_USE_OLD_RENDERER
  poly.mat_handle = perftest->GetMaterial (0);
#endif
  poly.mixmode = CS_FX_SETALPHA(.5);
}

void SinglePolygonTesterAlpha::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  g3d->DrawPolygonFX (poly);
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
#ifdef CS_USE_OLD_RENDERER
      poly[x][y].mat_handle = perftest->GetMaterial (0);
#endif
      poly[x][y].mixmode = CS_FX_COPY;
    }
}

void MultiPolygonTester::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  int x, y;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
      g3d->DrawPolygonFX (poly[x][y]);
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
#ifdef CS_USE_OLD_RENDERER
      poly[x][y].mat_handle = perftest->GetMaterial (0);
#endif
      poly[x][y].mixmode = CS_FX_COPY;
    }
}

void MultiPolygon2Tester::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  int x, y;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
      g3d->DrawPolygonFX (poly[x][y]);
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
#ifdef CS_USE_OLD_RENDERER
      poly[x][y].mat_handle = perftest->GetMaterial (i%4);
#endif
      poly[x][y].mixmode = CS_FX_COPY;
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
      g3d->DrawPolygonFX (poly[x][y]);
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
#ifdef CS_USE_OLD_RENDERER
  int div = (NUM_MULTIPOLTEST*NUM_MULTIPOLTEST)/4;
#endif
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
    {
      SetupPolygonDPFX (g3d, poly[x][y], 10+x*w/NUM_MULTIPOLTEST,
        10+y*h/NUM_MULTIPOLTEST,
      	10+(x+1)*w/NUM_MULTIPOLTEST, 10+(y+1)*h/NUM_MULTIPOLTEST);
#ifdef CS_USE_OLD_RENDERER
      poly[x][y].mat_handle = perftest->GetMaterial (i/div);
#endif
      poly[x][y].mixmode = CS_FX_COPY;
      i++;
    }
}

void MultiTexture2Tester::Draw (iGraphics3D* g3d)
{
  draw++;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL);
  int x, y;
  for (y = 0 ; y < NUM_MULTIPOLTEST ; y++)
    for (x = 0 ; x < NUM_MULTIPOLTEST ; x++)
      g3d->DrawPolygonFX (poly[x][y]);
}

Tester* MultiTexture2Tester::NextTester ()
{
  return new MeshTester ();
}

//-----------------------------------------------------------------------------

void MeshTester::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  num_mesh_vertices = (NUM_MULTIPOLTEST+1)*(NUM_MULTIPOLTEST/2+1);
  mesh.num_vertices_pool = 1;
  mesh.num_triangles = NUM_MULTIPOLTEST*NUM_MULTIPOLTEST;
  mesh.triangles = new csTriangle [mesh.num_triangles];
  mesh.use_vertex_color = false;
  mesh.clip_portal = CS_CLIP_NOT;
  mesh.clip_plane = CS_CLIP_NOT;
  mesh.do_fog = false;
  mesh.do_mirror = false;
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.vertex_mode = G3DTriangleMesh::VM_VIEWSPACE;
  mesh.mixmode = CS_FX_COPY;
  mesh.morph_factor = 0;
  mesh.vertex_fog = 0;
  mesh_vertices = new csVector3 [num_mesh_vertices];
  mesh_texels = new csVector2 [num_mesh_vertices];
  mesh.mat_handle = perftest->GetMaterial (0);
  vbuf = g3d->GetVertexBufferManager ()->CreateBuffer (0);
  mesh.buffers[0] = vbuf;

  int i;
  int x, y;
  float w = (g3d->GetWidth ()-20)/2;
  float h = (g3d->GetHeight ()-20);
  csBox3 bbox;
  bbox.StartBoundingBox ();
  i = 0;
  for (y = 0 ; y <= NUM_MULTIPOLTEST/2 ; y++)
    for (x = 0 ; x <= NUM_MULTIPOLTEST ; x++)
    {
      mesh_vertices[i].Set (
      	w * float (x-NUM_MULTIPOLTEST/2) / float (NUM_MULTIPOLTEST/2),
      	h * float (y-NUM_MULTIPOLTEST/2) / float (NUM_MULTIPOLTEST/2) + h/2,
	1.);
      bbox.AddBoundingVertex (mesh_vertices[i]);
      mesh_texels[i].Set (
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
  g3d->SetClipper (0, CS_CLIPPER_NONE);
  g3d->SetPerspectiveAspect (1);//g3d->GetHeight ());
  g3d->GetVertexBufferManager ()->LockBuffer (vbuf, mesh_vertices,
  	mesh_texels, 0, num_mesh_vertices, 0, bbox);
  g3d->DrawTriangleMesh (mesh);
  g3d->GetVertexBufferManager ()->UnlockBuffer (vbuf);
}

Tester* MeshTester::NextTester ()
{
  delete [] mesh.triangles;
  delete [] mesh_vertices;
  delete [] mesh_texels;
  return new PixmapTester ();
}

//-----------------------------------------------------------------------------

void PixmapTester::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  inc_w = g3d->GetWidth ()/10;
  inc_h = g3d->GetHeight ()/10;
  texture = perftest->GetMaterial (0)->GetTexture ();
  texture->GetMipMapDimensions (0, tex_w, tex_h);
}

void PixmapTester::Draw (iGraphics3D* g3d)
{
  draw++;
  int i, j;
  for (i = 0; i < 10; ++i)
    for (j = 0; j < 10; ++j)
      g3d->DrawPixmap (texture,
		       i*inc_w+5, j*inc_h+10, inc_w-5, inc_h-5,
		       0, 0, tex_w, tex_h);
}

Tester* PixmapTester::NextTester ()
{

  return new MultiTexturePixmapTester ();
}

//-----------------------------------------------------------------------------

void MultiTexturePixmapTester::Setup (iGraphics3D* g3d, PerfTest* perftest)
{
  draw = 0;
  inc_w = g3d->GetWidth ()/10;
  inc_h = g3d->GetHeight ()/10;
  int i;
  for (i = 0; i < 4; i++)
  {
    tex[i].texture = perftest->GetMaterial (i)->GetTexture ();
    tex[i].texture->GetMipMapDimensions (0, tex[i].tex_w, tex[i].tex_h);
  }
}

void MultiTexturePixmapTester::Draw (iGraphics3D* g3d)
{
  draw++;
  int i, j;
  for (i = 0; i < 10; ++i)
    for (j = 0; j < 10; ++j)
      g3d->DrawPixmap (tex[j&3].texture,
		       i*inc_w+5, j*inc_h+10, inc_w-5, inc_h-10,
		       0, 0, tex[j&3].tex_w, tex[j&3].tex_h);
}

Tester* MultiTexturePixmapTester::NextTester ()
{

  return 0;
}
