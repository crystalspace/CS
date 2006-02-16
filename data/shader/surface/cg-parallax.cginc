<include>
#ifndef __CS_SHADER_PARALLAX_CGINC__
#define __CS_SHADER_PARALLAX_CGINC__

/* Parallax mapping.
 * This is no snippet in the sense of e.g. the "surface-classic" snippet, it's
 * rather a component that can be used by other Cg. This also means that 
 * calling the vertex-to-fragment setup method, binding the texture etc. needs
 * to be done by the including snippet/shader as well.
 */

struct AppToVert_Parallax
{
  void _dummy_struct_non_empty() {}
<?if vars."tex height".texture ?>
  float4 Position : POSITION;
  uniform float4x4 ModelViewIT : state.matrix.modelview.invtrans;
<?endif?>
};

struct VertToFrag_Parallax
{
<?if vars."tex height".texture ?>
  float3 eyeVec;
<?endif?>

  void Setup (AppToVert_Parallax A2V)
  {
  <?if vars."tex height".texture ?>
    eyeVec = A2V.ModelViewIT[3] - A2V.Position;
  <?endif?>
  }
};

struct AppToFrag_Parallax
{
  void _dummy_struct_non_empty() {}
<?if vars."tex height".texture ?>
  uniform sampler2D heightTex;
<?endif?>
};

struct Frag_Parallax
{
  float2 GetTCOffset (VertToFrag_Parallax V2F, 
    AppToFrag_Parallax parallaxA2F, float2 texCoord)
  {
  <?if vars."tex height".texture ?>
    float4 height = 0.04 * tex2D(parallaxA2F.heightTex, texCoord) - 0.02;
    return (height * normalize (V2F.eyeVec)).xy;
  <?else?>
    return float2 (0, 0);
  <?endif?>
  }
};

#endif // __CS_SHADER_PARALLAX_CGINC__
</include>