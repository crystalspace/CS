<include>

<variablemap variable="mat flatcolor" destination="FlatColor" />
<variablemap variable="specular" destination="ConstSpecular" />
<variablemap variable="dynamic ambient" destination="DynamicAmbient" />
<variablemap variable="combined ambient" destination="CombinedAmbient" />
<variablemap variable="far clip distance" destination="FarClipDistance" />

<program>

<?Include /shader-snippets/lighting/directional_lms.cginc ?>
<?Include /shader-snippets/cg-common.cginc ?>
 
struct vertex2fragment 
{ 
  float2 TexCoord     : TEXCOORD0;
  float2 TexCoordLM   : TEXCOORD1;
  float3 ViewPosition : TEXCOORD2;
  float3 ViewNormal   : TEXCOORD3;
  float3 ViewTangent  : TEXCOORD4;
  float3 ViewBiNormal : TEXCOORD5;
  //float3 WorldNormal  : TEXCOORD6;
};

struct fragmentOutput
{
  float4 Color0 : COLOR0;
  float4 Color1 : COLOR1;
  float4 Color2 : COLOR2;
  float4 Color3 : COLOR3;
};

/**
 * Computes a new texture coord based on the given height map.
 * The given eye and normal vector are assumed to be normalized
 * and in tangent space.
 */
float2 ComputeParallaxOcclusion(sampler2D texHeight,
                                float2 texCoord, 
                                float3 eyeVec,
                                float3 normal, 
                                float2 sampleLimits, 
                                float heightMapScale)
{
#ifndef PROFILE_GP4FP
    // Currently gp4fp is the only profile that supports the ddx/ddy stuff
    return texCoord;
#else
    float2 dx = ddx (texCoord);
    float2 dy = ddy (texCoord);

    float rayheight = 1.0;
    float lheight = 1.0;

    float2 offset = float2 (0, 0);
    float2 loffset = float2 (0, 0);
    float2 offsetValue = -normalize (eyeVec.xy) * heightMapScale * length (eyeVec.xy) / eyeVec.z;

    int sampleSize = (int)lerp (sampleLimits.y, sampleLimits.x, dot (eyeVec, normal));
    float stepSize = 1.0 / sampleSize;

    float2 stepOffset = stepSize * offsetValue;

    for(int sample = 0; sample &lt; sampleSize; sample++)
    {
      float height = tex2D (texHeight, texCoord + offset, dx, dy).x;

      if(height > rayheight)
      {
        float f = (height - rayheight) / ((height + stepSize) - lheight);
        offset = lerp (offset, loffset, f);
        break;
      }

      rayheight -= stepSize;
      loffset = offset;
      offset += stepOffset;
      lheight = height;
    }
    
    return texCoord + offset;
#endif
}

fragmentOutput main(vertex2fragment IN,
                    uniform float3 FlatColor,
                    uniform float3 ConstSpecular,
                    uniform float3 DynamicAmbient,
                    uniform float3 CombinedAmbient,
                    uniform float FarClipDistance,
                    uniform sampler2D TexDiffuse,
                    uniform sampler2D TexNormal,
                    uniform sampler2D TexNormalCompressed,
                    uniform sampler2D TexSpecular,
                    uniform sampler2D TexHeight,
                    uniform sampler2D TexLightMap, 
                    uniform sampler2D TexLightMapDir1,
                    uniform sampler2D TexLightMapDir2,
                    uniform sampler2D TexLightMapDir3)
{ 
  <?if vars."tex normal".texture || (vars."tex height".texture && vars."tex diffuse".texture) ?>
    float3 N = normalize (IN.ViewNormal);
    float3 T = normalize (IN.ViewTangent);
    float3 B = normalize (IN.ViewBiNormal);

    float3x3 view2tan = float3x3 (T, B, N);
    float3x3 tan2view = transpose (float3x3 (T, B, N));
  <?endif?>

  // Computes a new texture coordinate using parallax mapping.
  <?if vars."tex height".texture && vars."tex diffuse".texture ?>
    // Makes sure the normal and eye are normalized and in tangent space.
    float3 eyeVec = normalize (mul (view2tan, IN.ViewPosition));
    float3 Ntan = normalize (mul (view2tan, N));

    float2 TexCoord = ComputeParallaxOcclusion (TexHeight,
                                                IN.TexCoord,
                                                eyeVec,
                                                Ntan,
                                                float2 (2, 6),
                                                0.02);
  <?else?>
    float2 TexCoord = IN.TexCoord;
  <?endif?>
  
  <?if vars."tex diffuse".texture ?>
    float3 diffuse = tex2D (TexDiffuse, TexCoord).rgb;
  <?else?>
    float3 diffuse = FlatColor;
  <?endif?>

  float3 specular = tex2D (TexSpecular, TexCoord).rgb;
  specular = specular * ConstSpecular;

  <?if vars."tex normal compressed".texture ?>
    float3 normal = float3 (tex2D (TexNormalCompressed, texCoord).wy * 2 - 1, 0);
    normal.z = sqrt (1 - dot (normal.xy, normal.xy));
    normal = mul (tan2view, normal);
  <?elsif vars."tex normal".texture ?>
    float3 normal = tex2D (TexNormal, TexCoord).rgb * 2 - 1;
    normal = mul (tan2view, normal);
  <?else?>
    float3 normal = IN.ViewNormal;
  <?endif?>
  
  <?if vars."tex lightmap dir 1".texture ?>
    float3 baseLit[3];
    baseLit[0] = tex2D (TexLightMapDir1, IN.TexCoordLM).rgb;
    baseLit[1] = tex2D (TexLightMapDir2, IN.TexCoordLM).rgb;
    baseLit[2] = tex2D (TexLightMapDir3, IN.TexCoordLM).rgb;
    float3 ambient = DynamicAmbient + 2 * ComputeDLMColor (normal, baseLit);
  <?elsif vars."tex lightmap".texture ?>
    float3 ambient = DynamicAmbient + tex2D (TexLightMap, IN.TexCoordLM).rgb;
  <?else?>
    float3 ambient = DynamicAmbient;
  <?endif?>

  // Normalize and map into the range [0,1].
  normal = (normalize (normal) + 1) * 0.5;
  float3 interpVertexNormal = (normalize (IN.ViewNormal) + 1) * 0.5;  
  
  //FarClipDistance = 29.8f;
  float linearDepth = IN.ViewPosition.z / FarClipDistance;
  //float linearDepth = length (IN.ViewPosition);

  fragmentOutput output;

  output.Color0 = float4 (diffuse, specular.r);
  output.Color1 = float4 (normal, specular.g);
  output.Color2 = float4 (ambient, specular.b);
  output.Color3 = float4 (interpVertexNormal, linearDepth);

  return output;
}

</program>
</include>
