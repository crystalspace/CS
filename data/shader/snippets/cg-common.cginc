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

#ifndef __CG_COMMON_CG_INC__
#define __CG_COMMON_CG_INC__

float2 ComputeParallaxOffset (sampler2D TexHeight, float2 OriginalCoord, 
			      float3 EyeVec, float OffsetScale)
{
  // Sample height
  float height = (tex2D (TexHeight, OriginalCoord).x - 0.5) * OffsetScale;
      
  // Compute offset
  float2 offset = EyeVec.xy * height;
    
  return offset;
}
 
float3 IntersectSurface(sampler2D TexSurfaceHeight,
			float2 StartPoint,
			float3 RayDirection)
{
  // Start values
  float3 I = float3 (StartPoint,0);
  
  float hScale = 0.25;
  
  // Step
  for (float i = 0; i < 4; i += 1)
  {
    float height = tex2D (TexSurfaceHeight, I.xy).w * hScale;
    float dh = height - I.z;
    I += dh * RayDirection; 
  }
  
  return I;
}
    
#endif // __CG_COMMON_CG_INC__
 
]]></include>
