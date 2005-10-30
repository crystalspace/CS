<include>
#ifndef __CS_SHADER_SURFACE_CLASSIC_CG__
#define __CS_SHADER_SURFACE_CLASSIC_CG__

struct AppToVert_Surface
{
  void _dummy_struct_non_empty() {}
  <?if vars."tex diffuse".texture ?>
    float2 texCoord;
  <?endif?>
};
struct VertToFrag_Surface
{
  void _dummy_struct_non_empty() {}
  <?if vars."tex diffuse".texture ?>
    float2 texCoord;
  <?endif?>
};
struct AppToFrag_Surface
{
<?if vars."tex diffuse".texture ?>
    sampler2D texture;
<?else?>
    float4 flatcolor;
<?endif?>
};

<?template ComputeVertex_Surface OUT IN?>
  <?if vars."tex diffuse".texture ?>
    $OUT$.texCoord = $IN$.texCoord;
  <?endif?>
<?endtemplate?>

<?template ComputeFragment_Surface OUT V2F A2F ?>
  <?if vars."tex diffuse".texture ?>
    $OUT$ = tex2D ($A2F$.texture, $V2F$.texCoord);
  <?else?>
    $OUT$ = $A2F$.flatcolor;
  <?endif?>
<?endtemplate?>

#endif // __CS_SHADER_SURFACE_CLASSIC_CG__
</include>
