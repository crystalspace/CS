/*
    Copyright (C) 1997, 1998, 1999 by Alex Pfaffe
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

#ifdef DDG
#include "ddgvec.h"
#include "ddggeom.h"
#else
#include "csgeom/math3d.h"
#include "csterr/ddg.h"
#include "csterr/ddggeom.h"
#endif

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

// Visibility data. 2 bits.
typedef unsigned char ddgVisState;

enum ddgVis { ddgOUT=0, ddgPART=1, ddgIN=2, ddgUNDEF = 3};

/**
 * Axis Aligned Bounding box object.
 * This object defines a 3d volume based on a centre point
 * and a 3d offset vector directions.
 */
class WEXP ddgBBox {
	/// left bottom near corner of the bbox.
	ddgVector3 _min;
	/// right top far corner of the bbox.
	ddgVector3 _max;
public:
	/// Indexes to the corner points of the bounding box.
	static short _corner[8][3];
	/// Split options.
	typedef enum { XLT, XGT, YLT, YGT, ZLT, ZGT } Split;
	/** Default bounding box contains the entire 3D space.
	 *  Set bounding box using centre point and delta.
	 */
	ddgBBox(float xc = 0.0, float xd = MAXFLOAT, 
	   float yc = 0.0, float yd = MAXFLOAT, 
	   float zc = 0.0, float zd = MAXFLOAT);
	/// Set bounding box using Min and Max vectors.
	ddgBBox(const ddgVector3 &min, const ddgVector3 &max);
	/// Set bounding box using Min Max coordinates.
	void set(float xmin = -MAXFLOAT, float xmax = MAXFLOAT,
		     float ymin = -MAXFLOAT, float ymax = MAXFLOAT,
		     float zmin = -MAXFLOAT, float zmax = MAXFLOAT);
	/// Set bounding box using another bounding box.
	void set(const ddgVector3 &min, const ddgVector3 &max)
	{ _min = min; _max = max; }

	/// Return bounding box minimum x value.
	float minx( void ) { return _min[0]; }
	/// Return bounding box minimum y value.
	float miny( void ) { return _min[1]; }
	/// Return bounding box minimum z value.
	float minz( void ) { return _min[2]; }
	/// Return bounding box maximum x value.
	float maxx( void ) { return _max[0]; }
	/// Return bounding box maximum y value.
	float maxy( void ) { return _max[1]; }
	/// Return bounding box maximum z value.
	float maxz( void ) { return _max[2]; }

	/// Return the minimum coordinate
	ddgVector3 *min(void) { return &_min; }
	/// Return the maximum coordinate
	ddgVector3 *max(void) { return &_max; }
	/// Set the min and max of the bounding box along X-axis.
	void setx(float min, float max)
	{
	  _min[0] = min;
	  _max[0] = max;
	}
	/// Set the min and max of the bounding box along Y-axis.
	void sety(float min, float max)
	{
	  _min[1] = min;
	  _max[1] = max;
	}
	/// Set the min and max of the bounding box along Z-axis.
	void setz(float min, float max)
	{
	  _min[2] = min;
	  _max[2] = max;
	}

	/// Return the X coordinate of a corner of the BBox.
	float cornerx(int n);
	/// Return the Y coordinate of a corner of the BBox.
	float cornery(int n);
	/// Return the Z coordinate of a corner of the BBox.
	float cornerz(int n);
	/// Copy the content of the src bbox.
	void copy(ddgBBox *src);
	/** Split a bounding box in a given direction.
	*  Optionally give a split location, default is in the middle of the box.
	*/
	void split(Split side, float value = 0 );
	/// Return the centre point of the bounding box.
	ddgVector3 centre(void)
	{ return ( _min + _max ) * 0.5f;}
	/// Move the bounding box by the specified vector.
	void move(ddgVector3 o)
	{ _min = _min + o; _max = _max + o; }
	/// Scale the bounding box by the specified vector about its centre.
	
	void scale(ddgVector3 s)
	{
		ddgVector3 c(_min + _max);
		c = c * 0.5f;
		ddgVector3 d(c - _min);
		d[0] *= s[0];
		d[1] *= s[1];
		d[2] *= s[2];
		_min = c - d;
		_max = c + d;
	}
	/// Return the distance between the centre of the box and the coordinate.
	float distancesq( ddgVector3 *p);
	/// Return the true distance of the box's centre point to the eye.
	float distance( ddgVector3 *p) { return sqrtf(distancesq(p)); }
	/// Test for intersection of line with bbox.
	bool intersect( ddgVector3 *p1, ddgVector3 *p2);
	/// Test for intersection of another bbox.
	bool intersect( ddgBBox *b );
    /** 
     * Test a bounding box against this bounding box in camera space.
     *
	 * Returns true if camera coordinates are visible from camera,
	 * also indicates which half space the point was in.
	 * Assumes bbox is already in camera space.
	 * vis indicates which half spaces don't need to be tested.
	 */
    ddgClipFlags visibleSpace( ddgBBox b, float tanHalfFOV );
	/**
	 * Test bounding box agains a set of planes (frustrum).
	 * Return 0 if out, 1 intersecting, 2 completely inside.
	 */
	ddgVis isVisible(ddgPlane Planes[6]);


};

#endif
