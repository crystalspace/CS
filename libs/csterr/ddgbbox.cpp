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
// 
#include "sysdef.h"
#include "csterr/ddgbbox.h"

// ----------------------------------------------------------------------
// Initialization of global variables.
short ddgBBox::_corner[8][3] = {
    {-1,-1,-1},{1,-1,-1},{-1,1,-1},{1,1,-1},
    {-1,-1, 1},{1,-1, 1},{-1,1, 1},{1,1, 1}};

// Calculate a corner of the ddgBBox.
	float ddgBBox::cornerx(int n) { return (n%2 == 0) ?_min.v[0] : _max.v[0];}
	float ddgBBox::cornery(int n) { return (n==0||n==1||n==4||n==5) ?_min.v[1]: _max.v[1];}
float ddgBBox::cornerz(int n) { return (n < 4) ?_min.v[2] : _max.v[2];}

// ----------------------------------------------------------------------
// MinMax:
//    Initialize the ddgBBox with minimum and maximum values.
//
void ddgBBox::set(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{
  _min.v[0] = xmin;
  _max.v[0] = xmax;
  _min.v[1] = ymin;
  _max.v[1] = ymax;
  _min.v[2] = zmin;
  _max.v[2] = zmax;
}
// ----------------------------------------------------------------------
// MinMax:
//    Initialize the ddgBBox with minimum and maximum vectors.
//
ddgBBox::ddgBBox(ddgVector3 *min, ddgVector3 *max)
{
  _min.set(min);
  _max.set(max);
}
// ----------------------------------------------------------------------
// ddgBBox:
//   Initialize the ddgBBox based on a centre point and deltas in 3 directions.
//
ddgBBox::ddgBBox(float xc, float xd, float yc, float yd, float zc, float zd)
{
  _min.set( xc - xd, yc - yd, zc - zd);
  _max.set( xc + xd, yc + yd, zc + zd);
}

// ----------------------------------------------------------------------
// copy:
//   Copy the contents from a given bbox into this one.
void ddgBBox::copy(ddgBBox *src)
{
  _min.set(src->_min);
  _max.set(src->_max);
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
void ddgBBox::split(Split side, float value )
{
  if (value == 0)
    {
    switch(side)
    {
      case XLT:  _max.v[0] = (_min.v[0] + _max.v[0])/2.0; break;
      case XGT:  _min.v[0] = (_min.v[0] + _max.v[0])/2.0; break;
      case YLT:  _max.v[1] = (_min.v[1] + _max.v[1])/2.0; break;
      case YGT:  _min.v[1] = (_min.v[1] + _max.v[1])/2.0; break;
      case ZLT:  _max.v[2] = (_min.v[2] + _max.v[2])/2.0; break;
      case ZGT:  _min.v[2] = (_min.v[2] + _max.v[2])/2.0; break;
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

float ddgBBox::distancesq(ddgVector3 *eye)
{
	ddgVector3 c(_min + _max);
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
	if (INSIDE2(_min[d2],_min[d3],_max[d2],_max[d3],iv,jv)) \
		return true; \
}
// Calculate the min and max values in a dimension and test.
#define CALCDIM(d1,d2,d3) CALCTEST(_min,d1,d2,d3) CALCTEST(_max,d1,d2,d3)
// Test for intersection of line with bbox.
bool ddgBBox::intersect( ddgVector3 *p1, ddgVector3 *p2)
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
bool ddgBBox::intersect( ddgBBox *b )
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


ddgClipFlags ddgBBox::visibleSpace( ddgBBox b, float tanHalfFOV )
{
	ddgClipFlags vis;
	vis.visibility = 0;

#ifdef DDG
	// Flip Z axis. (Could be eliminated).
	b.setz(-1*b.maxz(),-1*b.minz());
#endif
	if (tanHalfFOV != 1.0)		// Not 90 degree case
		b.scale(ddgVector3(1.0/tanHalfFOV,1.0/tanHalfFOV,1));
	// Test against near, and far plane and test viewing frustrum.
	if (b.maxz() >= minz())
		vis.flags.nin = true;
	if (b.minz() <= maxz())
		vis.flags.fin = true;
	if (vis.flags.fin && vis.flags.nin)
	{
		if (b.maxx() > 0 || fabs(b.maxx()) <= b.maxz() )
			vis.flags.lin = true;		// In left
		if (b.minx() < 0 || b.minx() <= b.maxz() )
			vis.flags.rin = true;		// In right
		if (b.miny() < 0 || b.miny() <= b.maxz() )
			vis.flags.tin = true;		// In top
		if (b.maxy() > 0 || fabs(b.maxy()) <= b.maxz() )
			vis.flags.bin = true;		// In bottom
	}
	// Check if bounding box is totally within the viewing volume.
	if (vis.visibility == 63)
	{
	    if ((b.minz() >= minz())
			&& (b.maxz() <= maxz())
			&& (b.minx() > 0 || fabs(b.minx()) <= b.minz() )
			&& (b.maxx() < 0 || b.maxx() <= b.minz() )
			&& (b.maxy() < 0 || b.maxy() <= b.minz() )
			&& (b.miny() > 0 || fabs(b.miny()) <= b.minz() ))
			vis.flags.all = true;			// All Inside
	}
	else
		vis.flags.none = true;				// All Outside.

	return vis;
}
