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
#ifndef _ddgCHull_Class_
#define _ddgCHull_Class_

#include "util/ddg.h"
#include "math/ddgvec.h"
class ddgBBox3;
class ddgPlane2;
class ddgRect2;
class ddgPlane3;
class ddgPrism3;
class ddgTriangle3;

enum ddgInside { ddgOUT=0, ddgPART=1, ddgIN=2, ddgUNDEF = 4};

/**
 * These classes define a clipping hierarchy of positive and negative
 * volumes, (portals and occluders).
 */
class ddgCHullSet;
/**
 *  The Convex Hull class defines a volume defined by a set of planes.
 */
class WEXP ddgCHull3 {
public:
	/// A set of planes defining the hull.
	ddgPlane3	*planes;
	/// The number of planes in the hull.
	int			noPlanes;
	/// A subtree of hulls which are applied in the negative sense of the current.
	ddgCHullSet	*subhulls;
	/// Default Constructor.
	ddgCHull3(void) : planes(0), noPlanes(0), subhulls(0) {}
	/// Constructor.
	ddgCHull3(ddgPlane3 *p, int np) : planes(p), noPlanes(np), subhulls(0) {}
	/**
	 *  Clip bounding box against hull, return ddgIN if bbox is entirely within,
	 *  return ddgOUT if entirely outside, return ddgPART if part in and part out.
	 */
	ddgInside clip( ddgBBox3 *bbox, bool occluder = false);
	/**
	 *  Clip prism against hull, return ddgIN if it is entirely within,
	 *  return ddgOUT if entirely outside, return ddgPART if part in and part out.
	 */
	ddgInside clip( ddgPrism3 *prism, bool occluder = false);
	/**
	 *  Clip triangle against hull, return ddgIN if it is entirely within,
	 *  return ddgOUT if entirely outside, return ddgPART if part in and part out.
	 */
	ddgInside clip( ddgTriangle3 *triangle, bool occluder = false);
};

/**
 *	The Hull set defines a set of clipping volumes.
 */
class WEXP ddgCHullSet {
public:
	/// A set of clipping volumes.
	ddgCHull3	*_hulls;
	/// Number of volumes.
	int			_noVolumes;
	/// Constructor.
	ddgCHullSet( int nv );
	/// Destructor
	~ddgCHullSet(void);
	/**
	 *  Clip a bounding box agains all hulls defined in the set.
	 *  returns the following
	 *             inclusive
	 *  ddgIN		true		- bbox is entirely within at least one volume.
	 *  ddgIN		false		- bbox is entirely outside all volumes.
	 *  ddgOUT		true		- bbox is entirely outside all volumes.
	 *  ddgOUT		false		- bbox is entirely within at least one volume.
	 *  ddgPART					- in all other cases.
	 */
	ddgInside clip( ddgBBox3 *bbox, bool occluder = false);
	///
	ddgInside clip( ddgTriangle3 *tri, bool occluder = false);
	///
	ddgInside clip( ddgPrism3 *tri, bool occluder = false);

};

/**
 * 2D version of the convex hull.  Equivalent to a convex polygon.
 */

class WEXP ddgCHull2 {
public:
	/// Number of points.
	unsigned short noLines;
	/// Array of points.
	ddgPlane2		*lines;
	/// Default Constructor.
	ddgCHull2(void) : noLines(0), lines(0) {}
	/// Constructor.
	ddgCHull2(ddgPlane2 *l, int nl) : noLines(nl), lines(l) {}
	/**
	 *  Clip bounding rectangle against hull, return ddgIN if rectangle is entirely within,
	 *  return ddgOUT if entirely outside, return ddgPART if part in and part out.
	 */
	ddgInside clip( ddgRect2 *rect, bool occluder = false);
};
#endif
