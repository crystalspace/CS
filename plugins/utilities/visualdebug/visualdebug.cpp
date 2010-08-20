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
#include "csqint.h"
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

  void VisualDebugger::DebugTransform (const csReversibleTransform& transform,
				       bool persist, float size)
  {
    TransformData data;
    data.transform = transform;
    data.persist = persist;
    data.size = size;
    transforms.Push (data);
  }

  void VisualDebugger::DebugPosition (const csVector3& position,
				      bool persist,
				      csColor color,
				      size_t size)
  {
    PositionData data;
    data.position = position;
    data.persist = persist;
    data.color = color;
    data.size = size;
    positions.Push (data);
  }

  void VisualDebugger::Display (iView* view)
  {
    iGraphics3D* g3d = view->GetContext ();
    iGraphics2D* g2d = g3d->GetDriver2D ();
    csTransform tr_w2c = view->GetCamera ()->GetTransform ();
    int fov = g2d->GetHeight ();

    if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
      return;

    // Display transforms
    size_t index = transforms.GetSize () - 1;
    for (csArray<TransformData>::ReverseIterator it = transforms.GetReverseIterator ();
	 it.HasNext (); )
    {
      TransformData& transform = it.Next ();

      csVector3 origin = transform.transform.GetOrigin ();
      csVector3 end = origin + transform.transform.This2OtherRelative
	(csVector3 (transform.size, 0.0f, 0.0f));
      int color = g2d->FindRGB (255, 0, 0);
      g3d->DrawLine (tr_w2c * origin, tr_w2c * end, fov, color);

      end = origin + transform.transform.This2OtherRelative (csVector3 (0.0f, transform.size, 0.0f));
      color = g2d->FindRGB (0, 255, 0);
      g3d->DrawLine (tr_w2c * origin, tr_w2c * end, fov, color);

      end = origin + transform.transform.This2OtherRelative (csVector3 (0.0f, 0.0f, transform.size));
      color = g2d->FindRGB (0, 0, 255);
      g3d->DrawLine (tr_w2c * origin, tr_w2c * end, fov, color);

      if (!transform.persist)
	transforms.DeleteIndex (index);

      index--;
    }

    // Display positions
    index = positions.GetSize () - 1;
    for (csArray<PositionData>::ReverseIterator it = positions.GetReverseIterator ();
	 it.HasNext (); )
    {
      PositionData& positionData = it.Next ();
      int color = g2d->FindRGB (255.0f * positionData.color[0],
				255.0f * positionData.color[1],
				255.0f * positionData.color[2]);
      csVector3 position = tr_w2c * positionData.position;

      float x1 = position.x, y1 = position.y, z1 = position.z;
      float iz1 = fov / z1;
      int px1 = csQint (x1 * iz1 + (g2d->GetWidth ()  / 2));
      int py1 = g2d->GetHeight () - 1 - csQint (y1 * iz1 + (g2d->GetHeight () / 2));
 
      for (size_t i = 0; i < positionData.size; i++)
	for (size_t j = 0; j < positionData.size; j++)
	  g3d->GetDriver2D ()->DrawPixel (px1 - positionData.size / 2 + i,
					  py1 - positionData.size / 2 + j,
					  color);

      if (!positionData.persist)
	positions.DeleteIndex (index);

      index--;
    }
  }

}
CS_PLUGIN_NAMESPACE_END(VisualDebug)
