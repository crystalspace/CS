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

#include "sysdef.h"
#include "types.h"
#include "qint.h"
#include "csgeom/math3d.h"
#include "csgeom/csrect.h"
#include "csengine/wirefrm.h"
#include "csengine/world.h"
#include "csengine/camera.h"
#include "csengine/sysitf.h"
#include "igraph3d.h"
#include "igraph2d.h"
#include "itxtmgr.h"

csWfColor::csWfColor (ITextureManager* txtmgr, int r, int g, int b)
{
  csWfColor::r = r;
  csWfColor::g = g;
  csWfColor::b = b;
  int i;
  for (i = 0 ; i < 16 ; i++)
    txtmgr->FindRGB (r*(20-i)/20, g*(20-i)/20, b*(20-i)/20, col_idx[i]);
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
  csVector3 cam;
  cam = c->Other2This (v);
  if (cam.z > SMALL_Z)
  {
    float iz = csCamera::aspect/cam.z;
    persp.x = cam.x * iz + csWorld::shift_x;
    persp.y = csWorld::frame_height - 1 - (cam.y * iz + csWorld::shift_y);
    pradius = radius * iz;
    return true;
  }
  else return false;
}

void csWfVertex::Draw (IGraphics3D* g, csCamera* c)
{
  csVector2 persp;
  float rad;
  int r, px, py;
  IGraphics2D* g2d;

  if (Perspective (c, loc, persp, PLANE_VERTEX_RADIUS, rad))
  {
    csVector3 cam;
    cam = c->Other2This (loc);
    int col = color->GetColor (cam.z);

    r = QInt (rad);
    px = QInt (persp.x);
    py = QInt (persp.y);
    g->Get2dDriver (&g2d);
    g2d->DrawLine (px-r, py-r, px+r, py+r, col);
    g2d->DrawLine (px+r, py-r, px-r, py+r, col);
    //if (cross)
    //{
      //g2d->DrawLine (px-r, py, px+r, py, col);
      //g2d->DrawLine (px, py-r, px, py+r, col);
    //}
  }
}

void csWfLine::Draw (IGraphics3D* g, csCamera* c)
{
  csVector3 cam1, cam2;
  cam1 = c->Other2This (v1);
  cam2 = c->Other2This (v2);
  g->DrawLine (cam1, cam2, csCamera::aspect, color->GetColor ((cam1.z+cam2.z)/2));
}

csWfPolygon::csWfPolygon ()
{
  vertices = NULL;
}

csWfPolygon::~csWfPolygon ()
{
  if (vertices) CHKB (delete [] vertices);
}

void csWfPolygon::SetNumVertices (int n)
{
  num_vertices = n;
  CHK (delete [] vertices);
  CHK (vertices = new csVector3 [num_vertices]);
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

  float d = sqrt (A*A + B*B + C*C);

  if (d < SMALL_EPSILON) d = SMALL_EPSILON;

  A /= d;
  B /= d;
  C /= d;

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
  return csPlane::Classify (A, B, C, D, camera->GetOrigin ()) < 0;
}

void csWfPolygon::Draw (IGraphics3D* g, csCamera* c)
{
  int i;
  csVector3 cam1, cam2, cen;
  bool vis = IsVisible (c);
  cen = c->Other2This (center);
  int col = color->GetColor (cen.z);
  int vcol = vcolor->GetColor (cen.z);
  for (i = 0 ; i < num_vertices ; i++)
  {
    cam1 = c->Other2This (vertices[i]);
    cam2 = c->Other2This (vertices[(i+1)%num_vertices]);
    g->DrawLine (cam1, cam2, csCamera::aspect, col);
    if (vis) g->DrawLine (cam1, cen, csCamera::aspect, vcol);
  }
}

//=================================================================================================

csWireFrame::csWireFrame (ITextureManager* txtmgr)
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
    CHK (delete colors);
    colors = n;
  }
}

void csWireFrame::Clear ()
{
  while (objects)
  {
    csWfObject* n = objects->GetNext ();
    CHK (delete objects);
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
  CHK (c = new csWfColor (txtmgr, r, g, b));
  c->next = colors;
  colors = c;
  return c;
}

csWfVertex* csWireFrame::AddVertex (csVector3& v)
{
  CHK (csWfVertex* vt = new csWfVertex ());
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
  CHK (csWfLine* li = new csWfLine ());
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
  CHK (csWfPolygon* po = new csWfPolygon ());
  po->SetColor (white);
  po->SetNext (objects);
  if (objects) objects->SetPrev (po);
  objects = po;

  numObjects++;

  return po;
}

void csWireFrame::Draw (IGraphics3D* g, csCamera* c)
{
  csWfObject* o = objects;
  while (o)
  {
    o->Draw (g, c);
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

csWireFrameCam::csWireFrameCam (ITextureManager* txtmgr)
{
  CHK (wf = new csWireFrame (txtmgr));
  CHK (c = new csCamera ());
}

csWireFrameCam::~csWireFrameCam ()
{
  CHK (delete wf);
  CHK (delete c);
}

void csWireFrameCam::KeyUp (bool shift, bool alt, bool ctrl)
{
  if (alt)
  {
    if (ctrl) c->MoveUnrestricted (.01*VEC_UP);
    else if (shift) c->MoveUnrestricted (.4*VEC_UP);
    else c->MoveUnrestricted (.2*VEC_UP);
  }
  else if (ctrl) c->MoveUnrestricted (.01*VEC_FORWARD);
  else if (shift) c->MoveUnrestricted (1.2*VEC_FORWARD);
  else c->MoveUnrestricted (.6*VEC_FORWARD);
}

void csWireFrameCam::KeyDown (bool shift, bool alt, bool ctrl)
{
  if (alt)
  {
    if (ctrl) c->MoveUnrestricted (.01*VEC_DOWN);
    else if (shift) c->MoveUnrestricted (.4*VEC_DOWN);
    else c->MoveUnrestricted (.2*VEC_DOWN);
  }
  else if (ctrl) c->MoveUnrestricted (.01*VEC_BACKWARD);
  else if (shift) c->MoveUnrestricted (1.2*VEC_BACKWARD);
  else c->MoveUnrestricted (.6*VEC_BACKWARD);
}

void csWireFrameCam::KeyLeft (bool shift, bool alt, bool ctrl)
{
  if (alt)
  {
    if (ctrl) c->MoveUnrestricted (.01*VEC_LEFT);
    else if (shift) c->MoveUnrestricted (.4*VEC_LEFT);
    else c->MoveUnrestricted (.2*VEC_LEFT);
  }
  else if (ctrl) c->Rotate (VEC_ROT_LEFT, .005);
  else if (shift) c->Rotate (VEC_ROT_LEFT, .2);
  else c->Rotate (VEC_ROT_LEFT, .1);
}

void csWireFrameCam::KeyRight (bool shift, bool alt, bool ctrl)
{
  if (alt)
  {
    if (ctrl) c->MoveUnrestricted (.01*VEC_RIGHT);
    else if (shift) c->MoveUnrestricted (.4*VEC_RIGHT);
    else c->MoveUnrestricted (.2*VEC_RIGHT);
  }
  else if (ctrl) c->Rotate (VEC_ROT_RIGHT, .005);
  else if (shift) c->Rotate (VEC_ROT_RIGHT, .2);
  else c->Rotate (VEC_ROT_RIGHT, .1);
}

void csWireFrameCam::KeyPgUp (bool shift, bool alt, bool ctrl)
{
  if (alt)
  {
    if (ctrl) c->Rotate (VEC_TILT_LEFT, .005);
    else c->Rotate (VEC_TILT_LEFT, .1);
  }
  else if (ctrl) c->Rotate (VEC_TILT_UP, .005);
  else if (shift) c->Rotate (VEC_TILT_UP, .2);
  else c->Rotate (VEC_TILT_UP, .1);
}

void csWireFrameCam::KeyPgDn (bool shift, bool alt, bool ctrl)
{
  if (alt)
  {
    if (ctrl) c->Rotate (VEC_TILT_RIGHT, .005);
    else c->Rotate (VEC_TILT_RIGHT, .1);
  }
  else if (ctrl) c->Rotate (VEC_TILT_DOWN, .005);
  else if (shift) c->Rotate (VEC_TILT_DOWN, .2);
  else c->Rotate (VEC_TILT_DOWN, .1);
}

