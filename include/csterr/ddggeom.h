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
#ifndef _ddgGeometry_Class_
#define _ddgGeometry_Class_
#ifdef DDG
#include "ddgvec.h"
#include "ddgerror.h"
#else
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csterr/ddg.h"
#include "csterr/ddgvec.h"
#endif

#define ddgVertex ddgVector3
#define ddgPoint3 ddgVector3

/**
 * General form of plane equation:
 * <br>			ax + by + cz = d
 * <br>			n = <a,b,c> = normal.
 */

class ddgPlane {
	ddgVector3		_normal;
	float _d;
public:
	inline float a(void) { return _normal[0];};
	inline float b(void) { return _normal[1];};
	inline float c(void) { return _normal[2];};
	inline float d(void) { return _d;};
	inline ddgVector3 *normal(void) { return &_normal; }
	/// Default constructor.
	ddgPlane(void) {}
	/// Create a plane given 3 points on its surface (counter clock wise)
	ddgPlane( const ddgVector3 p0, const ddgVector3 p1, const ddgVector3 p2)
	{
#ifdef DDG
		_normal.normal(&p0,&p1,&p2);
		// Solve for D.
		_d = -1 * (_normal.dot(&p0));
#else
		_normal = ddgVector3(p2 - p1) * ddgVector3(p0 - p1);
		// Solve for D.
		_d = -1 * (_normal * p0);
#endif
	}
	/// Create a plane given a point on its surface and a normal.
	ddgPlane( const ddgVector3 p0, const ddgVector3 n)
	{
		_normal = n;
		// Solve for D.
#ifdef DDG
		_d = -1 * (_normal.dot(&p0));
#else
		_d = -1 * (_normal * p0);
#endif
	}
	/// Given y and z find x in plane.
	bool projectAlongX(ddgVector3 *p)
	{
		if (_normal[0] == 0.0)
			return ddgFailure;
		(*p)[0] = -1 * (_d + _normal[1] * (*p)[1] + _normal[2] * (*p)[2])/ _normal[0];
		return ddgSuccess;
	}
	/// Given x and z find y in plane.
	bool projectAlongY(ddgVector3 *p)
	{
		if (_normal[1] == 0.0)
			return ddgFailure;
		(*p)[1] = -1 * (_d + _normal[0] * (*p)[0] + _normal[2] * (*p)[2])/ _normal[1];
		return ddgSuccess;
	}
	/// Given x and y find z in plane.
	bool projectAlongZ(ddgVector3 *p)
	{
		if (_normal[2] == 0.0)
			return ddgFailure;
		(*p)[2] = -1 * (_d + _normal[1] * (*p)[1] + _normal[0] * (*p)[0])/ _normal[2];
		return ddgSuccess;
	}
	void set( const ddgVector3 n, float d )
	{
		_normal = n;
		_d = d;
	}
	void normalize( void )
	{
#ifdef DDG
		_d /= _normal.size();
		_normal.normalize();
#else
		_d /= _normal.Norm();
		_normal.Normalize();
#endif
	}
    ///
	ddgVector3 project(ddgVector3 *p0);
    /// Intersect a plane with a line (defined by 2 points) return point of intersection.
    bool intersectPlaneWithLine( const ddgVector3 l1, const ddgVector3 l2, ddgVector3 *pi);
    /// Intersect a point with plane.  Returns if point is on the plane
    bool intersectPointPlane( const ddgVector3 pt);
	/// Return distance between point and plane.
	float distToPoint(const ddgVector3 p);
	/// Returns positive if point is above plane, negative if below, 0 if on.
	float isPointAbovePlane(const ddgVector3 p);
};
/**
 * Ray triangle intersection tests.
 */

class ddgTriangle {
    /// Vertices of the triangle.
    ddgVertex  _v1;
    ///
    ddgVertex  _v2;
    ///
    ddgVertex  _v3;
public:
    /// Constructor
    ddgTriangle(ddgVector3 p1, ddgVector3 p2, ddgVector3 p3 ) :
        _v1(p1), _v2(p2), _v3(p3) {}
    /// Intersect a triangle with line.  Returns if point is 
    bool intersectTriangleWithLine( const ddgVector3 l1, const ddgVector3 l2, ddgVector3 *pi);
    /** Intersect a point with triangle.  Returns if point is inside the triangle.
     *  point is assumed to lie in plane of triangle.
     */
    bool intersectPointTriangle( const ddgVector3 pt);
};

/**
 *
 */

class ddgPolygon {
	unsigned short _n;
	ddgVector3		*_p;
public:
	unsigned int vertices(void) { return _n; }
	ddgVector3		*vertex(unsigned int n) { return &(_p[n]); }
	ddgPolygon *clip( ddgPlane *p, bool rotated);
};

#endif
