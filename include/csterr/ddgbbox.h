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

#include "csterr/ddgvec.h"

// Visibility data. 1 byte.
typedef union ddgCFlags {
	struct {
		bool lin:1;				/// Is point inside left halfspace.
		bool rin:1;				/// Right.
		bool tin:1;				/// Top.
		bool bin:1;				/// Bottom.
		bool nin:1;				/// Near.
		bool fin:1;				/// Far.
		bool all:1;				/// 0 = dontknow, 1 = all in, 2 = all out, 3 = undefined.
		bool none:1;
		} flags; 
	unsigned char visibility;
} ddgClipFlags; 

enum ddgClip { ddgINIT=0, ddgLEFT=1, ddgRIGHT=2, ddgTOP=4, ddgBOTTOM=8, ddgNEAR=16, ddgFAR=32, ddgALLIN=64, ddgALLOUT = 128};
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
	// Default bounding box contains the entire 3D space.
	/// Set bounding box using centre point and delta.
	ddgBBox(float xc = 0.0, float xd = MAXFLOAT, 
	   float yc = 0.0, float yd = MAXFLOAT, 
	   float zc = 0.0, float zd = MAXFLOAT);
	/// Set bounding box using Min and Max vectors.
	ddgBBox(ddgVector3 *min, ddgVector3 *max);
	/// Set bounding box using Min Max coordinates.
	void set(float xmin = -MAXFLOAT, float xmax = MAXFLOAT,
		   float ymin = -MAXFLOAT, float ymax = MAXFLOAT,
		   float zmin = -MAXFLOAT, float zmax = MAXFLOAT);
	/// Set bounding box using another bounding box.
	void set(ddgBBox *b)
	{ _min = b->min(); _max = b->max(); }
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
	ddgVector3 *min(void) { return _min; }
	/// Return the maximum coordinate
	ddgVector3 *max(void) { return _max; }
	/// Set the min and max of the bounding box along X-axis.
	void setx(float min, float max)
	{
	  _min.v[0] = min;
	  _max.v[0] = max;
	}
	/// Set the min and max of the bounding box along Y-axis.
	void sety(float min, float max)
	{
	  _min.v[1] = min;
	  _max.v[1] = max;
	}
	/// Set the min and max of the bounding box along Z-axis.
	void setz(float min, float max)
	{
	  _min.v[2] = min;
	  _max.v[2] = max;
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
	void centre(ddgVector3 *c)
	{ c->assign( _min + _max ); c->divide(2);}
	/// Move the bounding box by the specified vector.
	void move(ddgVector3 *o)
	{ _min.add( o ); _max.add(o); }
	/// Scale the bounding box by the specified vector about its centre.
	void scale(ddgVector3 *s)
	{
	  ddgVector3 c(_min + _max);
	  c.divide(2);
	  ddgVector3 d(c - _min);
	  d.multiply(s);
	  _min = c - d;
	  _max = c + d;
	  }
	/// Return the distance between the centre of the box and the coordinate.
	float distancesq( ddgVector3 *p);
	/// Return the true distance of the box's centre point to the eye.
	float distance( ddgVector3 *p) { return sqrtf(distancesq(p)); }
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

};

#endif
