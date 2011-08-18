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
/*
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
    
    return color;
*/

    float4 sum = 0;
/*
    float2 offset;
    offset = (float)(frac(position.xy * 0.5) > 0.25);  // mod
    offset.y += offset.x;  // y ^= x in floating point

    if (offset.y > 1.1)
      offset.y = 0;
      
    sum = (tex2D(tex, position + bias * (offset + float2(-1.5, 0.5))) +
           tex2D(tex, position + bias * (offset + float2(0.5, 0.5))) +
           tex2D(tex, position + bias * (offset + float2(-1.5, -1.5))) +
           tex2D(tex, position + bias * (offset + float2(0.5, -1.5)))) * 0.25;
*/
    float x, y;

    for (y = -1.5; y <= 1.5; y += 1)
      for (x = -1.5; x <= 1.5; x += 1)
        sum += tex2D(tex, position + float2(bias * x, bias * y));
      
    sum /= 16.0; 

    return sum;
  }
  
  // PCF - Percentage Close Filtering
  float getVisibility(sampler2D tex, float2 position, float compareDepth)
  {
/*  
    float vis = 0;
    //bias = 0.01;
    vis += compareDepth > tex2D(tex, position + float2(0, bias));
    vis += compareDepth > tex2D(tex, position + float2(0, -bias));
    vis += compareDepth > tex2D(tex, position + float2(bias, bias));
    vis += compareDepth > tex2D(tex, position + float2(bias, -bias));
    vis += compareDepth > tex2D(tex, position + float2(-bias, bias));
    vis += compareDepth > tex2D(tex, position + float2(-bias, -bias));
    vis += compareDepth > tex2D(tex, position + float2(0, 0));
    vis += compareDepth > tex2D(tex, position + float2(bias, 0));
    vis += compareDepth > tex2D(tex, position + float2(-bias, 0));    
    vis /= 9.0;
    
    return vis;
*/    
    float4 sum = 0;
    float x, y;

    for (y = -1.5; y <= 1.5; y += 1)
      for (x = -1.5; x <= 1.5; x += 1)
        sum += compareDepth > tex2D(tex, position + float2(bias * x, bias * y));
      
    sum /= 16.0; 

    return sum;    
  }
  
  float getMapValue(int i, float2 position)
  {
    // already checked when depthStart = 0
/*    
    if (position.x < 0 || position.y < 0 || position.x > 1 || position.y > 1)
      return 0;
*/
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
    bias = 1.0 / lightPropsOM.size[lightNum];
    half inLight;
    int numSplits = lightPropsOM.opacityMapNumSplits[lightNum];
    float previousSplit, nextSplit;
  
    float3 shadowMapCoordsBiased = (float3(0.5)*shadowMapCoordsProj.xyz) + float3(0.5);
    float2 position = shadowMapCoordsBiased.xy;
   
    int i;
    float eps = 0.0001;

    float compareDepth = (1 - shadowMapCoordsBiased.z) - eps;
    float depthStart = tex2D(lightPropsOM.shadowMapStart[lightNum], position).x;
    float depthEnd = tex2D(lightPropsOM.shadowMapEnd[lightNum], position).x;    
/*    
    if (position.x < 0 || position.y < 0 || position.x > 1 || position.y > 1)
      depthStart = 0;    
*/
    depthStart *= !(position.x < 0 || position.y < 0 || position.x > 1 || position.y > 1);
/*
    i = min( ( (compareDepth - depthStart) / (0.95 - depthStart) ) * (numSplits - 1), numSplits - 1);

    i = min( tex2D( lightPropsOM.splitFunc[lightNum], 
      float2( min( (compareDepth - depthStart) / (0.95 - depthStart) , 0.99 ) , 0 ) ) * (numSplits - 1), 
      numSplits - 1);
*/      
    i = min( tex2D( lightPropsOM.splitFunc[lightNum], 
      float2( min( (compareDepth - depthStart) / 
      (1 - depthEnd - depthStart) , 0.9999 ) , 0 ) ).x * (numSplits - 1), 
      numSplits - 1);      
/*      
    i = min ( ( (compareDepth - depthStart) / (1 - depthEnd - depthStart) ) * 
      (numSplits - 1), numSplits - 1);      
/*    
    if (compareDepth < depthStart)
      i = -1;
*/
//    i = -(compareDepth < depthStart) + i * (compareDepth < depthStart);
    
    float previousMap, nextMap;
    previousMap = getMapValue(i, position) * 
      getVisibility( lightPropsOM.shadowMapStart[lightNum] , position, compareDepth );
//    nextMap = getMapValue( min ( i + 1, numSplits - 1 ) , position);   

    i = i * (i != -1);
    previousSplit = (float)i / (numSplits - 1);
/*
    inLight = lerp(previousMap, nextMap, ( ( (compareDepth - depthStart) / (1 - depthEnd - depthStart) )  
      - previousSplit) * (numSplits - 1) );
*/  
    inLight = previousMap;//lerp(previousMap, nextMap, 0.5);
    //inLight = ( ( (compareDepth - depthStart) / (0.95 - depthStart) ) - previousSplit) * (numSplits - 1);
    
    inLight = exp(-1.0 * inLight);
    //inLight = 1 - inLight;
    //inLight = 1 - ((float)i / (numSplits - 1));
/*
    inLight = 1;
//    if (compareDepth > depthStart && depthStart != 0)
    if (compareDepth > depthStart )//&& compareDepth < (1 - depthEnd + 0.05))
      inLight = 0;//1 - (compareDepth - depthStart) / (1 - depthEnd - depthStart + 0.05);

    inLight =  1 - getVisibility( lightPropsOM.shadowMapStart[lightNum] , position, compareDepth ); 
*/
    //inLight = tex2D(lightPropsOM.shadowMapStart[lightNum], float3(position, compareDepth)).x;
    
    //inLight = inLight * (depthStart != 0) + (depthStart == 0);
    return inLight;
  }
};

#endif // __SHADOW_OSM_CG_INC__
 
]]>
</include>
