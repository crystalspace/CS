/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#include <math.h>
#include "cssysdef.h"
#include "csgeom/plane3.h"
#include "csgeom/math3d.h"

//---------------------------------------------------------------------------

csPlane3::csPlane3 (const csVector3& v1, const csVector3& v2,
	const csVector3& v3)
{
  csMath3::CalcNormal (norm, v1, v2, v3);
  DD = - norm * v1;
}

void csPlane3::Set (const csVector3& v1, const csVector3& v2,
	const csVector3& v3)
{
  csMath3::CalcNormal (norm, v1, v2, v3);
  DD = - norm * v1;
}

//---------------------------------------------------------------------------
