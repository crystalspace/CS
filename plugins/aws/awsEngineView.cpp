/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "awsEngineView.h"
#include "ivaria/view.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

awsEngineView::awsEngineView () : view (0)
{
}

awsEngineView::~awsEngineView ()
{
  if (view) view->DecRef ();
}

bool awsEngineView::SetProperty (const char* name, intptr_t parm)
{
  if (strcmp (name, "view") == 0)
  {
    if (view)
      view->DecRef ();
    view = (iView*) parm;
    if (view)
      view->IncRef ();
    return true;
  }
  else
    return awsComponent::SetProperty (name, parm);
}

bool awsEngineView::GetProperty (const char* name, intptr_t *parm)
{
  if (strcmp (name, "view") == 0)
  {
    *parm = (intptr_t) view;
    return true;
  }
  else
    return awsComponent::GetProperty (name, parm);
}

const char* awsEngineView::Type ()
{
  return "Engine View";
}

void awsEngineView::OnDraw (csRect clip)
{
  if (view)
  {
    iGraphics3D* g3d = WindowManager ()->G3D ();
    iGraphics3D* og3d = view->GetContext ();
    view->SetContext (g3d);

    view->SetRectangle (
      Frame ().xmin,
      g3d->GetHeight () - Frame ().ymax,
      Frame ().Width (),
      Frame ().Height ());
    view->GetCamera ()->SetPerspectiveCenter (
      Frame ().xmin + (Frame ().Width () >> 1),
      (g3d->GetHeight () - Frame ().Height () - Frame ().ymin) +
      (Frame ().Height () >> 1));
    //view->GetCamera ()->SetFOV (
      //view->GetCamera ()->GetFOV (),
      //Frame ().Width ());
    view->GetCamera ()->SetFOV (
      Frame ().Height (),
      Frame ().Width ());
    g3d->BeginDraw (
    	view->GetEngine ()->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS);
    view->Draw ();
    g3d->BeginDraw (CSDRAW_2DGRAPHICS);
    view->SetContext (og3d);
  }
}

awsEngineViewFactory::awsEngineViewFactory (iAws* wmgr)
  : awsComponentFactory (wmgr)
{
  Register ("Engine View");
}

awsEngineViewFactory::~awsEngineViewFactory ()
{
}

iAwsComponent* awsEngineViewFactory::Create ()
{
  return (new awsEngineView ())->GetComponent ();
}
