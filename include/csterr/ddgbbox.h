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

#include "csgeom/math3d.h"
#include "csterr/ddg.h"

// Visibility data. 1 byte.
typedef union ddgCFlags {
	struct {
		bool lin:1;				/// Is point inside left halfspace.
		bool rin:1;				/// Right.
		bool tin:1;				/// Top.
		bool bin:1;				/// Bottom.
		bool nin:1;				/// Near.
		bool fin:1;				/// Far.
		bool allin:1;			/// all in.
		bool allout:1;			/// all out.
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
	csVector3 _min;
	/// right top far corner of the bbox.
	csVector3 _max;
public:
	/// Indexes to the corner points of the bounding box.
	static short _corner[8][3];
	/// Split options.
	typedef enum { XLT, XGT, YLT, YGT, ZLT, ZGT } Split;
	// Default bounding box contains the entire 3D space.
	/// Set bounding box using Min and Max vectors.
	ddgBBox(csVector3 min, csVector3 max);
	/// Return bounding box minimum x value.
	float minx( void ) { return _min.x; }
	/// Return bounding box minimum y value.
	float miny( void ) { return _min.y; }
	/// Return bounding box minimum z value.
	float minz( void ) { return _min.z; }
	/// Return bounding box maximum x value.
	float maxx( void ) { return _max.x; }
	/// Return bounding box maximum y value.
	float maxy( void ) { return _max.y; }
	/// Return bounding box maximum z value.
	float maxz( void ) { return _max.z; }
	/// Return the minimum coordinate
	csVector3 *min(void) { return &_min; }
	/// Return the maximum coordinate
	csVector3 *max(void) { return &_max; }
	/// Return the X coordinate of a corner of the BBox.
	float cornerx(int n);
	/// Return the Y coordinate of a corner of the BBox.
	float cornery(int n);
	/// Return the Z coordinate of a corner of the BBox.
	float cornerz(int n);
	/// Scale the bounding box by the specified vector about its centre.
	void scale(csVector3 s)
	{
	  csVector3 c(_min + _max);
	  c = c * 0.5;
	  csVector3 d(c - _min);
	  d.x *=  s.x;
	  d.y *=  s.y;
	  d.z *=  s.z;
	  _min = c - d;
	  _max = c + d;
	  }
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
