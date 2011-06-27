<!--
  Copyright (C) 2008 by Frank Richter

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
  <?SIfNDef __SHADERCOND_TOOLS_INC__ ?>
  <?Define __SHADERCOND_TOOLS_INC__ ?>
  
  <?Template Lighting_Variablemaps?>
    <!-- Cg shader plugin magic makes arrays work -->
    <variablemap variable="light count" 
		 destination="lightProps.count" />
    <variablemap variable="light position world" 
		 destination="lightProps.posWorld" />
    <variablemap variable="light diffuse" 
		 destination="lightProps.colorDiffuse" />
    <variablemap variable="light specular" 
		 destination="lightProps.colorSpecular" />
    <variablemap variable="light attenuation" 
		 destination="lightProps.attenuationVec" />
    <variablemap variable="light inner falloff" 
		 destination="lightProps.falloffInner" />
    <variablemap variable="light outer falloff" 
		 destination="lightProps.falloffOuter" />
    <variablemap variable="light transform world" 
		 destination="lightProps.transform" />
    <variablemap variable="light transform world inverse" 
		 destination="lightProps.transformInv" />
  <?Endtemplate?>
  
  <?Template Lighting_ShadowMap_Variablemaps?>
    <variablemap variable="light shadow map projection" 
		 destination="lightPropsSM.shadowMapTF" />
    <variablemap variable="light shadow map pixel size" 
		 destination="lightPropsSM.shadowMapPixels" />
    <variablemap variable="light shadow map unscale" 
		 destination="lightPropsSM.shadowMapUnscale" />
    <variablemap variable="light numSplits" 
		 destination="lightPropsSM.shadowMapNumSplits" />		 
    <variablemap variable="light splitDists" 
		 destination="lightPropsSM.splitDists" />		 
  <?Endtemplate?>
  
  <?SEndIf?>
</include>
