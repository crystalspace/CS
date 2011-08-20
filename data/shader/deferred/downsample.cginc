<include>
<![CDATA[

uniform sampler2D NormalDepthBuffer;
uniform float4 ViewportSize; // (width, height, 1/width, 1/height)
uniform float DepthThreshold;

float4 main(in float2 texCoord : TEXCOORD0) : COLOR
{  
#ifndef PROFILE_GP4FP
  //According to Nvidia documentation: "Relative indexing of uniform arrays is not supported"
  //in fragment profiles "below" GP4FP
  return tex2D (NormalDepthBuffer, texCoord);
#else
  // Taken from "Multi-resolution screen-space ambient occlusion" paper
  // http://www.comp.nus.edu.sg/~duong/
  DepthThreshold = 0.1f;

  float4 nz[4];
  nz[0] = tex2D (NormalDepthBuffer, texCoord + ViewportSize.zw * float2 (-1.0f, 1.0f));
  nz[1] = tex2D (NormalDepthBuffer, texCoord + ViewportSize.zw * float2 (1.0f, 1.0f));
  nz[2] = tex2D (NormalDepthBuffer, texCoord + ViewportSize.zw * float2 (1.0f, -1.0f));
  nz[3] = tex2D (NormalDepthBuffer, texCoord + ViewportSize.zw * float2 (-1.0f, -1.0f));
  
  float maxZ = max (max (nz[0].w, nz[1].w), max (nz[2].w, nz[3].w));
  float minZ = min (min (nz[0].w, nz[1].w), min (nz[2].w, nz[3].w));

  int minPos = 0, maxPos = 0;
  for (int i=0; i < 4; ++i)
  {
    if (nz[i].w == minZ)
      minPos = i;
    if (nz[i].w == maxZ)
      maxPos = i;
  }

  float zDiff = nz[maxPos].w - nz[minPos].w;
  int median[2] = {0, 0};
  int index = 0;
  for (int i=0; i < 4 && index < 2; ++i)
  {
    if (i != minPos && i != maxPos)
      median[index++] = i;
  }
  
  float4 normalAndDepth;
  if (zDiff < DepthThreshold)
  {
    normalAndDepth.xyz = (nz[median[0]].xyz + nz[median[1]].xyz) / 2.0f;
    normalAndDepth.w = (nz[median[0]].w + nz[median[1]].w) / 2.0f;
  }
  else
  {
    normalAndDepth = nz[median[0]];
  }
  
  return normalAndDepth;
#endif
}

]]>
</include>
