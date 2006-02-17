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
#ifndef __CS_SHADER_SECTORFOG_CG__
#define __CS_SHADER_SECTORFOG_CG__

struct AppToVert_Fog_Sector
{
  void _dummy_struct_non_empty() {}
<?if vars."fog density".float &gt; 0 ?>
  uniform float4 fogPlaneS;
  uniform float4 fogPlaneT;
  uniform float4x4 ModelView : state.matrix.modelview;
  varying float4 Position : POSITION;
<?endif?>
};

AppToVert_Fog_Sector fogSectorA2V;

struct VertToFrag_Fog_Sector
{
<?if vars."fog density".float &gt; 0 ?>
  float2 fogTC;
<?endif?>

  void Setup ()
  {
  <?if vars."fog density".float &gt; 0 ?>
    float4 eyePos = mul (fogSectorA2V.ModelView, fogSectorA2V.Position);
    fogTC.x = dot (eyePos, fogSectorA2V.fogPlaneS);
    fogTC.y = dot (eyePos, fogSectorA2V.fogPlaneT);
  <?endif?>
  }
};

struct AppToFrag_Fog_Sector
{
  void _dummy_struct_non_empty() {}
<?if vars."fog density".float &gt; 0 ?>
  uniform sampler2D fogTex;
  uniform float4 fogColor;
<?endif?>
};

AppToFrag_Fog_Sector fogSectorA2F;

struct Frag_Fog_Sector
{
<?if vars."fog density".float &gt; 0 ?>
  float2 fogTC;
<?endif?>

  void Setup (VertToFrag_Fog_Sector V2F)
  {
  <?if vars."fog density".float &gt; 0 ?>
    fogTC = V2F.fogTC;
  <?endif?>
  }
  
  float4 ApplyFog (float4 surfaceColor)
  {
  <?if vars."fog density".float &gt; 0 ?>
    float4 fog = tex2D (fogSectorA2F.fogTex, fogTC);
    float4 result;
    result.rgb = lerp (surfaceColor.rgb, 
      fogSectorA2F.fogColor.rgb, fog.a);
    result.a = surfaceColor.a;
    return result;
  <?else?>
    return surfaceColor;
  <?endif?>
  }
};

<?SetSnippet Fog Fog_Sector?>

#endif // __CS_SHADER_SECTORFOG_CG__
</include>
