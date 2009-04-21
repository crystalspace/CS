<include>
  <?if vars."light 0 type".int == 2 ?>
    <pass>
      <mixmode>
        <copy />
      </mixmode>
      <writemask r="false" g="false" b="false" a="true" />
      <?if vars."light 0 type".int == consts.CS_LIGHT_SPOTLIGHT ?>
        <vp plugin="glfixed">
          <fixedvp>
            <light num="0"/>
            <texgen layer="0" type="projection"/>
          </fixedvp>
        </vp>
      <?else?>
        <vp plugin="glarb" file="/shader/flat_ppl_attenuation.avp" />
      <?endif?>

      <?if vars."light 0 type".int == consts.CS_LIGHT_SPOTLIGHT ?>
        <texture destination="unit 0" name="tex projection" />
      <?else?>
        <texture destination="unit 0" name="light 0 attenuationtex" />
        <texture destination="unit 1" name="light 0 attenuationtex" />
      <?endif?>
      <buffer source="texture coordinate 0" destination="texture coordinate 0" />
      <fp plugin="glfixed">
        <fixedfp>
          <layer>
            <alphasource num="0" source="texture" modifier="alpha" />
            <alphaoperation operation="replace" />
          </layer>
        <?if vars."light 0 type".int == consts.CS_LIGHT_SPOTLIGHT ?>
          <layer>
            <alphasource num="0" source="texture" modifier="alpha" />
            <alphasource num="1" source="previous layer" modifier="alpha" />
            <alphaoperation operation="modulate" scale="2" />
          </layer>
      <?endif?>
        </fixedfp>
      </fp>
    </pass>
  <?endif?>
</include>
