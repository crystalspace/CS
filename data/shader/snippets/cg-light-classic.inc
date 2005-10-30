<include>
  <?template Cg_Pass_Lighting ?>
    <?if vars."tex lightmap".texture ?>
      <buffer source="texture coordinate lightmap" 
	destination="A2V.lighting.texCoordLM" />
      <texture name="tex lightmap" destination="A2F.lighting.lightmap" />
    <?endif?>
  <?endtemplate?>
  <?template Cg_VariableMap_Lighting ?>
  <?endtemplate?>
  <?template Cg_Include_Lighting?>
    <?include cg-light-classic.cginc ?>
  <?endtemplate?>
</include>