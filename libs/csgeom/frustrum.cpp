/*
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
#include "csgeom/frustrum.h"
#include "csgeom/transfrm.h"

void csFrustrum::Clear ()
{
  CHK (delete [] vertices); vertices = NULL;
  num_vertices = max_vertices = 0;
  CHK (delete backplane); backplane = NULL;
  wide = false;
  mirrored = false;
}

csFrustrum::csFrustrum (csVector3& o, csVector3* verts, int num_verts, csPlane* backp)
{
  origin = o;
  num_vertices = num_verts;
  max_vertices = num_verts;
  wide = false;
  mirrored = false;
  if (verts)
  {
    CHK (vertices = new csVector3 [max_vertices]);
    memcpy (vertices, verts, sizeof (csVector3) * num_vertices);
  }
  else vertices = NULL;

  if (backp)
  {
    CHK (backplane = new csPlane (*backp));
  }
  else backplane = NULL;
}

csFrustrum::csFrustrum (const csFrustrum &copy)
{
  origin = copy.origin;
  num_vertices = copy.num_vertices;
  max_vertices = copy.max_vertices;
  wide = copy.wide;
  mirrored = copy.mirrored;
  if (copy.vertices)
  {
    CHK (vertices = new csVector3 [max_vertices]);
    memcpy (vertices, copy.vertices, sizeof (csVector3) * num_vertices);
  }
  else vertices = NULL;

  if (copy.backplane)
  {
    CHK (backplane = new csPlane (*copy.backplane));
  }
  else backplane = NULL;
}

void csFrustrum::SetBackPlane (csPlane& plane)
{
  CHK (delete backplane);
  CHK (backplane = new csPlane (plane));
}

void csFrustrum::RemoveBackPlane ()
{
  CHK (delete backplane); backplane = NULL;
}

void csFrustrum::ExtendVertexArray (int num)
{
  CHK (csVector3* new_vertices = new csVector3 [max_vertices+num]);
  if (vertices)
  {
    memcpy (new_vertices, vertices, sizeof (csVector3)*num_vertices);
    CHK (delete [] vertices);
  }
  vertices = new_vertices;
  max_vertices += num;
}

void csFrustrum::AddVertex (const csVector3& v)
{
  if (num_vertices >= max_vertices) ExtendVertexArray (10);
  vertices[num_vertices] = v;
  num_vertices++;
}

void csFrustrum::MakeInfinite ()
{
  Clear ();
  wide = true;
}

void csFrustrum::MakeEmpty ()
{
  Clear ();
  wide = false;
}

void csFrustrum::Transform (csTransform* trans)
{
  int i;
  origin = trans->Other2This (origin);
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i] = trans->Other2ThisRelative (vertices[i]);
  if (backplane) (*backplane) *= (*trans);
}

void csFrustrum::ClipPolyToPlane (csPlane* plane)
{
  // First classify all vertices of the current polygon with regards to this
  // plane.
  int i, i1;

  bool front[100];	// @@@ Hard coded limit.
  int count_front = 0;
  for (i = 0 ; i < num_vertices ; i++)
  {
    front[i] = csMath3::Visible (vertices[i], *plane);
    if (front[i]) count_front++;
  }
  if (count_front == 0)
  {
    // None of the vertices of the new polygon are in front of the back
    // plane of this frustrum. So intersection is empty.
    MakeEmpty ();
    return;
  }
  if (count_front == num_vertices)
  {
    // All vertices are in front. So nothing happens.
    return;
  }

  // Some of the vertices are in front, others are behind. So we
  // need to do real clipping.
  bool zs, z1s;
  csVector3 clipped_verts[100];	// @@@ Hard coded limit.
  int num_clipped_verts = 0;

  float A = plane->A ();
  float B = plane->B ();
  float C = plane->C ();
  float D = plane->D ();
  float r;
  i1 = num_vertices-1;
  num_clipped_verts = 0;
  for (i = 0 ; i < num_vertices ; i++)
  {
    zs = !front[i];
    z1s = !front[i1];

    if (z1s && !zs)
    {
      csIntersect3::Plane (vertices[i1], vertices[i], A, B, C, D,
      	clipped_verts[num_clipped_verts], r);
      num_clipped_verts++;
      clipped_verts[num_clipped_verts++] = vertices[i];
    }
    else if (!z1s && zs)
    {
      csIntersect3::Plane (vertices[i1], vertices[i], A, B, C, D,
      	clipped_verts[num_clipped_verts], r);
      num_clipped_verts++;
    }
    else if (!z1s && !zs)
    {
      clipped_verts[num_clipped_verts] = vertices[i];
      num_clipped_verts++;
    }
    i1 = i;
  }
  // Copy the clipped vertices. @@@ Is this efficient? Can't we clip in place?
  if (num_clipped_verts >= max_vertices) ExtendVertexArray (num_clipped_verts-max_vertices+2);
  num_vertices = num_clipped_verts;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i] = clipped_verts[i];
}

void csFrustrum::ClipToPlane (csVector3& v1, csVector3& v2)
{
  int cw_offset = -1;
  int ccw_offset;
  bool first_vertex_side;
  csVector3 isect_cw, isect_ccw;
  csVector3 Plane_Normal;
  int i;

  // Make sure that we have space in the array for at least three extra
  // vertices.
  if (num_vertices >= max_vertices-3) ExtendVertexArray (3);

  // Do the check only once at the beginning instead of twice during the routine.
  if (mirrored)
    Plane_Normal = v2%v1;
  else
    Plane_Normal = v1%v2;

  // On which side is the first vertex?
  first_vertex_side = (Plane_Normal*(vertices[num_vertices -1] - v1) > 0);

  for (i = 0; i < num_vertices - 1; i++)
  {
    if ( (Plane_Normal*(vertices[i] - v1) > 0) != first_vertex_side)
    {
      cw_offset = i;
      break;
    }
  }

  if (cw_offset == -1)
  {
    // Return, if there is no intersection.
    if (first_vertex_side)
      MakeEmpty ();      // The whole polygon is behind the plane because the first is.
    return;
  }

  for (ccw_offset = num_vertices -2; ccw_offset >= 0; ccw_offset--)
  {
    if ((Plane_Normal*(vertices[ccw_offset] - v1) > 0) != first_vertex_side)
      break;
  }

  // Calculate the intersection points.
  i = cw_offset - 1;
  if (i < 0) i = num_vertices-1;
  csIntersect3::Plane (vertices[cw_offset], vertices[i], Plane_Normal, v1, isect_cw);
  csIntersect3::Plane (vertices[ccw_offset], vertices[ccw_offset + 1], Plane_Normal, v1, isect_ccw);

  // Remove the obsolete point and insert the intersection points.
  if (first_vertex_side)
  {
    for (i = 0; i < ccw_offset - cw_offset + 1; i++)
      vertices[i] = vertices[i + cw_offset];
    vertices[i] = isect_ccw;
    vertices[i+1] = isect_cw;
    num_vertices = 3 + ccw_offset - cw_offset;
  }
  else
  {
    if (cw_offset + 1 < ccw_offset)
      for (i = 0; i < num_vertices - ccw_offset - 1; i++)
        vertices[cw_offset + 2 + i] = vertices[ccw_offset + 1 + i];
    else if (cw_offset + 1 > ccw_offset)
      for (i = num_vertices - 2 - ccw_offset;i >= 0; i--)
        vertices[cw_offset + 2 + i] = vertices[ccw_offset + 1 + i];

    vertices[cw_offset] = isect_cw;
    vertices[cw_offset+1] = isect_ccw;
    num_vertices = 2 + cw_offset + num_vertices - ccw_offset - 1;
  }
}

csFrustrum* csFrustrum::Intersect (const csFrustrum& other)
{
  if (other.IsEmpty ()) return NULL;
  if (other.IsInfinite ()) { CHK (csFrustrum* f = new csFrustrum (*this)); return f; }
  return Intersect (other.vertices, other.num_vertices);
}

csFrustrum* csFrustrum::Intersect (csVector3* poly, int num)
{
  csFrustrum* new_frustrum;
  if (IsInfinite ())
  {
    // If this frustrum is infinite then the intersection of this
    // frustrum with the other is equal to the other.
    CHK (new_frustrum = new csFrustrum (origin, poly, num));
    new_frustrum->SetMirrored (IsMirrored ());
  }
  else if (IsEmpty ())
  {
    // If this frustrum is empty then the intersection will be empty
    // as well.
    return NULL;
  }
  else
  {
    // General case. Create a new frustrum from the given polygon with
    // the origin of this frustrum and clip it to every plane from this
    // frustrum.
    CHK (new_frustrum = new csFrustrum (GetOrigin (), poly, num));
    new_frustrum->SetMirrored (IsMirrored ());
    int i, i1;
    i1 = num_vertices-1;
    for (i = 0 ; i < num_vertices ; i++)
    {
      new_frustrum->ClipToPlane (vertices[i1], vertices[i]);
      if (new_frustrum->IsEmpty ())
      {
        // Intersection has become empty. Return NULL.
	CHK (delete new_frustrum);
	return NULL;
      }
      i1 = i;
    }

    // If this frustrum has a back plane then we also need to clip the polygon
    // in the new frustrum to that.
    if (backplane)
    {
      new_frustrum->ClipPolyToPlane (backplane);
      if (new_frustrum->IsEmpty ())
      {
        // Intersection has become empty. Return NULL.
	CHK (delete new_frustrum);
	return NULL;
      }
    }
  }
  return new_frustrum;
}

bool csFrustrum::Contains (const csVector3& point)
{
  if (backplane)
  {
    if (!csMath3::Visible (point, *backplane)) return false;
  }
  int i, i1;
  i1 = num_vertices-1;
  for (i = 0 ; i < num_vertices ; i++)
  {
    if (csMath3::WhichSide3D (point, vertices[i], vertices[i1]) > 0) return false;
    i1 = i;
  }
  return true;
}

