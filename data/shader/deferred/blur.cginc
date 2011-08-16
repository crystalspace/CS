<include>
<![CDATA[

uniform sampler2D NormalBuffer;
uniform sampler2D DiffuseBuffer;
uniform sampler2D AmbientBuffer;            
uniform sampler2D DirectRadianceBuffer;
uniform sampler2D GlobalIllumBuffer;
uniform float FarClipDistance;
uniform float4 ViewportSize; // (width, height, 1/width, 1/height)
uniform int KernelSize;
uniform float2 Direction; // X:(1,0) - Y:(0,1)
uniform float Combine;
uniform float PositionThreshold;
uniform float NormalThreshold;
uniform int ShowAO;
uniform int ShowGlobalIllum;

const float TWO_PI = 6.28318f;
const float EPSILON = 0.01f;
]]>
// Cg profiles for older hardware don't support data-dependent loops
<? if vars."ssao blur kernelsize".int <= 1 ?>
const int KERNEL_SIZE = 1;
<? elsif vars."ssao blur kernelsize".int == 2 ?>
const int KERNEL_SIZE = 2;
<? elsif vars."ssao blur kernelsize".int == 3 ?>
const int KERNEL_SIZE = 3;
<? elsif vars."ssao blur kernelsize".int == 4 ?>
const int KERNEL_SIZE = 4;
<? elsif vars."ssao blur kernelsize".int == 5 ?>
const int KERNEL_SIZE = 5;
<? elsif vars."ssao blur kernelsize".int == 6 ?>
const int KERNEL_SIZE = 6;
<? else ?>
const int KERNEL_SIZE = 7;
<? endif ?>

<![CDATA[
float weightedAO = 0.0f;
float AO = 0.0f;
float4 weightedRadiance = float4(0.0f);
float4 radiance = float4(0.0f);

float GaussianCoeff(float sampleDist, float sqrSigma)
{  
  float g = 1.0f / sqrt (TWO_PI * sqrSigma);
  return g * exp (-sampleDist * sampleDist / (2.0f * sqrSigma));
}

float WeightSample(float sampleDist, float2 sampleTexCoord,
                   float pixelDepth, float3 pixelNormalVS, float sqrGaussianSigma)
{  
  float4 sampleDepthNormalVS = tex2D (NormalBuffer, sampleTexCoord);
  float3 sampleNormalVS = sampleDepthNormalVS.rgb * 2.0f - 1.0f;
  float sampleDepth = sampleDepthNormalVS.a;
      
  float deltaZ = abs (sampleDepth - pixelDepth);
  float dotN = dot (pixelNormalVS, sampleNormalVS);       
  float4 sampleGI = tex2D (GlobalIllumBuffer, sampleTexCoord);
  
  float totalWeight = 0.0f;
  if (deltaZ < PositionThreshold && dotN > 1.0f - NormalThreshold)
  {
    totalWeight = GaussianCoeff (sampleDist, sqrGaussianSigma) * pow (dotN, 32.0f) / 
        (EPSILON + deltaZ);
    //totalWeight = exp (-i * i * NormalThreshold - deltaZ * deltaZ * PositionThreshold * 1024.0f);
    
    //weightedAO += sampleGI.a * totalWeight;
    weightedRadiance += sampleGI * totalWeight;    
  }
  
  //AO += sampleGI.a;
  radiance += sampleGI;
  return totalWeight;
}

float4 main(in float2 texCoord : TEXCOORD0) : COLOR
{  
  float4 normalDepthVS = tex2D (NormalBuffer, texCoord);
  float3 pixelNormalVS = normalDepthVS.rgb * 2.0f - 1.0f;
  float pixelDepth = normalDepthVS.a;
  
  float weightSum = 0.0f;
  float sqrGaussianSigma = KERNEL_SIZE * 0.3333f;
  sqrGaussianSigma *= sqrGaussianSigma;
  ViewportSize.zw *= 3.0f * Direction; // strided blur
  
  weightSum += WeightSample (0, texCoord, pixelDepth, pixelNormalVS, sqrGaussianSigma); 
  
	for (int i=1; i <= KERNEL_SIZE; i++)
	{
    float2 tcOffset = float2 (i * ViewportSize.z, i * ViewportSize.w);
	      
    weightSum += WeightSample (i, texCoord + tcOffset, pixelDepth, pixelNormalVS, sqrGaussianSigma);    
    weightSum += WeightSample (i, texCoord - tcOffset, pixelDepth, pixelNormalVS, sqrGaussianSigma);
	}
  
  if (weightSum > 0.0f)
  {
    //weightSum = 1.0f / weightSum;
    //AO = weightedAO * weightSum;
    radiance = weightedRadiance / weightSum;
  }
  else
  {
    //weightSum = 1.0f / (2.0f * KERNEL_SIZE + 1.0f);
    //AO *= weightSum;
    //radiance *= weightSum;
    radiance /= 2.0f * KERNEL_SIZE + 1.0f;
  }
  
  return radiance;
  //return float4 (radiance, AO);
}

]]>

</include>
