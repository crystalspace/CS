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
	float ddgBBox::cornerx(int n) { return (n%2 == 0) ?_min.x : _max.x;}
	float ddgBBox::cornery(int n) { return (n==0||n==1||n==4||n==5) ?_min.y: _max.y;}
float ddgBBox::cornerz(int n) { return (n < 4) ?_min.z : _max.z;}


// ----------------------------------------------------------------------
// MinMax:
//    Initialize the ddgBBox with minimum and maximum vectors.
//
ddgBBox::ddgBBox(csVector3 min, csVector3 max)
{
  _min = min;
  _max = max;
}


ddgClipFlags ddgBBox::visibleSpace( ddgBBox b, float tanHalfFOV )
{
	ddgClipFlags vis;
	vis.visibility = 0;

	if (tanHalfFOV != 1.0)		// Not 90 degree case
		b.scale(csVector3(1.0/tanHalfFOV,1.0/tanHalfFOV,1));
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
