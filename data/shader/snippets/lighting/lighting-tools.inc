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
  <?Endtemplate?>
    
  <?Template Lighting_Compute PROG LightNum?>
    diffuseColor = float3(0);
    specularColor = float3(0);
    <![CDATA[
    for (int l = 0; l < $LightNum$; l++)
    {
      if (l >= $PROG$In.lightCount) break;
    
      int i = l + lightOffset;
      float3 surfToLight = ($PROG$In.lightPosWorld[i] - position).xyz;
      float lightDist = length (surfToLight);
      surfToLight = normalize (surfToLight);
    ]]>
    
      float spot = 1;
      float3 L;
      <?if vars."light type".int == consts.CS_LIGHT_DIRECTIONAL ?>
      L = -normalize ($PROG$In.lightDir[i]);
      <?elsif vars."light type".int == consts.CS_LIGHT_SPOTLIGHT ?>
      L = surfToLight;
      spot = Light_Spot (normal, L,
        normalize ($PROG$In.lightDir[i]),
        $PROG$In.lightFalloffInner[i],
        $PROG$In.lightFalloffOuter[i]);
      <?else?>
      <?! Assume point light ?>
      L = surfToLight;
      <?endif?>
      float3 H = 0.5 * (surfToLight - normalize (eyeToSurf));
      
      float4 lightCoeff = lit (dot (normal, L), dot (normal, H),
        32);
      
      float4 lightAttenuationVec = $PROG$In.lightAttenuationVec[i];
      float attn;
      float attnRadius = lightAttenuationVec.w;
      if (attnRadius > 0)
        attn = Attenuation_Linear (lightDist, 1 / attnRadius);
      else
        attn = Attenuation_CLQ (lightDist, lightAttenuationVec.xyz);
      
      diffuseColor += $PROG$In.lightColorDiffuse[i] * lightCoeff.y * spot * attn;
      specularColor += $PROG$In.lightColorSpecular[i] * lightCoeff.z * spot * attn;
    }
  <?Endtemplate?>
    
  <?SEndIf?>
</include>