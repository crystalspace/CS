/*
    Copyright (C) 2000 by Norman Kramer
  
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

#include "cssysdef.h"
#include "csgeom/quaterni.h"

#define EULERTOQUATCONST 0.0174532925199 // PI*1/(360/2)
#define QUATTOAXISANGLECONST 114.591559026 // (360/2)*(1/PI)*2

void csQuaternion::SetWithEuler(const csVector3 &rot)
{

	float rx,ry,rz,tx,ty,tz,cx,cy,cz,sx,sy,sz,cc,cs,sc,ss;

	// FIRST STEP, CONVERT ANGLES TO RADIANS
	rx = rot.x * (float)EULERTOQUATCONST;
	ry = rot.y * (float)EULERTOQUATCONST;
	rz = rot.z * (float)EULERTOQUATCONST;
	// GET THE HALF ANGLES
	tx = rx * (float)0.5;
	ty = ry * (float)0.5;
	tz = rz * (float)0.5;
	cx = (float)cos(tx);
	cy = (float)cos(ty);
	cz = (float)cos(tz);
	sx = (float)sin(tx);
	sy = (float)sin(ty);
	sz = (float)sin(tz);

	cc = cx * cz;
	cs = cx * sz;
	sc = sx * cz;
	ss = sx * sz;

	x = (cy * sc) - (sy * cs);
	y = (cy * ss) + (sy * cc);
	z = (cy * cs) - (sy * sc);
	r = (cy * cc) + (sy * ss);
}

csQuaternion csQuaternion::ToAxisAngle() const
{
  float invscale,tr;

	tr = (float)acos(r);
	invscale = 1.0f / ((float)sin(tr));
	return csQuaternion(tr * (float)QUATTOAXISANGLECONST, 
                            x * invscale, y * invscale, z * invscale);
}


#define SLERPDELTA 0.0001 // Difference at which to lerp instead of slerp

csQuaternion csQuaternion::Slerp(const csQuaternion &quat2, float slerp) const
{

	double omega,cosom,invsinom,scale0,scale1;

	// Calculate dot between quats
	cosom = x * quat2.x + y * quat2.y + z * quat2.z + r * quat2.r;

	// Make sure the two quaternions are not exactly opposite? (within a little slop)
	if ((1.0 + cosom) > SLERPDELTA)
	{
		// Are they more than a little bit different?  Avoid a divided by zero and lerp if not
		if ((1.0 - cosom) > SLERPDELTA) {
			// Yes, do a slerp
			omega = acos(cosom);
			invsinom = 1.0 / sin(omega);
			scale0 = sin((1.0 - slerp) * omega) * invsinom;
			scale1 = sin(slerp * omega) * invsinom;
		} else {
			// Not a very big difference, do a lerp
			scale0 = 1.0 - slerp;
			scale1 = slerp;
		}
		return csQuaternion(scale0 * r + scale1 * quat2.r, 
                                    scale0 * x + scale1 * quat2.x, 
                                    scale0 * y + scale1 * quat2.y, 
                                    scale0 * z + scale1 * quat2.z);
	}
	// The quaternions are nearly opposite so to avoid a divided by zero error
	// Calculate a perpendicular quaternion and slerp that direction
	scale0 = sin((1.0 - slerp) * PI);
	scale1 = sin(slerp * PI);
	return csQuaternion(scale0 * r + scale1 * quat2.z, 
                            scale0 * x + scale1 * -quat2.y, 
                            scale0 * y + scale1 * quat2.x, 
                            scale0 * z + scale1 * -quat2.r);
}

