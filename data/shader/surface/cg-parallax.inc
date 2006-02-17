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
  <?! Parallax mapping.
      This is no snippet in the sense of e.g. the "surface-classic" snippet, 
      it's rather a component that can be used by other Cg. This also means 
      that  calling the vertex-to-fragment setup method and invoking the
      Pass_Parallax/VariableMap_Parallax templates needs to be done by the 
      including snippet/shader as well.
  ?>
  <?Template Pass_Parallax ?>
    <?if vars."tex height".texture ?>
      <texture name="tex height" destination="parallaxA2F.heightTex" />
      <buffer source="normal" destination="parallaxA2V.Normal" />
      <buffer source="tangent" destination="parallaxA2V.Tangent" />
      <buffer source="binormal" destination="parallaxA2V.BiNormal" />
    <?endif?>
  <?Endtemplate?>
  
  <?Template VariableMap_Parallax ?>
  <?Endtemplate?>
</include>
