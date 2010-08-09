/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
  Sensing Laboratory of the School of Engineering at the 
  Universite catholique de Louvain, Belgium
  http://www.tele.ucl.ac.be

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
#include "csutil/scf.h"

#include "visualdebug.h"
#include "csgeom/transfrm.h"
#include "iengine/camera.h"
#include "ivaria/view.h"
#include "ivideo/graph2d.h"

CS_PLUGIN_NAMESPACE_BEGIN(VisualDebug)
{

  SCF_IMPLEMENT_FACTORY(VisualDebugger);

  CS_LEAKGUARD_IMPLEMENT(VisualDebugger);

  VisualDebugger::VisualDebugger (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  bool VisualDebugger::Initialize (iObjectRegistry* object_reg)
  {
    this->object_reg = object_reg;
    return true;
  }

  void VisualDebugger::DebugTransform (csReversibleTransform& transform)
  {
    transforms.Push (transform);
  }

  void VisualDebugger::Display (iView* view, float axisSize)
  {
    iGraphics3D* g3d = view->GetContext ();
    iGraphics2D* g2d = g3d->GetDriver2D ();
    csTransform tr_w2c = view->GetCamera ()->GetTransform ();
    int fov = g2d->GetHeight ();

    if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
      return;

    for (csArray<csReversibleTransform>::Iterator it = transforms.GetIterator (); it.HasNext (); )
    {
      csReversibleTransform& transform = it.Next ();

      csVector3 origin = transform.GetOrigin ();
      csVector3 end = origin + transform.This2OtherRelative (csVector3 (axisSize, 0.0f, 0.0f));
      int color = g2d->FindRGB (255, 0, 0);
      g3d->DrawLine (tr_w2c * origin, tr_w2c * end, fov, color);

      end = origin + transform.This2OtherRelative (csVector3 (0.0f, axisSize, 0.0f));
      color = g2d->FindRGB (0, 255, 0);
      g3d->DrawLine (tr_w2c * origin, tr_w2c * end, fov, color);

      end = origin + transform.This2OtherRelative (csVector3 (0.0f, 0.0f, axisSize));
      color = g2d->FindRGB (0, 0, 255);
      g3d->DrawLine (tr_w2c * origin, tr_w2c * end, fov, color);
    }

    transforms.DeleteAll ();
  }

}
CS_PLUGIN_NAMESPACE_END(VisualDebug)
