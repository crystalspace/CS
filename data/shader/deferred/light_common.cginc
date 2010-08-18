<!--
  Copyright (C) 2007 by Frank Richter
	        (C) 2007 by Jorrit Tyberghein
            (C) 2010 by Joe Forte

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
<include>
<![CDATA[

#ifndef __LIGHT_COMMON_CG_INC__
#define __LIGHT_COMMON_CG_INC__

// Computes linear attenuation.
float Attenuation_Linear(float d, float invLightRadius)
{
  return saturate (1 - d * invLightRadius);
}

// Computes constant, linear, quadratic (CLQ) attenuation.
float Attenuation_CLQ(float d, float3 coeff)
{
  return 1 / dot (float3 (1, d, d*d), coeff);
}

// Computes the attenuation for the current point light.
float Attenuation(float dist, float4 attenVec)
{
  float invAttenRadius = attenVec.w;
  
  if (invAttenRadius > 0)
    return Attenuation_Linear (dist, invAttenRadius);

  return Attenuation_CLQ (dist, attenVec.xyz);
}

// Extracts the depth from the given depth buffer sample.
float ExtractDepth(float4 depth)
{
  return 1 - 2 * depth.x;
}

// Extracts the normal from the given normal buffer sample.
float3 ExtractNormal(float4 normal)
{
  return normal.xyz * 2.0 - 1.0;
  
  /* Extract normal where only x and y are stored.
  float2 nxy = normal.xy * 2.0 - 1.0;
  float z = -sqrt (1 - dot (nxy));
  
  return float3 (nxy, z);
  */
}

// Returns the screen position given the projected screen XY 
// position and the depth buffer sample.
float3 GetScreenPosition(float2 screenXY, float4 depth)
{
  return float3 (screenXY.x, -screenXY.y, ExtractDepth (depth));
}

// Unproject screen position.
float3 ScreenToWorldPosition(float3 screen, float4x4 ProjInv)
{  
  float4 world = mul (ProjInv, float4 (screen, 1.0));
  return world.xyz / world.w;
}

#endif // __LIGHT_COMMON_CG_INC__
 
]]></include>
