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

<?CgUseShared texCoord?>
<?CgUseShared texCoordV2F?>
  
<?Template Parallax_Code?>
#ifndef __CS_SHADER_PARALLAX_CGINC__
#define __CS_SHADER_PARALLAX_CGINC__

<?Include /shader/snippets/cg-common.cginc?>
  
struct Vert_Parallax
{
  varying float4 Position : POSITION;
  uniform float4x4 ModelViewIT : state.matrix.modelview.invtrans;
  /* @@@ FIXME Question: Undoubtedly, other components will use the 
   * normal/tangent/binormal as well in the future. How to share that? Ie all 
   * instances of use should use the *same* vertex attribute. */
  varying float3 Normal;
  varying float3 Tangent;
  varying float3 BiNormal;
};
Vert_Parallax parallaxVert;

struct Frag_Parallax
{
  uniform sampler2D heightTex;
  uniform float2 tcScale;
};
Frag_Parallax parallaxFrag;

struct Parallax
{
<?if vars."tex height".texture ?>
  float3 eyeVec;
<?endif?>

  void SetupVert ()
  {
  <?if vars."tex height".texture ?>
    float3x3 obj2tang;
    obj2tang[0] = parallaxVert.Tangent;
    obj2tang[1] = parallaxVert.BiNormal;
    obj2tang[2] = parallaxVert.Normal;
    float3 eyeVecObj = parallaxVert.ModelViewIT[3] - parallaxVert.Position;
    eyeVec =  mul (obj2tang, eyeVecObj);
  <?endif?>
  }
  float2 GetTCOffset ()
  {
    float2 offset = float2 (0, 0);
  <?if vars."tex height".texture ?>
    float2 tc = texCoord * parallaxFrag.tcScale;
    offset = ComputeParallaxOffset (parallaxFrag.heightTex, tc, 
      normalize (eyeVec), 0.04);
  <?endif?>
    return offset;
  }
};

#endif // __CS_SHADER_PARALLAX_CGINC__
<?Endtemplate?>

<?CgAddSnippet Parallax_Code?>

<?BeginGlue Parallax?>
  <?Template Pass_Parallax?>
    <?if vars."tex height".texture ?>
      <texture name="tex height" destination="parallaxFrag.heightTex" />
      <buffer source="normal" destination="parallaxVert.Normal" />
      <buffer source="tangent" destination="parallaxVert.Tangent" />
      <buffer source="binormal" destination="parallaxVert.BiNormal" />
    <?endif?>
  <?Endtemplate?>
  <?AddToList PassMappings Pass_Parallax?>
  
  <?Template VariableMap_Parallax ?>
    <?if vars."tex height".texture?>
      <variablemap variable="tex height scale" 
	destination="parallaxFrag.tcScale" />
    <?endif?>
  <?Endtemplate?>
  <?AddToList ProgramMappings VariableMap_Parallax?>
  
  <?Template ShaderVar_Parallax?>
    <shadervar name="tex height scale" type="vector2">1,1</shadervar>
  <?Endtemplate?>
  <?AddToList ShaderVars ShaderVar_Parallax?>
<?EndGlue?>
</include>