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
#include "csengine/sysitf.h"
#include "csengine/csview.h"
#include "csengine/world.h"
#include "csengine/pol2d.h"
#include "csengine/camera.h"

csView::csView (csWorld *iWorld, IGraphics3D* ig3d)
{
  world = iWorld;
  g3d = ig3d;
  CHK (view = new csPolygon2D ());
  CHK (camera = new csCamera ());
  clipper = NULL;
}

csView::~csView ()
{
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
  CHK (delete view);  view = NULL;
  bview = csBox (x, y, x+w-1, y+h-1);
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
  if (!clipper)
    if (view)
      CHKB (clipper = new csPolygonClipper (view->GetVertices (), view->GetNumVertices ()))
    else
      CHKB (clipper = new csBoxClipper (bview));

  world->Draw (g3d, camera, clipper);
}

void csView::SetSector (csSector *sector)
{
  camera->SetSector (sector);
}

