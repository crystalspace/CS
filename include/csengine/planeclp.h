/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __CS_PLANECLP_H__
#define __CS_PLANECLP_H__

#include "csgeom/math3d.h"
#include "csutil/garray.h"

/**
 * A special version of csPlane which can clip polygons.
 */
class csPlaneClip : public csPlane
{
public:
  /// Initialize the plane.
  csPlaneClip (const csVector3& plane_norm, float d = 0) :
    csPlane(plane_norm, d) { init(100); }

  /// Initialize the plane
  csPlaneClip (float d = 0) : csPlane(0, 0, -1, d) { init(100); }

  /// Initialize the plane.
  csPlaneClip (float a, float b, float c, float d = 0) :
    csPlane(a,b,c,d) { init(100); }

  /**
   * Clip the polygon in pverts (having num_verts vertices) to this plane.
   * The vertices are expected in camera space.
   * Method returns true if there is something visible, false otherwise.
   */
  bool ClipPolygon (csVector3*& pverts, int& num_verts )
  {
    int i,i1, num_vertices = num_verts, cnt_vis = 0;
    bool zs, z1s;
    float r;

    if (num_verts > verts.GetLimit())
      init(num_verts);

    for (i = 0 ; i < num_vertices ; i++)
    {
      vis[i] = Classify (pverts[i]) >= 0;
      if (vis[i])
        cnt_vis++;
    }

    if (cnt_vis == 0)
      return false; // Polygon is not visible.

    // If all vertices are visible then everything is ok.
    if (cnt_vis == num_vertices)
    {
      num_verts = num_vertices;
      return true;
    }

    // We really need to clip.
    num_verts = 0;

    i1 = num_vertices - 1;

    for (i = 0 ; i < num_vertices ; i++)
    {
      zs = vis[i];
      z1s = vis[i1];

      if (!z1s && zs)
      {
	csIntersect3::Plane(pverts[i1], pverts[i], *this, verts[num_verts], r);
	num_verts++;
	verts[num_verts++] = pverts[i];
      }
      else if (z1s && !zs)
      {
	csIntersect3::Plane(pverts[i1], pverts[i], *this, verts[num_verts], r);
	num_verts++;
      }
      else if (z1s && zs)
      {
	verts[num_verts++] = pverts[i];
      }
      i1 = i;
    }
    pverts = verts.GetArray();
    return true;
  }

protected:
  DECLARE_GROWING_ARRAY(verts, csVector3);
  DECLARE_GROWING_ARRAY(vis, bool);
  void init (int len) { verts.SetLimit (len); vis.SetLimit (len); }
};

#endif // __CS_PLANECLP_H__
