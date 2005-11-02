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
<?if vars."tex diffuse".texture ?>
  float2 texCoord;
<?endif?>
  
  void Compute (AppToVert_Surface IN)
  {
  <?if vars."tex diffuse".texture ?>
    texCoord = IN.texCoord;
  <?endif?>
  }
};

struct AppToFrag_Surface
{
<?if vars."tex diffuse".texture ?>
  uniform sampler2D texture;
<?else?>
  uniform float4 flatcolor;
<?endif?>
  
  float4 GetDiffuse (VertToFrag_Surface V2F)
  {
    float4 result;
  <?if vars."tex diffuse".texture ?>
    result = tex2D (texture, V2F.texCoord);
  <?else?>
    result = flatcolor;
  <?endif?>
    return result;
  }
};

#endif // __CS_SHADER_SURFACE_CLASSIC_CG__
</include>
