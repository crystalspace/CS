/*
    CrystalSpace 3D renderer view
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
#include "csqint.h"
#include "csgeom/polyclip.h"
#include "csgeom/poly2d.h"
#include "ivideo/graph3d.h"
#include "cstool/csview.h"
#include "iengine/camera.h"
#include "iengine/engine.h"

SCF_IMPLEMENT_IBASE (csView)
  SCF_IMPLEMENTS_INTERFACE (iView)
SCF_IMPLEMENT_IBASE_END


csView::csView (iEngine *e, iGraphics3D* ig3d) :
  Engine (e), G3D (ig3d), RectView (0), PolyView (0), AutoResize (true)
{
  SCF_CONSTRUCT_IBASE (0);

  Camera = e->CreateCamera ();

  OldWidth = G3D->GetWidth ();
  OldHeight = G3D->GetHeight ();
}

csView::~csView ()
{
  delete RectView;
  delete PolyView;
  SCF_DESTRUCT_IBASE();
}

iEngine* csView::GetEngine ()
{
  return Engine;
}

void csView::SetEngine (iEngine* e)
{
  Engine = e;
}

iCamera *csView::GetCamera ()
{
  return Camera;
}

void csView::SetCamera (iCamera* c)
{
  Camera = c;
}

iGraphics3D* csView::GetContext ()
{
  return G3D;
}

void csView::SetContext (iGraphics3D *ig3d)
{
  G3D = ig3d;
}

void csView::SetRectangle (int x, int y, int w, int h)
{
  OldWidth = G3D->GetWidth ();
  OldHeight = G3D->GetHeight ();
  delete PolyView; PolyView = 0;
  Clipper = 0;

  // Do not allow the rectangle to go out of the screen
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > OldWidth) { w = OldWidth - x; }
  if (y + h > OldHeight) { h = OldHeight - y; }

  if (RectView)
    RectView->Set (x, y, x + w, y + h);
  else
    RectView = new csBox2 (x, y, x + w, y + h);
}

void csView::ClearView ()
{
  OldWidth = G3D->GetWidth ();
  OldHeight = G3D->GetHeight ();

  Clipper = 0;
  delete RectView; RectView = 0;

  if (PolyView) PolyView->MakeEmpty ();
}

void csView::AddViewVertex (int x, int y)
{
  if (!PolyView)
    PolyView = new csPoly2D ();
  PolyView->AddVertex (x, y);

  Clipper = 0;
  delete RectView; RectView = 0;
}

void csView::UpdateView ()
{
  if (OldWidth == G3D->GetWidth () && OldHeight == G3D->GetHeight ())
    return;

  float scale_x = ((float)G3D->GetWidth ())  / ((float)OldWidth);
  float scale_y = ((float)G3D->GetHeight ()) / ((float)OldHeight);

  Camera->SetPerspectiveCenter (Camera->GetShiftX() * scale_x,
                                Camera->GetShiftY() * scale_y);

  Camera->SetFOVAngle (Camera->GetFOVAngle(), G3D->GetWidth());

  OldWidth = G3D->GetWidth ();
  OldHeight = G3D->GetHeight ();
  
  if (PolyView)
  {
    int i;
    csVector2 *pverts = PolyView->GetVertices ();
    int InCount = PolyView->GetVertexCount ();
    // scale poly
    for (i = 0; i < InCount; i++)
    {
      pverts[i].x *= scale_x;
      pverts[i].y *= scale_y;
    }
  }
  else if (RectView)
  {
    RectView->Set (csQround (scale_x * RectView->MinX()),
		   csQround (scale_y * RectView->MinY()),
		   csQround (scale_x * RectView->MaxX()),
		   csQround (scale_y * RectView->MaxY()) );
  }

  Clipper = 0;
}

void csView::Draw ()
{
  UpdateClipper();
  G3D->SetPerspectiveCenter ( (int)Camera->GetShiftX (),
			      (int)Camera->GetShiftY () );

  Engine->Draw (Camera, Clipper);
}

void csView::UpdateClipper ()
{
  if (AutoResize) UpdateView ();

  if (!Clipper)
  {
    if (PolyView)
      Clipper.AttachNew (new csPolygonClipper (PolyView));
    else
    {
      if (!RectView)
        RectView = new csBox2 (0, 0, OldWidth - 1, OldHeight - 1);
      Clipper.AttachNew (new csBoxClipper (*RectView));
    }
  }
}

void csView::RestrictClipperToScreen ()
{
  // rectangular views are automatically restricted to screen borders,
  // so we only have to update polygon-based views
  if (PolyView)
  {
    int InCount = PolyView->GetVertexCount (), OutCount;
    csBoxClipper bc (0., 0., (float)G3D->GetWidth (), (float)G3D->GetHeight());
    csVector2 *TempPoly = new csVector2[InCount + 5];
    uint8 rc = bc.Clip (PolyView->GetVertices (), InCount , TempPoly, OutCount);
    if (rc != CS_CLIP_OUTSIDE)
    {
      PolyView->MakeRoom (OutCount);
      PolyView->SetVertices (TempPoly, OutCount);
      //@@@PolyView->UpdateBoundingBox ();
    }
    delete [] TempPoly;
  }
}

iClipper2D* csView::GetClipper ()
{
  UpdateClipper ();
  return Clipper;
}

