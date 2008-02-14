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
    <variablemap variable="light position world" 
		  destination="$PROG$In.lightPosWorld" />
    <variablemap variable="light diffuse" 
		  destination="$PROG$In.lightColorDiffuse" />
    <variablemap variable="light attenuation" 
		  destination="$PROG$In.lightAttenuationVec" />
    <variablemap variable="light direction world" 
		  destination="$PROG$In.lightDir" />
    <variablemap variable="light inner falloff" 
		  destination="$PROG$In.lightFalloffInner" />
  <?Endtemplate?>
    
  <?Template Lighting_Compute PROG?>
    diffuseColor = float3(0);
    <?Generate N 0 4 ?>
      <?if (vars."light count".int &gt; $$N$$)?>
      {
	float3 surfToLight = ($PROG$In.lightPosWorld[$$N$$] - position).xyz;
	float lightDistSq = dot (surfToLight, surfToLight);
	float lightDist = sqrt (lightDistSq);
	surfToLight /= lightDistSq;
	float diffuse = 0;
	float attn = 1;
	<?if vars."light attenuation mode[$N$]".int == consts.CS_ATTN_LINEAR ?>
	attn = Attenuation_Linear (lightDist, 1 / $PROG$In.lightAttenuationVec[$$N$$].x);
	<?else?>
	// Everything else can be mapped to CLQ attn
	attn = Attenuation_CLQ (lightDist, $PROG$In.lightAttenuationVec[$$N$$]);
	<?endif?>
	<?if vars."light type[$N$]".int == consts.CS_LIGHT_DIRECTIONAL ?>
	float3 lightDir = normalize ($PROG$In.lightDir[$$N$$]);
	diffuse = Light_Directional (normal, lightDir);
	<?elsif vars."light type[$N$]".int == consts.CS_LIGHT_SPOTLIGHT ?>
	float3 lightDir = normalize ($PROG$In.lightDir[$$N$$]);
	diffuse = Light_Spot (normal, normalize (surfToLight),
	  lightDir,
	  $PROG$In.lightFalloffInner[$$N$$],
	  $PROG$In.lightFalloffOuter[$$N$$]);
	<?else?>
	<?! Assume point light ?>
	diffuse = Light_Point (normal, normalize (surfToLight));
	<?endif?>
	diffuseColor += $PROG$In.lightColorDiffuse[$$N$$] * diffuse * attn;
      }
      <?endif?>
    <?Endgenerate?>
    specularColor = float3(0);
  <?Endtemplate?>
    
  <?SEndIf?>
</include>