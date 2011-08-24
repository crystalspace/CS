/*
    Copyright (C) 2000 by Jorrit Tyberghein
		      (C) 2007 by Scott Johnson

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
#include "igeom/clip2d.h"
#include <math.h>
#include "csgeom/plane3.h"
#include "csgeom/math3d.h"
#include "csutil/csstring.h"
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
  switch (norm.DominantAxis())
  {
    case CS_AXIS_X: return csVector3 (-DD / norm.x, 0, 0);
    case CS_AXIS_Y: return csVector3 (0, -DD / norm.y, 0);
    default:
      CS_ASSERT(false);
    case CS_AXIS_Z: return csVector3 (0, 0, -DD / norm.z);
  }
}

#define CS_INV_SQRT2 float(0.7071067811865475244008443621048490)

void csPlane3::FindOrthogonalPoints (const csVector3& norm,
    csVector3& p, csVector3& q)
{
  if (fabs (norm.z) > CS_INV_SQRT2)
  {
    // Choose p in y-z plane
    float a = norm.y*norm.y + norm.z*norm.z;
    float k = 1.0f / sqrt (a);
    p.Set (0, -norm.z*k, norm.y*k);
    // Set q = norm x p
    q.Set (a*k, -norm.x*p.z, norm.x*p.y);
  }
  else
  {
    // Choose p in x-y plane
    float a = norm.x*norm.x + norm.y*norm.y;
    float k = 1.0f / sqrt (a);
    p.Set (-norm.y*k, norm.x*k, 0);
    // Set q = n x p
    q.Set (-norm.z*p.y, norm.z*p.x, a*k);
  }
}

csVector3 csPlane3::ProjectOnto(const csVector3& p)
{
	// make sure our normal is actually normalized
	Normalize();

	// since a plane in 3-space is infinite, the normal of the
	// plane gives us a directional ray from the point p to the plane
	// we need to find the distance between p and the plane
	// create a vector triDist which is equivalent to a vector from a point
	// on the plane to p
	csVector3 triDist =  p - FindPoint();

	// now, the distance from p to the plane is equivalent to the absolute
	// value of the scalar projection of triDist onto the normal of the plane
	// equation for scalar projection of b onto a = (a . b) / norm(a)
	// so we want (n . triDist) / norm(n)
	float aDOTb = Normal() * triDist;
	float distance = aDOTb / csVector3::Norm(Normal());
	distance = fabs(distance);

	// finally, we need to multiply this with the normal of the plane, and
	// add the resulting vector to our current vector, to get our projection
	csVector3 result;

	if (Classify(p) > 0)
	{
		result = (-distance * Normal()) + p;
	}

	else
	{
		result = (distance * Normal()) + p;
	}

	return result;

}


namespace
{
  typedef csDirtyAccessArray<csVector3> csgeom_csPlane3_Verts;
  typedef csDirtyAccessArray<bool> csgeom_csPlane3_Vis;
  CS_IMPLEMENT_STATIC_VAR (GetStatic_csgeom_csPlane3_Verts, csgeom_csPlane3_Verts,())
  CS_IMPLEMENT_STATIC_VAR (GetStatic_csgeom_csPlane3_Vis, csgeom_csPlane3_Vis,())
}

bool csPlane3::ClipPolygon (
  csVector3 * &pverts,
  int &num_verts,
  bool reversed)
{
  int i, i1, num_vertices = num_verts, cnt_vis = 0;
  bool zs, z1s;
  float r;
  csgeom_csPlane3_Verts *verts = GetStatic_csgeom_csPlane3_Verts ();
  csgeom_csPlane3_Vis *vis = GetStatic_csgeom_csPlane3_Vis ();

  if (!reversed) Invert ();

  // Check if the static output array is large enought to hold
  // the cliped polygon. If only one point is outside
  // the total size will increase by one.
  if ((size_t)num_verts >= verts->GetSize ())
  {
    verts->SetSize (num_verts+1);
    vis->SetSize (num_verts+1);
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

uint8 csPlane3::ClipPolygon (const csVector3* InVerts, size_t InCount,
                             csVector3* OutPolygon, size_t& OutCount,
                             csVertexStatus* OutStatus, bool reversed) const
{
  size_t i, i1, num_vertices = InCount, cnt_vis = 0;
  bool zs, z1s;
  csgeom_csPlane3_Vis *vis = GetStatic_csgeom_csPlane3_Vis ();
  csPlane3 clipPlane (*this);
  if (!reversed) clipPlane.Invert ();

  if (InCount > vis->GetSize ())
  {
    vis->SetSize (InCount);
  }

  for (i = 0; i < InCount; i++)
  {
    (*vis)[i] = clipPlane.Classify (InVerts[i]) >= 0;
    if ((*vis)[i]) cnt_vis++;
  }

  if (cnt_vis == 0)
  {
    return CS_CLIP_OUTSIDE; // Polygon is not visible.
  }

  // If all vertices are visible then everything is ok.
  if (cnt_vis == num_vertices)
  {
    return CS_CLIP_INSIDE;
  }

  // We really need to clip.
  size_t num_verts = 0;

  i1 = num_vertices - 1;

  for (i = 0; i < num_vertices; i++)
  {
    float r;
    csVector3 v;
    zs = (*vis)[i];
    z1s = (*vis)[i1];

    if (!z1s && zs)
    {
      csIntersect3::SegmentPlane (InVerts[i1], InVerts[i], clipPlane,
      	v, r);
      if (OutStatus && (num_verts < OutCount))
      {
        OutStatus->Type = CS_VERTEX_ONEDGE;
        OutStatus->Vertex = i1;
        OutStatus->Pos = r;
        OutStatus++;
      }
      if (OutPolygon && (num_verts < OutCount)) *OutPolygon++ = v;
      num_verts++;
      if (OutStatus && (num_verts < OutCount))
      {
        OutStatus->Type = CS_VERTEX_ORIGINAL;
        OutStatus->Vertex = i;
        OutStatus++;
      }
      if (OutPolygon && (num_verts < OutCount)) *OutPolygon++ = InVerts[i];
      num_verts++;
    }
    else if (z1s && !zs)
    {
      csIntersect3::SegmentPlane (InVerts[i1], InVerts[i], clipPlane,
      	v, r);
      if (OutStatus && (num_verts < OutCount))
      {
        OutStatus->Type = CS_VERTEX_ONEDGE;
        OutStatus->Vertex = i1;
        OutStatus->Pos = r;
        OutStatus++;
      }
      if (OutPolygon && (num_verts < OutCount)) *OutPolygon++ = v;
      num_verts++;
    }
    else if (z1s && zs)
    {
      if (OutStatus && (num_verts < OutCount))
      {
        OutStatus->Type = CS_VERTEX_ORIGINAL;
        OutStatus->Vertex = i;
        OutStatus++;
      }
      if (OutPolygon && (num_verts < OutCount)) *OutPolygon++ = InVerts[i];
      num_verts++;
    }

    i1 = i;
  }

  OutCount = num_verts;
  return CS_CLIP_CLIPPED;
}

csString csPlane3::Description() const
{
  csString s;
  s.Format("%g,%g,%g,%g", norm.x, norm.y, norm.z, DD);
  return s;
}

//---------------------------------------------------------------------------
