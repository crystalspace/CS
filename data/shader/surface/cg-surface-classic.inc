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
  <?! Simple surface. Supports a diffuse texture or flat color, glow map,
      parallax mapping and per-vertex normals.
 
      Note: must be included *after* the lighting snippet, since that provides
      the "ifSurfaceNormalsNeeded" template which specifies whether the
      lighting step needs a normal (if it doesn't, passing around the normal
      can be optimized away). ?>

  <?Template Pass_Surface ?>
    <?if vars."tex diffuse".texture ?>
      <buffer source="texture coordinate" destination="surface.texCoord" />
      <texture name="tex diffuse" destination="surfaceClassicA2F.texture" />
      <?if vars."tex height".texture ?>
	<texture name="tex height" 
	  destination="surfaceClassicA2F.parallax.heightTex" />
	<buffer source="normal" destination="surface.parallax.Normal" />
	<buffer source="tangent" destination="surface.parallax.Tangent" />
	<buffer source="binormal" destination="surface.parallax.BiNormal" />
      <?endif?>
      <?ifSurfaceNormalsNeeded?>
	<buffer source="normals" destination="surface.normal" />
      <?endif?>
    <?endif?>
  <?Endtemplate?>
  <?AddToList Passes Pass_Surface ?>
  
  <?Template VariableMap_Surface ?>
    <?if !vars."tex diffuse".texture ?>
      <variablemap variable="mat flatcolor" destination="surfaceClassicA2F.flatcolor" />
    <?endif?>
  <?Endtemplate?>
  <?AddToList VariableMaps VariableMap_Surface ?>
  
  <?Template Include_Surface?>
    <?Include /shader/surface/cg-surface-classic.cginc ?>
  <?Endtemplate?>
  <?AddToList Includes Include_Surface ?>
</include>
