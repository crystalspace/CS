<include>
  <?template Cg_Pass_Surface ?>
    <?if vars."tex diffuse".texture ?>
      <buffer source="texture coordinate" destination="A2V.surface.texCoord" />
      <texture name="tex diffuse" destination="A2F.surface.texture" />
    <?endif?>
  <?endtemplate?>
  <?template Cg_VariableMap_Surface ?>
    <?if !vars."tex diffuse".texture ?>
      <variablemap variable="mat flatcolor" destination="A2F.surface.flatcolor" />
    <?endif?>
  <?endtemplate?>
  <?template Cg_Include_Surface?>
    <?include cg-surface-classic.cginc ?>
  <?endtemplate?>
</include>
