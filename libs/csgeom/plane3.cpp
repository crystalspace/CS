/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include <math.h>
#include "csgeom/plane3.h"
#include "csgeom/math3d.h"
#include "csutil/dirtyaccessarray.h"

//---------------------------------------------------------------------------
csPlane3::csPlane3 (
  const csVector3 &v1,
  const csVector3 &v2,
  const csVector3 &v3)
{
  csMath3::CalcNormal (norm, v1, v2, v3);
  DD = -norm * v1;
}

void csPlane3::Set (
  const csVector3 &v1,
  const csVector3 &v2,
  const csVector3 &v3)
{
  csMath3::CalcNormal (norm, v1, v2, v3);
  DD = -norm * v1;
}

csVector3 csPlane3::FindPoint () const
{
  if (norm.x >= norm.y && norm.x >= norm.z)
  {
    return csVector3 (-DD / norm.x, 0, 0);
  }
  else if (norm.y >= norm.x && norm.y >= norm.z)
  {
    return csVector3 (0, -DD / norm.y, 0);
  }
  else
  {
    return csVector3 (0, 0, -DD / norm.z);
  }
}

typedef csDirtyAccessArray<csVector3> csgeom_csPlane3_Verts;
typedef csDirtyAccessArray<bool> csgeom_csPlane3_Vis;
CS_IMPLEMENT_STATIC_VAR (GetStatic_csgeom_csPlane3_Verts, csgeom_csPlane3_Verts,())
CS_IMPLEMENT_STATIC_VAR (GetStatic_csgeom_csPlane3_Vis, csgeom_csPlane3_Vis,())

bool csPlane3::ClipPolygon (
  csVector3 * &pverts,
  int &num_verts,
  bool reversed)
{
  int i, i1, num_vertices = num_verts, cnt_vis = 0;
  bool zs, z1s;
  float r;
  static csgeom_csPlane3_Verts *verts = GetStatic_csgeom_csPlane3_Verts ();
  static csgeom_csPlane3_Vis *vis = GetStatic_csgeom_csPlane3_Vis ();

  if (!reversed) Invert ();
  if ((size_t)num_verts > verts->Capacity ())
  {
    verts->SetCapacity (num_verts);
    vis->SetCapacity (num_verts);
  }

  for (i = 0; i < num_vertices; i++)
  {
    (*vis)[i] = Classify (pverts[i]) >= 0;
    if ((*vis)[i]) cnt_vis++;
  }

  if (cnt_vis == 0)
  {
    if (!reversed) Invert ();
    return false; // Polygon is not visible.
  }

  // If all vertices are visible then everything is ok.
  if (cnt_vis == num_vertices)
  {
    num_verts = num_vertices;
    if (!reversed) Invert ();
    return true;
  }

  // We really need to clip.
  num_verts = 0;

  i1 = num_vertices - 1;

  for (i = 0; i < num_vertices; i++)
  {
    zs = (*vis)[i];
    z1s = (*vis)[i1];

    if (!z1s && zs)
    {
      csIntersect3::SegmentPlane (pverts[i1], pverts[i], *this,
      	(*verts)[num_verts], r);
      num_verts++;
      (*verts)[num_verts++] = pverts[i];
    }
    else if (z1s && !zs)
    {
      csIntersect3::SegmentPlane (pverts[i1], pverts[i], *this,
      	(*verts)[num_verts], r);
      num_verts++;
    }
    else if (z1s && zs)
    {
      (*verts)[num_verts++] = pverts[i];
    }

    i1 = i;
  }

  pverts = verts->GetArray ();
  if (!reversed) Invert ();
  return true;
}

csString csPlane3::Description() const
{
  csString s;
  s.Format("%g,%g,%g,%g", norm.x, norm.y, norm.z, DD);
  return s;
}

//---------------------------------------------------------------------------
