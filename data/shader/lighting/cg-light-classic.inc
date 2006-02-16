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
  <?Template Pass_Lighting ?>
    <?if vars."tex lightmap".texture ?>
      <buffer source="texture coordinate lightmap" 
	destination="lighting.texCoordLM" />
      <texture name="tex lightmap" destination="lightingClassicA2F.lightmap" />
    <?else?>
      <buffer source="color" destination="lighting.color" />
    <?endif?>
  <?Endtemplate?>
  <?AddToList Passes Pass_Lighting ?>
  
  <?Template VariableMap_Lighting ?>
  <?Endtemplate?>
  <?AddToList VariableMaps VariableMap_Lighting ?>
  
  <?Template Include_Lighting?>
    <?Include /shader/lighting/cg-light-classic.cginc ?>
  <?Endtemplate?>
  <?AddToList Includes Include_Lighting ?>

  <?Template ifSurfaceNormalsNeeded?>
    <?if consts.false?>
  <?Endtemplate ?>
</include>