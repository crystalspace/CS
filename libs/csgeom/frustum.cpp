/*
  Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "csgeom/frustum.h"
#include "csgeom/transfrm.h"

csFrustum::csFrustum (const csVector3& o, csVector3* verts, int num_verts,
  csPlane3* backp) : pool (&csDefaultVertexArrayPool::GetDefaultPool())
{
  origin = o;
  num_vertices = num_verts;
  max_vertices = num_verts;
  wide = false;
  mirrored = false;
  ref_count = 1;

  if (verts)
  {
    vertices = pool->GetVertexArray (max_vertices);
    memcpy (vertices, verts, sizeof (csVector3) * num_vertices);
  }
  else
    vertices = NULL;

  backplane = backp ? new csPlane3 (*backp) : NULL;
}

csFrustum::csFrustum (const csVector3& o, int num_verts, csVertexArrayPool* pl,
  csPlane3* backp) : pool (pl)
{
  origin = o;
  num_vertices = num_verts;
  max_vertices = num_verts;
  wide = false;
  mirrored = false;
  ref_count = 1;

  vertices = pool->GetVertexArray (max_vertices);
  backplane = backp ? new csPlane3 (*backp) : NULL;
}

csFrustum::csFrustum (const csFrustum &copy)
{
  pool = copy.pool;
  origin = copy.origin;
  num_vertices = copy.num_vertices;
  max_vertices = copy.max_vertices;
  wide = copy.wide;
  mirrored = copy.mirrored;
  ref_count = 1;

  if (copy.vertices)
  {
    vertices = pool->GetVertexArray (max_vertices);
    memcpy (vertices, copy.vertices, sizeof (csVector3) * num_vertices);
  }
  else
    vertices = NULL;

  backplane = copy.backplane ? new csPlane3 (*copy.backplane) : NULL;
}

csFrustum::~csFrustum ()
{
  if (ref_count != 1)
  {
    printf ("###### Ref_count wrong!\n");
    DEBUG_BREAK;
  }
  Clear ();
}

void csFrustum::Clear ()
{
  pool->FreeVertexArray (vertices, max_vertices);
  vertices = NULL;
  num_vertices = max_vertices = 0;
  delete backplane; backplane = NULL;
  wide = false;
  mirrored = false;
}

void csFrustum::SetBackPlane (csPlane3& plane)
{
  delete backplane;
  backplane = new csPlane3 (plane);
}

void csFrustum::RemoveBackPlane ()
{
  delete backplane; backplane = NULL;
}

void csFrustum::ExtendVertexArray (int num)
{
  csVector3* new_vertices = pool->GetVertexArray (max_vertices+num);
  if (vertices)
  {
    memcpy (new_vertices, vertices, sizeof (csVector3)*num_vertices);
    pool->FreeVertexArray (vertices, max_vertices);
  }
  vertices = new_vertices;
  max_vertices += num;
}

void csFrustum::AddVertex (const csVector3& v)
{
  if (num_vertices >= max_vertices) ExtendVertexArray (10);
  vertices[num_vertices] = v;
  num_vertices++;
}

void csFrustum::MakeInfinite ()
{
  Clear ();
  wide = true;
}

void csFrustum::MakeEmpty ()
{
  Clear ();
  wide = false;
}

void csFrustum::Transform (csTransform* trans)
{
  int i;
  origin = trans->Other2This (origin);
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i] = trans->Other2ThisRelative (vertices[i]);
  if (backplane) (*backplane) *= (*trans);
}

void csFrustum::ClipPolyToPlane (csPlane3* plane)
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
    // plane of this frustum. So intersection is empty.
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
      if (csIntersect3::Plane (vertices[i1], vertices[i], A, B, C, D,
      	clipped_verts[num_clipped_verts], r))
        num_clipped_verts++;
      clipped_verts[num_clipped_verts++] = vertices[i];
    }
    else if (!z1s && zs)
    {
      if (csIntersect3::Plane (vertices[i1], vertices[i], A, B, C, D,
      	clipped_verts[num_clipped_verts], r))
        num_clipped_verts++;
    }
    else if (!z1s && !zs)
    {
      clipped_verts[num_clipped_verts] = vertices[i];
      num_clipped_verts++;
    }
    i1 = i;
  }

  // If we have too little vertices, make frustrum empty
  if (num_clipped_verts < 3)
  {
    MakeEmpty ();
    return;
  }

  // Copy the clipped vertices. @@@ Is this efficient? Can't we clip in place?
  if (num_clipped_verts >= max_vertices) ExtendVertexArray (num_clipped_verts-max_vertices+2);
  num_vertices = num_clipped_verts;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i] = clipped_verts[i];
}

void csFrustum::ClipToPlane (csVector3& v1, csVector3& v2)
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
  first_vertex_side = (Plane_Normal*vertices[num_vertices -1] > 0);

  for (i = 0; i < num_vertices - 1; i++)
  {
    if ( (Plane_Normal*vertices[i] > 0) != first_vertex_side)
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
    if ((Plane_Normal*vertices[ccw_offset] > 0) != first_vertex_side)
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

csFrustum* csFrustum::Intersect (const csFrustum& other)
{
  if (other.IsEmpty ()) return NULL;
  if (other.IsInfinite ()) { csFrustum* f = new csFrustum (*this); return f; }
  return Intersect (other.vertices, other.num_vertices);
}

csFrustum* csFrustum::Intersect (csVector3* poly, int num)
{
  csFrustum* new_frustum;
  if (IsInfinite ())
  {
    // If this frustum is infinite then the intersection of this
    // frustum with the other is equal to the other.
    new_frustum = new csFrustum (origin, poly, num);
    new_frustum->SetMirrored (IsMirrored ());
  }
  else if (IsEmpty ())
  {
    // If this frustum is empty then the intersection will be empty
    // as well.
    return NULL;
  }
  else
  {
    // General case. Create a new frustum from the given polygon with
    // the origin of this frustum and clip it to every plane from this
    // frustum.
    new_frustum = new csFrustum (GetOrigin (), poly, num);
    new_frustum->SetMirrored (IsMirrored ());
    int i, i1;
    i1 = num_vertices-1;
    for (i = 0 ; i < num_vertices ; i++)
    {
      new_frustum->ClipToPlane (vertices[i1], vertices[i]);
      if (new_frustum->IsEmpty ())
      {
        // Intersection has become empty. Return NULL.
	delete new_frustum;
	return NULL;
      }
      i1 = i;
    }

    // If this frustum has a back plane then we also need to clip the polygon
    // in the new frustum to that.
    if (backplane)
    {
      new_frustum->ClipPolyToPlane (backplane);
      if (new_frustum->IsEmpty ())
      {
        // Intersection has become empty. Return NULL.
	delete new_frustum;
	return NULL;
      }
    }
  }
  return new_frustum;
}

bool csFrustum::Contains (const csVector3& point)
{
  if (backplane)
    return Contains (vertices, num_vertices, *backplane, point);
  return Contains (vertices, num_vertices, point);
}

bool csFrustum::Contains (csVector3* frustum, int num_frust, const csVector3& point)
{
  int i, i1;
  i1 = num_frust-1;
  for (i = 0 ; i < num_frust ; i++)
  {
    if (csMath3::WhichSide3D (point, frustum[i], frustum[i1]) > 0) return false;
    i1 = i;
  }
  return true;
}

bool csFrustum::Contains (csVector3* frustum, int num_frust,
  const csPlane3& plane, const csVector3& point)
{
  if (!csMath3::Visible (point, plane)) return false;
  int i, i1;
  i1 = num_frust-1;
  for (i = 0 ; i < num_frust ; i++)
  {
    if (csMath3::WhichSide3D (point, frustum[i], frustum[i1]) > 0)
      return false;
    i1 = i;
  }
  return true;
}

int csFrustum::Classify (csVector3* frustum, int num_frust,
  csVector3* poly, int num_poly)
{
  // All poly vertices are inside frustum?
  bool all_inside = true;

  // Loop through all planes that makes the frustum
  for (int fv = 0, fvp = num_frust - 1; fv < num_frust; fvp = fv++)
  {
    // Find the equation of the Nth plane
    // Since the origin of frustum is at (0,0,0) the plane equation
    // has the form A*x + B*y + C*z = 0, where (A,B,C) is the plane normal.
    csVector3 &v1 = frustum [fvp];
    csVector3 &v2 = frustum [fv];
    csVector3 fn = v1 % v2;

    float prev_d = fn * poly [num_poly - 1];
    for (int pv = 0, pvp = num_poly - 1; pv < num_poly; pvp = pv++)
    {
      // The distance from plane to polygon vertex
      float d = fn * poly [pv];
      if (all_inside && d > 0) all_inside = false;

      if ((prev_d < 0 && d > 0)
       || (prev_d > 0 && d < 0))
      {
#if 1
	// This version should be faster.
	if (((poly [pvp] % v1) * poly [pv])*prev_d  >= 0)
	  if (((v2 % poly [pvp]) * poly [pv])*prev_d >= 0)
	    return CS_FRUST_PARTIAL;

	//float f1 = (poly [pvp] % v1) * poly [pv];
	//float f2 = (v2 % poly [pvp]) * poly [pv]; 
	//if (!(f1 > 0 || f2 > 0)
         //|| (f1 == 0) || (f2 == 0))
          //return CS_FRUST_PARTIAL;
#else
        // If the segment intersects with the frustum plane somewhere
        // between two limiting lines, we have the CS_FRUST_PARTIAL case.
        csVector3 p = poly [pvp] - (poly [pv] - poly [pvp])
		* (prev_d / (d - prev_d));
        // If the vector product between both vectors making the frustum
        // plane and the intersection point between polygon edge and plane
        // have same sign, the intersection point is outside frustum
        //@@@ This is the old code but I think this code
	// should test against the frustum.
	//--- if ((poly [pvp] % p) * (poly [pv] % p) < 0)
        if ((v1 % p) * (v2 % p) <= 0)
          return CS_FRUST_PARTIAL;
#endif
      }
      prev_d = d;
    }
  }

  if (all_inside) return CS_FRUST_INSIDE;

  // Now we know that all polygon vertices are outside frustum and no polygon
  // edge crosses the frustum. We should choose between CS_FRUST_OUTSIDE and
  // CS_FRUST_COVERED cases. For this we check if all frustum vertices are
  // inside the frustum created by polygon (reverse roles). In fact it is
  // enough to check just the first vertex of frustum is outside polygon
  // frustum: this lets us make the right decision.

  for (int pv = 0, pvp = num_poly - 1; pv < num_poly; pvp = pv++)
  {
    csVector3 pn = poly [pvp] % poly [pv];
    if (pn * frustum [0] >= 0)
      return CS_FRUST_OUTSIDE;
  }

  return CS_FRUST_COVERED;
}

/**
 * This is like the above version except that it takes a vector of precalculated frustum plane normals.
 * Use this if you have to classify a batch of polygons against the same frustum.
 */
int csFrustum::BatchClassify (csVector3* frustum, csVector3* frustumNormals, int num_frust,
  csVector3* poly, int num_poly)
{
  // All poly vertices are inside frustum?
  bool all_inside = true;

  // Loop through all planes that makes the frustum
  for (int fv = 0, fvp = num_frust - 1; fv < num_frust; fvp = fv++)
  {
    csVector3 &v1 = frustum [fvp];
    csVector3 &v2 = frustum [fv];
    csVector3 &fn = frustumNormals[fvp];

    float prev_d = fn * poly [num_poly - 1];
    for (int pv = 0, pvp = num_poly - 1; pv < num_poly; pvp = pv++)
    {
      // The distance from plane to polygon vertex
      float d = fn * poly [pv];
      if (all_inside && d > 0) all_inside = false;

      if ((prev_d < 0 && d > 0)
       || (prev_d > 0 && d < 0))
      {
#if 1
	// This version should be faster.
	if (((poly [pvp] % v1) * poly [pv])*prev_d  >= 0)
	  if (((v2 % poly [pvp]) * poly [pv])*prev_d >= 0)
	    return CS_FRUST_PARTIAL;

#else
        // If the segment intersects with the frustum plane somewhere
        // between two limiting lines, we have the CS_FRUST_PARTIAL case.
        csVector3 p = poly [pvp] - (poly [pv] - poly [pvp])
		* (prev_d / (d - prev_d));
        // If the vector product between both vectors making the frustum
        // plane and the intersection point between polygon edge and plane
        // have same sign, the intersection point is outside frustum
        //@@@ This is the old code but I think this code
	// should test against the frustum.
	//--- if ((poly [pvp] % p) * (poly [pv] % p) < 0)
        if ((v1 % p) * (v2 % p) <= 0)
          return CS_FRUST_PARTIAL;
#endif
      }
      prev_d = d;
    }
  }

  if (all_inside) return CS_FRUST_INSIDE;

  // Now we know that all polygon vertices are outside frustum and no polygon
  // edge crosses the frustum. We should choose between CS_FRUST_OUTSIDE and
  // CS_FRUST_COVERED cases. For this we check if all frustum vertices are
  // inside the frustum created by polygon (reverse roles). In fact it is
  // enough to check just the first vertex of frustum is outside polygon
  // frustum: this lets us make the right decision.

  for (int pv = 0, pvp = num_poly - 1; pv < num_poly; pvp = pv++)
  {
    csVector3 pn = poly [pvp] % poly [pv];
    if (pn * frustum [0] >= 0)
      return CS_FRUST_OUTSIDE;
  }

  return CS_FRUST_COVERED;
}

