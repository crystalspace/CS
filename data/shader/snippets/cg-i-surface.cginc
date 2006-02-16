<include>
#ifndef __CG_I_SURFACE_CGINC__
#define __CG_I_SURFACE_CGINC__

/* An interface for common surface properties.
 */
interface iSurface
{
  float4 GetDiffuse ();
  float3 GetNormal ();
  float3 GetSpecularColor ();
  float GetSpecularExponent ();
  float3 GetEmissive ();
  float2 GetTexCoordOffset ();
};

#endif // __CG_I_SURFACE_CGINC__
</include>
