/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#ifndef _ddgBBox_Class_
#define _ddgBBox_Class_

#include "math/ddgvec.h"
#include "math/ddggeom.h"

// Visibility data. 1 byte.
typedef unsigned char ddgClipFlags;
enum {
  DDGCF_LIN    = 1 << 0, /// Is point inside left halfspace.
  DDGCF_RIN    = 1 << 1, /// Right.
  DDGCF_TIN    = 1 << 2, /// Top.
  DDGCF_BIN    = 1 << 3, /// Bottom.
  DDGCF_NIN    = 1 << 4, /// Near.
  DDGCF_FIN    = 1 << 5, /// Far.
  DDGCF_ALLIN  = 1 << 6, /// all in.
  DDGCF_ALLOUT = 1 << 7	 /// all out.
};

#define DDGCF_VISIBILITY \
  (DDGCF_LIN | DDGCF_RIN | DDGCF_TIN | DDGCF_BIN | DDGCF_NIN | DDGCF_FIN)

// Visibility data. 3 bits.
typedef unsigned char ddgVisState;

/**
 * Axis Aligned Bounding box object.
 * This object defines a 3d volume based a min and max point in 3D space.
 */
class WEXP ddgBBox3 {
public:
	/// left bottom near corner of the bbox.
	ddgVector3 min;
	/// right top far corner of the bbox.
	ddgVector3 max;
	/// Indexes to the corner points of the bounding box.
	static short _corner[8][3];
	/// Split options.
	typedef enum { XLT, XGT, YLT, YGT, ZLT, ZGT } Split;
	/** Default bounding box contains the entire 3D space.
	 *  Set bounding box using centre point and delta.
	 */
	ddgBBox3(float xc = 0.0, float xd = MAXFLOAT, 
	   float yc = 0.0, float yd = MAXFLOAT, 
	   float zc = 0.0, float zd = MAXFLOAT);
	/// Set bounding box using Min and Max vectors.
	ddgBBox3(const ddgVector3 &min, const ddgVector3 &max);
	/// Set bounding box using Min Max coordinates.
	void set(float xmin = -MAXFLOAT, float xmax = MAXFLOAT,
		     float ymin = -MAXFLOAT, float ymax = MAXFLOAT,
		     float zmin = -MAXFLOAT, float zmax = MAXFLOAT);
	/// Set bounding box using another bounding box.
	void set(const ddgVector3 &pmin, const ddgVector3 &pmax)
	{ min = pmin; max = pmax; }

	/// Return bounding box minimum x value.
	float minx( void ) { return min[0]; }
	/// Return bounding box minimum y value.
	float miny( void ) { return min[1]; }
	/// Return bounding box minimum z value.
	float minz( void ) { return min[2]; }
	/// Return bounding box maximum x value.
	float maxx( void ) { return max[0]; }
	/// Return bounding box maximum y value.
	float maxy( void ) { return max[1]; }
	/// Return bounding box maximum z value.
	float maxz( void ) { return max[2]; }

	/// Set the min and max of the bounding box along X-axis.
	void setx(float pmin, float pmax)
	{
	  min[0] = pmin;
	  max[0] = pmax;
	}
	/// Set the min and max of the bounding box along Y-axis.
	void sety(float pmin, float pmax)
	{
	  min[1] = pmin;
	  max[1] = pmax;
	}
	/// Set the min and max of the bounding box along Z-axis.
	void setz(float pmin, float pmax)
	{
	  min[2] = pmin;
	  max[2] = pmax;
	}
	/** Split a bounding box in a given direction.
	*  Optionally give a split location, default is in the middle of the box.
	*/
	void split(Split side, float value = 0 );
	/// Return the X coordinate of a corner of the BBox.
	float cornerx(int n);
	/// Return the Y coordinate of a corner of the BBox.
	float cornery(int n);
	/// Return the Z coordinate of a corner of the BBox.
	float cornerz(int n);
	/// Copy the content of the src bbox.
	void copy(ddgBBox3 *src);
	/// Return the centre point of the bounding box.
	ddgVector3 centre(void)
	{ return ( min + max ) * 0.5;}
	/// Move the bounding box by the specified vector.
	void move(ddgVector3 o)
	{ min = min + &o; max = max + &o; }
	/// Scale the bounding box by the specified vector about its centre.
	void scale(ddgVector3 s)
	{
		ddgVector3 c(min + max);
		c = c * 0.5;
		ddgVector3 d(c - min);
		d[0] *= s[0];
		d[1] *= s[1];
		d[2] *= s[2];
		min = c - d;
		max = c + d;
	}
	/// Return the distance between the centre of the box and the coordinate.
	float distancesq( ddgVector3 *p);
	/// Return the true distance of the box's centre point to the eye.
	float distance( ddgVector3 *p) { return sqrtf(distancesq(p)); }
	/// Test for intersection of line with bbox.
	bool intersect( ddgVector3 *p1, ddgVector3 *p2);
	/// Test for intersection of another bbox.
	bool intersect( ddgBBox3 *b );
	/// Return the size of the box.
	ddgVector3 size(void) { return max - min; }

    /** 
     * Test a bounding box against this bounding box in camera space.
     *
	 * Returns true if camera coordinates are visible from camera,
	 * also indicates which half space the point was in.
	 * Assumes bbox is already in camera space.
	 * vis indicates which half spaces don't need to be tested.
	 */
    ddgClipFlags visibleSpace( ddgBBox3 b, float tanHalfFOV );

};

#endif
