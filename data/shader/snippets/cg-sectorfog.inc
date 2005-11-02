<include>
  <?template Cg_Pass_Fog ?>
    <?if vars."fog density".float &gt; 0 ?>
      <texture name="standardtex fog" destination="A2F.fog.fogTex" />
    <?endif?>
  <?endtemplate?>
  <?template Cg_VariableMap_Fog ?>
    <?if vars."fog density".float &gt; 0 ?>
      <variablemap variable="fog color" destination="A2F.fog.fogColor" />
      <?if vars."fogplane".z &gt; 0 ?>
	<!-- 
	  coeff1[] = {0, 0, density, -0.1*density};
	  float blend = 1.0f - pow (1.0f + fogplane.w*0.2f, 5); 
          blend = MIN (MAX (blend, 0), 1);
          const float dist =  fogplane.w*density;
          coeff1[3] = (MIN (MAX (dist, -1.0f), 0) - 0.1f*density);
	  -->
	<variablemap destination="A2V.fog.fogPlaneS" type="expr">
	  <sexp>
	  (make-vector 
	    0 
	    0 
	    "fog density" 
	    (- (min (max (* (elt4 fogplane) "fog density") -1) 0) (* 0.1 "fog density"))
	  )</sexp>
	</variablemap>
	<!--
	  coeff2[0] = fogplane.x*density/fogplane.z;
          coeff2[1] = fogplane.y*density/fogplane.z;
          coeff2[2] = density;
          coeff2[3] = (1-blend)+(fogplane.w/fogplane.z-0.1)*density;
	  -->
	<variablemap destination="A2V.fog.fogPlaneT" type="expr">
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
	<variablemap destination="A2V.fog.fogPlaneS" type="expr">
	  <sexp>(make-vector 0 0 "fog density" (* -0.1 "fog density"))</sexp>
	</variablemap>
	<variablemap destination="A2V.fog.fogPlaneT" type="vector4">0,0,0,1</variablemap>
      <?endif?>
    <?endif?>
  <?endtemplate?>
  <?template Cg_Include_Fog?>
    <?include cg-sectorfog.cginc ?>
  <?endtemplate?>
</include>
