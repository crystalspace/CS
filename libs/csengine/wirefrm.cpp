/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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
#include "cstypes.h"
#include "qint.h"
#include "qsqrt.h"
#include "csgeom/math3d.h"
#include "csengine/wirefrm.h"
#include "csengine/engine.h"
#include "csengine/camera.h"
#include "ivideo/igraph3d.h"
#include "ivideo/igraph2d.h"
#include "ivideo/itxtmgr.h"

csWfColor::csWfColor (iTextureManager* txtmgr, int r, int g, int b)
{
  csWfColor::r = r;
  csWfColor::g = g;
  csWfColor::b = b;
  int i;
  for (i = 0 ; i < 16 ; i++)
    col_idx[i] = txtmgr->FindRGB (r*(20-i)/20, g*(20-i)/20, b*(20-i)/20);
}

int csWfColor::GetColor (float z)
{
  int zi = QInt (z);
  zi /= 2;
  if (zi > 15) zi = 15;
  return col_idx[zi];
}


csWfObject::csWfObject ()
{
  next = prev = NULL;
}

csWfObject::~csWfObject ()
{
}

bool csWfObject::Perspective (csCamera* c, csVector3& v, csVector2& persp, float radius, float& pradius)
{
  csVector3 cam = c->Other2This (v);
  if (cam.z > SMALL_Z)
  {
    float iz = c->GetFOV ()/cam.z;
    persp.x = cam.x * iz + c->GetShiftX ();
    persp.y = csEngine::frame_height - 1 - (cam.y * iz + c->GetShiftY ());
    pradius = radius * iz;
    return true;
  }
  else return false;
}

bool csWfObject::Orthographic (csCamera* c, int ortho,
	csVector3& v, csVector2& persp)
{
  const csVector3& o = c->GetOrigin ();
  csVector3 v2 = (v-o)*20;
  switch (ortho)
  {
    case WF_ORTHO_X: persp.x = v2.y; persp.y = v2.z; break;
    case WF_ORTHO_Y: persp.x = v2.x; persp.y = v2.z; break;
    case WF_ORTHO_Z: persp.x = v2.x; persp.y = v2.y; break;
  }
  persp.x += csEngine::frame_width/2;
  persp.y += csEngine::frame_height/2;
  return true;
}

void csWfVertex::Draw (iGraphics3D* g, csCamera* c, int ortho)
{
  if (ortho == WF_ORTHO_PERSP)
  {
    csVector2 persp;
    float rad;
    int r, px, py;
    iGraphics2D* g2d;

    if (Perspective (c, loc, persp, PLANE_VERTEX_RADIUS, rad))
    {
      csVector3 cam = c->Other2This (loc);
      int col = color->GetColor (cam.z);

      r = QInt (rad);
      px = QInt (persp.x);
      py = QInt (persp.y);
      g2d = g->GetDriver2D ();
      g2d->DrawLine (px-r, py-r, px+r, py+r, col);
      g2d->DrawLine (px+r, py-r, px-r, py+r, col);
      //if (cross)
      //{
        //g2d->DrawLine (px-r, py, px+r, py, col);
        //g2d->DrawLine (px, py-r, px, py+r, col);
      //}
    }
  }
  else
  {
    csVector2 persp;
    int r, px, py;
    iGraphics2D* g2d;

    if (Orthographic (c, ortho, loc, persp))
    {
      int col = color->GetColor (loc[ortho]);

      r = 3;
      px = QInt (persp.x);
      py = QInt (persp.y);
      g2d = g->GetDriver2D ();
      g2d->DrawLine (px-r, py-r, px+r, py+r, col);
      g2d->DrawLine (px+r, py-r, px-r, py+r, col);
      //if (cross)
      //{
        //g2d->DrawLine (px-r, py, px+r, py, col);
        //g2d->DrawLine (px, py-r, px, py+r, col);
      //}
    }
  }
}

void csWfLine::Draw (iGraphics3D* g, csCamera* c, int ortho)
{
  if (ortho == WF_ORTHO_PERSP)
  {
    csVector3 cam1 = c->Other2This (v1);
    csVector3 cam2 = c->Other2This (v2);
    g->DrawLine (cam1, cam2, c->GetFOV (), color->GetColor ((cam1.z+cam2.z)/2));
  }
  else
  {
    csVector2 p1, p2;
    iGraphics2D* g2d = g->GetDriver2D ();
    Orthographic (c, ortho, v1, p1);
    Orthographic (c, ortho, v2, p2);
    int col = color->GetColor (v1[ortho]+v2[ortho]);
    g2d->DrawLine (p1.x, p1.y, p2.x, p2.y, col);
  }
}

csWfPolygon::csWfPolygon ()
{
  vertices = NULL;
}

csWfPolygon::~csWfPolygon ()
{
  if (vertices) delete [] vertices;
}

void csWfPolygon::SetNumVertices (int n)
{
  num_vertices = n;
  delete [] vertices;
  vertices = new csVector3 [num_vertices];
}

void csWfPolygon::SetVertex (int i, csVector3& v)
{
  if (i >= num_vertices)
  {
    CsPrintf (MSG_INTERNAL_ERROR, "Bad vertex number %d in csWfPolygon::set_vertex!\n", i);
    fatal_exit (0, false);
  }
  vertices[i] = v;
}

void csWfPolygon::Prepare ()
{
  A = 0;
  B = 0;
  C = 0;
  int i, i1;
  float x1, y1, z1, x, y, z;

  csVector3 vmin (1000000,1000000,1000000);
  csVector3 vmax (-1000000,-10000000,-1000000);
  i1 = num_vertices-1;
  for (i = 0 ; i < num_vertices ; i++)
  {
    x = vertices[i].x;
    y = vertices[i].y;
    z = vertices[i].z;
    if (x < vmin.x) vmin.x = x;
    if (y < vmin.y) vmin.y = y;
    if (z < vmin.z) vmin.z = z;
    if (x > vmax.x) vmax.x = x;
    if (y > vmax.y) vmax.y = y;
    if (z > vmax.z) vmax.z = z;
    x1 = vertices[i1].x;
    y1 = vertices[i1].y;
    z1 = vertices[i1].z;
    A += (z1+z) * (y-y1);
    B += (x1+x) * (z-z1);
    C += (y1+y) * (x-x1);
    i1 = i;
  }

  float invd = qisqrt (A*A + B*B + C*C);

  A *= invd;
  B *= invd;
  C *= invd;

  x = vertices[0].x;
  y = vertices[0].y;
  z = vertices[0].z;
  D = -(A*x+B*y+C*z);

  center.x = (vmin.x+vmax.x)/2;
  center.y = (vmin.y+vmax.y)/2;
  center.z = (vmin.z+vmax.z)/2;
}

bool csWfPolygon::IsVisible (csCamera* camera)
{
  return csPlane3::Classify (A, B, C, D, camera->GetOrigin ()) < 0;
}

void csWfPolygon::Draw (iGraphics3D* g, csCamera* c, int ortho)
{
  if (ortho == WF_ORTHO_PERSP)
  {
    int i;
    csVector3 cam1, cam2;
    bool vis = IsVisible (c);
    csVector3 cen = c->Other2This (center);
    int col = color->GetColor (cen.z);
    int vcol = vcolor->GetColor (cen.z);
    for (i = 0 ; i < num_vertices ; i++)
    {
      cam1 = c->Other2This (vertices[i]);
      cam2 = c->Other2This (vertices[(i+1)%num_vertices]);
      g->DrawLine (cam1, cam2, c->GetFOV (), col);
      if (vis) g->DrawLine (cam1, cen, c->GetFOV (), vcol);
    }
  }
  else
  {
    int i;
    csVector2 p1, p2;
    int col = color->GetColor (center[ortho]);
    iGraphics2D* g2d = g->GetDriver2D ();
    for (i = 0 ; i < num_vertices ; i++)
    {
      Orthographic (c, ortho, vertices[i], p1);
      Orthographic (c, ortho, vertices[(i+1)%num_vertices], p2);
      g2d->DrawLine (p1.x, p1.y, p2.x, p2.y, col);
    }
  }
}

//=================================================================================================

csWireFrame::csWireFrame (iTextureManager* txtmgr)
{
  objects = NULL;
  numObjects = 0;
  csWireFrame::txtmgr = txtmgr;
  colors = NULL;
  white = RegisterColor (255, 255, 255);
  red = RegisterColor (255, 0, 0);
  green = RegisterColor (0, 255, 0);
  blue = RegisterColor (0, 0, 255);
  yellow = RegisterColor (255, 255, 0);
}

csWireFrame::~csWireFrame ()
{
  Clear ();
  while (colors)
  {
    csWfColor* n = colors->next;
    delete colors;
    colors = n;
  }
}

void csWireFrame::Clear ()
{
  while (objects)
  {
    csWfObject* n = objects->GetNext ();
    delete objects;
    objects = n;
  }
  numObjects = 0;
}

csWfColor* csWireFrame::FindColor (int r, int g, int b)
{
  csWfColor* c = colors;
  while (c)
  {
    if (c->r == r && c->g == g && c->b == b) return c;
    c = c->next;
  }
  return NULL;
}

csWfColor* csWireFrame::RegisterColor (int r, int g, int b)
{
  csWfColor* c = FindColor (r, g, b);
  if (c) return c;
  c = new csWfColor (txtmgr, r, g, b);
  c->next = colors;
  colors = c;
  return c;
}

csWfVertex* csWireFrame::AddVertex (const csVector3& v)
{
  csWfVertex* vt = new csWfVertex ();
  vt->SetLocation (v);
  vt->SetColor (white);
  vt->SetNext (objects);
  if (objects) objects->SetPrev (vt);
  objects = vt;

  numObjects++;

  return vt;
}

csWfLine* csWireFrame::AddLine (csVector3& v1, csVector3& v2)
{
  csWfLine* li = new csWfLine ();
  li->SetLine (v1, v2);
  li->SetColor (white);
  li->SetNext (objects);
  if (objects) objects->SetPrev (li);
  objects = li;

  numObjects++;

  return li;
}

csWfPolygon* csWireFrame::AddPolygon ()
{
  csWfPolygon* po = new csWfPolygon ();
  po->SetColor (white);
  po->SetNext (objects);
  if (objects) objects->SetPrev (po);
  objects = po;

  numObjects++;

  return po;
}

void csWireFrame::Draw (iGraphics3D* g, csCamera* c, int ortho)
{
  csWfObject* o = objects;
  while (o)
  {
    o->Draw (g, c, ortho);
    o = o->GetNext ();
  }
}

void csWireFrame::Apply (void (*func)( csWfObject*, void*), void *param)
{
  csWfObject* o = objects;
  while (o)
  {
    func (o, param);
    o = o->GetNext ();
  }
}

csWireFrameCam::csWireFrameCam (iTextureManager* txtmgr)
{
  wf = new csWireFrame (txtmgr);
  c = new csCamera ();
}

csWireFrameCam::~csWireFrameCam ()
{
  delete wf;
  delete c;
}

void csWireFrameCam::KeyUp (float speed, bool slow, bool fast)
{
  if (slow) c->MoveUnrestricted (speed*0.01f*VEC_FORWARD);
  else if (fast) c->MoveUnrestricted (speed*1.2f*VEC_FORWARD);
  else c->MoveUnrestricted (speed*0.6f*VEC_FORWARD);
}

void csWireFrameCam::KeyDown (float speed, bool slow, bool fast)
{
  if (slow) c->MoveUnrestricted (speed*0.01f*VEC_BACKWARD);
  else if (fast) c->MoveUnrestricted (speed*1.2f*VEC_BACKWARD);
  else c->MoveUnrestricted (speed*0.6f*VEC_BACKWARD);
}

void csWireFrameCam::KeyLeftStrafe (float speed, bool slow, bool fast)
{
  if (slow) c->MoveUnrestricted (speed*0.01f*VEC_LEFT);
  else if (fast) c->MoveUnrestricted (speed*1.2f*VEC_LEFT);
  else c->MoveUnrestricted (speed*0.6f*VEC_LEFT);
}

void csWireFrameCam::KeyRightStrafe (float speed, bool slow, bool fast)
{
  if (slow) c->MoveUnrestricted (speed*0.01f*VEC_RIGHT);
  else if (fast) c->MoveUnrestricted (speed*1.2f*VEC_RIGHT);
  else c->MoveUnrestricted (speed*0.6f*VEC_RIGHT);
}

void csWireFrameCam::KeyLeft (float speed, bool slow, bool fast)
{
  if (slow) c->Rotate (VEC_ROT_LEFT, speed*.005);
  else if (fast) c->Rotate (VEC_ROT_LEFT, speed*.2);
  else c->Rotate (VEC_ROT_LEFT, speed*.1);
}

void csWireFrameCam::KeyRight (float speed, bool slow, bool fast)
{
  if (slow) c->Rotate (VEC_ROT_RIGHT, speed*.005);
  else if (fast) c->Rotate (VEC_ROT_RIGHT, speed*.2);
  else c->Rotate (VEC_ROT_RIGHT, speed*.1);
}

void csWireFrameCam::KeyPgUp (float speed, bool slow, bool fast)
{
  if (slow) c->Rotate (VEC_TILT_UP, speed*.005);
  else if (fast) c->Rotate (VEC_TILT_UP, speed*.2);
  else c->Rotate (VEC_TILT_UP, speed*.1);
}

void csWireFrameCam::KeyPgDn (float speed, bool slow, bool fast)
{
  if (slow) c->Rotate (VEC_TILT_DOWN, speed*.005);
  else if (fast) c->Rotate (VEC_TILT_DOWN, speed*.2);
  else c->Rotate (VEC_TILT_DOWN, speed*.1);
}
