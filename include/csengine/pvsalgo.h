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

#ifndef _CS_PVSALGO_H
#define _CS_PVSALGO_H

#include "csgeom/math3d.h"
#include "csgeom/box.h"

class csOctree;
class csOctreeNode;
class csCBuffer;

typedef csPlane3 csFrustum5[5];

/**
 * The PVS algorithms.
 */
class csPVSAlgo
{
  friend class csOctree;

private:
  /**
   * Help function for BoxCanSeeOccludee.
   */
  static void CalculatePolygonShadowArea (
	const csBox3& occludee_box,
	const csPoly3D& poly, const csPlane3& poly_plane,
	csPoly2D& result_poly,
	int plane_nr, float plane_pos,
	csFrustum5* frustums,
	bool bf);

  /**
   * Help function for BoxCanSeeOccludee.
   */
  static bool CalculatePolygonsShadowArea (
	const csBox3& occludee_box,
	const csPoly3D& poly1, const csPlane3& plane1, int edge1,
	const csPoly3D& poly2, const csPlane3& plane2,
	csPoly2D& result_poly,
	int plane_nr, float plane_pos,
	csFrustum5* frustums);
  /**
   * Help function for BoxCanSeeOccludee.
   */
  static void CalculatePolygonShadow (
	csPoly2D& cur_poly, csPoly2D& result_poly, bool& first_time);

  /**
   * Help function for BoxCanSeeOccludee.
   */
  static bool CalculatePolygonShadow (
	const csVector3& corner,
	const csPoly3D& cur_poly,
	csPoly2D& result_poly, bool& first_time,
	int plane_nr, float plane_pos,
	csFrustum5 frustum);

  /**
   * Help function for BoxCanSeeOccludee.
   */
  static bool InsertShadowIntoCBuffer (const csPoly2D& result_poly,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift);

  /**
   * Help function for BoxCanSeeOccludee.
   */
  static bool TestShadowIntoCBuffer (const csPoly2D& result_poly,
	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift);

  /**
   * Help function for BoxCanSeeOccludee.
   * This function returns false if it couldn't do the test (because
   * the box is too close to the test plane for example).
   */
  static bool BoxOccludeeShadowOutline (
  	const csBox3& occluder_box,
  	const csBox3& occludee_box,
	int plane_nr, float plane_pos, csPoly2D& result_poly,
	csFrustum5* frustums);

  /**
   * Help function for BoxCanSeeOccludee.
   */
  static void BoxOccludeeShadowPolygons (
  	const csBox3& occludee_box,
	csPolygonInt** polygons, int num_polygons,
	csCBuffer* cbuffer,
  	const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos,
	csFrustum5* frustums);

  /**
   * Help function for BoxCanSeeOccludee.
   */
  static void BoxOccludeeShadowSolidBoundaries (csOctreeNode* occluder,
	const csBox3& occludee_box,
    	csCBuffer* cbuffer, const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos,
	csFrustum5* frustums);
  /**
   * Help function for BoxCanSeeOccludee.
   * If 'do_polygons' is true then this function will also call
   * BoxOccludeeShadowPolygons on the polygons in the occluder.
   * Otherwise the only thing that is tested is if the occluder
   * can itself see the occludee in which case the entire occluder
   * can be seen as a solid polygon.
   */
  static void BoxOccludeeAddShadows (csOctreeNode* occluder, csCBuffer* cbuffer,
  	const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos,
  	const csBox3& box, const csBox3& occludee_box,
	csVector3& box_center, csVector3& occludee_center,
	bool do_polygons,
	csFrustum5* frustums);
};

#endif /*_CS_PVSALGO_H*/

