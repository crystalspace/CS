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

#include "math/ddgvec.h"
#include "util/ddgerror.h"

#define ddgVertex ddgVector3
#define ddgPoint3 ddgVector3

/**
 * General form of plane equation:
 * <br>			ax + by + cz = d
 * <br>			n = <a,b,c> = normal.
 */

class WEXP ddgPlane {
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
		_normal.normal(&p0,&p1,&p2);
		// Solve for D.
		_d = -1 * (_normal.dot(&p0));
	}
	/// Create a plane given a point on its surface and a normal.
	ddgPlane( const ddgVector3 p0, const ddgVector3 n)
	{
		_normal = n;
		// Solve for D.
		_d = -1 * (_normal.dot(&p0));
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
		_d /= _normal.size();
		_normal.normalize();
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
	inline float ddgPlane::isPointAbovePlane( ddgVector3 *q)
	{
		return _normal.dot(q)+d();
	}
};
/**
 * Ray triangle intersection tests.
 */

class WEXP ddgTriangle {
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
 * Polygon object
 */

class WEXP ddgPolygon {
	unsigned short _n;
	ddgVector3		*_p;
public:
	unsigned int vertices(void) { return _n; }
	ddgVector3		*vertex(unsigned int n) { return &(_p[n]); }
	ddgPolygon *clip( ddgPlane *p, bool rotated);
};

/**
 * 2D rectangle object.
 */
class WEXP ddgRect {
public:
	/// The minimum point.
	ddgVector2 min;
	/// The maximum point.
	ddgVector2 max;
	ddgRect(void) {}
	/// Constructor from 2 points.
	ddgRect( ddgVector2 *p1, ddgVector2 *p2)
	{
			min = p1;
			max = p1;
			min.minimum(p2);
			max.maximum(p2);
	}
	/**
	 * Test if rectangles intersect.
	 * and the rectangle which is the intersection of the given rectangles.
	 */
	bool intersect(ddgRect *r, ddgRect *i = NULL)
	{
		if (r->min[0] > max[0] || min[0] > r->max[0] || r->min[1] > max[1] || min[1] > r->max[1])
			return false;
		if (i)
		{
			i->min = min;
			i->max = max;
			i->min.maximum(&(r->min));
			i->max.minimum(&(r->max));
		}
		return true;
	}
};
/**
 * 2D Line object
 */
class WEXP ddgLine2 {
	/// A point on the line.
	ddgVector2	_p;
	/// Slope vector of the line.
	ddgVector2	_d;
public:
	/// Constructor using 2 points.
	ddgLine2( ddgVector2 *p1, ddgVector2 *p2 )
	{
		_p.set(p1);
		_d.set(p2);
		_d -=p1;
		_d.normalize();
	}
	/// Return point vector.
	ddgVector2 p(void) { return _p; }
	/// Return slope vector.
	ddgVector2 d(void) { return _d; }
	/// Given X, or Y, find the other.  Return ddgFailure if unsolvable.
	inline bool solve( ddgVector2 *v, int dim)
	{
		if (_d[dim] == 0.0)
			return ddgFailure;
		float t = (v->v[dim] - _p.v[dim])/_d[dim];
		*v = _p + (_d * t);
		return ddgSuccess;
	}
	/// Calculate the intersection point of this line with another.
	bool intersect( ddgLine2 *l, ddgVector2 *p)
	{
		if (_d[1] == 0 )
		{
			ddgAsserts(0,"Special case");
		}
		if (_d[1] == l->d()[1] && _d[0] == l->d()[0])
		{
			return ddgFailure;		// Parallel lines.
		}
		float x = (_d[0]*_p[0] - l->d()[0]*l->p()[0]+_d[1]*+_p[1]- l->d()[1]*l->p()[1])*(l->d()[1]*_d[1]);
		x /= (_d[1]*_d[0] - l->d()[1]*l->d()[0]);
		float y = (_d[1]*_p[1]+_d[0]*_p[0]-_d[0]*x ) / _d[1];
		p->set(x,y);
		return ddgSuccess;
	}
};

/**
 * 3D Line object
 */
class WEXP ddgLine3 {
	/// A point on the line.
	ddgVector3	_p;
	/// Slope vector of the line.
	ddgVector3	_d;
public:
	/// Constructor using 2 points.
	ddgLine3( const ddgVector3 p1, const ddgVector3 p2 )
	{
		_p.set(&p1);
		_d.set(&p2);
		_d -= p1;
		_d.normalize();
	}
	/// Return point on line.
	ddgVector3 p(void) { return _p; }
	/// Return slope
	ddgVector3 d(void) { return _d; }
	/// Given X, Y or Z coord, find the other 2.  Return ddgFailure if unsolvable.
	inline bool solve( ddgVector3 *u, int dim)
	{
		if (_d[dim] == 0.0)
			return ddgFailure;
		float n = ((*u)[dim] - _p[dim])/_d[dim];

		*u =_d + ( _p * n);
		return ddgSuccess;
	}

	/// Return the intersection point of this line with another.
	ddgVector3 intersect( ddgLine3 *)
	{
		ddgVector3 p(0,0,0);
		return p;
	}
	/// Calculate the intersection point of this line with another.
	bool intersect( ddgLine3 *l, ddgVector3 *p)
	{
		// $TODO this function is simply wrong...
		if (_d[1] == 0 )
		{
			ddgAsserts(0,"Special case");
		}
		if (_d[1] == l->d()[1] && _d[0] == l->d()[0])
		{
			return ddgFailure;		// Parallel lines.
		}
		float x = (_d[0]*_p[0] - l->d()[0]*l->p()[0]+_d[1]*+_p[1]- l->d()[1]*l->p()[1])*(l->d()[1]*_d[1]);
		x /= (_d[1]*_d[0] - l->d()[1]*l->d()[0]);
		float y = (_d[1]*_p[1]+_d[0]*_p[0]-_d[0]*x ) / _d[1];
		// Dirty hack.
		p->set(x,y,(*p)[2]);
		return ddgSuccess;
	}
};
#endif
