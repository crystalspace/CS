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

#include "sysdef.h"
#include "csgeom/polyclip.h"
#include "csengine/csview.h"
#include "csengine/world.h"
#include "csengine/pol2d.h"
#include "csengine/camera.h"
#include "igraph3d.h"
#include "qint.h"

csView::csView (csWorld *iWorld, iGraphics3D* ig3d)
{
  bview = NULL;
  world = iWorld;
  (G3D = ig3d)->IncRef ();

  CHK (view = new csPolygon2D ());
  CHK (camera = new csCamera ());
  clipper = NULL;
}

csView::~csView ()
{
  G3D->DecRef ();
  CHK (delete bview);
  CHK (delete camera);
  CHK (delete view);
  CHK (delete clipper);
}

void csView::ClearView ()
{
  if (view) view->MakeEmpty ();
}

void csView::SetRectangle (int x, int y, int w, int h)
{
  orig_width = G3D->GetWidth ();
  orig_height = G3D->GetHeight();

  // Do not allow the rectangle to go out of the screen
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > orig_width) { w = orig_width - x; }
  if (y + h > orig_height) { h = orig_height - y; }

  CHK (delete view);  view = NULL;
  CHK (delete bview);
  CHK (bview = new csBox (x, y, x + w - 1, y + h - 1));
  CHK (delete clipper); clipper = NULL;
}

void csView::AddViewVertex (int x, int y)
{
  if (!view)
    CHKB (view = new csPolygon2D ());
  view->AddVertex (x, y);
  CHK (delete clipper); clipper = NULL;
}

void csView::Draw ()
{
  if ((orig_width != G3D->GetWidth ()) || (orig_height != G3D->GetHeight()))
  {
    float xscale = ((float)G3D->GetWidth ())  / ((float)orig_width);
    float yscale = ((float)G3D->GetHeight ()) / ((float)orig_height);
    orig_width = G3D->GetWidth ();
    orig_height = G3D->GetHeight ();

    int xmin, ymin, xmax, ymax;
    if (!bview)
    {
      xmin = 0;
      ymin = 0;
      xmax = orig_width - 1;
      ymax = orig_height - 1;
    }
    else
    {
      xmin = QRound (xscale * bview->MinX());
      xmax = QRound (xscale * (bview->MaxX() + 1));
      ymin = QRound (yscale * bview->MinY());
      ymax = QRound (yscale * (bview->MaxY() + 1));
      CHK (delete bview);
    }
    CHK (bview = new csBox (xmin, ymin, xmax - 1, ymax - 1));
    CHK (delete clipper);
    clipper = NULL;
  }

  if (!clipper)
    if (view)
      CHKB (clipper = new csPolygonClipper (view))
    else
      CHKB (clipper = new csBoxClipper (*bview));

  G3D->SetPerspectiveCenter ((int)camera->shift_x, (int)camera->shift_y);
  world->Draw (camera, clipper);
}

void csView::SetSector (csSector *sector)
{
  camera->SetSector (sector);
}

void csView::SetPerspectiveCenter (float x, float y)
{
  camera->SetPerspectiveCenter (x, y);
}
