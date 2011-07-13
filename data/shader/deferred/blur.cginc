<include>
<![CDATA[

//const float EPSILON 0.01f;
float weightedAO = 0.0f;
float AO = 0.0f;
float3 weightedRadiance = float3(0.0f);
float3 radiance = float3(0.0f);

float GaussianCoeff(float sampleDist, float sqrSigma)
{  
  float g = 1.0f / sqrt (2.0f * 3.14159f * sqrSigma);
  return g * exp (-sampleDist * sampleDist / (2.0f * sqrSigma));
}

float WeightSample(float sampleDist, float2 sampleTexCoord, sampler2D NormalBuffer, sampler2D GlobalIllumBuffer,
                   float pixelDepth, float3 pixelNormalVS, 
                   float PositionThreshold, float NormalThreshold, float sqrGaussianSigma)
{  
  float4 sampleDepthNormalVS = tex2D (NormalBuffer, sampleTexCoord);
  float3 sampleNormalVS = sampleDepthNormalVS.rgb * 2.0f - 1.0f;
  float sampleDepth = sampleDepthNormalVS.a;
      
  float deltaZ = /*abs*/ (sampleDepth - pixelDepth);
  float dotN = dot (pixelNormalVS, sampleNormalVS);       
  float4 sampleGI = tex2D (GlobalIllumBuffer, sampleTexCoord);
  
  float totalWeight = 0.0f;
  if (deltaZ < PositionThreshold && dotN > 1.0f - NormalThreshold)
  {      
    totalWeight = GaussianCoeff (sampleDist, sqrGaussianSigma); // * pow (dotN, 32.0f) / 
        //(EPSILON + deltaZ);
    //totalWeight = exp (-i * i * NormalThreshold - deltaZ * deltaZ * PositionThreshold * 1024.0f);
    
    weightedAO += sampleGI.a * totalWeight;
    weightedRadiance += sampleGI.rgb * totalWeight;    
  }
  
  AO += sampleGI.a;
  radiance += sampleGI.rgb;
  return totalWeight;
}

float4 main(in float4 ScreenPos : TEXCOORD0,
            //uniform float4x4 ProjInv, //: state.matrix.inverse.projection,
            uniform sampler2D NormalBuffer,
            uniform sampler2D DiffuseBuffer,
            uniform sampler2D AmbientBuffer,            
            //uniform sampler2D DepthBuffer,
            uniform sampler2D DirectRadianceBuffer,
            uniform sampler2D GlobalIllumBuffer,
            uniform float FarClipDistance,
            uniform float2 InvViewportSize, // (1/viewport.width, 1/viewport.height)
            uniform int KernelSize,
            uniform float2 Direction, // X:(1,0) - Y:(0,1)
            uniform float Combine,
            uniform float PositionThreshold,
            uniform float NormalThreshold,
            uniform int ShowAO,
            uniform int ShowGlobalIllum) : COLOR
{
  float2 texCoord = (ScreenPos.xy / ScreenPos.w) * 0.5f + 0.5f;
  float4 normalDepthVS = tex2D (NormalBuffer, texCoord);
  float3 pixelNormalVS = normalDepthVS.rgb * 2.0f - 1.0f;
  float pixelDepth = normalDepthVS.a;  
  
  float weightSum = 0.0f;
  float sqrGaussianSigma = KernelSize * 0.3333f;
  sqrGaussianSigma *= sqrGaussianSigma;
  InvViewportSize *= 2.0f * Direction; // strided blur
  
  weightSum += WeightSample (0, texCoord, NormalBuffer, GlobalIllumBuffer,
      pixelDepth, pixelNormalVS, PositionThreshold, NormalThreshold, sqrGaussianSigma); 
  
	for (int i=1; i <= KernelSize; i++)
	{
    float2 tcOffset = float2 (i * InvViewportSize.x, i * InvViewportSize.y);
	      
    weightSum += WeightSample (i, texCoord + tcOffset, NormalBuffer, GlobalIllumBuffer,
        pixelDepth, pixelNormalVS, PositionThreshold, NormalThreshold, sqrGaussianSigma);
    
    weightSum += WeightSample (i, texCoord - tcOffset, NormalBuffer, GlobalIllumBuffer,
        pixelDepth, pixelNormalVS, PositionThreshold, NormalThreshold, sqrGaussianSigma); 
	}  
  
  if (weightSum > 0.0f)
  {
    weightSum = 1.0f / weightSum;
    AO = weightedAO * weightSum;
    radiance = weightedRadiance * weightSum;
  }
  else
  {
    weightSum = 1.0f / (2.0f * KernelSize + 1.0f);
    AO *= weightSum;
    radiance *= weightSum;
  }

  return float4 (radiance, AO);
}

]]>

</include>
