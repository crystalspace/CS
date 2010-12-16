<!--
  Copyright (C) 2006-2008 by Frank Richter
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

//
// WORKAROUNDS
//

/* 'half' texture functions result in an "error C5201: invalid internal
   function declaration" on non-NV */
#if !defined(PROFILE_FP30) && !defined(PROFILE_FP40) && !defined(PROFILE_GP4FP)
#define h4tex2D   tex2D
#endif

//
// TOOL FUNCTIONS
//

// EyeVec - eye to vertex
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
    
// Copied this from some NVidia doc
float fresnel_2(float3 eyeToVert, float3 normal, float scale, float bias, float power) 
{ 
    return saturate (bias + scale * pow (1.0+dot(eyeToVert, normal), power));
} 

float fresnel(float3 view, float3 normal) 
{ 
    // Calculated from 1.3333, which is approx. refraction index for water
    // R0 is: 
    // float const R0 =  pow(1.0-refractionIndexRatio, 2.0)  
    //                 / pow(1.0+refractionIndexRatio, 2.0); 
    const float R0 = 0.02040781341208;

    // light and normal are assumed to be normalized  
    return fresnel_2 (-view, normal, 1.0-R0, R0, 5);
} 

// From:
//     "An Approximate Image-Space Approach for Interactive Refraction"
//     by Chris Wyman, University of Iowa.  
// http://www.cs.uiowa.edu/~cwyman/publications/files/approxISRefract/refrShader.fragment.cg
//
// Function to compute a refraction direction.  Evidently, the built-in Cg refraction
//    computes pseudo-refraction vectors, as per previous work in the area.
float3 refraction( float3 incident, float3 normal, float ni_nt, float ni_nt_sqr )
{
	float IdotN = dot( -incident, normal );
	float cosSqr = 1 - ni_nt_sqr*(1 - IdotN*IdotN);
	if (cosSqr < 0) 
		cosSqr = 0;
	else
		cosSqr = sqrt( cosSqr );
	return  normalize( ni_nt * incident + (ni_nt* IdotN - cosSqr) * normal ); 
}

// Calculates a vertex offset for wind simulation
// currTime: standard_time
// maxOffset: wind_direction (normalised vec2) * wind_speed (float)
// biasFactor: The bias towards the wind direction (0.0 - 1.0)
// randomFactor: per-instance randomness
float2 calcWindOffset(float currTime, float2 maxOffset, float biasFactor, float randomFactor)
{
	// The motion of the vertex follows a sine wave.
	// Each instance has a random factor to make the wave a little unique.
	// A bias is applied to the result of the sine, tending the movement towards the wind direction.
	// This result (-1.0 to 1.0) is then multiplied with the maxOffset to determine the actual offset.

	float sinePosition = sin(currTime + randomFactor);
	float biasedPosition = (1.0 - biasFactor) * sinePosition + biasFactor;
	return maxOffset * biasedPosition;
}

#endif // __CG_COMMON_CG_INC__
 
]]></include>
