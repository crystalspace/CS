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
#ifndef _ddgGeometry_Class_
#define _ddgGeometry_Class_

#include "math/ddgvec.h"
#include "util/ddgerror.h"

#define ddgVertex3 ddgVector3
#define ddgVertex2 ddgVector2
#define ddgVertex1 float
#define ddgPoint3 ddgVector3
#define ddgPoint2 ddgVector2
#define ddgPoint1 float


/**
 * General form of plane equation:
 * <br>			ax + by + cz = d
 * <br>			n = <a,b,c> = normal.
 */

class WEXP ddgPlane3 {
public:
	///
	ddgVector3		n;
	///
	float d;
	///
	inline float a(void) { return n[0];};
	///
	inline float b(void) { return n[1];};
	///
	inline float c(void) { return n[2];};

	/// Default constructor.
	ddgPlane3(void) {}
	/// Create a plane given 3 points on its surface (counter clock wise)
	ddgPlane3( const ddgVector3 p0, const ddgVector3 p1, const ddgVector3 p2)
	{
		n.normal(&p0,&p1,&p2);
		// Solve for D.
		d = -1 * (n.dot(&p0));
	}
	/// Create a plane given a point on its surface and a normal.
	ddgPlane3( const ddgVector3 p0, const ddgVector3 pn)
	{
		n = pn;
		// Solve for D.
		d = -1 * (n.dot(&p0));
	}
	/// Given y and z find x in plane.
	bool projectAlongX(ddgVector3 *p)
	{
		if (n[0] == 0.0)
			return ddgFailure;
		(*p)[0] = -1 * (d + n[1] * (*p)[1] + n[2] * (*p)[2])/ n[0];
		return ddgSuccess;
	}
	/// Given x and z find y in plane.
	bool projectAlongY(ddgVector3 *p)
	{
		if (n[1] == 0.0)
			return ddgFailure;
		(*p)[1] = -1 * (d + n[0] * (*p)[0] + n[2] * (*p)[2])/ n[1];
		return ddgSuccess;
	}
	/// Given x and y find z in plane.
	bool projectAlongZ(ddgVector3 *p)
	{
		if (n[2] == 0.0)
			return ddgFailure;
		(*p)[2] = -1 * (d + n[1] * (*p)[1] + n[0] * (*p)[0])/ n[2];
		return ddgSuccess;
	}
	///
	void set( const ddgVector3 pn, float pd )
	{
		n = pn;
		d = pd;
	}
	///
	void normalize( void )
	{
		float s = n.size();
		ddgAssert(s != 0.0);
		n.divide(s);
		d /= s;
	}
    ///
	ddgVector3 project(ddgVector3 *p0);
    /// Intersect a plane with a line (defined by 2 points) return point of intersection.
    bool intersectPlaneWithLine( const ddgVector3* l1, const ddgVector3* l2, ddgVector3 *pi);
    /// Intersect a point with plane.  Returns if point is on the plane
    bool intersectPointPlane( const ddgVector3* pt);
	/// Return distance between point and plane.
	float distToPoint(const ddgVector3* p);
	/// Returns positive if point is above plane, negative if below, 0 if on.
	inline float isPointAbovePlane( ddgVector3 *q)
	{
		return n.dot(q)+d;
	}
};

/**
 * This is a special 2 dimensional plane, it is defined by a normal like a 3d plane
 * but is actually a line defined by the points which are orthogonal to the line.
 */

class WEXP ddgPlane2 {
public:
	///
	ddgVector2		n;
	///
	float d;
	///
	inline float a(void) { return n[0];};
	///
	inline float b(void) { return n[1];};

	/// Default constructor.
	ddgPlane2(void) {}
	/// Create a plane given a point on its surface and a normal.
	ddgPlane2( const ddgVector2 p0, const ddgVector2 pn)
	{
		n = pn;
		// Solve for D.
		d = -1 * (n.dot(&p0));
	}
	///
	void set( const ddgVector2 pn, float pd )
	{
		n = pn;
		d = pd;
	}
	///
	void normalize( void )
	{
		n.normalize();
	}
	///
	inline float isPointAbovePlane( ddgVector2 *q)
	{
		return n.dot(q)+d;
	}
};

/**
 * Ray triangle intersection tests.
 */

class WEXP ddgTriangle3 {
public:
    /// Vertices of the triangle.
    ddgVertex3  v[3];
    /// Constructor
    ddgTriangle3(ddgVector3 *p1, ddgVector3 *p2, ddgVector3 *p3 ) 
        {v[0].set(p1), v[1].set(p2), v[2].set(p3); }
    /// Set method
    void set(ddgVector3 *p1, ddgVector3 *p2, ddgVector3 *p3 ) 
        {v[0].set(p1), v[1].set(p2), v[2].set(p3); }
    /// Default bConstructor
    ddgTriangle3(void) {}
    /// Intersect a triangle with line.  Returns if point is 
    bool intersectTriangleWithLine( const ddgVector3* l1, const ddgVector3* l2, ddgVector3 *pi);
    /** Intersect a point with triangle.  Returns if point is inside the triangle.
     *  point is assumed to lie in plane of triangle.
     */
    bool intersectPointTriangle( const ddgVector3* pt);
    void operator=(const ddgTriangle3& t)
	{v[0].set(&t.v[0]); v[0].set(&t.v[0]); v[0].set(&t.v[0]); }
};

class WEXP ddgPrism3 {
public:
	/// Top and bottom triangle that forms the prism.
	ddgTriangle3	t[2];
    /// Constructor
    ddgPrism3(ddgVector3 *pt1, ddgVector3 *pt2, ddgVector3 *pt3, ddgVector3 *pb1, ddgVector3 *pb2, ddgVector3 *pb3 )
        { t[0].set(pt1,pt2,pt3), t[1].set(pb1,pb2,pb3); }
    /// Constructor
    ddgPrism3(ddgTriangle3* top, ddgTriangle3* bottom )
		{
		t[0] = *top;
		t[1] = *bottom;
		}
	/// Default constructor.
	ddgPrism3(void) {}
};

/**
 * 3D Polygon object
 */

class WEXP ddgPolygon3 {
public:
	/// Number of points.
	unsigned short noPoints;
	/// Array of points.
	ddgVector3		*vertex;
	/// Clip polygon against a plane.
	ddgPolygon3 *clip( ddgPlane3 *p, bool rotated);
};

/**
 * 2D rectangle object.
 */
class WEXP ddgRect2 {
public:
	/// The minimum point.
	ddgVector2 min;
	/// The maximum point.
	ddgVector2 max;
	ddgRect2(void) {}
	/// Constructor from 2 points.
	ddgRect2( ddgVector2 *p1, ddgVector2 *p2)
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
	bool intersect(ddgRect2 *r, ddgRect2 *i = NULL)
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
public:
	/// A point on the line.
	ddgVector2	p;
	/// Slope vector of the line.
	ddgVector2	d;
	/// Constructor using 2 points.
	ddgLine2( ddgVector2 *p1, ddgVector2 *p2 )
	{
		set(p1,p2);
	}
	/// Constructor using 2 points.
	void set( ddgVector2 *p1, ddgVector2 *p2 )
	{
		p.set(p1);
		d.set(p2);
		d -=p1;
		d.normalize();
	}
	/// Default constructor.
	ddgLine2(void) {}
	/// Return the orthogonal slope vector.  Equivalent of 2D normal.
	ddgVector2 o(void) { return ddgVector2(d[1],d[0]); }

	/// Given X, or Y, find the other.  Return ddgFailure if unsolvable.
	inline bool solve( ddgVector2 *v, int dim)
	{
		if (d[dim] == 0.0)
			return ddgFailure;
		float t = (v->v[dim] - p.v[dim])/d[dim];
		*v = p + (d * t);
		return ddgSuccess;
	}
	/// Calculate the intersection point of this line with another.
	bool intersect( ddgLine2 *l, ddgVector2 *pt)
	{
		if (d[1] == 0 )
		{
			ddgAsserts(0,"Special case");
		}
		if (d[1] == l->d[1] && d[0] == l->d[0])
		{
			return ddgFailure;		// Parallel lines.
		}
		float x = (d[0]*p[0] - l->d[0]*l->p[0]+d[1]*+p[1]- l->d[1]*l->p[1])*(l->d[1]*d[1]);
		x /= (d[1]*d[0] - l->d[1]*l->d[0]);
		float y = (d[1]*p[1]+d[0]*p[0]-d[0]*x ) / d[1];
		pt->set(x,y);
		return ddgSuccess;
	}
	/// Returns [-1,1], positive if point is left of line, negative if right, 0 if on.
	inline float isPointLeftOfLine( ddgVector2 *q)
	{
		ddgVector2 vq = *q - p;

	    return o().dot(&vq);
	}
};

/**
 * 3D Line object
 */
class WEXP ddgLine3 {
public:
	/// A point on the line.
	ddgVector3	p;
	/// Slope vector of the line.
	ddgVector3	d;
	/// Constructor using 2 points.
	ddgLine3( const ddgVector3 p1, const ddgVector3 p2 )
	{
		p.set(&p1);
		d.set(&p2);
		d -= p1;
		d.normalize();
	}
	/// Given X, Y or Z coord, find the other 2.  Return ddgFailure if unsolvable.
	inline bool solve( ddgVector3 *u, int dim)
	{
		if (d[dim] == 0.0)
			return ddgFailure;
		float n = ((*u)[dim] - p[dim])/d[dim];

		*u =d + ( p * n);
		return ddgSuccess;
	}

	/// Return the intersection point of this line with another.
	ddgVector3 intersect( ddgLine3 *)
	{
		ddgVector3 pt(0,0,0);
		return pt;
	}
	/// Calculate the intersection point of this line with another.
	bool intersect( ddgLine3 *l, ddgVector3 *pt)
	{
		// $TODO this function is simply wrong...
		if (d[1] == 0 )
		{
			ddgAsserts(0,"Special case");
		}
		if (d[1] == l->d[1] && d[0] == l->d[0])
		{
			return ddgFailure;		// Parallel lines.
		}
		float x = (d[0]*p[0] - l->d[0]*l->p[0]+d[1]*+p[1]- l->d[1]*l->p[1])*(l->d[1]*d[1]);
		x /= (d[1]*d[0] - l->d[1]*l->d[0]);
		float y = (d[1]*p[1]+d[0]*p[0]-d[0]*x ) / d[1];
		// Dirty hack.
		pt->set(x,y,p[2]);
		return ddgSuccess;
	}
};


#endif
