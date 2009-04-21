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

<?! Shared resources we use ?>
<?CgUseShared texCoord?>
<?CgUseShared texCoordV2F?>

<?Template SurfaceClassic_Code?>
/* This block of code will be inserted into the Cg program pretty much like
 * an include file, so guards are not a bad idea. */
#ifndef __CS_SHADER_SURFACE_CLASSIC_CG__
#define __CS_SHADER_SURFACE_CLASSIC_CG__

/*struct Vert_SurfaceClassic
{
};
Vert_SurfaceClassic surfaceClassicVert;*/

struct Frag_SurfaceClassic
{
  uniform sampler2D texture;
  uniform float4 flatColor;
};
Frag_SurfaceClassic surfaceClassicFrag;

struct SurfaceClassic
{
  void SetupVert ()
  {
  }

  float4 GetDiffuse ()
  {
    float4 diffuse;
    <?if vars."tex diffuse".texture?>
      diffuse = tex2D (surfaceClassicFrag.texture, texCoord);
    <?else?>
      diffuse = surfaceClassicFrag.flatColor;
    <?endif?>
    return diffuse;
  }
  float4 GetDiffuse (float2 offset)
  {
    <?if vars."tex diffuse".texture?>
      return tex2D (surfaceClassicFrag.texture, texCoord + offset);
    <?else?>
      return surfaceClassicFrag.flatColor;
    <?endif?>
  }
};

#endif // __CS_SHADER_SURFACE_CLASSIC_CG__
<?Endtemplate?>

<?! This will add the code above to the emitted Cg program(s) ?>
<?CgAddSnippet SurfaceClassic_Code?>

<?! Glue - texture, buffer and variable mappings ?>
<?BeginGlue SurfaceClassic?>
  <?Template Pass_Surface_Classic ?>
    <?if vars."tex diffuse".texture?>
      <texture name="tex diffuse" destination="surfaceClassicFrag.texture" />
    <?endif?>
  <?Endtemplate?>
  <?AddToList PassMappings Pass_Surface_Classic ?>
  
  <?Template VariableMap_Surface ?>
    <?if !vars."tex diffuse".texture?>
      <variablemap variable="mat flatcolor" 
	destination="surfaceClassicFrag.flatColor" />
    <?endif?>
  <?Endtemplate?>
  <?AddToList ProgramMappings VariableMap_Surface ?>
<?EndGlue?>
</include>
