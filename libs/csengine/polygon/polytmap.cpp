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
#include "csengine/polytmap.h"
#include "csengine/engine.h"
#include "csgeom/transfrm.h"
#include "csgeom/textrans.h"

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csPolyTxtPlane)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPolyTxtPlane)
  SCF_IMPLEMENTS_INTERFACE (csPolyTxtPlane);
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPolyTxtPlane::PolyTxtPlane)
  SCF_IMPLEMENTS_INTERFACE (iPolyTxtPlane)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csPolyTxtPlane::csPolyTxtPlane () : csObject ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolyTxtPlane);
  csEngine::current_engine->AddToCurrentRegion (this);
}

csPolyTxtPlane::~csPolyTxtPlane ()
{
}

void csPolyTxtPlane::SetTextureSpace (
	const csPlane3& plane_wor,
	const csVector3& v_orig,
	const csVector3& v1, float len)
{
  SetTextureSpace (plane_wor, v_orig.x, v_orig.y, v_orig.z,
  	v1.x, v1.y, v1.z, len);
}

void csPolyTxtPlane::SetTextureSpace (
	const csPlane3& plane_wor,
	float xo, float yo, float zo,
       	float x1, float y1, float z1,
	float len1)
{
  float A = plane_wor.A ();
  float B = plane_wor.B ();
  float C = plane_wor.C ();
  csTextureTrans::compute_texture_space (
  	m_obj2tex, v_obj2tex,
  	xo, yo, zo, x1, y1, z1, len1,
  	A, B, C);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}
							 
void csPolyTxtPlane::SetTextureSpace (
	const csVector3& v_orig,
	const csVector3& v1, float len1,
	const csVector3& v2, float len2)
{
  csTextureTrans::compute_texture_space (
  	m_obj2tex, v_obj2tex,
	v_orig, v1, len1, v2, len2);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}

void csPolyTxtPlane::SetTextureSpace (
	const csVector3& v_orig,
	const csVector3& v_u,
	const csVector3& v_v)
{
  csTextureTrans::compute_texture_space (
  	m_obj2tex, v_obj2tex,
	v_orig, v_u, v_v);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}

void csPolyTxtPlane::SetTextureSpace (float xo, float yo, float zo,
  			  float xu, float yu, float zu,
  			  float xv, float yv, float zv)
{
  const csVector3 o(xo, yo, zo);
  const csVector3 u(xu, yu, zu);
  const csVector3 v(xv, yv, zv);
  csTextureTrans::compute_texture_space (m_obj2tex, v_obj2tex, o, u, v);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}

void csPolyTxtPlane::SetTextureSpace (
	float xo, float yo, float zo,
	float xu, float yu, float zu,
	float xv, float yv, float zv,
	float xw, float yw, float zw)
{
  csTextureTrans::compute_texture_space (
  	m_obj2tex, v_obj2tex,
	xo, yo, zo,xu, yu, zu, xv, yv, zv, xw, yw, zw);
  m_world2tex = m_obj2tex;
  v_world2tex = v_obj2tex;
}

void csPolyTxtPlane::SetTextureSpace (
	const csMatrix3& tx_matrix,
	const csVector3& tx_vector)
{
  m_obj2tex = tx_matrix;
  m_world2tex = tx_matrix;
  v_obj2tex = tx_vector;
  v_world2tex = tx_vector;
}

void csPolyTxtPlane::GetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector)
{
  tx_matrix = m_obj2tex;
  tx_vector = v_obj2tex;
}

void csPolyTxtPlane::WorldToCamera (
	const csReversibleTransform& t,
	csMatrix3& m_cam2tex, csVector3& v_cam2tex)
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
}

void csPolyTxtPlane::ObjectToWorld (
	const csReversibleTransform& obj)
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
}

void csPolyTxtPlane::HardTransform (
	const csReversibleTransform& obj)
{
  ObjectToWorld (obj);
  m_obj2tex = m_world2tex;
  v_obj2tex = v_world2tex;
}
