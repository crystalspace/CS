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

#ifndef __LIGHTFUNCS_CG_INC__
#define __LIGHTFUNCS_CG_INC__

float Attenuation_Linear (float d, float invLightRadius)
{
  return max (1 - d * invLightRadius, 0);
}

float Attenuation_Inverse (float d)
{
  return 1/d;
}

float Attenuation_Realistic (float d)
{
  return 1/(d*d);
}

float Attenuation_CLQ (float d, float3 coeff)
{
  return 1/(coeff.x + d*coeff.y + d*d*coeff.z);
}


float Light_Spot (float3 surfNorm, float3 surfToLight, 
                  float3 lightDir, float falloffInner, float falloffOuter)
{
  float a = smoothstep (falloffOuter, falloffInner, -dot (surfToLight, lightDir));
  return max (a, 0);
}

#endif // __LIGHTFUNCS_CG_INC__
 
]]></include>
