<include>
#ifndef __CS_SHADER_LIGHT_CLASSIC_CG__
#define __CS_SHADER_LIGHT_CLASSIC_CG__

<?include /shader/snippets/cg-i-surface.cginc ?>

struct AppToVert_Lighting_Classic
{
  float4 color : COLOR;
<?if vars."tex lightmap".texture ?>
  float2 texCoordLM;
<?endif?>
};

struct VertToFrag_Lighting_Classic
{
  float4 color : COLOR;
<?if vars."tex lightmap".texture ?>
  float2 texCoordLM;
<?endif?>

  void Setup (AppToVert_Lighting_Classic A2V)
  {
    color = A2V.color;
  <?if vars."tex lightmap".texture ?>
    texCoordLM = A2V.texCoordLM;
  <?endif?>
  }
};

struct AppToFrag_Lighting_Classic
{
<?if vars."tex lightmap".texture ?>
  uniform sampler2D lightmap;
<?endif?>
};

AppToFrag_Lighting_Classic lightingClassicA2F;

struct Frag_Lighting_Classic
{
<?if vars."tex lightmap".texture ?>
  float2 texCoordLM;
<?endif?>
  float4 color;

  void Setup (VertToFrag_Lighting_Classic V2F)
  {
    color = V2F.color;
  <?if vars."tex lightmap".texture ?>
    texCoordLM = V2F.texCoordLM;
  <?endif?>
  }
  
  float4 Illuminate (iSurface surface)
  {
    float4 result;
    
    float4 light_diffuse;
  <?if vars."tex lightmap".texture ?>
    light_diffuse = 2 * tex2D (lightingClassicA2F.lightmap, 
      texCoordLM /*+ surface.GetTexCoordOffset ()*/);
    light_diffuse.a *= color.a;
  <?else?>
    light_diffuse = color;
  <?endif?>
    result = light_diffuse * surface.GetDiffuse();
  
    result.rgb = result.rgb + surface.GetEmissive ();
  
    return result;
  }
};

<?SetSnippet Lighting Lighting_Classic?>

#endif // __CS_SHADER_LIGHT_CLASSIC_CG__
</include>
