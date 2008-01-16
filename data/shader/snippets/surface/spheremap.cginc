<!--
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Jorrit Tyberghein
	    (C) 2006 by Marten Svanfeldt

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
-->
<include><![CDATA[

#ifndef __SPHEREMAP_CG_INC__
#define __SPHEREMAP_CG_INC__

float2 spheremap (float3 dir)
{
  // Derived from GL spec
  float inv_m = 0.5*rsqrt (dir.x*dir.x + dir.y*dir.y + (dir.z+1)*(dir.z+1));
  return (dir*inv_m+0.5).xy;
}
  
#endif // __SPHEREMAP_CG_INC__
 
]]></include>
