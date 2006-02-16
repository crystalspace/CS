<include>
  <?! Standard CS sector fog. The fog intensity is the minimum of the distance
      from the portal plane and view plane. ?>
  <?template Pass_Fog ?>
    <?if vars."fog density".float &gt; 0 ?>
      <!-- We need the fog lookup texture -->
      <texture name="standardtex fog" destination="fogSectorA2F.fogTex" />
    <?endif?>
  <?endtemplate?>
  <?AddToList Passes Pass_Fog ?>
  
  <?template VariableMap_Fog ?>
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
  <?endtemplate?>
  <?AddToList VariableMaps VariableMap_Fog ?>
  
  <?template Include_Fog?>
    <?include /shader/fog/cg-sectorfog.cginc ?>
  <?endtemplate?>
  <?AddToList Includes Include_Fog ?>
</include>
