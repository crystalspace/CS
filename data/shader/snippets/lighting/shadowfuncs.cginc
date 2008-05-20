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
<include>
<![CDATA[

#ifndef __SHADOWFUNCS_CG_INC__
#define __SHADOWFUNCS_CG_INC__

/* Point and spot light shadows are computed by separately lighting up 
    to 6 pyramids with an opening of 90 degs with a shadow map for each
    of these light volumes.
    Clip so points outside the light volume aren't lit.
  */
struct ShadowClipper
{
  bool IsClipped (float2 shadowMapCoordsProjUnscaled,
                  float4 shadowMapCoords)
  {
  ]]>
  <?if (vars."light type".int != consts.CS_LIGHT_DIRECTIONAL) ?>
  <![CDATA[
    bool2 compResLT = shadowMapCoordsProjUnscaled.xy >= float2 (-1);
    bool2 compResBR = shadowMapCoordsProjUnscaled.xy < float2 (1);
    return compResLT.x && compResLT.y && compResBR.x && compResBR.y
      && (shadowMapCoords.z <= shadowMapCoords.w);
  ]]>
  <?endif?>
  <![CDATA[
    return false;
  }
  
  half ClipFactor (float2 shadowMapCoordsProjUnscaled,
                  float4 shadowMapCoords)
  {
    half factor = 1;
  ]]>
  <?if (vars."light type".int != consts.CS_LIGHT_DIRECTIONAL) ?>
  <![CDATA[
    float2 compResLT = shadowMapCoordsProjUnscaled.xy >= float2 (-1);
    float2 compResBR = shadowMapCoordsProjUnscaled.xy < float2 (1);
    factor = compResLT.x*compResLT.y*compResBR.x*compResBR.y;
    factor *= (shadowMapCoords.z <= shadowMapCoords.w);
  ]]>
  <?endif?>
  <![CDATA[
    return factor;
  }
};

interface ShadowSampler
{
  half GetVisibility ();
};

struct ShadowSamplerSimple : ShadowSampler
{
  half GetVisibility (sampler2D shadowMap, float2 coords, float compareDepth)
  {
    return h4tex2D (shadowMap, float3 (coords, compareDepth)).x;
  }
};

struct ShadowSamplerNoisy : ShadowSampler
{
  float2 shadowMapCoordsProjUnscaled;

  void Init (float2 _shadowMapCoordsProjUnscaled)
  {
    shadowMapCoordsProjUnscaled = _shadowMapCoordsProjUnscaled;
  }

  half GetVisibility (sampler2D shadowMap, float2 coords, float compareDepth)
  {
    // Noise pattern coordinates
    // @@@ FIXME: These should probably be parameters
    float2 noiseMapScale = float2(32.0);
    float2 noiseScale = float2(1.0/256.0);
    
    float2 noiseCoords = shadowMapCoordsProjUnscaled * noiseMapScale;
    half2 noise = h4tex2D (lightPropsSM.shadowMapNoise, noiseCoords).xy * noiseScale;
      
    // @@@ The offsets could probably be better.
    half inLight;
    inLight = h4tex2D (shadowMap, float3 (coords, compareDepth)).x;
    inLight += h4tex2D (shadowMap, float3 (coords+noise*half2(1,1), compareDepth)).x;
    inLight += h4tex2D (shadowMap, float3 (coords+noise*half2(-1,1), compareDepth)).x;
    inLight += h4tex2D (shadowMap, float3 (coords+noise*half2(1,-1), compareDepth)).x;
    inLight += h4tex2D (shadowMap, float3 (coords+noise*half2(-1,-1), compareDepth)).x;
    inLight += h4tex2D (shadowMap, float3 (coords+noise*half2(0.5,0.5), compareDepth)).x;
    inLight += h4tex2D (shadowMap, float3 (coords+noise*half2(-0.5,0.5), compareDepth)).x;
    inLight += h4tex2D (shadowMap, float3 (coords+noise*half2(0.5,-0.5), compareDepth)).x;
    inLight += h4tex2D (shadowMap, float3 (coords+noise*half2(-0.5,-0.5), compareDepth)).x;
    inLight *= 1.0/9.0;
    
    return inLight;
  }
};

#endif // __SHADOWFUNCS_CG_INC__
 
]]>
</include>
