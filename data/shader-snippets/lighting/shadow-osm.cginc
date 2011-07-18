<!--
  Copyright (C) 2011 Alexandru - Teodor Voicu
      Imperial College London
      http://www3.imperial.ac.uk/

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
  float distance;
  int lightNum;
  float bias;
  
  void InitVP (int lightNum, float4 surfPositionWorld,
               float3 normWorld,
               out float4 vp_shadowMapCoords,
               out float vp_distance)
  {
    float4x4 lightTransformInv = lightProps.transformInv[lightNum];
    // Transform world position into light space
    float4 view_pos = mul(lightTransformInv, surfPositionWorld);

    float4 shadowMapCoords;
    shadowMapTF = lightPropsOM.opacityMapTF[lightNum];
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
    vp_distance = view_pos.z;
  }
  
  void Init (int lightN, float4 vp_shadowMapCoords, float vp_distance)
  {
    shadowMapCoords = vp_shadowMapCoords;
    distance = vp_distance;
    lightNum = lightN;
    
    // Project SM coordinates
    shadowMapCoordsProj = shadowMapCoords;
    shadowMapCoordsProj.xyz /= shadowMapCoordsProj.w;    
  }
  
  float4 blurTex2D(sampler2D tex, float2 position)
  {
    float4 color = 0;
    color += tex2D(tex, position + float2(0, bias));
    color += tex2D(tex, position + float2(0, -bias));
    color += tex2D(tex, position + float2(bias, bias));
    color += tex2D(tex, position + float2(bias, -bias));
    color += tex2D(tex, position + float2(-bias, bias));
    color += tex2D(tex, position + float2(-bias, -bias));
    color += tex2D(tex, position + float2(0, 0));
    color += tex2D(tex, position + float2(bias, 0));
    color += tex2D(tex, position + float2(-bias, 0));    
    color /= 9;
    
    return tex2D(tex, position);
  }
  
  float getMapValue(int i, float2 position)
  {
    if (position.x < 0 || position.y < 0 || position.x > 1 || position.y > 1)
      return 0;
]]>      
  
    <?Generate I 0 7?>
      if (i == 4 * $I$)
        return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + $I$], position).r;
      if (i == 4 * $I$ + 1)
        return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + $I$], position).g;
      if (i == 4 * $I$ + 2)
        return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + $I$], position).b;
      if (i == 4 * $I$ + 3)
        return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + $I$], position).a;	    
    <?Endgenerate?>
        
<![CDATA[  
      
    return 0;
  }
  
  half GetVisibility()
  {
    bias = 1.0 / 512.0;
    half inLight;
    int numSplits = lightPropsOM.opacityMapNumSplits[lightNum];
    float previousSplit, nextSplit;
  
    float3 shadowMapCoordsBiased = (float3(0.5)*shadowMapCoordsProj.xyz) + float3(0.5);
    float2 position = shadowMapCoordsBiased.xy;
   
    int i;

    float compareDepth = (1 - shadowMapCoordsBiased.z) + 0.03;
    float depth = tex2D(lightPropsOM.shadowMap[lightNum], position).x;

    if (position.x < 0 || position.y < 0 || position.x > 1 || position.y > 1)
      depth = 0;    

    i = min(((compareDepth - depth) * 4) * 75, numSplits - 1);
    previousSplit = (float)i / 75;
    nextSplit = (float)(i + 1) / 75;    
    if (abs(compareDepth - depth) < 0.04)
      i = -1;
/*
    for (i = 0 ; i < numSplits ; i ++)
    {
      previousSplit = lightPropsOM.splitDists[8 * lightNum + i];
      nextSplit = lightPropsOM.splitDists[8 * lightNum + i + 1];
      
      if (distance < nextSplit || i == numSplits - 1)
        break;

      previousSplit = nextSplit;
    }
*/
    float previousMap = 0, nextMap = 0;
/*    
    int prevIndex = min((i / 4), numSplits - 1);
    
    for (int j = 0 ; j < prevIndex ; j ++)
      previousMap += getMapValue(4 * (j + 1) - 1, position);
    
    int nextIndex = min((i + 1) / 4, numSplits - 1);
    
    nextMap = previousMap;
    for (int j = prevIndex ; j < nextIndex ; j ++)
      nextMap += getMapValue(4 * (j + 1) - 1, position);
*/
    previousMap += getMapValue(i, position);
    nextMap += getMapValue(i + 1, position);   
    
    inLight = lerp(previousMap, nextMap, (float) (((compareDepth - depth) * 4) - previousSplit) 
      / (nextSplit - previousSplit) );
    //inLight = previousMap;
      
    inLight = inLight * (i != numSplits - 1) + previousMap * (i == numSplits - 1);
    inLight = exp(-1.0 * inLight);
    
/*
    inLight = 1;
    if (compareDepth > depth && depth > 0) 
      inLight = 0;
    //inLight = 1 - tex2D(lightPropsOM.shadowMap[lightNum], float2(position)).x;
*/
    //inLight = ((float)i / (numSplits - 1));
    
    inLight = inLight * (depth != 0) + (depth == 0 /*|| compareDepth < depth*/);
    return inLight;
  }
};

#endif // __SHADOW_OSM_CG_INC__
 
]]>
</include>
