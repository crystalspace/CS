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
// For timing info.
#include "sysdef.h"
#ifdef OS_WIN32
#include <time.h>
#elif OS_MACOS
#include <time.h>
#else
#include <sys/types.h>
#include <sys/times.h>
#endif
#include "csterr/ddgutil.h"

// ----------------------------------------------------------------------
//
// ddgUtil:
//

// ----------------------------------------------------------------------
float ddgAngle::_cosTable[180*ddgAngle_res];
float ddgAngle::pi = M_PI;
ddgAngle ddg_init;	// Ensure table gets initialized.

ddgAngle::ddgAngle()
{
  // Calculate from 0.0 to 180.0 degrees.
  for (int i = 0; i < (int)(180*ddgAngle_res); i++)
    {
    _cosTable[i] = ::cos(M_PI*(float)i/(float)(ddgAngle_res*180));
    }
}

// This should be inlined, but W32 wont let me use _cosTable in a DLL
// if it is inlined.
float ddgAngle::cos( float angle)
{
	float a = mod(angle);
	return a < 180.0f ?
		_cosTable[((int)a*ddgAngle_res)]:
		-_cosTable[((int)(a-180.0f)*ddgAngle_res)];
}





