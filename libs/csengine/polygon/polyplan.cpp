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
#include "csengine/polyplan.h"
#include "csengine/textrans.h"
#include "csgeom/transfrm.h"

//---------------------------------------------------------------------------

CSOBJTYPE_IMPL(csPolyPlane,csObject);

csPolyPlane::csPolyPlane () : csObject ()
{
}

csPolyPlane::~csPolyPlane ()
{
}

void csPolyPlane::SetTextureSpace (csVector3& v_orig, csVector3& v1, float len)
{
  SetTextureSpace (v_orig.x, v_orig.y, v_orig.z, v1.x, v1.y, v1.z, len);
}

void csPolyPlane::SetTextureSpace (
	float xo, float yo, float zo,
       	float x1, float y1, float z1,
	float len1)
{
  float A = plane_wor.A ();
  float B = plane_wor.B ();
  float C = plane_wor.C ();
  TextureTrans::compute_texture_space (
  	m_obj2tex, v_obj2tex,
  	xo, yo, zo, x1, y1, z1, len1,
  	A, B, C);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}
							 
void csPolyPlane::SetTextureSpace (csVector3& v_orig, csVector3& v1, float len1, csVector3& v2, float len2)
{
  TextureTrans::compute_texture_space (
  	m_obj2tex, v_obj2tex,
	v_orig, v1, len1, v2, len2);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}

void csPolyPlane::SetTextureSpace (
	float xo, float yo, float zo,
	float x1, float y1, float z1,
	float len1,
	float x2, float y2, float z2,
	float len2)
{
  TextureTrans::compute_texture_space (
  	m_obj2tex, v_obj2tex,
	xo, yo, zo, x1, y1, z1, len1, x2, y2, z2, len2);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}

void csPolyPlane::SetTextureSpace (csVector3& v_orig, csVector3& v_u, csVector3& v_v)
{
  TextureTrans::compute_texture_space (
  	m_obj2tex, v_obj2tex,
	v_orig, v_u, v_v);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}

void csPolyPlane::SetTextureSpace (float xo, float yo, float zo,
  			  float xu, float yu, float zu,
  			  float xv, float yv, float zv)
{
  TextureTrans::compute_texture_space (
  	m_obj2tex, v_obj2tex,
	xo, yo, zo,xu, yu, zu, xv, yv, zv);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}

void csPolyPlane::SetTextureSpace (
	float xo, float yo, float zo,
	float xu, float yu, float zu,
	float xv, float yv, float zv,
	float xw, float yw, float zw)
{
  TextureTrans::compute_texture_space (
  	m_obj2tex, v_obj2tex,
	xo, yo, zo,xu, yu, zu, xv, yv, zv, xw, yw, zw);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}

void csPolyPlane::SetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector)
{
  m_obj2tex = tx_matrix;
  m_world2tex = tx_matrix;
  v_obj2tex = tx_vector;
  v_world2tex = tx_vector;
}

void csPolyPlane::GetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector)
{
  tx_matrix = m_obj2tex;
  tx_vector = v_obj2tex;
}

void csPolyPlane::WorldToCamera(const csReversibleTransform& t, csVector3& vertex1)
{
  // Create the matrix to transform camera space to texture space.
  // From: T = Mwt * (W - Vwt)
  //       C = Mwc * (W - Vwc)
  // To:   T = Mct * (C - Vct)

  // Mcw * C + Vwc = W
  // T = Mwt * (Mcw * C + Mcw * Mwc * (Vwc - Vwt))
  // T = Mwt * Mcw * (C - Mwc * (Vwt-Vwc))
  // ===>
  // Mct = Mwt * Mcw
  // Vct = Mwc * (Vwt - Vwc)

  m_cam2tex = m_world2tex;
  m_cam2tex *= t.GetT2O ();

  v_cam2tex = t.Other2This (v_world2tex);

  // Now we transform the plane itself.
  t.Other2This (plane_wor, vertex1, plane_cam);
}

void csPolyPlane::ObjectToWorld (const csReversibleTransform& obj,
	csVector3& vertex1)
{
  // From: T = Mot * (O - Vot)
  //       W = Mow * O - Vow
  // To:   T = Mwt * (W - Vwt)

  // Mwo * (W + Vow) = O
  // T = Mot * (Mwo * (W + Vow) - (Mwo * Mow) * Vot)
  // T = Mot * Mwo * (W + Vow - Mow * Vot)
  // ===>
  // Mwt = Mot * Mwo
  // Vwt = Mow * Vot - Vow

  m_world2tex = m_obj2tex;
  m_world2tex *= obj.GetO2T ();
  v_world2tex = obj.This2Other (v_obj2tex);

  // Now we transform the plane itself.
  obj.This2Other (plane_obj, vertex1, plane_wor);
}

void csPolyPlane::GetCameraNormal (float* p_A, float* p_B, float* p_C, float* p_D)
{
  *p_A = plane_cam.A ();
  *p_B = plane_cam.B ();
  *p_C = plane_cam.C ();
  *p_D = plane_cam.D ();
}

void csPolyPlane::GetWorldNormal (float* p_A, float* p_B, float* p_C, float* p_D)
{
  *p_A = plane_wor.A ();
  *p_B = plane_wor.B ();
  *p_C = plane_wor.C ();
  *p_D = plane_wor.D ();
}

void csPolyPlane::GetObjectNormal (float* p_A, float* p_B, float* p_C, float* p_D)
{
  *p_A = plane_obj.A ();
  *p_B = plane_obj.B ();
  *p_C = plane_obj.C ();
  *p_D = plane_obj.D ();
}

void csPolyPlane::ClosestPoint (csVector3& v, csVector3& isect)
{
  float r = Classify (v);
  isect.x = r*(-plane_wor.A ()-v.x)+v.x;
  isect.y = r*(-plane_wor.B ()-v.y)+v.y;
  isect.z = r*(-plane_wor.C ()-v.z)+v.z;
}

bool csPolyPlane::IntersectSegment (const csVector3& start, const csVector3& end,
                                   csVector3& isect, float* pr)
{
  float x1 = start.x;
  float y1 = start.y;
  float z1 = start.z;
  float x2 = end.x;
  float y2 = end.y;
  float z2 = end.z;
  float r, num, denom;

  // So now we have the plane equation of the polygon:
  // A*x + B*y + C*z + D = 0
  //
  // We also have the parameter line equations of the ray
  // going through 'start' and 'end':
  // x = r*(x2-x1)+x1
  // y = r*(y2-y1)+y1
  // z = r*(z2-z1)+z1
  //
  // =>   A*(r*(x2-x1)+x1) + B*(r*(y2-y1)+y1) + C*(r*(z2-z1)+z1) + D = 0

  denom = plane_wor.A ()*(x2-x1) + plane_wor.B ()*(y2-y1) + plane_wor.C ()*(z2-z1);
  if (ABS (denom) < SMALL_EPSILON) return false;	// Lines are parallel

  num = - (plane_wor.A ()*x1 + plane_wor.B ()*y1 + plane_wor.C ()*z1 + plane_wor.D ());
  r = num / denom;

  // Calculate 'r' and 'isect' even if the intersection point is
  // not on the segment. That way we can use this function for testing
  // with rays as well.

  if (pr) *pr = r;
 
  isect.x = r*(x2-x1)+x1;
  isect.y = r*(y2-y1)+y1;
  isect.z = r*(z2-z1)+z1;

  // If r is not in [0,1] the intersection point is not on the segment.
  if (r < 0 /*-SMALL_EPSILON*/ || r > 1) return false;

  return true;
}

//---------------------------------------------------------------------------
