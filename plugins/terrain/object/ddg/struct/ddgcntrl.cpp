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
#include "struct/ddgcntrl.h"

ddgControl::ddgControl( ddgVector3 *pos, ddgVector3 *orientation)
{
	if (orientation)
		_rot.set(orientation);
	if (pos)
		_pos.set(pos);
	_dirty = true;
}

// Call this at the end of the subclass update method.
bool ddgControl::update(void)
{
	// Bring the angles within a reasonable range (0-360).
	ddgAngle::mod(_rot.v[YAW]);
	ddgAngle::mod(_rot.v[PITCH]);
	ddgAngle::mod(_rot.v[ROLL]);
	return _dirty;
}


