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
// 
#include "math/ddgbbox.h"

// ----------------------------------------------------------------------
// Initialization of global variables.
short ddgBBox3::_corner[8][3] = {
    {-1,-1,-1},{1,-1,-1},{-1,1,-1},{1,1,-1},
    {-1,-1, 1},{1,-1, 1},{-1,1, 1},{1,1, 1}};

// Calculate a corner of the ddgBBox.
float ddgBBox3::cornerx(int n) { return (n%2 == 0) ?min[0] : max[0];}
float ddgBBox3::cornery(int n) { return (n==0||n==1||n==4||n==5) ?min[1]: max[1];}
float ddgBBox3::cornerz(int n) { return (n < 4) ?min[2] : max[2];}

// ----------------------------------------------------------------------
// MinMax:
//    Initialize the ddgBBox with minimum and maximum values.
//
void ddgBBox3::set(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{
	min = ddgVector3(xmin,ymin,zmin);
	max = ddgVector3(xmax,ymax,zmax);
}
// ----------------------------------------------------------------------
// MinMax:
//    Initialize the ddgBBox with minimum and maximum vectors.
//
ddgBBox3::ddgBBox3(const ddgVector3 &pmin, const ddgVector3 &pmax)
{
	min = pmin;
	max = pmax;
}

// ----------------------------------------------------------------------
// ddgBBox3:
//   Initialize the ddgBBox based on a centre point and deltas in 3 directions.
//
ddgBBox3::ddgBBox3(float xc, float xd, float yc, float yd, float zc, float zd)
{
	min = ddgVector3( xc - xd, yc - yd, zc - zd);
	max = ddgVector3( xc + xd, yc + yd, zc + zd);
}

// ----------------------------------------------------------------------
// copy:
//   Copy the contents from a given bbox into this one.
void ddgBBox3::copy(ddgBBox3 *src)
{
  min = src->min;
  max = src->max;
}

// ----------------------------------------------------------------------
// split:
//   Split the current Bounding Box in to two pieces and reassign the box
//   to be one of the remaining parts.
// side:
//    The side indicates along which axis the split should take place.
//    It also indicates which part the box should take.
// value:
// If the value is 0, the box is split along the middle.
// If the value is not 0, the box is split at value, where value must
// be within the axis bounds along that axis.
// 
void ddgBBox3::split(Split side, float value )
{
  if (value == 0)
    {
    switch(side)
    {
      case XLT:  max.v[0] = (min.v[0] + max.v[0])/2.0; break;
      case XGT:  min.v[0] = (min.v[0] + max.v[0])/2.0; break;
      case YLT:  max.v[1] = (min.v[1] + max.v[1])/2.0; break;
      case YGT:  min.v[1] = (min.v[1] + max.v[1])/2.0; break;
      case ZLT:  max.v[2] = (min.v[2] + max.v[2])/2.0; break;
      case ZGT:  min.v[2] = (min.v[2] + max.v[2])/2.0; break;
      }
    }
  else
    {
	/*
    float ftmp;
    switch(side)
    {
      case XLT:  ftmp = _delta[0]; _delta.v[0] = (value - cornerx(0))/2.0;
                 _centre.v[0] += -ftmp + _delta[0]; break;
      case XGT:  ftmp = _delta.v[0]; _delta.v[0] = (cornerx(7) - value)/2.0;
                 _centre.v[0] += ftmp - _delta[0]; break;
      case YLT:  ftmp = _delta[1]; _delta.v[1] = (value - cornery(0))/2.0;
                 _centre.v[1] += -ftmp + _delta[1]; break;
      case YGT:  ftmp = _delta[1]; _delta.v[1] = (cornery(7) - value)/2.0;
                 _centre.v[1] += ftmp - _delta[1]; break;
      case ZLT:  ftmp = _delta[2]; _delta.v[2] = (value - cornerz(0))/2.0;
                 _centre.v[2] += -ftmp + _delta[2]; break;
      case ZGT:  ftmp = _delta[2]; _delta.v[2] = (cornerz(7) - value)/2.0;
                 _centre.v[2] += ftmp - _delta[2]; break;
      }
	  */
    }
}

// ----------------------------------------------------------------------
// distance:
//   Return the square of the distance to the centre of the box and the
// given coordinate.
// ex, ey, ez:
//    3D point in space.

float ddgBBox3::distancesq(ddgVector3 *eye)
{
	ddgVector3 c(min + max);
	c.divide(2);
	ddgVector3 d(c - *eye);
	return d.sizesq();
}

// Is 1D point inside range.
#define INSIDE(m1,m2,v) (m1 <= v && m2 >= v)
// Is 2D point inside rect.
#define INSIDE2(i1,j1,i2,j2,iv,jv) (INSIDE(i1,i2,iv)&&INSIDE(j1,j2,jv))
// Calculate values in given dimension and test them.
#define CALCTEST(mm,d1,d2,d3) \
{ \
	t = (mm[d1] - p1->v[d1]) / d[d1]; \
	iv = p1->v[d2] + t * d[d2]; \
	jv = p1->v[d3] + t * d[d3]; \
	if (INSIDE2(min[d2],min[d3],max[d2],max[d3],iv,jv)) \
		return true; \
}
// Calculate the min and max values in a dimension and test.
#define CALCDIM(d1,d2,d3) CALCTEST(min,d1,d2,d3) CALCTEST(max,d1,d2,d3)
// Test for intersection of line with bbox.
bool ddgBBox3::intersect( ddgVector3 *p1, ddgVector3 *p2)
{
	ddgVector3 d(*p2 - *p1); // Slope of line.
	float	t;
	float   iv,jv;
	// Test each face for inter section as follows:
	// For each dimension
	// Substitute a min and max value in the equation
	// and find the point in 3space. then test if this
	// point is within the bounds of the other 2 dimensions.
	// If any point satisfies, we intersect.
	CALCDIM(0,1,2)
	CALCDIM(1,0,2)
	CALCDIM(2,0,1)

	return false;
}
// Test for intersection of another bbox.
bool ddgBBox3::intersect( ddgBBox3 *b )
{
	if (cornerx(0) > b->cornerx(1)
		|| cornerx(1) < b->cornerx(0)
		|| cornery(1) > b->cornery(5)
		|| cornery(5) < b->cornery(1)
		|| cornerz(0) > b->cornerz(2)
		|| cornerz(2) < b->cornerz(0))
		return false;
	else return true;
}


ddgClipFlags ddgBBox3::visibleSpace( ddgBBox3 b, float tanHalfFOV )
{
	ddgClipFlags vis = 0;

	// Flip Z axis. (Could be eliminated).
	b.setz(-1*b.maxz(),-1*b.minz());

	if (tanHalfFOV != 1.0)		// Not 90 degree case
		b.scale(ddgVector3(1.0/tanHalfFOV,1.0/tanHalfFOV,1));
	// Test against near, and far plane and test viewing frustrum.
	if (b.maxz() >= minz())
		DDG_BSET(vis, DDGCF_NIN);
	if (b.minz() <= maxz())
		DDG_BSET(vis, DDGCF_FIN);
	if (DDG_BGET(vis, DDGCF_FIN) && DDG_BGET(vis, DDGCF_NIN))
	{
		if (b.maxx() > 0 || fabs(b.maxx()) <= b.maxz() )
			DDG_BSET(vis, DDGCF_LIN);		// In left
		if (b.minx() < 0 || b.minx() <= b.maxz() )
			DDG_BSET(vis, DDGCF_RIN);		// In right
		if (b.miny() < 0 || b.miny() <= b.maxz() )
			DDG_BSET(vis, DDGCF_TIN);		// In top
		if (b.maxy() > 0 || fabs(b.maxy()) <= b.maxz() )
			DDG_BSET(vis, DDGCF_BIN);		// In bottom
	}
	// Check if bounding box is totally within the viewing volume.
	if (DDG_BGET(vis, DDGCF_VISIBILITY) == DDGCF_VISIBILITY)
	{
	    if ((b.minz() >= minz())
			&& (b.maxz() <= maxz())
			&& (b.minx() > 0 || fabs(b.minx()) <= b.minz() )
			&& (b.maxx() < 0 || b.maxx() <= b.minz() )
			&& (b.maxy() < 0 || b.maxy() <= b.minz() )
			&& (b.miny() > 0 || fabs(b.miny()) <= b.minz() ))
			DDG_BSET(vis, DDGCF_ALLIN);		// All Inside
	}
	else
		DDG_BSET(vis, DDGCF_ALLOUT);			// All Outside.


	return vis;
}
