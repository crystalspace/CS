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
  
<?Template SectorFog_Code?>
#ifndef __CS_SHADER_SECTORFOG_CG__
#define __CS_SHADER_SECTORFOG_CG__

struct Vert_FogSector
{
  uniform float4 fogPlaneS;
  uniform float4 fogPlaneT;
  uniform float4x4 ModelView : state.matrix.modelview;
  varying float4 Position : POSITION;
//<?if vars."fog density".float &gt; 0 ?>
//<?endif?>
};
Vert_FogSector fogSectorVert;

struct Frag_FogSector
{
  uniform sampler2D fogTex;
  uniform float4 fogColor;
};
Frag_FogSector fogSectorFrag;

struct FogSector
{
<?if vars."fog density".float &gt; 0 ?>
  float2 fogTC;
<?endif?>

  void SetupVert()
  {
  <?if vars."fog density".float &gt; 0 ?>
    float4 eyePos = mul (fogSectorVert.ModelView, fogSectorVert.Position);
    fogTC.x = dot (eyePos, fogSectorVert.fogPlaneS);
    fogTC.y = dot (eyePos, fogSectorVert.fogPlaneT);
  <?endif?>
  }

  bool HasFog()
  {
  <?if vars."fog density".float &gt; 0 ?>
    return true;
  <?else?>
    return false;
  <?endif?>
  }
  
  float GetIntensity()
  {
    float intens = 0;
  <?if vars."fog density".float &gt; 0 ?>
    intens = tex2D (fogSectorFrag.fogTex, fogTC).a;
  <?endif?>
    return intens;
  }
  
  float3 GetColor()
  {
    float3 fogcol = float3(0,0,0);
  <?if vars."fog density".float &gt; 0 ?>
    fogcol = fogSectorFrag.fogColor.rgb;
  <?endif?>
    return fogcol;
  }
};

#endif // __CS_SHADER_SECTORFOG_CG__
<?Endtemplate?>

<?CgAddSnippet SectorFog_Code?>

<?BeginGlue SectorFog?>
  <?Template Pass_SectorFog ?>
    <?if vars."fog density".float &gt; 0 ?>
      <!-- We need the fog lookup texture -->
      <texture name="standardtex fog" destination="fogSectorFrag.fogTex" />
    <?endif?>
  <?Endtemplate?>
  <?AddToList PassMappings Pass_SectorFog ?>

  <?Template VariableMap_SectorFog ?>
    <?if vars."fog density".float &gt; 0 ?>
      <variablemap variable="fog color" destination="fogSectorFrag.fogColor" />
      <!-- See the glshader_fixed plugin source for a detailed explanation 
	   of the formular below. -->
      <?if vars."fogplane".z &gt; 0 ?>
	<variablemap destination="fogSectorVert.fogPlaneS" type="expr">
	  <sexp>
	  (make-vector 
	    0 
	    0 
	    "fog density" 
	    (- (min (max (* (elt4 fogplane) "fog density") -1) 0) (* 0.1 "fog density"))
	  )</sexp>
	</variablemap>
	<variablemap destination="fogSectorVert.fogPlaneT" type="expr">
	  <sexp>
	  (make-vector
	    (/ (* (elt1 fogplane) "fog density") (elt3 fogplane))
	    (/ (* (elt2 fogplane) "fog density") (elt3 fogplane))
	    "fog density"
	    (+ (min (max (pow (+ 1 (* (elt4 fogplane) 0.2)) 5) 0) 1)
	       (* "fog density" (- (/ (elt4 fogplane) (elt3 fogplane)) 0.1)))
	  )
	  </sexp>
	</variablemap>
      <?else?>
	<variablemap destination="fogSectorVert.fogPlaneS" type="expr">
	  <sexp>(make-vector 0 0 "fog density" (* -0.1 "fog density"))</sexp>
	</variablemap>
	<variablemap destination="fogSectorVert.fogPlaneT" type="vector4">0,0,0,1</variablemap>
      <?endif?>
    <?endif?>
  <?Endtemplate?>
  <?AddToList ProgramMappings VariableMap_SectorFog ?>
<?EndGlue?>
    
</include>
