<include>
  <pass>
    <?! This inserts the pass-specific settings required by the snippets
	(notably buffer mappings, texture mappings). ?>
    <?InsertList Passes?>
    
    <?! std_lighting.cscg contains both the VP and FP code in different
	functions. ?>
    <vp plugin="glcg">
      <cgvp>
	<entry>VertexMain</entry>
	<?include std_lighting.cscg ?>
      </cgvp>
    </vp>
    <fp plugin="glcg">
      <cgfp>
	<entry>FragmentMain</entry>
	<?include std_lighting.cscg ?>
      </cgfp>
    </fp>
  </pass>
</include>