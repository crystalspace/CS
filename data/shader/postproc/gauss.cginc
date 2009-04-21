<include>
  <![CDATA[
  float4 main (in float2 TexCoord : TEXCOORD0,
	    uniform sampler2D Texture1,
	    uniform float2 pixelSize, uniform float2 direction,
	    uniform float radius) : COLOR
  {
    const float PI = 3.141592654;
  
    float sigma = radius/3;
    int rPixels = ceil (radius);
    float coeff = 1/(sqrt(2*PI)*sigma);
    float coeffSum = coeff;
    float4 blurred = tex2D (Texture1, TexCoord) * coeff;
    for (int r = 1; r <= rPixels; r++)
    {
      coeff = 1/(sqrt(2*PI)*sigma)*exp(-(r*r)/(2*sigma*sigma));
      float2 offset = r*pixelSize*direction;
      blurred += (tex2D (Texture1, TexCoord + offset)
	+ tex2D (Texture1, TexCoord - offset)) * coeff;
      coeffSum += coeff*2;
    }
    return blurred / coeffSum;
  }

  ]]>
</include>
