<include>
#ifndef __CS_SHADER_SECTORFOG_CG__
#define __CS_SHADER_SECTORFOG_CG__

struct AppToVert_Fog_Sector
{
  void _dummy_struct_non_empty() {}
<?if vars."fog density".float &gt; 0 ?>
  uniform float4 fogPlaneS;
  uniform float4 fogPlaneT;
  uniform float4x4 ModelView : state.matrix.modelview;
  float4 Position : POSITION;
<?endif?>
};

struct VertToFrag_Fog_Sector
{
<?if vars."fog density".float &gt; 0 ?>
  float2 fogTC;
<?endif?>

  void Setup (AppToVert_Fog_Sector A2V)
  {
  <?if vars."fog density".float &gt; 0 ?>
    float4 eyePos = mul (A2V.ModelView, A2V.Position);
    fogTC.x = dot (eyePos, A2V.fogPlaneS);
    fogTC.y = dot (eyePos, A2V.fogPlaneT);
  <?endif?>
  }
};

struct AppToFrag_Fog_Sector
{
  void _dummy_struct_non_empty() {}
<?if vars."fog density".float &gt; 0 ?>
  uniform sampler2D fogTex;
  uniform float4 fogColor;
<?endif?>
};

AppToFrag_Fog_Sector fogSectorA2F;

struct Frag_Fog_Sector
{
<?if vars."fog density".float &gt; 0 ?>
  float2 fogTC;
<?endif?>

  void Setup (VertToFrag_Fog_Sector V2F)
  {
  <?if vars."fog density".float &gt; 0 ?>
    fogTC = V2F.fogTC;
  <?endif?>
  }
  
  float4 ApplyFog (float4 surfaceColor)
  {
  <?if vars."fog density".float &gt; 0 ?>
    float4 fog = tex2D (fogSectorA2F.fogTex, fogTC);
    float4 result;
    result.rgb = lerp (surfaceColor.rgb, 
      fogSectorA2F.fogColor.rgb, fog.a);
    result.a = surfaceColor.a;
    return result;
  <?else?>
    return surfaceColor;
  <?endif?>
  }
};

<?SetSnippet Fog Fog_Sector?>

#endif // __CS_SHADER_SECTORFOG_CG__
</include>
