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
<![CDATA[

#ifndef __SHADOW_OSM_CG_INC__
#define __SHADOW_OSM_CG_INC__

struct ShadowShadowMapDepth : ShadowShadowMap
{
  float4x4 shadowMapTF;
  float4 shadowMapCoords;
  float4 shadowMapCoordsProj;
  sampler2D shadowMap;
  float bias;
  float gradient;

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
    
    // Project SM coordinates
    shadowMapCoordsProj = shadowMapCoords;
    shadowMapCoordsProj.xyz /= shadowMapCoordsProj.w;
    
    // FWIW, this should prolly be made some kind of setting.
    bias = 0.0001;
    //bias *= 1 + (gradient*gradient*256);
  }
  
  float getMapValue(int i, float2 position)
  {
    if (i == 0)
      return tex2D(lightPropsSM.shadowMap[0], position);
    else if (i == 1)
      return tex2D(lightPropsSM.shadowMap[1], position);
    else if (i == 2)
      return tex2D(lightPropsSM.shadowMap[2], position);
    else if (i == 3)
      return tex2D(lightPropsSM.shadowMap[3], position);	
    else if (i == 4)
      return tex2D(lightPropsSM.shadowMap[4], position);	
    else if (i == 5)
      return tex2D(lightPropsSM.shadowMap[5], position);	
    else if (i == 6)
      return tex2D(lightPropsSM.shadowMap[6], position);	
    
    return tex2D(lightPropsSM.shadowMap[7], position);
  }
  
  half GetVisibility()
  {
    float3 shadowMapCoordsBiased = (float3(0.5)*shadowMapCoordsProj.xyz) + float3(0.5);    
    half inLight;
	
    int numSplits = lightPropsSM.shadowMapNumSplits;
    float farZ = lightPropsSM.shadowMapFarZ;
	
    for (int i = 1 ; i <= numSplits ; i ++)
    {
      float previousSplit = (i - 1) * farZ / numSplits;
      float nextSplit = i * farZ / numSplits;
      
      if (-shadowMapCoords.z < nextSplit)
      {
        float previousMap = getMapValue(i - 1, shadowMapCoordsBiased.xy);
        float nextMap = getMapValue(i, shadowMapCoordsBiased.xy);
		
        inLight = lerp(previousMap, nextMap, nextSplit + shadowMapCoords.z - previousSplit);
        break;
      }
    }

    return inLight;
  }
};

#endif // __SHADOW_OSM_CG_INC__
 
]]>
</include>
