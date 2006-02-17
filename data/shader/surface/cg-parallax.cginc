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
#ifndef __CS_SHADER_PARALLAX_CGINC__
#define __CS_SHADER_PARALLAX_CGINC__

struct AppToVert_Parallax
{
  void _dummy_struct_non_empty() {}
<?if vars."tex height".texture ?>
  varying float4 Position : POSITION;
  uniform float4x4 ModelViewIT : state.matrix.modelview.invtrans;
  /* @@@ FIXME Question: Undoubtedly, other components will use the 
   * normal/tangent/binormal as well in the future. How to share that? Ie all 
   * instances of use should use the *same* vertex attribute. */
  varying float3 Normal;
  varying float3 Tangent;
  varying float3 BiNormal;
<?endif?>
};

AppToVert_Parallax parallaxA2V;

struct VertToFrag_Parallax
{
<?if vars."tex height".texture ?>
  float3 eyeVec;
<?endif?>

  void Setup ()
  {
  <?if vars."tex height".texture ?>
    float3x3 obj2tang;
    obj2tang[0] = parallaxA2V.Tangent;
    obj2tang[1] = parallaxA2V.BiNormal;
    obj2tang[2] = parallaxA2V.Normal;
    float3 eyeVecObj = parallaxA2V.ModelViewIT[3] - parallaxA2V.Position;
    eyeVec =  mul (obj2tang, eyeVecObj);
  <?endif?>
  }
};

struct AppToFrag_Parallax
{
  void _dummy_struct_non_empty() {}
<?if vars."tex height".texture ?>
  uniform sampler2D heightTex;
<?endif?>
};

AppToFrag_Parallax parallaxA2F;

struct Frag_Parallax
{
  float2 GetTCOffset (VertToFrag_Parallax V2F, float2 texCoord)
  {
  <?if vars."tex height".texture ?>
    float4 height = 0.04 * tex2D(parallaxA2F.heightTex, texCoord) - 0.02;
    return (height * normalize (V2F.eyeVec)).xy;
  <?else?>
    return float2 (0, 0);
  <?endif?>
  }
};

#endif // __CS_SHADER_PARALLAX_CGINC__
</include>