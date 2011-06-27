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
  float4 viewPos;
  int lightNum;
  
  void InitVP (int lightNum, float4 surfPositionWorld,
               float3 normWorld,
               out float4 vp_viewPos,
               out float vp_distance)
  {
    float4x4 lightTransformInv = lightProps.transformInv[lightNum];
    // Transform world position into light space
    float4 view_pos = mul(lightTransformInv, surfPositionWorld);

    vp_viewPos = view_pos;
  }
  
  void Init (int lightN, float4 vp_viewPos, float vp_distance)
  {
    viewPos = vp_viewPos;
    lightNum = lightN;
  }
  
  float getMapValue(int i, float2 position)
  {
    if (position.x < 0 || position.y < 0 || position.x > 1 || position.y > 1)
      return 0;
  
    if (i == 0)
      return tex2D(lightPropsOM.opacityMap[8 * lightNum + 0], position).a;
    if (i == 1)
      return tex2D(lightPropsOM.opacityMap[8 * lightNum + 1], position).a;
    if (i == 2)
      return tex2D(lightPropsOM.opacityMap[8 * lightNum + 2], position).a;
    if (i == 3)
      return tex2D(lightPropsOM.opacityMap[8 * lightNum + 3], position).a;	
    if (i == 4)
      return tex2D(lightPropsOM.opacityMap[8 * lightNum + 4], position).a;	
    if (i == 5)
      return tex2D(lightPropsOM.opacityMap[8 * lightNum + 5], position).a;	
    if (i == 6)
      return tex2D(lightPropsOM.opacityMap[8 * lightNum + 6], position).a;	
    if (i == 7)
      return tex2D(lightPropsOM.opacityMap[8 * lightNum + 7], position).a;
      
    return 0;
  }
  
  half GetVisibility()
  {
    half inLight;
	
    int numSplits = lightPropsOM.opacityMapNumSplits;
    float previousSplit, nextSplit;
  
    float4x4 flipY;
    flipY[0] = float4 (1, 0, 0, 0);
    flipY[1] = float4 (0, -1, 0, 0);
    flipY[2] = float4 (0, 0, 1, 0);
    flipY[3] = float4 (0, 0, 0, 1);      
  
    int i;
    for (i = 0 ; i <= numSplits ; i ++)
    {
      previousSplit = lightPropsOM.splitDists[i];
      nextSplit = lightPropsOM.splitDists[i + 1];
      
      if (viewPos.z < nextSplit || i == numSplits)
        break;

      previousSplit = nextSplit;
    }
    float4x4 shadowMapTFPrev = mul (flipY, lightPropsOM.opacityMapTF[i]);
    float4 shadowMapCoordsPrev = mul (shadowMapTFPrev, viewPos);      
    float4 shadowMapCoordsProjPrev = shadowMapCoordsPrev;
    shadowMapCoordsProjPrev.xyz /= shadowMapCoordsProjPrev.w;      
    float3 shadowMapCoordsBiasedPrev = 
      (float3(0.5)*shadowMapCoordsProjPrev.xyz) + float3(0.5);          
    
    float4x4 shadowMapTFNext = mul (flipY, lightPropsOM.opacityMapTF[i + 1]);
    float4 shadowMapCoordsNext = mul (shadowMapTFNext, viewPos);      
    float4 shadowMapCoordsProjNext = shadowMapCoordsNext;
    shadowMapCoordsProjNext.xyz /= shadowMapCoordsProjNext.w;      
    float3 shadowMapCoordsBiasedNext = 
      (float3(0.5)*shadowMapCoordsProjNext.xyz) + float3(0.5);
      
    float previousMap = getMapValue(i, shadowMapCoordsBiasedPrev.xy);
    float nextMap = getMapValue(i + 1, shadowMapCoordsBiasedNext.xy);   
    
    inLight = lerp(1 - previousMap, 1 - nextMap, (float) (viewPos.z - previousSplit) 
      / (nextSplit - previousSplit) );
      
    inLight = inLight * (i != numSplits) + (1 - previousMap) * (i == numSplits) ;
      
    //inLight = (float)i/numSplits;   
              
    return inLight;
  }
};

#endif // __SHADOW_OSM_CG_INC__
 
]]>
</include>
