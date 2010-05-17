/*
    Copyright (C) 2008 by Jorrit Tyberghein

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
#include "csgeom/box.h"
#include "igeom/clip2d.h"
#include "ivaria/view.h"
#include "iengine/camera.h"
#include "iengine/rendermanager.h"
#include "iengine/campos.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "ivaria/reporter.h"
#include "cstool/csview.h"
#include "splitview.h"
#include "walktest.h"


// Use a view's clipping rect to calculate a bounding box
static void BoundingBoxForView (iView *view, csBox2 *box)
{
  size_t vertexCount = view->GetClipper()->GetVertexCount();
  csVector2 *clip = view->GetClipper()->GetClipPoly();
  for (size_t i = 0; i < vertexCount; i++)
    box->AddBoundingVertex (clip[i]);
}

WalkTestViews::WalkTestViews (WalkTest* walktest)
    : walktest (walktest), split (-1)
{
  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  views[0] = csPtr<iView> (new csView (walktest->Engine, walktest->myG3D));
  views[1] = csPtr<iView> (new csView (walktest->Engine, walktest->myG3D));
  view = views[0];

  int w3d = walktest->myG3D->GetWidth ();
  int h3d = walktest->myG3D->GetHeight ();
#ifdef CS_DEBUG
  view->SetRectangle (2, 2, w3d - 4, h3d - 4);
#else
  view->SetRectangle (0, 0, w3d, h3d);
#endif

}

bool WalkTestViews::SetupViewStart ()
{
  // Look for the start sector in this map.
  iEngine* Engine = walktest->Engine;
  bool camok = false;
  if (!camok && Engine->GetCameraPositions ()->GetCount () > 0)
  {
    iCameraPosition *cp = Engine->GetCameraPositions ()->Get (0);
    if (cp->Load(views[0]->GetCamera (), Engine) &&
	cp->Load(views[1]->GetCamera (), Engine))
      camok = true;
  }
  if (!camok)
  {
    iSector* room = Engine->GetSectors ()->FindByName ("room");
    if (room)
    {
      views[0]->GetCamera ()->SetSector (room);
      views[1]->GetCamera ()->SetSector (room);
      views[0]->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
      views[1]->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
      camok = true;
    }
  }
  return camok;
}

bool WalkTestViews::SplitView ()
{
  if (split == -1)
  {	
    csBox2 bbox;
    BoundingBoxForView (view, &bbox);
        
    int width = csQint(bbox.MaxX() - bbox.MinX());
    int height = csQint(bbox.MaxY() - bbox.MinY());
    views[0]->SetRectangle((int)bbox.MinX(), (int)bbox.MinY(), width / 2, height);
    views[0]->GetCamera()->SetViewportSize (width, height);
    views[1]->GetCamera()->SetViewportSize (width, height);
    views[1]->SetRectangle((int)bbox.MinX() + (width / 2), (int)bbox.MinY(), 
                                    width / 2, height);
    split = (view == views[0]) ? 0 : 1;
    walktest->Report(CS_REPORTER_SEVERITY_NOTIFY, "Splitting to 2 views");
    return true;
  }
  return false;
}

bool WalkTestViews::UnsplitView ()
{
  if (split != -1)
  {
    csBox2 bbox1, bbox2;
    BoundingBoxForView(views[0], &bbox1);
    BoundingBoxForView(views[1], &bbox2);

    int width = csQint(bbox2.MaxX() - bbox1.MinX());
    int height = csQint(bbox1.MaxY() - bbox1.MinY());
    view->GetCamera()->SetViewportSize (width, height);
    view->SetRectangle((int)bbox1.MinX(), (int)bbox1.MinY(), width, height);
    split = -1;
    walktest->Report(CS_REPORTER_SEVERITY_NOTIFY, "Unsplitting view");
    return true;
  }
  return false;
}

bool WalkTestViews::ToggleView ()
{
  if (split != -1)
  {
    split = (split + 1) % 2;
    view = views[split];
    walktest->Report(CS_REPORTER_SEVERITY_NOTIFY, "Switching to view %d", split);
    return true;
  }
  return false;
}

void WalkTestViews::Draw ()
{
  iRenderManager* rm = walktest->Engine->GetRenderManager ();
  if (split == -1)
    rm->RenderView (view);
  else 
  {	
    rm->RenderView (views[0]);
    rm->RenderView (views[1]);
  }
}

