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
  float bias;
  
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

    if (i == 0)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 0], position).r;
    if (i == 1)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 0], position).g;
    if (i == 2)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 0], position).b;
    if (i == 3)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 0], position).a;	
    if (i == 4)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 1], position).r;	
    if (i == 5)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 1], position).g;	
    if (i == 6)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 1], position).b;	

    if (i == 7)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 1], position).a;
    if (i == 8)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 2], position).r;
    if (i == 9)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 2], position).g;
    if (i == 10)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 2], position).b;
    if (i == 11)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 2], position).a;	
    if (i == 12)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 3], position).r;	
    if (i == 13)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 3], position).g;	
    if (i == 14)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 3], position).b;	
    if (i == 15)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 3], position).a;	

    if (i == 16)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 4], position).r;
    if (i == 17)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 4], position).g;
    if (i == 18)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 4], position).b;
    if (i == 19)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 4], position).a;
    if (i == 20)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 5], position).r;	
    if (i == 21)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 5], position).g;	
    if (i == 22)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 5], position).b;	
    if (i == 23)
      return blurTex2D(lightPropsOM.opacityMap[8 * lightNum + 5], position).a;	
      
    return 0;
  }
  
  float2 getPosition(int index)
  {
    float4x4 flipY;
    flipY[0] = float4 (1, 0, 0, 0);
    flipY[1] = float4 (0, -1, 0, 0);
    flipY[2] = float4 (0, 0, 1, 0);
    flipY[3] = float4 (0, 0, 0, 1);      
    
    float4x4 shadowMapTF = mul (flipY, lightPropsOM.opacityMapTF[index]);
    float4 shadowMapCoords = mul (shadowMapTF, viewPos);      
    float4 shadowMapCoordsProj = shadowMapCoords;
    shadowMapCoordsProj.xyz /= shadowMapCoordsProj.w;      
    float3 shadowMapCoordsBiased = 
      (float3(0.5)*shadowMapCoordsProj.xyz) + float3(0.5);

    return shadowMapCoordsBiased.xy;
  }
  
  half GetVisibility()
  {
    bias = 1.0 / 512.0;
    half inLight;
    int numSplits = lightPropsOM.opacityMapNumSplits[lightNum];
    float previousSplit, nextSplit;
  
    int i;
    for (i = 0 ; i <= numSplits ; i ++)
    {
      previousSplit = lightPropsOM.splitDists[8 * lightNum + i];
      nextSplit = lightPropsOM.splitDists[8 * lightNum + i + 1];
      
      if ((viewPos.z + 0.0) < nextSplit || i == numSplits)
        break;

      previousSplit = nextSplit;
    }

    float previousMap = 0, nextMap = 0;
    
    int prevIndex = min((i / 4), numSplits);
    float2 prevPos = getPosition(prevIndex);
    
    for (int j = 0 ; j < prevIndex ; j ++)
      previousMap += getMapValue(4 * (j + 1) - 1, prevPos);
    
    int nextIndex = min((i + 1) / 4, numSplits);
    float2 nextPos = getPosition(nextIndex);
    
    nextMap = previousMap;
    for (int j = prevIndex ; j < nextIndex ; j ++)
      nextMap += getMapValue(4 * (j + 1) - 1, nextPos);
      
    previousMap += getMapValue(i, prevPos);
    nextMap += getMapValue(i + 1, nextPos);   
    
    inLight = lerp(previousMap, nextMap, (float) (viewPos.z - previousSplit) 
      / (nextSplit - previousSplit) );
      
    inLight = inLight * (i != numSplits) + previousMap * (i == numSplits);
    inLight = exp(-1.0 * inLight);
      
    //inLight = (float)i/numSplits;   
      
    return inLight;
  }
};

#endif // __SHADOW_OSM_CG_INC__
 
]]>
</include>
