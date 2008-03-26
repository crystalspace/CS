<!--
  Copyright (C) 2008 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
-->
<include><![CDATA[

#ifndef __LIGHTFUNCS_CG_INC__
#define __LIGHTFUNCS_CG_INC__

#define MAX_LIGHTS 8

float Attenuation_Linear (float d, float invLightRadius)
{
  return max (1 - d * invLightRadius, 0);
}

float Attenuation_Inverse (float d)
{
  return 1/d;
}

float Attenuation_Realistic (float d)
{
  return 1/(d*d);
}

float Attenuation_CLQ (float d, float3 coeff)
{
  return 1/(coeff.x + d*coeff.y + d*d*coeff.z);
}

float Light_Spot (float3 surfNorm, float3 surfToLight, 
                  float3 lightDir, float falloffInner, float falloffOuter)
{
  float a = smoothstep (falloffOuter, falloffInner, -dot (surfToLight, lightDir));
  return max (a, 0);
}


struct LightProperties
{
  // Number of lights provided
  int count;
  // Light world-space position
  float4 posWorld[MAX_LIGHTS];
  // Transformation from light space to world space
  float4x4 transform[MAX_LIGHTS];
  // Transformation from world space to light space
  float4x4 transformInv[MAX_LIGHTS];
  // Diffuse color
  float3 colorDiffuse[MAX_LIGHTS];
  // Specular color
  float3 colorSpecular[MAX_LIGHTS];
  // Attenuation vector (XYZ are CLQ coefficients; W is light radius)
  float4 attenuationVec[MAX_LIGHTS];
  // Cosine of inner falloff angle
  float falloffInner[MAX_LIGHTS];
  // Cosine of outerr falloff angle
  float falloffOuter[MAX_LIGHTS];
};
LightProperties lightProps;

interface LightSpace
{
  float4 GetPosition();
  float3 GetDirection();
  float3 GetSurfaceToLight();
  float GetLightDistance ();
};

struct LightSpaceWorld : LightSpace
{
  float4 pos;
  float3 dir;
  float3 surfToLight;
  float lightDist;

  void Init (int lightNum, float4 surfPositionWorld)
  {
    pos = lightProps.posWorld[lightNum];
    float4x4 lightTransformInv = lightProps.transformInv[lightNum];
    dir = lightTransformInv[2].xyz;
    surfToLight = (lightProps.posWorld[lightNum] - surfPositionWorld).xyz;
    lightDist = length (surfToLight);
    surfToLight = normalize (surfToLight);
  }
  float4 GetPosition() { return pos; }
  float3 GetDirection() { return dir; }
  float3 GetSurfaceToLight() { return surfToLight; }
  float GetLightDistance () {return lightDist; }
};

interface Shadow
{
  float GetVisibility();
};

struct ShadowNone : Shadow
{
  float GetVisibility() { return 1; }
};

#if 0
  // @@@ TODO: Translate to interface implementation
  <?Template Lighting_Shadow_Shadowmap PROG?>
    <?if vars."light shadow map" ?>
    // Transform fragment position into light space
    float4 view_pos = mul($PROG$In.lightTransformInv[i], position);
    // Transform fragment position in light space into "shadow map space"
    float4 shadowMapCoords;
    shadowMapCoords = mul ($PROG$In.lightShadowMapProject[i], view_pos);
    // Project SM coordinates
    shadowMapCoords.xyz /= shadowMapCoords.w;
    shadowMapCoords.xyz = (0.5*shadowMapCoords.xyz) + float3(0.5);
    float4 shadowVal = tex2D ($PROG$In.lightShadowMap[i], shadowMapCoords.xy);
    
    // Depth to compare against
    float compareDepth = shadowMapCoords.z + (1.0/32768.0);
    // Depth from the shadow map
    float shadowMapDepth = shadowVal.r;
    // Shadow value
    float inLight = compareDepth > shadowMapDepth;
    attn *= inLight;
    <?endif?>
  <?Endtemplate?>
#endif

interface Light
{
  float3 GetIncidence();
  float GetAttenuation();
};

struct LightDirectional : Light
{
  float3 dir;
  
  void Init (LightSpace space)
  {
    dir = -space.GetDirection();
  }
  float3 GetIncidence() { return dir; }
  float GetAttenuation() { return 1; }
};

struct LightPoint : Light
{
  float3 dir;
  
  void Init (LightSpace space)
  {
    dir = space.GetSurfaceToLight();
  }
  float3 GetIncidence() { return dir; }
  float GetAttenuation() { return 1; }
};

struct LightSpot : Light
{
  float3 dir;
  float spot;
  
  void Init (LightSpace space, float3 normal, float falloffInner, float falloffOuter)
  {
    dir = space.GetDirection();
    spot = Light_Spot (normal, space.GetSurfaceToLight(),
      dir, falloffInner, falloffOuter);
  }
  float3 GetIncidence() { return dir; }
  float GetAttenuation() { return spot; }
};
]]>

Light GetCurrentLight (LightSpace lightSpace, int lightNum, float3 surfNormal)
{
<?if vars."light type".int == consts.CS_LIGHT_DIRECTIONAL ?>
  LightDirectional ld;
  ld.Init (lightSpace);
  return ld;
<?elsif vars."light type".int == consts.CS_LIGHT_SPOTLIGHT ?>
  LightSpot ls;
  ls.Init (lightSpace, surfNormal, 
    lightProps.falloffInner[lightNum],
    lightProps.falloffOuter[lightNum]);
  return ls;
<?else?>
<?! Assume point light ?>
  LightPoint lp;
  lp.Init (lightSpace);
  return lp;
<?endif?>
}

<![CDATA[
void ComputeLight (LightSpace lightSpace, Light light, 
                   float3 eyeToSurf, float3 surfNormal,
                   float surfShininess, 
                   float3 lightDiffuse, float3 lightSpecular,
                   float4 lightAttenuationVec,
                   Shadow shadow, out float3 d, out float3 s)
{
  float3 L = light.GetIncidence();
  float3 H = normalize (lightSpace.GetSurfaceToLight() - normalize (eyeToSurf));
  float spot = light.GetAttenuation();
  
  float4 lightCoeff = lit (dot (surfNormal, L), dot (surfNormal, H),
    surfShininess);
  
  float lightDist = lightSpace.GetLightDistance();
  float attn;
  float attnRadius = lightAttenuationVec.w;
  if (attnRadius > 0)
    attn = Attenuation_Linear (lightDist, 1 / attnRadius);
  else
    attn = Attenuation_CLQ (lightDist, lightAttenuationVec.xyz);
  
  attn *= shadow.GetVisibility();

  d = lightDiffuse * lightCoeff.y * spot * attn;
  s = lightSpecular * lightCoeff.z * spot * attn;
}

#endif // __LIGHTFUNCS_CG_INC__
 
]]>
</include>
