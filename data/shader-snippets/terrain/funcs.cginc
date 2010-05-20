<include>
<![CDATA[

#ifndef __TERRAINFUNCS_CG_INC__
#define __TERRAINFUNCS_CG_INC__

float HeightWeight(float4 attribute, float height)
{
  float minh = attribute.x;
  float maxh = attribute.y;

  float range = maxh - minh;

  return max(0.0f, ((range - abs( height - maxh )) / range));
}

float SlopeWeight(float4 attribute, float angle)
{
  float minh = attribute.z;
  float maxh = attribute.w;

  float range = maxh - minh;

  return max(0.0f, ((range - abs( angle - maxh )) / range));
}


#endif // __LIGHTFUNCS_CG_INC__
 
]]>
</include>
