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

#include <math.h>
#include "sysdef.h"
#include "csengine/textrans.h"

//---------------------------------------------------------------------------

void TextureTrans::compute_texture_space (
	csMatrix3& m, csVector3& v,
	csVector3& v_orig, csVector3& v1, float len1,
	float A, float B, float C)
{
  compute_texture_space (m, v, v_orig.x, v_orig.y, v_orig.z,
  	v1.x, v1.y, v1.z, len1, A, B, C);
}

void TextureTrans::compute_texture_space (
	csMatrix3& m, csVector3& v,
	float xo, float yo, float zo,
	float x1, float y1, float z1, float len1,
	float A, float B, float C)
{
  float l1 = sqrt ((xo-x1)*(xo-x1) + (yo-y1)*(yo-y1) + (zo-z1)*(zo-z1));
  x1 = (x1-xo) / l1;
  y1 = (y1-yo) / l1;
  z1 = (z1-zo) / l1;

  float x2, y2, z2;

  // The cross product of the given vector and the normal of
  // the polygon plane is a vector which is perpendicular on
  // both (and thus is also on the plane).
  x2 = y1*C-z1*B;
  y2 = z1*A-x1*C;
  z2 = x1*B-y1*A;

  float l2 = sqrt (x2*x2 + y2*y2 + z2*z2);

  x1 *= len1;
  y1 *= len1;
  z1 *= len1;
  x2 = x2*len1 / l2;
  y2 = y2*len1 / l2;
  z2 = z2*len1 / l2;

  float l3 = sqrt (A*A + B*B + C*C);
  float a, b, c;
  a = A*len1 / l3;
  b = B*len1 / l3;
  c = C*len1 / l3;

  compute_texture_space (m, v, xo, yo, zo, x1, y1, z1, x2, y2, z2, a, b, c);
}

void TextureTrans::compute_texture_space (
	csMatrix3& m, csVector3& v,
	csVector3& v_orig, csVector3& v1, float len1, csVector3& v2, float len2)
{
  float xo = v_orig.x;
  float yo = v_orig.y;
  float zo = v_orig.z;
  float x1 = v1.x;
  float y1 = v1.y;
  float z1 = v1.z;
  float x2 = v2.x;
  float y2 = v2.y;
  float z2 = v2.z;
  compute_texture_space (m, v, xo, yo, zo, x1, y1, z1, len1, x2, y2, z2, len2);
}

void TextureTrans::compute_texture_space (
	csMatrix3& m, csVector3& v,
	float xo, float yo, float zo,
	float x1, float y1, float z1,
	float len1,
	float x2, float y2, float z2,
	float len2)
{
  float l1 = sqrt ((xo-x1)*(xo-x1) + (yo-y1)*(yo-y1) + (zo-z1)*(zo-z1));
  float l2 = sqrt ((xo-x2)*(xo-x2) + (yo-y2)*(yo-y2) + (zo-z2)*(zo-z2));
  x1 = (x1-xo) * len1 / l1;
  y1 = (y1-yo) * len1 / l1;
  z1 = (z1-zo) * len1 / l1;
  x2 = (x2-xo) * len2 / l2;
  y2 = (y2-yo) * len2 / l2;
  z2 = (z2-zo) * len2 / l2;

  compute_texture_space (m, v, xo, yo, zo, x1, y1, z1, x2, y2, z2, 1, 1, 1);
}

void TextureTrans::compute_texture_space (
	csMatrix3& m, csVector3& v,
	csVector3& v_orig, csVector3& v_u, csVector3& v_v)
{
  float xo = v_orig.x;
  float yo = v_orig.y;
  float zo = v_orig.z;
  float x1 = v_u.x;
  float y1 = v_u.y;
  float z1 = v_u.z;
  float x2 = v_v.x;
  float y2 = v_v.y;
  float z2 = v_v.z;
  compute_texture_space (m, v, xo, yo, zo, x1, y1, z1, x2, y2, z2);
}

void TextureTrans::compute_texture_space (
	csMatrix3& m, csVector3& v,
	float xo, float yo, float zo,
	float xu, float yu, float zu,
	float xv, float yv, float zv)
{
  compute_texture_space (m, v, xo, yo, zo, xu-xo, yu-yo, zu-zo,
  	xv-xo, yv-yo, zv-zo, 1, 1, 1);
}

void TextureTrans::compute_texture_space (
	csMatrix3& m, csVector3& v,
	float xo, float yo, float zo,
	float xu, float yu, float zu,
	float xv, float yv, float zv,
	float xw, float yw, float zw)
{
  m.m11 = xu; m.m12 = xv; m.m13 = xw;
  m.m21 = yu; m.m22 = yv; m.m23 = yw;
  m.m31 = zu; m.m32 = zv; m.m33 = zw;
  m.Invert ();
  v.x = xo; v.y = yo; v.z = zo;
}

