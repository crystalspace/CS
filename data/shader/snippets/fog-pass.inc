      <?if vars."fog density".float &gt; 0 ?>
        <pass>
          <mixmode><alpha /></mixmode><!-- @@@ probably not always desired -->
          <zmode><zmesh2 /></zmode>
          <texture name="standardtex fog" destination="unit 0" />
          <vp plugin="glfixed">
            <fixedvp>
              <texgen type="fog" layer="0"/>
              <constantcolor layer="0" type="shadervar">fog color</constantcolor>
            </fixedvp>
          </vp>
          <fp plugin="glfixed">
            <fixedfp>
              <layer>
                <colorsource num="0" source="constant color" modifier="color"/>
                <coloroperation operation="replace" />
                <alphasource num="0" source="texture" modifier="alpha"/>
                <alphaoperation operation="replace" />
              </layer>
            </fixedfp>
          </fp>
        </pass>
      <?endif?>
