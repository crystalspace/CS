<include>
#ifndef __CS_SHADER_LIGHT_CLASSIC_CG__
#define __CS_SHADER_LIGHT_CLASSIC_CG__

struct AppToVert_Lighting
{
  float4 color : COLOR;
  <?if vars."tex lightmap".texture ?>
    float2 texCoordLM;
  <?endif?>
};
struct VertToFrag_Lighting
{
  float4 color : COLOR;
  <?if vars."tex lightmap".texture ?>
    float2 texCoordLM;
  <?endif?>
};
struct AppToFrag_Lighting
{
  sampler2D lightmap;
};

<?template ComputeVertex_Lighting V2F A2V?>
  $V2F$.color = $A2V$.color;
  <?if vars."tex lightmap".texture ?>
    $V2F$.texCoordLM = $A2V$.texCoordLM;
  <?endif?>
<?endtemplate?>

<?template ComputeFragment_Lighting OUT V2F A2F ?>
  <?if vars."tex lightmap".texture ?>
    $OUT$ = 2 * tex2D ($A2F$.lightmap, $V2F$.texCoordLM);
    $OUT$.a *= $V2F$.color.a;
  <?else?>
    $OUT$ = $V2F$.color;
  <?endif?>
<?endtemplate?>

#endif // __CS_SHADER_LIGHT_CLASSIC_CG__
</include>
