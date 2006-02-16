<include>
  <?! Simple surface. Supports a diffuse texture or flat color, glow map,
      parallax mapping and per-vertex normals.
 
      Note: must be included *after* the lighting snippet, since that provides
      the "IfSurfaceNormalsNeeded" template which specifies whether the
      lighting step needs a normal (if it doesn't, passing around the normal
      can be optimized away). ?>

  <?template Pass_Surface ?>
    <?if vars."tex diffuse".texture ?>
      <buffer source="texture coordinate" destination="surface.texCoord" />
      <texture name="tex diffuse" destination="surfaceClassicA2F.texture" />
      <?if vars."tex height".texture ?>
	<texture name="tex height" 
	  destination="surfaceClassicA2F.parallax.heightTex" />
      <?endif?>
      <?IfSurfaceNormalsNeeded?>
	<buffer source="normals" destination="surface.normal" />
      <?endif?>
    <?endif?>
  <?endtemplate?>
  <?AddToList Passes Pass_Surface ?>
  
  <?template VariableMap_Surface ?>
    <?if !vars."tex diffuse".texture ?>
      <variablemap variable="mat flatcolor" destination="surface.flatcolor" />
    <?endif?>
  <?endtemplate?>
  <?AddToList VariableMaps VariableMap_Surface ?>
  
  <?template Include_Surface?>
    <?include /shader/surface/cg-surface-classic.cginc ?>
  <?endtemplate?>
  <?AddToList Includes Include_Surface ?>
</include>
