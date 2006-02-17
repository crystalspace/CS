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
#ifndef __CS_SHADER_LIGHT_CLASSIC_CG__
#define __CS_SHADER_LIGHT_CLASSIC_CG__

<?Include /shader/snippets/cg-i-surface.cginc ?>

struct AppToVert_Lighting_Classic
{
  varying float4 color : COLOR;
<?if vars."tex lightmap".texture ?>
  varying float2 texCoordLM;
<?endif?>
};

AppToVert_Lighting_Classic lightingClassicA2V;

struct VertToFrag_Lighting_Classic
{
  float4 color : COLOR;
<?if vars."tex lightmap".texture ?>
  float2 texCoordLM;
<?endif?>

  void Setup ()
  {
    color = lightingClassicA2V.color;
  <?if vars."tex lightmap".texture ?>
    texCoordLM = lightingClassicA2V.texCoordLM;
  <?endif?>
  }
};

struct AppToFrag_Lighting_Classic
{
  void _dummy_struct_non_empty() {}
<?if vars."tex lightmap".texture ?>
  uniform sampler2D lightmap;
<?endif?>
};

AppToFrag_Lighting_Classic lightingClassicA2F;

struct Frag_Lighting_Classic
{
<?if vars."tex lightmap".texture ?>
  float2 texCoordLM;
<?endif?>
  float4 color;

  void Setup (VertToFrag_Lighting_Classic V2F)
  {
    color = V2F.color;
  <?if vars."tex lightmap".texture ?>
    texCoordLM = V2F.texCoordLM;
  <?endif?>
  }
  
  float4 Illuminate (iSurface surface)
  {
    float4 result;
    
    float4 light_diffuse;
  <?if vars."tex lightmap".texture ?>
    light_diffuse = 2 * tex2D (lightingClassicA2F.lightmap, 
      texCoordLM /*+ surface.GetTexCoordOffset ()*/);
    light_diffuse.a = color.a;
  <?else?>
    light_diffuse = color;
  <?endif?>
    result = light_diffuse * surface.GetDiffuse();
  
    result.rgb = result.rgb + surface.GetEmissive ();
  
    return result;
  }
};

<?SetSnippet Lighting Lighting_Classic?>

#endif // __CS_SHADER_LIGHT_CLASSIC_CG__
</include>
