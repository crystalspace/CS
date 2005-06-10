/*
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "lmmapper.h"


litLightMapMapper::litLightMapMapper ()
{
}

void litLightMapMapper::InitializeMapping (const csArray<csVector3>& poly,
  	const csPlane3& plane, float density)
{
  litLightMapMapper::density = density;

  csVector3 abs_normal (fabs (plane.A ()),
			fabs (plane.B ()),
			fabs (plane.C ()));

  // Find the axis from where we can see most of the polygon. i.e. the
  // axis that maximizes the visible lightmap space.
  if (abs_normal.x >= abs_normal.y && abs_normal.x >= abs_normal.z)
    axis = LIT_AXIS_X;
  else if (abs_normal.y >= abs_normal.x && abs_normal.y >= abs_normal.z)
    axis = LIT_AXIS_Y;
  else
    axis = LIT_AXIS_Z;

  // Set the offset to 0,0 to start with. That way we can already use
  // PolyToLM() function.
  offset.Set (0, 0);
  // Now find the minimum u and v.
  csVector2 min_uv(0,0);
  PolyToLM (poly[0], min_uv);
  size_t i;
  for (i = 1 ; i < poly.Length () ; i++)
  {
    csVector2 uv(0,0);
    PolyToLM (poly[i], uv);
    if (uv.x < min_uv.x) uv.x = min_uv.x;
    if (uv.y < min_uv.y) uv.y = min_uv.y;
  }

  // Make sure we have a properly aligned uv-mapping (aligning with
  // adjacent polygons for example):
  offset.x = floor (min_uv.x);
  offset.y = floor (min_uv.y);
}

void litLightMapMapper::PolyToLM (const csVector3& vpoly, csVector2& vlm)
{
  switch (axis)
  {
    case LIT_AXIS_X:
      vlm.x = vpoly.z * density - offset.x;
      vlm.y = -vpoly.y * density - offset.y;
      break;
    case LIT_AXIS_Y:
      vlm.x = vpoly.x * density - offset.x;
      vlm.y = -vpoly.z * density - offset.y;
      break;
    case LIT_AXIS_Z:
      vlm.x = vpoly.x * density - offset.x;
      vlm.y = -vpoly.y * density - offset.y;
      break;
  }
}

void litLightMapMapper::LMToPoly (const csVector2& vlm, csVector3& vpoly)
{
  // @@@ TODO
}
