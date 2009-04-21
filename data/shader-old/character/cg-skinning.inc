<!--
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Hristo Hristov
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

<?Template Skinning_Code?>
#ifndef __CS_SHADER_SKINNING_CGINC__
#define __CS_SHADER_SKINNING_CGINC__

struct Vert_Skinning
{
  <?if vars."bones"?> 
  varying short4 BonesIndices;
  varying float4 BonesWeights;
  uniform float4 Bones[78];
  <?endif?>
};
Vert_Skinning skinningVert;

struct Skinning
{

  float3 QuatMultVert( float4 q1, float3 v )
  {
   float4 r1 = float4(q1.w*v.xyz + cross( q1.xyz, v.xyz ), - dot( q1.xyz, v.xyz));
   float4 r2 = float4(-q1.xyz, q1.w);
   return r1.w*r2.xyz + r2.w*r1.xyz + cross( r1.xyz, r2.xyz );
  }

  void SetupVert ()
  {
  }
  float4 SkinnedPosition (float4 pos)
  {
   <?if vars."bones"?> 
        short idx1 = skinningVert.BonesIndices.x*2;
        short idx2 = skinningVert.BonesIndices.y*2;
        short idx3 = skinningVert.BonesIndices.z*2;
        short idx4 = skinningVert.BonesIndices.w*2;

        float4 Pos;
        Pos.xyz =  (QuatMultVert(skinningVert.Bones[idx1], pos.xyz) + 
	  skinningVert.Bones[idx1+1].xyz) * skinningVert.BonesWeights.x;
        Pos.xyz += (QuatMultVert(skinningVert.Bones[idx2], pos.xyz) + 
	  skinningVert.Bones[idx2+1].xyz) * skinningVert.BonesWeights.y;
        Pos.xyz += (QuatMultVert(skinningVert.Bones[idx3], pos.xyz) + 
	  skinningVert.Bones[idx3+1].xyz) * skinningVert.BonesWeights.z;
        Pos.xyz += (QuatMultVert(skinningVert.Bones[idx4], pos.xyz) + 
	  skinningVert.Bones[idx4+1].xyz) * skinningVert.BonesWeights.w;
        Pos.w = 1;
	return Pos;
  <?else?>
	return pos;
  <?endif?>
  }
};

#endif // __CS_SHADER_SKINNING_CGINC__
<?Endtemplate?>

<?CgAddSnippet Skinning_Code?>

<?BeginGlue Skinning?>
  <?Template Pass_Skinning?>
  <?if vars."bones"?> 
    <buffer source="bones indices" destination="skinningVert.BonesIndices" />
    <buffer source="bones weights" destination="skinningVert.BonesWeights" />
  <?endif ?>
  <?Endtemplate?>
  <?AddToList PassMappings Pass_Skinning ?>
  
  <?Template VariableMap_Skinning ?>
  <?if vars."bones"?> 
    <variablemap variable="bones" destination="skinningVert.Bones"/>
  <?endif ?>
  <?Endtemplate?>
  <?AddToList ProgramMappings VariableMap_Skinning ?>
<?EndGlue?>
</include>
