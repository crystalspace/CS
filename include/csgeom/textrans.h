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

#ifndef __CS_TEXTRANS_H__
#define __CS_TEXTRANS_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

class csMatrix3;
class csVector3;

/**
 * This is a static class which encapsulates a few functions
 * that can transform texture information into a texture matrix/vector.
 * This class makes it easiers to define textures for polygons given
 * various things.
 */
class CS_CRYSTALSPACE_EXPORT csTextureTrans
{
public:
  /**
   * Calculate the matrix using two vertices (which are preferably on the
   * plane of the polygon and are possibly (but not necessarily) two vertices
   * of the polygon). The first vertex is seen as the origin and the second
   * as the u-axis of the texture space coordinate system. The v-axis is
   * calculated on the plane of the polygon and orthogonal to the given
   * u-axis. The length of the u-axis and the v-axis is given as the 'len1'
   * parameter.<p>
   * For example, if 'len1' is equal to 2 this means that texture will
   * be tiled exactly two times between vertex 'v_orig' and 'v1'.
   * I hope this explanation is clear since I can't seem to make it
   * any clearer :-)
   */
  static void compute_texture_space (
	csMatrix3& m, csVector3& v,
	const csVector3& v_orig, const csVector3& v1, float len1,
	float A, float B, float C);
  ///
  static void compute_texture_space (
	csMatrix3& m, csVector3& v,
	float xo, float yo, float zo,
	float x1, float y1, float z1, float len1,
	float A, float B, float C);

  /**
   * Use 'v1' and 'len1' for the u-axis and 'v2' and 'len2' for the
   * v-axis. Otherwise this function is the same as the previous one.
   */
  static void compute_texture_space (
	csMatrix3& m, csVector3& v,
	const csVector3& v_orig,
	const csVector3& v1, float len1,
	const csVector3& v2, float len2);

  /**
   * Similar to the previous function but treat as if the lengths
   * are set to 1.
   */
  static void compute_texture_space (
	csMatrix3& m, csVector3& v,
	const csVector3& v_orig, const csVector3& v_u, const csVector3& v_v);

  /**
   * The most general function. With these you provide the matrix
   * directly.
   */
  static void compute_texture_space (
	csMatrix3& m, csVector3& v,
	float xo, float yo, float zo,
	float xu, float yu, float zu,
	float xv, float yv, float zv,
	float xw, float yw, float zw);
};

/** @} */

#endif // __CS_TEXTRANS_H__
