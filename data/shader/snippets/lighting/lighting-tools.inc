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
  <?SIfNDef __SHADERCOND_TOOLS_INC__ ?>
  <?Define __SHADERCOND_TOOLS_INC__ ?>
  
  <?Template Lighting_Variablemaps PROG?>
    <!-- Cg shader plugin magic makes arrays work -->
    <variablemap variable="light count" 
		  destination="$PROG$In.lightCount" />
    <variablemap variable="light position world" 
		  destination="$PROG$In.lightPosWorld" />
    <variablemap variable="light diffuse" 
		  destination="$PROG$In.lightColorDiffuse" />
    <variablemap variable="light specular" 
		 destination="$PROG$In.lightColorSpecular" />
    <variablemap variable="light attenuation" 
		  destination="$PROG$In.lightAttenuationVec" />
    <variablemap variable="light direction world" 
		  destination="$PROG$In.lightDir" />
    <variablemap variable="light inner falloff" 
		  destination="$PROG$In.lightFalloffInner" />
    <variablemap variable="light transform world" 
		 destination="$PROG$In.lightTransform" />
    <variablemap variable="light shadow map projection" 
		 destination="$PROG$In.lightShadowMapProject" />
    <variablemap variable="light transform world inverse" 
		 destination="$PROG$In.lightTransformInv" />
  <?Endtemplate?>
  
  <?Template Lighting_Shadow_None?>
  <?Endtemplate?>
  
  <?Template Lighting_Shadow_Shadowmap PROG?>
    <?if vars."light shadow map" ?>
    // Transform fragment position into light space
    float4 view_pos = mul($PROG$In.lightTransformInv[i], position);
    // Transform fragment position in light space into "shadow map space"
    float4 shadowMapCoords;
    shadowMapCoords = mul ($PROG$In.lightShadowMapProject[i], view_pos);
    // Project SM coordinates
    shadowMapCoords.xyz /= shadowMapCoords.w;
    shadowMapCoords.xyz = (0.5*shadowMapCoords.xyz) + float3(0.5);
    float4 shadowVal = tex2D ($PROG$In.lightShadowMap[i], shadowMapCoords.xy);
    
    // Depth to compare against
    float compareDepth = shadowMapCoords.z + (1.0/32768.0);
    // Depth from the shadow map
    float shadowMapDepth = shadowVal.r;
    // Shadow value
    float inLight = compareDepth > shadowMapDepth;
    attn *= inLight;
    <?endif?>
  <?Endtemplate?>
    
  <?Template Lighting_Compute PROG LightNum Shadow?>
    diffuseColor = float3(0);
    specularColor = float3(0);
    <![CDATA[
    for (int l = 0; l < $LightNum$; l++)
    {
      if (l >= $PROG$In.lightCount) break;
    
      int i = l + lightOffset;
      float4x4 lightTransformInv = $PROG$In.lightTransformInv[i];
      float3 lightDir = lightTransformInv[2].xyz;
      
      float3 surfToLight = ($PROG$In.lightPosWorld[i] - position).xyz;
      float lightDist = length (surfToLight);
      surfToLight = normalize (surfToLight);
    ]]>
    
      float spot = 1;
      float3 L;
      <?if vars."light type".int == consts.CS_LIGHT_DIRECTIONAL ?>
      L = -lightDir;
      <?elsif vars."light type".int == consts.CS_LIGHT_SPOTLIGHT ?>
      L = surfToLight;
      spot = Light_Spot (normal, L,
        lightDir,
        $PROG$In.lightFalloffInner[i],
        $PROG$In.lightFalloffOuter[i]);
      <?else?>
      <?! Assume point light ?>
      L = surfToLight;
      <?endif?>
      float3 H = normalize (surfToLight - normalize (eyeToSurf));
      
      float4 lightCoeff = lit (dot (normal, L), dot (normal, H),
        32);
      
      float4 lightAttenuationVec = $PROG$In.lightAttenuationVec[i];
      float attn;
      float attnRadius = lightAttenuationVec.w;
      if (attnRadius > 0)
        attn = Attenuation_Linear (lightDist, 1 / attnRadius);
      else
        attn = Attenuation_CLQ (lightDist, lightAttenuationVec.xyz);
	
      <?Lighting_Shadow_$Shadow$ $PROG$?>

      float3 d = $PROG$In.lightColorDiffuse[i] * lightCoeff.y * spot * attn;
      float3 s = $PROG$In.lightColorSpecular[i] * lightCoeff.z * spot * attn;
      diffuseColor += d;
      specularColor += s;
    }
  <?Endtemplate?>
    
  <?SEndIf?>
</include>