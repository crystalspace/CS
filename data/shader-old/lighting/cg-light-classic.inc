<!--
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Jorrit Tyberghein

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

<?CgUseSharedCond primaryColor "!vars.\"tex lightmap\".texture && !vars.\"static color\".buffer" ?>
<?CgUseSharedCond primaryColorV2F "!vars.\"tex lightmap\".texture && !vars.\"static color\".buffer" ?>

<?Template LightingClassic_Code?>
#ifndef __CS_SHADER_LIGHT_CLASSIC_CG__
#define __CS_SHADER_LIGHT_CLASSIC_CG__
  
struct Vert_LightingClassic
{
  varying float2 texCoordLM;
  varying float3 staticPVLColor;
};
Vert_LightingClassic lightingClassicVert;

struct Frag_LightingClassic
{
  uniform sampler2D lightmap;
  uniform float3 ambient;
};
Frag_LightingClassic lightingClassicFrag;

struct LightingClassic
{
<?if vars."tex lightmap".texture ?>
  float2 texCoordLM;
<?elsif vars."static color".buffer ?>
  float3 staticPVLColor;
<?endif?>

  void SetupVert ()
  {
  <?if vars."tex lightmap".texture ?>
    texCoordLM = lightingClassicVert.texCoordLM;
  <?elsif vars."static color".buffer ?>
    staticPVLColor = lightingClassicVert.staticPVLColor;
  <?endif?>
  }
  
  float3 IllumAmbient ()
  {
    return lightingClassicFrag.ambient;
  }
  
  float3 IllumDiffuse ()
  {
    float3 light_diffuse;
  <?if vars."tex lightmap".texture ?>
    light_diffuse = 2 * tex2D (lightingClassicFrag.lightmap, 
      texCoordLM).rgb;
    /*float grid_x = (frac (texCoordLM.x * 1024) &lt; 0.02) ? 0 : 1;
    float grid_y = (frac (texCoordLM.y * 1024) &lt; 0.02) ? 0 : 1;
    light_diffuse = light_diffuse * grid_x * grid_y;*/
  <?elsif vars."static color".buffer ?>
    light_diffuse = 2 * staticPVLColor;
  <?else?>
    light_diffuse = primaryColor.rgb;
  <?endif?>
    return light_diffuse;
  }
};

#endif // __CS_SHADER_LIGHT_CLASSIC_CG__
<?Endtemplate?>

<?CgAddSnippet LightingClassic_Code?>

<?BeginGlue LightingClassic?>
  <?Template Pass_LightingClassic ?>
    <?if vars."tex lightmap".texture?>
      <texture name="tex lightmap" destination="lightingClassicFrag.lightmap" />
      <buffer source="texture coordinate lightmap" 
	destination="lightingClassicVert.texCoordLM" />
  <?elsif vars."static color".buffer ?>
      <buffer source="static color" 
	destination="lightingClassicVert.staticPVLColor" />
    <?endif?>
  <?Endtemplate?>
  <?AddToList PassMappings Pass_LightingClassic ?>

  <?Template VariableMap_LightingClassic ?>
    <variablemap variable="dynamic ambient" 
      destination="lightingClassicFrag.ambient" />
  <?Endtemplate?>
  <?AddToList ProgramMappings VariableMap_LightingClassic ?>
<?EndGlue?>
</include>
