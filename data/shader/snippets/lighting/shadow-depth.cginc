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
<?Include lightfuncs.cginc ?>
<?Include shadowfuncs.cginc ?>
<![CDATA[

#ifndef __SHADOW_DEPTH_CG_INC__
#define __SHADOW_DEPTH_CG_INC__

struct ShadowShadowMapDepth : ShadowShadowMap
{
  float4x4 shadowMapTF;
  float4 shadowMapCoords;
  float4 shadowMapCoordsProj;
  sampler2D shadowMap;
  float bias;
  float gradient;
  float4 shadowMapUnscale;

  void InitVP (int lightNum, float4 surfPositionWorld,
               float3 normWorld,
               out float4 vp_shadowMapCoords,
               out float vp_gradientApprox)
  {
    float4x4 lightTransformInv = lightProps.transformInv[lightNum];
    // Transform world position into light space
    float4 view_pos = mul(lightTransformInv, surfPositionWorld);
    // Transform position in light space into "shadow map space"
    float4 shadowMapCoords;
    shadowMapTF = lightPropsSM.shadowMapTF[lightNum];
    /* CS' render-to-texture Y-flips render targets (so the upper left
       gets rendered to 0,0), we need to unflip here again. */
    float4x4 flipY;
    flipY[0] = float4 (1, 0, 0, 0);
    flipY[1] = float4 (0, -1, 0, 0);
    flipY[2] = float4 (0, 0, 1, 0);
    flipY[3] = float4 (0, 0, 0, 1);
    shadowMapTF = mul (flipY, shadowMapTF);
    shadowMapCoords = mul (shadowMapTF, view_pos);
    
    vp_shadowMapCoords = shadowMapCoords;
    
    float3 normL = mul(lightTransformInv, float4 (normWorld, 0)).xyz;
    float3 normShadow = normalize (mul (shadowMapTF, float4 (normL, 0)).xyz);
    //float3 viewDirShadow = normalize (mul (shadowMapTF, float4 (0, 0, -1, 0)).xyz);
    vp_gradientApprox = 1-saturate (dot (normShadow, float3 (0, 0, -1)));
    
    /* @@@ FIXME: This should prolly be:
    float3 viewDirShadow = -normalize (shadowMapCoords.xyz);
    vp_gradientApprox = 1-saturate (dot (normShadow, viewDirShadow));
    */
  }
  
  void Init (int lightNum, float4 vp_shadowMapCoords, float vp_gradient)
  {
    shadowMapCoords = vp_shadowMapCoords;
    shadowMap = lightPropsSM.shadowMap[lightNum];
    gradient = vp_gradient;
    shadowMapUnscale = lightPropsSM.shadowMapUnscale[lightNum];
    
    // Project SM coordinates
    shadowMapCoordsProj = shadowMapCoords;
    shadowMapCoordsProj.xyz /= shadowMapCoordsProj.w;
    
    // FWIW, this should prolly be made some kind of setting.
    bias = 0.0000001;
    //bias *= 1 + (gradient*gradient*256);
  }
  
  half GetVisibility()
  {
    float2 shadowMapCoordsProjUnscaled = 
      (shadowMapCoordsProj.xy) * shadowMapUnscale.xy + shadowMapUnscale.zw;
    
    ShadowClipper clipper;
    if (clipper.IsClipped (shadowMapCoordsProjUnscaled, shadowMapCoordsProj.xyz,
        shadowMapCoords))
      return clipper.ClippedFactor ();

    float3 shadowMapCoordsBiased = (float3(0.5)*shadowMapCoordsProj.xyz) + float3(0.5);
    // Depth to compare against
    float compareDepth = (1-shadowMapCoordsBiased.z) - bias;
    
    // Depth compare with shadow map texel
    half inLight;
    //ShadowSamplerSimple sampler;
    ShadowSamplerNoisy sampler;
    sampler.Init (shadowMapCoordsProjUnscaled);
    inLight = sampler.GetVisibility (shadowMap, shadowMapCoordsBiased.xy, compareDepth);
    
    return inLight;
  }
};

#endif // __SHADOW_DEPTH_CG_INC__
 
]]>
</include>
