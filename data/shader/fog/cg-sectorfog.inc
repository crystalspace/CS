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
  <?! Standard CS sector fog. The fog intensity is the minimum of the distance
      from the portal plane and view plane. ?>
  <?Template Pass_Fog ?>
    <?if vars."fog density".float &gt; 0 ?>
      <!-- We need the fog lookup texture -->
      <texture name="standardtex fog" destination="fogSectorA2F.fogTex" />
    <?endif?>
  <?Endtemplate?>
  <?AddToList Passes Pass_Fog ?>
  
  <?Template VariableMap_Fog ?>
    <?if vars."fog density".float &gt; 0 ?>
      <variablemap variable="fog color" destination="fogSectorA2F.fogColor" />
      <!-- See the glshader_fixed plugin source for a detailed explanation 
	   of the formular below. -->
      <?if vars."fogplane".z &gt; 0 ?>
	<variablemap destination="fog.fogPlaneS" type="expr">
	  <sexp>
	  (make-vector 
	    0 
	    0 
	    "fog density" 
	    (- (min (max (* (elt4 fogplane) "fog density") -1) 0) (* 0.1 "fog density"))
	  )</sexp>
	</variablemap>
	<variablemap destination="fog.fogPlaneT" type="expr">
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
	<variablemap destination="fog.fogPlaneS" type="expr">
	  <sexp>(make-vector 0 0 "fog density" (* -0.1 "fog density"))</sexp>
	</variablemap>
	<variablemap destination="fog.fogPlaneT" type="vector4">0,0,0,1</variablemap>
      <?endif?>
    <?endif?>
  <?Endtemplate?>
  <?AddToList VariableMaps VariableMap_Fog ?>
  
  <?Template Include_Fog?>
    <?Include /shader/fog/cg-sectorfog.cginc ?>
  <?Endtemplate?>
  <?AddToList Includes Include_Fog ?>
</include>
