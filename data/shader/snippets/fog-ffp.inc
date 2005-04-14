  <?if vars."fog density".float &gt; 0 ?>
    <layer name="pea soup">
      <colorsource num="0" source="constant color" modifier="color"/>
      <colorsource num="1" source="previous layer" modifier="color"/>
      <colorsource num="2" source="texture" modifier="alpha"/>
      <coloroperation operation="interpolate" />
      <alphasource num="0" source="previous layer" modifier="alpha"/>
      <alphaoperation operation="replace"/>
    </layer>
  <?endif?>
