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
#include "csgeom/polyclip.h"
#include "csengine/csview.h"
#include "csengine/engine.h"
#include "csengine/pol2d.h"
#include "csengine/camera.h"
#include "ivideo/igraph3d.h"
#include "iengine/isector.h"
#include "qint.h"

IMPLEMENT_IBASE (csView)
  IMPLEMENTS_INTERFACE (iBase)
  IMPLEMENTS_EMBEDDED_INTERFACE (iView)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csView::View)
  IMPLEMENTS_INTERFACE (iView)
IMPLEMENT_EMBEDDED_IBASE_END

csView::csView (csEngine *e, iGraphics3D* ig3d)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiView);
  bview = NULL;
  engine = e;
  (G3D = ig3d)->IncRef ();

  pview = new csPolygon2D ();
  camera = new csCamera ();
  icamera = QUERY_INTERFACE (camera, iCamera);
  clipper = NULL;

  orig_width = orig_height = 0;
}

csView::~csView ()
{
  if (icamera) icamera->DecRef ();
  G3D->DecRef ();
  delete bview;
  delete camera;
  delete pview;
  delete clipper;
}

void csView::SetCamera (csCamera* c)
{
  iCamera* icam = QUERY_INTERFACE (c, iCamera);
  if (icamera) icamera->DecRef ();
  icamera = icam;
  camera = c;
}

void csView::ClearView ()
{
  if (pview) pview->MakeEmpty ();
}

void csView::SetRectangle (int x, int y, int w, int h)
{  
  orig_width = G3D->GetWidth ();
  orig_height = G3D->GetHeight ();
  // Do not allow the rectangle to go out of the screen
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > orig_width) { w = orig_width - x; }
  if (y + h > orig_height) { h = orig_height - y; }

  delete pview;  pview = NULL;
  delete bview;
  bview = new csBox2 (x, y, x + w, y + h);
  delete clipper; clipper = NULL;
}

void csView::SetContext (iGraphics3D *ig3d)
{
  orig_width = G3D->GetWidth ();
  orig_height = G3D->GetHeight();
  G3D->DecRef ();
  (G3D = ig3d)->IncRef ();

  UpdateView ();
}

void csView::UpdateView ()
{
  if (!orig_width || !orig_height)
  {
    orig_width = G3D->GetWidth ();
    orig_height = G3D->GetHeight ();
    return;
  }

  float scale_x = ((float)G3D->GetWidth ())  / ((float)orig_width);
  float scale_y = ((float)G3D->GetHeight ()) / ((float)orig_height);

  orig_width = G3D->GetWidth ();
  orig_height = G3D->GetHeight ();

  if (pview)
  {
    int i;
    csVector2 *pverts = pview->GetVertices ();
    int InCount = pview->GetNumVertices ();
    // scale poly
    for (i = 0; i < InCount; i++)
    {
      (*(pverts + i)).x *= scale_x;
      (*(pverts + i)).y *= scale_y;
    }

    // clip to make sure we are not exceeding screen bounds
    int OutCount;
    csBoxClipper bc (0., 0., 
		     (float)G3D->GetWidth (), (float)G3D->GetHeight());
    csVector2 *TempPoly = new csVector2[InCount + 5];
    UByte rc = bc.Clip (pview->GetVertices (), InCount , TempPoly, OutCount);
    if (rc != CS_CLIP_OUTSIDE)
    {
      pview->MakeRoom (OutCount);
      pview->SetVertices (&(TempPoly[0]), OutCount);
      pview->UpdateBoundingBox ();
    } 
    delete [] TempPoly;
  } 
  else if (bview)
  {
    csBox2 *new_bview = new csBox2 ( QRound (scale_x * bview->MinX()),
				     QRound (scale_y * bview->MinY()),
				     QRound (scale_x * (bview->MaxX())),
				     QRound (scale_y * (bview->MaxY())) );
    delete bview;
    bview = new_bview;
  }
  else
    bview = new csBox2 (0, 0, orig_width - 1, orig_height - 1);

  delete clipper;
  clipper = NULL;
}

void csView::AddViewVertex (int x, int y)
{
  if (!pview)
    pview = new csPolygon2D ();

  pview->AddVertex (x, y);
  delete clipper; clipper = NULL;
}

void csView::Draw ()
{
  UpdateClipper();
  G3D->SetPerspectiveCenter ( (int)camera->GetShiftX (), 
			      (int)camera->GetShiftY () );

  engine->SetContext (G3D);
  engine->Draw (camera, clipper);
}

void csView::UpdateClipper ()
{
  if ((orig_width != G3D->GetWidth ()) || (orig_height != G3D->GetHeight()))
    UpdateView ();

  if (!clipper)
  {
    if (pview)
      clipper = new csPolygonClipper (pview);
    else
      clipper = new csBoxClipper (*bview);
  }
}

void csView::SetSector (csSector *sector)
{
  camera->SetSector (sector);
}

void csView::SetPerspectiveCenter (float x, float y)
{
  camera->SetPerspectiveCenter (x, y);
}

void csView::View::SetSector (iSector* sector)
{
  scfParent->SetSector (sector->GetPrivateObject ());
}

void csView::View::SetCamera (iCamera* camera)
{
  scfParent->SetCamera (camera->GetPrivateObject ());
}

