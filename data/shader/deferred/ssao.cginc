<include>
<variablemap variable="projection transform inverse" destination="ProjInv" />
<variablemap variable="viewport size" destination="viewportSize" />
<variablemap variable="far clip distance" destination="farClipDistance" />
<variablemap variable="sample radius" destination="sampleRadius" />
<variablemap variable="detail sample radius" destination="detailSampleRadius" />
<variablemap variable="num passes" destination="numPasses" />
<variablemap variable="self occlusion" destination="selfOcclusion" />
<variablemap variable="occlusion strength" destination="occlusionStrength" />
<variablemap variable="max occluder distance" destination="maxOccluderDistance" />
<variablemap variable="bounce strength" destination="indirectLightStrength" />
<variablemap variable="occluder angle bias" destination="occluderAngleBias" />
<variablemap variable="enable ambient occlusion" destination="enableAO" />
<variablemap variable="enable indirect light" destination="enableIndirectLight" />

<program>
<?Include /shader/deferred/light_common.cginc?>
<![CDATA[

uniform float4x4 Projection : state.matrix.projection;
uniform float4x4 ModelView : state.matrix.modelview;
uniform float4x4 ProjInv;
uniform sampler2D DiffuseBuffer;
uniform sampler2D NormalBuffer;
uniform sampler2D AmbientBuffer;
uniform sampler2D DepthBuffer;
uniform sampler2D DirectRadianceBuffer;
uniform sampler2D SeedTexture;
uniform sampler2D EnvmapTexture;
uniform sampler2D RandNormalsTexture;
uniform float4 viewportSize;  // width, height, 1/width, 1/height
uniform float farClipDistance;
uniform float sampleRadius;
uniform float detailSampleRadius;
uniform int numPasses;
uniform float selfOcclusion;
uniform float occlusionStrength;
uniform float maxOccluderDistance;
uniform float indirectLightStrength;
uniform float occluderAngleBias;
uniform float enableAO;
uniform float enableIndirectLight;
uniform float enableGlobalAO;

const float TWO_PI         = 6.283185f;
const float ONE_OVER_PI    = 0.318310f;
const float ONE_OVER_TWOPI = 0.159155f;

struct vertex2fragment 
{
  float2 TexCoord : TEXCOORD0;
  float4 ScreenPos : TEXCOORD1;
  float4 ViewPos : TEXCOORD2;
};

void GetSamplePositionAndNormal(float2 texCoord, float3 ray, float3 scale, out float2 sampleTC,
                                out float3 sampleScreenPos, out float3 sampleNormal)
{
  ray *= scale;
  sampleTC = texCoord + ray.xy;
  float4 sampleNormalDepth = tex2D (NormalBuffer, sampleTC);
  float2 sampleXY = (sampleTC - 0.5) * 2.0;
  float sampleDepth = sampleNormalDepth.a * farClipDistance;
  
  sampleScreenPos = float3 (sampleXY.x, -sampleXY.y, sampleDepth);
  sampleNormal = sampleNormalDepth.rgb * 2.0 - 1.0;
}

float ComputeOcclusion(float3 vecToOccluder, float3 normal, float3 sampleNormal)
{  
  float distance = length (vecToOccluder);
  float deltaN = (1.0 - max (0.0, dot (normal, sampleNormal))) *
      max (0.0 - selfOcclusion, dot (normal, normalize (vecToOccluder)) - occluderAngleBias);
  return /*step (0.0, distance) * step (distance, maxOccluderDistance) **/  deltaN * occlusionStrength 
      // / (1.0 + distance);
      * (1.0f - smoothstep (0.01, maxOccluderDistance, distance));
  
  /*float fRangeIsInvalid = saturate((screenPos.z - sampleDepth) / scale.z);			
  // accumulate accessibility, use default value of 0.5 if right computations are not possible
  return lerp (sampleDepth > (screenPos.z + ray.z), 0.1, fRangeIsInvalid) * deltaN * 
      occlusionStrength * step (fRangeIsInvalid, maxOccluderDistance);*/
}

float3 ComputeIndirectRadiance(float2 sampleTC, float3 vecToOccluder, float3 normal, float3 sampleNormal)
{
  float occluderDist = max (1.0, length (vecToOccluder));
  vecToOccluder = normalize (vecToOccluder);  
  float cosRi = max (0.0, dot (normal, vecToOccluder));
  float cosSi = max (0.0, dot (sampleNormal, -vecToOccluder));
  float occluderGeometricTerm = cosSi * cosRi / (occluderDist * occluderDist);
  float3 occluderRadiance = tex2D (DirectRadianceBuffer, sampleTC).rgb;
  return indirectLightStrength * occluderRadiance * occluderGeometricTerm;
}

float4 main(vertex2fragment IN) : COLOR
{
  const float3 samples[8] =
	{
		normalize (float3 ( 1, 1, 1)), //* 0.5625f, //0.125f,
		normalize (float3 (-1,-1,-1)), //* 0.6250f, //0.250f,
		normalize (float3 (-1,-1, 1)), //* 0.6875f, //0.375f,
		normalize (float3 (-1, 1,-1)), //* 0.7500f, //0.500f,
		normalize (float3 (-1, 1 ,1)), //* 0.8125f, //0.625f,
		normalize (float3 ( 1,-1,-1)), //* 0.8750f, //0.750f,
		normalize (float3 ( 1,-1, 1)), //* 0.9375f, //0.875f,
		normalize (float3 ( 1, 1,-1)) //* 0.9999f //0.999f
	};	
  
  float2 screenXY = IN.ScreenPos.xy / IN.ScreenPos.w;
  float2 texCoord = screenXY * 0.5 + 0.5;
  
  float4 normalDepth = tex2D (NormalBuffer, texCoord);
  float depth = normalDepth.a * farClipDistance;
  float3 screenPos = float3 (screenXY.x, -screenXY.y, depth);
  float3 normal = normalDepth.rgb * 2.0 - 1.0;
  //float3 randomNormal = tex2D (RandNormalsTexture, texCoord * viewportSize.xy / 16).rgb * 2.0 - 1.0;
  //randomNormal = normalize (randomNormal);
  
  float invDepth = 1.0 / depth;
  float3 scale[2];
  scale[0] = float3 (sampleRadius * invDepth, sampleRadius * invDepth, sampleRadius / farClipDistance);
]]>
<?if (vars."enable ambient occlusion".float == 1) && (vars."detail sample radius".float != 0) ?>
  scale[1] = float3 (detailSampleRadius * invDepth, detailSampleRadius * invDepth, detailSampleRadius / farClipDistance);
<?else?>
  scale[1] = scale[0];
<?endif?>
<![CDATA[
  float AO = 0.0;
  float AOsum = 0.0;
  float3 indirectRadiance = float3(0.0);
  float totalSamples = 8.0 * numPasses;
  float sampleStep = 0.5 / totalSamples;
  float sampleLength = 0.5;
  //float3 bentNormal = float3(0.0);
  
  for (int n=0; n < numPasses; n++)
  {
    float3 randomNormal = tex2D (RandNormalsTexture, texCoord * ((viewportSize.xy / 16) + float2(n))).rgb * 2.0 - 1.0;
    randomNormal = normalize (randomNormal);
    for (int i=0; i < 8; i++)
    {
      float3 ray = reflect (samples[i] * sampleLength, randomNormal);
      if (dot (ray, normal) < 0.0)
        ray = -ray;
      
      sampleLength += sampleStep;
      float3 sampleScreenPos, sampleNormal;
      float2 sampleTC;
      GetSamplePositionAndNormal (texCoord, ray, scale[i % 2], sampleTC, sampleScreenPos, sampleNormal);
      float3 vecToOccluder = sampleScreenPos - screenPos;
]]>
<?if vars."enable ambient occlusion".float == 1 ?>
      AO = ComputeOcclusion (vecToOccluder, normal, sampleNormal);
<?endif?>
<?if vars."enable indirect light".float == 1 ?>
      indirectRadiance += ComputeIndirectRadiance (sampleTC, vecToOccluder, normal, sampleNormal);
<?endif?>
<![CDATA[
      //bentNormal += vecToOccluder * AO;
      AOsum += AO;
    }
  }
    
  AOsum /= totalSamples;
  AOsum += selfOcclusion;
  indirectRadiance /= totalSamples;
  
  /*bentNormal = normalize (bentNormal);
  // convert bent normal to spherical coords
  float theta = acos (bentNormal.y);
  float phi = atan2 (bentNormal.z, bentNormal.x);
  //phi += lightRotationAngle;
  if (phi < 0) phi += TWO_PI; //TODO: replace if for step function?
  if (phi > TWO_PI) phi -= TWO_PI;
  float3 envRadiance = tex2D (EnvmapTexture, float2 (phi * ONE_OVER_TWOPI, 1.0 - theta * ONE_OVER_PI)).rgb;
  indirectRadiance += envRadiance * max (0.0, dot (bentNormal, normal));*/
  
  return float4 (indirectRadiance, 1.0 - saturate (AOsum));
}

]]>
</program>
</include>