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

<?Template Instancing_Code?>
#ifndef __CS_SHADER_INSTANCING_CGINC__
#define __CS_SHADER_INSTANCING_CGINC__

#ifdef PROGRAM_TYPE_VERTEX
struct Vert_Instancing
{
  /* a float4x4 or float4[4] with a TEXCOORD semantic doesn't work right -
   * all rows/array elements get the same semantic ... */
  varying float4 object2world0 : TEXCOORD0;
  varying float4 object2world1 : TEXCOORD1;
  varying float4 object2world2 : TEXCOORD2;
  varying float4 object2world3 : TEXCOORD3;
};
Vert_Instancing instancingVert;

float4 GetInstancedPosition (float4 position)
{
<?if vars."instancing transforms" ?>
  float4x4 o2w_matrix = float4x4 (instancingVert.object2world0,
				  instancingVert.object2world1,
				  instancingVert.object2world2,
				  instancingVert.object2world3);
  return mul (o2w_matrix, position);
<?else?>
  return position;
<?endif?>
}
#endif // PROGRAM_TYPE_VERTEX

#endif // __CS_SHADER_INSTANCING_CGINC__
<?Endtemplate?>

<?CgAddSnippet Instancing_Code?>

<?BeginGlue Instancing?>
  <?Template Pass_Instancing?>
    <?if vars."instancing transforms" ?>
      <instanceparam source="instancing transforms" 
	destination="instancingVert.object2world0" />
    <?endif?>
  <?Endtemplate?>
  <?AddToList PassMappings Pass_Instancing?>
<?EndGlue?>
</include>
