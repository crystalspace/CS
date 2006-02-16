<include>
#ifndef __CS_SHADER_SURFACE_CLASSIC_CG__
#define __CS_SHADER_SURFACE_CLASSIC_CG__

<?! Include some other used code.
    Why this include and no Cg #include? One reason is that Cg knows nothing
    about VFS, while with a custom include VFS paths are no problem. But more
    importantly, files included like this are parsed by the shader conditional
    code, so shader conditionals can be used here. ?>
<?include /shader/snippets/cg-i-surface.cginc ?>
<?include /shader/surface/cg-parallax.cginc ?>

struct AppToVert_Surface_Classic
{
<?IfSurfaceNormalsNeeded?>
  float3 normal;
<?endif?>
<?if vars."tex diffuse".texture ?>
  float2 texCoord;
<?endif?>
  AppToVert_Parallax parallax;
};

struct VertToFrag_Surface_Classic
{
<?IfSurfaceNormalsNeeded?>
  float3 normal;
<?endif?>
<?if vars."tex diffuse".texture ?>
  float2 texCoord;
<?endif?>
  VertToFrag_Parallax parallax;

  void Setup (AppToVert_Surface_Classic A2V)
  {
  <?IfSurfaceNormalsNeeded?>
    normal = A2V.normal;
  <?endif?>
  <?if vars."tex diffuse".texture ?>
    texCoord = A2V.texCoord;
  <?endif?>
    parallax.Setup (A2V.parallax);
  }
};

struct AppToFrag_Surface_Classic
{
<?if vars."tex diffuse".texture ?>
  uniform sampler2D texture;
<?else?>
  uniform float4 flatcolor;
<?endif?>
<?if vars."tex glow".texture ?>
  uniform sampler2D texGlow;
<?endif?>
  AppToFrag_Parallax parallax;
};

AppToFrag_Surface_Classic surfaceClassicA2F;

struct Frag_Surface_Classic : iSurface
{
<?IfSurfaceNormalsNeeded?>
  float3 normal;
<?endif?>
<?if vars."tex diffuse".texture || vars."tex glow".texture ?>
  float2 tc;
  float2 tcOffs;
  Frag_Parallax parallax;
<?endif?>

  void Setup (VertToFrag_Surface_Classic V2F)
  {
  <?IfSurfaceNormalsNeeded?>
    normal = V2F.normal;
  <?endif?>
  <?if vars."tex diffuse".texture || vars."tex glow".texture ?>
    tc = V2F.texCoord;
    tcOffs = parallax.GetTCOffset (V2F.parallax, surfaceClassicA2F.parallax,
      tc);
  <?endif?>
  }
  
  float4 GetDiffuse ()
  {
    float4 result;
  <?if vars."tex diffuse".texture ?>
    result = tex2D (surfaceClassicA2F.texture, tc + tcOffs);
  <?else?>
    result = surfaceClassicA2F.flatcolor;
  <?endif?>
    return result;
  }
  
  float3 GetNormal ()
  {
  <?IfSurfaceNormalsNeeded?>
    return normal;
  <?else?>
    return float3 (0, 0, 0);
  <?endif?>
  }

  float3 GetSpecularColor ()
  {
    return float3 (1, 1, 1);
  }
  
  float GetSpecularExponent ()
  {
    return 32.0;
  }
  
  float3 GetEmissive ()
  {
  <?if vars."tex glow".texture ?>
    return tex2D (surfaceClassicA2F.texGlow, tc + tcOffs);
  <?else?>
    return float3 (0, 0, 0);
  <?endif?>
  }
  
  float2 GetTexCoordOffset ()
  {
    return tcOffs;
  }
};

<?SetSnippet Surface Surface_Classic?>

#endif // __CS_SHADER_SURFACE_CLASSIC_CG__
</include>
