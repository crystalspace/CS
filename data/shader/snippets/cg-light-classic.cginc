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
  
  void Compute (AppToVert_Lighting IN)
  {
    color = IN.color;
  <?if vars."tex lightmap".texture ?>
    texCoordLM = IN.texCoordLM;
  <?endif?>
  }
};

struct AppToFrag_Lighting
{
  uniform sampler2D lightmap;
  
  float4 GetLighting (VertToFrag_Lighting V2F)
  {
    float4 result;
  <?if vars."tex lightmap".texture ?>
    result = 2 * tex2D (lightmap, V2F.texCoordLM);
    result.a *= V2F.color.a;
  <?else?>
    result = V2F.color;
  <?endif?>
    return result;
  }
};

#endif // __CS_SHADER_LIGHT_CLASSIC_CG__
</include>
