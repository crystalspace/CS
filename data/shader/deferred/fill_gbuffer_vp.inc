<include>
<program>

struct app2vertex 
{
  float3 Position   : POSITION;
  float3 Normal     : NORMAL;
  float2 TexCoord   : TEXCOORD0;
  float2 TexCoordLM : TEXCOORD1;
  float3 Tangent    : TEXCOORD2;
  float3 BiNormal   : TEXCOORD3;
};

struct vertex2fragment 
{ 
  float4 Position     : POSITION;
  float2 TexCoord     : TEXCOORD0;
  float2 TexCoordLM   : TEXCOORD1;
  float3 ViewPosition : TEXCOORD2;
  float3 ViewNormal   : TEXCOORD3;
  float3 ViewTangent  : TEXCOORD4;
  float3 ViewBiNormal : TEXCOORD5;
};

vertex2fragment main(app2vertex IN,
                     uniform float4x4 ModelView : state.matrix.modelview,
                     uniform float4x4 ModelViewIT : state.matrix.modelview.invtrans,
                     uniform float4x4 ModelViewProj : state.matrix.mvp)
{
  vertex2fragment OUT;

  float4 pos = mul (ModelViewProj, float4 (IN.Position, 1.0));
  
  OUT.Position      = mul (ModelViewProj, float4 (IN.Position, 1.0));
  OUT.ViewPosition  = mul (ModelView, float4 (IN.Position, 1.0)).xyz;
  OUT.TexCoord      = IN.TexCoord;
  OUT.TexCoordLM    = IN.TexCoordLM;
  OUT.ViewNormal    = mul (ModelViewIT, float4 (IN.Normal, 0.0)).xyz;
  OUT.ViewTangent   = mul (ModelViewIT, float4 (IN.Tangent, 0.0)).xyz;
  OUT.ViewBiNormal  = mul (ModelViewIT, float4 (IN.BiNormal, 0.0)).xyz;

  return OUT;
}

</program>
</include>
