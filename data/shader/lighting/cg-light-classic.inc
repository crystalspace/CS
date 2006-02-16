<include>
  <?template Pass_Lighting ?>
    <?if vars."tex lightmap".texture ?>
      <buffer source="texture coordinate lightmap" 
	destination="lighting.texCoordLM" />
      <texture name="tex lightmap" destination="lightingClassicA2F.lightmap" />
    <?else?>
      <buffer source="color" destination="lighting.color" />
    <?endif?>
  <?endtemplate?>
  <?AddToList Passes Pass_Lighting ?>
  
  <?template VariableMap_Lighting ?>
  <?endtemplate?>
  <?AddToList VariableMaps VariableMap_Lighting ?>
  
  <?template Include_Lighting?>
    <?include /shader/lighting/cg-light-classic.cginc ?>
  <?endtemplate?>
  <?AddToList Includes Include_Lighting ?>

  <?template IfSurfaceNormalsNeeded?>
    <?if consts.false?>
  <?endtemplate ?>
</include>