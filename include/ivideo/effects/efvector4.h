/*
    Copyright (C) 2002 by Mårten Svanfeldt
    Written by Mårten Svanfeldt

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

#ifndef __EFVECTOR4_H__
#define __EFVECTOR4_H__

#include "csgeom/vector3.h"
#include "csgfx/rgbpixel.h"

struct csEffectVector4
{
	float x, y, z, w;

	csEffectVector4() {x=0.0f;y=0.0f;z=0.0f;w=1.0f;}
	csEffectVector4(float value)  { x=value; y=value; z=value; w=1.0f;}
	csEffectVector4(float _x, float _y,float _z) { x=_x; y=_y; z=_z; w=1.0f;}
	csEffectVector4(float _x, float _y,float _z,float _w) { x=_x; y=_y; z=_z; w=_w;}
	csEffectVector4(const csVector3 &vec) { x=vec.x; y = vec.y; z = vec.z; w=1.0f;}
	csEffectVector4(const csRGBpixel &color) {x=color.red; y=color.green; z=color.blue; w=color.alpha;}
	csEffectVector4(const csEffectVector4 &vec){  x=vec.x; y = vec.y; z = vec.z; w=vec.w;}
};

#endif // __EFVECTOR4_H__
