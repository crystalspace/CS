<!--
  Copyright (C) 2008 by Frank Richter

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

#ifndef __CG_GLOBAL_OBJECTTOWORLD_CG_INC__
#define __CG_GLOBAL_OBJECTTOWORLD_CG_INC__

/*
  objectToWorld transform, set by the position snippet.
  Global variable instance of an (as usual) privately pulled input
  as objectToWorld may be set in different ways (notably in instanced
  vs 'normal' rendering) which only the position snippet really
  knows.
 */
float4x4 objectToWorld;

void SetGlobalObjectToWorld (float4x4 o2w)
{
  objectToWorld = o2w;
}

#endif // __CG_GLOBAL_OBJECTTOWORLD_CG_INC__
 
]]></include>
