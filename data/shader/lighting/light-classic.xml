<!--
  Copyright (C) 2007 by Frank Richter
	    (C) 2007 by Jorrit Tyberghein

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
<snippet>
  <technique priority="150">
    <combiner name="cg" plugin="crystalspace.graphics3d.shader.combiner.glcg" />
    
    <input file="/shader-snippets/primaryColor.inp" />
    <input name="color_dir1" type="rgb" default="complex" private="yes">
      <block location="pass">
	<buffer source="lit color dir 1" destination="vertexIn.color_dir1" />
      </block>
      
      <block location="cg:vertexIn">
	<varying type="rgb" name="color_dir1" />
      </block>
      
      <block location="cg:vertexToFragment">
	<varying type="rgb" name="color_dir1" />
      </block>
      
      <block location="cg:vertexMain">
	color_dir1 = vertexIn.color_dir1;
      </block>
    </input>
    <input name="color_dir2" type="rgb" default="complex" private="yes">
      <block location="pass">
	<buffer source="lit color dir 2" destination="vertexIn.color_dir2" />
      </block>
      
      <block location="cg:vertexIn">
	<varying type="rgb" name="color_dir2" />
      </block>
      
      <block location="cg:vertexToFragment">
	<varying type="rgb" name="color_dir2" />
      </block>
      
      <block location="cg:vertexMain">
	color_dir2 = vertexIn.color_dir2;
      </block>
    </input>
    <input name="color_dir3" type="rgb" default="complex" private="yes">
      <block location="pass">
	<buffer source="lit color dir 3" destination="vertexIn.color_dir3" />
      </block>
      
      <block location="cg:vertexIn">
	<varying type="rgb" name="color_dir3" />
      </block>
      
      <block location="cg:vertexToFragment">
	<varying type="rgb" name="color_dir3" />
      </block>
      
      <block location="cg:vertexMain">
	color_dir3 = vertexIn.color_dir3;
      </block>
    </input>
    
    <input name="ambient" type="rgb" default="complex">
      <block location="cg:variablemap">
	<variablemap variable="dynamic ambient" destination="fragmentIn.lightingAmbient" />
      </block>
      
      <block location="cg:fragmentIn">
	<uniform type="rgb" name="lightingAmbient" />
      </block>
      
      <block location="cg:fragmentMain">
	ambient = fragmentIn.lightingAmbient;
      </block>
    </input>
    
    <input name="texCoordLM" type="texcoord2" default="complex" private="yes">
      <block location="pass">
	<buffer source="texture coordinate lightmap" 
	  destination="vertexIn.texcoordLM" />
      </block>
      
      <block location="cg:vertexToFragment">
	<varying type="texcoord2" name="texCoordLM" />
      </block>
      
      <block location="cg:vertexIn">
	<varying type="texcoord2" name="texcoordLM" />
      </block>
      
      <block location="cg:vertexMain">
	texCoordLM = vertexIn.texcoordLM;
      </block>
    </input>
    
    <!-- Surface normal -->
    <input name="normal" type="normal_tangent" default="value" defval="0,0,1">
      <attribute name="perfragment" type="bool" defval="false" />
    </input>

    <input name="lightmap" type="tex2d" default="complex" private="yes">
      <block location="pass">
	<texture name="tex lightmap" destination="fragmentIn.lightmap" />
      </block>
      
      <block location="cg:fragmentIn">
	<uniform type="tex2d" name="lightmap" />
      </block>
      
      <block location="cg:fragmentMain">
	lightmap = fragmentIn.lightmap;
      </block>
    </input>
    <input name="lightmap_dir1" type="tex2d" default="complex" private="yes">
      <block location="pass">
	<texture name="tex lightmap dir 1" destination="fragmentIn.lightmap_dir1" />
      </block>
      
      <block location="cg:fragmentIn">
	<uniform type="tex2d" name="lightmap_dir1" />
      </block>
      
      <block location="cg:fragmentMain">
	lightmap_dir1 = fragmentIn.lightmap_dir1;
      </block>
    </input>
    <input name="lightmap_dir2" type="tex2d" default="complex" private="yes">
      <block location="pass">
	<texture name="tex lightmap dir 2" destination="fragmentIn.lightmap_dir2" />
      </block>
      
      <block location="cg:fragmentIn">
	<uniform type="tex2d" name="lightmap_dir2" />
      </block>
      
      <block location="cg:fragmentMain">
	lightmap_dir2 = fragmentIn.lightmap_dir2;
      </block>
    </input>
    <input name="lightmap_dir3" type="tex2d" default="complex" private="yes">
      <block location="pass">
	<texture name="tex lightmap dir 3" destination="fragmentIn.lightmap_dir3" />
      </block>
      
      <block location="cg:fragmentIn">
	<uniform type="tex2d" name="lightmap_dir3" />
      </block>
      
      <block location="cg:fragmentMain">
	lightmap_dir3 = fragmentIn.lightmap_dir3;
      </block>
    </input>
      
    <block location="cg:vertexMain">
      illumination = float3 (0);
    </block>
    
    <block location="cg:fragmentMain">
      float3 illum_ambient = ambient;
      float3 illum_diffuse;
      <?if vars."tex lightmap dir 1".texture || vars."lit color dir 1".buffer ?>
      <![CDATA[
	// Utilize directional LM/colors only if normal is changed per-fragment
	if (normal_attr_perfragment)
	{
	  /* These should really match up with those in lighter2... */
	  float3 base[3] = 
	  { 
	    float3 (/* -1/sqrt(6) */ -0.408248, /* 1/sqrt(2) */ 0.707107, /* 1/sqrt(3) */ 0.577350),
	    float3 (/* sqrt(2/3) */ 0.816497, 0, /* 1/sqrt(3) */ 0.577350),
	    float3 (/* -1/sqrt(6) */ -0.408248, /* -1/sqrt(2) */ -0.707107, /* 1/sqrt(3) */ 0.577350)
	  };
	  float3 coeff = float3 (saturate (dot (normal, base[0])),
				 saturate (dot (normal, base[1])),
				 saturate (dot (normal, base[2])));
	  coeff *= coeff;
	  
 	  float3 baseLit[3];
	  ]]>
	<?if vars."tex lightmap dir 1".texture ?>
	<![CDATA[
	  baseLit[0] = tex2D (lightmap_dir1, texCoordLM).rgb;
	  baseLit[1] = tex2D (lightmap_dir2, texCoordLM).rgb;
	  baseLit[2] = tex2D (lightmap_dir3, texCoordLM).rgb;
	]]>
	<?else?>
	<![CDATA[
	  baseLit[0] = color_dir1;
	  baseLit[1] = color_dir2;
	  baseLit[2] = color_dir3;
	]]>
	<?endif?>
      <![CDATA[
	  illum_diffuse = 2 * (coeff.x * baseLit[0]
			     + coeff.y * baseLit[1]
			     + coeff.z * baseLit[2]);
	}
	else
	// Use plain ol' less detailed lighting otherwise
      ]]>
      <?endif?>
	{
	<?if vars."tex lightmap".texture ?>
	  illum_diffuse = 2 * tex2D (lightmap, texCoordLM).rgb;
	<?else?>
	  illum_ambient = float3 (0, 0, 0);
	  illum_diffuse = primaryColor.rgb;
	<?endif?>
	}
      illumination = illum_ambient + illum_diffuse;
    </block>
    
    <output name="illumination" type="rgb" />
  </technique>
  
  <technique priority="100">
    <combiner name="cg" plugin="crystalspace.graphics3d.shader.combiner.glcg" />
    
    <input file="/shader-snippets/primaryColor.inp" />
    
    <input name="ambient" type="rgb" default="complex">
      <block location="cg:variablemap">
	<variablemap variable="dynamic ambient" destination="fragmentIn.lightingAmbient" />
      </block>
      
      <block location="cg:fragmentIn">
	<uniform type="rgb" name="lightingAmbient" />
      </block>
      
      <block location="cg:fragmentMain">
	ambient = fragmentIn.lightingAmbient;
      </block>
    </input>
    
    <input name="texCoordLM" type="texcoord2" default="complex" private="yes">
      <block location="pass">
	<buffer source="texture coordinate lightmap" 
	  destination="vertexIn.texcoordLM" />
      </block>
      
      <block location="cg:vertexToFragment">
	<varying type="texcoord2" name="texCoordLM" />
      </block>
      
      <block location="cg:vertexIn">
	<varying type="texcoord2" name="texcoordLM" />
      </block>
      
      <block location="cg:vertexMain">
	texCoordLM = vertexIn.texcoordLM;
      </block>
    </input>
    
    <input name="lightmap" type="tex2d" default="complex" private="yes">
      <block location="pass">
	<texture name="tex lightmap" destination="fragmentIn.lightmap" />
      </block>
      
      <block location="cg:fragmentIn">
	<uniform type="tex2d" name="lightmap" />
      </block>
      
      <block location="cg:fragmentMain">
	lightmap = fragmentIn.lightmap;
      </block>
    </input>
      
    <block location="cg:vertexMain">
      illumination = float3 (0);
    </block>
    
    <block location="cg:fragmentMain">
      <?if vars."tex lightmap".texture ?>
	float3 illum_ambient = ambient;
	float3 illum_diffuse = 2 * tex2D (lightmap, texCoordLM).rgb;
      <?else?>
	float3 illum_ambient = float3 (0, 0, 0);
	float3 illum_diffuse = primaryColor.rgb;
      <?endif?>
      illumination = illum_ambient + illum_diffuse;
    </block>
    
    <output name="illumination" type="rgb" />
  </technique>
</snippet>
