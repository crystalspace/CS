/*
  Copyright (C) 2005 by Marten Svanfeldt

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSGFX_VERTEXLIGHT_H__
#define __CS_CSGFX_VERTEXLIGHT_H__

#include "csqsqrt.h"
#include "csgeom/math.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csgfx/lightsvcache.h"
#include "csgfx/vertexlistwalker.h"
#include "csutil/cscolor.h"
#include "cstool/rbuflock.h"

#include "iengine/light.h"
#include "iengine/movable.h"
#include "ivideo/shader/shader.h"

/**\file 
 * Attenuation functors
 */

/**
 * Light properties, as needed by the attenuation and lighting functors.
 */
struct csLightProperties
{
protected:
  csShaderVariable* LookupSVArrayItem (const csShaderVariableStack& stack,
    CS::ShaderVarStringID id, size_t index)
  {
    csShaderVariable* sv;
    if ((stack.GetSize () > id) && ((sv = stack[id]) != 0))
      return sv->GetArrayElement (index);
    return 0;
  }
  csShaderVariable* LookupSV (const csShaderVariableStack& stack,
    CS::ShaderVarStringID id)
  {
    csShaderVariable* sv = 0;
    if (stack.GetSize () > id)
      sv = stack[id];
    return sv;
  }
public:
  /// Attenuation coefficients (for CLQ attenuation)
  csVector3 attenuationConsts;
  /// Light position (object space)
  csVector3 posObject;
  /**
   * Light direction (object space).
   * \remark Should be a unit vector.
   */
  csVector3 dirObject;
  /// Light diffuse color
  csColor color;
  /// Spotlight inner falloff
  float spotFalloffInner;
  /// Spotlight outer falloff
  float spotFalloffOuter;
  /// Light type
  csLightType type;
  /// Light attenuation mode
  csLightAttenuationMode attenuationMode;
  /// Specular color
  csColor specular;

  csLightProperties () : spotFalloffInner(0.0f), spotFalloffOuter(0.0f),
    type(CS_LIGHT_POINTLIGHT) {}
  /**
   * Convenience constructor to fill the structure from a set of shader
   * variables.
   */
  csLightProperties (size_t lightNum, csLightShaderVarCache& svcache,
    const csShaderVariableStack& stack,
    const csReversibleTransform& objectToWorld = csReversibleTransform ())
  {
    CS::ShaderVarStringID id;
    csShaderVariable* sv;
    csVector3 tmp;

    id = svcache.GetLightSVId (csLightShaderVarCache::lightAttenuation);
    if (((sv = LookupSVArrayItem (stack, id, lightNum)) != 0))
      sv->GetValue (attenuationConsts);

    id = svcache.GetLightSVId (csLightShaderVarCache::lightPositionWorld);
    if (((sv = LookupSVArrayItem (stack, id, lightNum)) != 0))
    {
      sv->GetValue (tmp);
      posObject = objectToWorld.Other2This (tmp);
    }

    id = svcache.GetLightSVId (csLightShaderVarCache::lightDirectionWorld);
    if (((sv = LookupSVArrayItem (stack, id, lightNum)) != 0))
    {
      sv->GetValue (tmp);
      dirObject = objectToWorld.Other2ThisRelative (tmp);
    }

    id = svcache.GetLightSVId (csLightShaderVarCache::lightDiffuse);
    if (((sv = LookupSVArrayItem (stack, id, lightNum)) != 0))
      sv->GetValue (color);

    id = svcache.GetLightSVId (csLightShaderVarCache::lightInnerFalloff);
    if (((sv = LookupSVArrayItem (stack, id, lightNum)) != 0))
      sv->GetValue (spotFalloffInner);

    id = svcache.GetLightSVId (csLightShaderVarCache::lightOuterFalloff);
    if (((sv = LookupSVArrayItem (stack, id, lightNum)) != 0))
      sv->GetValue (spotFalloffOuter);

    int t = CS_LIGHT_POINTLIGHT;
    id = svcache.GetLightSVId (csLightShaderVarCache::lightType);
    if (((sv = LookupSV (stack, id)) != 0))
      sv->GetValue (t);
    type = (csLightType)t;

    t = CS_ATTN_NONE;
    id = svcache.GetLightSVId (csLightShaderVarCache::lightAttenuationMode);
    if (((sv = LookupSVArrayItem (stack, id, lightNum)) != 0))
      sv->GetValue (t);
    attenuationMode = (csLightAttenuationMode)t;
  
    id = svcache.GetLightSVId (csLightShaderVarCache::lightSpecular);
    if (((sv = LookupSVArrayItem (stack, id, lightNum)) != 0))
      sv->GetValue (specular);
  }
};

/**
 * No attenuation. 
 */
struct csNoAttenuation
{
  csNoAttenuation (const csLightProperties& /*light*/)
  {}

  CS_FORCEINLINE_TEMPLATEMETHOD 
  void operator() (float /*distance*/, float & /*dp*/) const
  {}
};

/**
 * Linear attenuation.
 * Out = in * (1 - distance/radius)
 */
struct csLinearAttenuation
{
  csLinearAttenuation (const csLightProperties& light)
  {
    invrad = 1/light.attenuationConsts.x;
  }

  CS_FORCEINLINE_TEMPLATEMETHOD 
  void operator() (float distance, float& dp) const
  {
    dp = csMax (dp * (1 - distance * invrad), 0.0f);
  }

  float invrad;
};

/**
 * Inverse linear attenuation.
 * Out = in * / distance
 */
struct csInverseAttenuation
{
  csInverseAttenuation (const csLightProperties& /*light*/)
  {}

  CS_FORCEINLINE_TEMPLATEMETHOD
  void operator() (float distance, float& dp) const
  {
    dp = dp / distance;
  }
};


/**
 * Inverse quadratic attenuation.
 * Out = in * / distance^2
 */
struct csRealisticAttenuation
{
  csRealisticAttenuation (const csLightProperties& /*light*/)
  {}

  CS_FORCEINLINE_TEMPLATEMETHOD
  void operator() (float distance, float& dp) const
  {
    dp = dp / (distance*distance);
  }
};

/**
 * Constant, Linear, Quadratic attenuation
 * Out = in /(const + distance*lin + distance^2*quad)
 */
struct csCLQAttenuation
{
  csCLQAttenuation (const csLightProperties& light)
    : attnVec (light.attenuationConsts)
  {}

  CS_FORCEINLINE_TEMPLATEMETHOD
  void operator() (float distance, float& dp) const
  {
    dp = dp/(csVector3 (1.0, distance, distance*distance)*attnVec);
  }

  csVector3 attnVec;
};


/**
 * Preform pointlight lighting calculation without shadowing.
 * Template parameters:
 *   AttenuationProc - Functor for attenuation
 */
template<class AttenuationProc>
class csPointLightProc
{
public:
  csPointLightProc (const csLightProperties& light, float blackLimit = 0.0001f)
    : attn (light), blackLimit (blackLimit)
  {    
    lightPos = light.posObject;
  }
  class PerVertex
  {
    csVector3 direction;
    float invDistance;
    float a;
    float dp;
    bool vertexLit;
  public:
    CS_FORCEINLINE_TEMPLATEMETHOD
    PerVertex (const csPointLightProc& parent, const csVector3 &v,
      const csVector3 &n)
    {
      direction = parent.lightPos-v;
      float distance = csQsqrt (direction.SquaredNorm ());
      invDistance = 1.0f/distance;
      dp = (direction*n) * invDistance;
      if ((vertexLit = (dp > parent.blackLimit)))
      {
	a = 1.0f;
	parent.attn (distance, a);
      }
      else
	a = 0.0f;
    }
    bool IsLit() const { return vertexLit; }
    float Attenuation() const { return a; }
    float DiffuseAttenuated() const { return a*dp; }
    const csVector3& LightDirection() const { return direction; }
    const float LightInvDistance() const { return invDistance; }
  };
private:
  AttenuationProc attn;
  csVector3 lightPos; //localspace
  float blackLimit;
};

/**
 * Preform directional light lighting calculation without shadowing.
 * Template parameters:
 *   AttenuationProc - Functor for attenuation
 */
template<class AttenuationProc>
class csDirectionalLightProc
{
public:
  csDirectionalLightProc (const csLightProperties& light, 
                          float blackLimit = 0.0001f) : attn (light), 
                          blackLimit (blackLimit)
  {
    lightPos = light.posObject;
    lightDir = light.dirObject;
  }
  class PerVertex
  {
    csVector3 direction;
    float invDistance;
    float a;
    float dp;
    bool vertexLit;
  public:
    CS_FORCEINLINE_TEMPLATEMETHOD
    PerVertex (const csDirectionalLightProc& parent, const csVector3 &v,
      const csVector3 &n)
    {
      //compute gouraud shading..
      dp = -parent.lightDir*n;
      if ((vertexLit = (dp > parent.blackLimit)))
      {
	csVector3 direction = parent.lightPos-v;
	a = 1.0f;
        float distance = csQsqrt(direction.SquaredNorm ());
        invDistance = 1.0f/distance;
	parent.attn (distance, a);
      }
      else
      {
	direction = csVector3 (0);
	invDistance = 0.0f;
	a = 0.0f;
      }
    }
    bool IsLit() const { return vertexLit; }
    float Attenuation() const { return a; }
    float DiffuseAttenuated() const { return a*dp; }
    const csVector3& LightDirection() const { return direction; }
    const float LightInvDistance() const { return invDistance; }
  };
private:
  AttenuationProc attn;
  csVector3 lightPos; //localspace
  csVector3 lightDir; //localspace
  float blackLimit;
};

/**
 * Perform spotlight lighting calculation without shadowing.
 * Template parameters:
 *   AttenuationProc - Functor for attenuation
 */
template<class AttenuationProc>
class csSpotLightProc
{
public:
  csSpotLightProc (const csLightProperties& light, 
                   float blackLimit = 0.0001f) : attn (light), 
                   blackLimit (blackLimit)
  {
    lightPos = light.posObject;
    lightDir = light.dirObject;

    falloffInner = light.spotFalloffInner;
    falloffOuter = light.spotFalloffOuter;
  }

  class PerVertex
  {
    csVector3 direction;
    float invDistance;
    float a;
    float cosfact;
    bool vertexLit;
  public:
    CS_FORCEINLINE_TEMPLATEMETHOD
    PerVertex (const csSpotLightProc& parent, const csVector3 &v,
      const csVector3 &n)
    {
      //compute gouraud shading..
      direction = parent.lightPos-v;
      csVector3 dirUnit (direction.Unit ());
  
      //compute gouraud shading..
      float dp = dirUnit*n;
      if (dp > parent.blackLimit)
      {
	cosfact =
	  csSmoothStep (-(dirUnit*parent.lightDir), 
	    parent.falloffInner, parent.falloffOuter);
	if ((vertexLit = (cosfact > 0)))
	{
	  cosfact *= dp;
	  float distance = csQsqrt(direction.SquaredNorm ());
	  invDistance = 1.0f/distance;
	  a = 1.0f;
	  parent.attn (distance, a);
	}
	else
	{
	  invDistance = 0.0f;
	  a = 0.0f;
	}
      }
      else
      {
	invDistance = 0.0f;
	a = 0.0f;
	cosfact = 0.0f;
        vertexLit = false;
      }
    }
    bool IsLit() const { return vertexLit; }
    float Attenuation() const { return a; }
    float DiffuseAttenuated() const { return a*cosfact; }
    const csVector3& LightDirection() const { return direction; }
    const float LightInvDistance() const { return invDistance; }
  };
private:
  AttenuationProc attn;
  csVector3 lightPos; //localspace
  csVector3 lightDir; //localspace
  float blackLimit;
  float falloffInner, falloffOuter;
};

/**
 * Interface to calculate lighting for a number of vertices.
 */
struct iVertexLightCalculator
{
public:
  virtual ~iVertexLightCalculator() {}
  
  /**
   * Compute lighting, overwrite the destination colors.
   * \param light Properties of the light to compute.
   * \param eyePos Position of the eye, in object space.
   * \param shininess Specular exponent.
   * \param numvert Number of vertices and normals.
   * \param vb Vertices. Buffer should contain (at least) 3 component vectors.
   * \param nb Normals. Buffer should contain (at least) 3 component vectors.
   * \param litColor Destination buffer for diffuse colors.
   * \param specColor Destination buffer for specular colors.
   */
  virtual void CalculateLighting (const csLightProperties& light,
    const csVector3& eyePos, float shininess,
    size_t numvert, iRenderBuffer* vb, iRenderBuffer* nb, 
    iRenderBuffer* litColor, iRenderBuffer* specColor = 0) const = 0;

  /**
   * Compute lighting, add lit colors to the destination colors.
   * \copydoc CalculateLighting 
   */
  virtual void CalculateLightingAdd (const csLightProperties& light,
    const csVector3& eyePos, float shininess,
    size_t numvert, iRenderBuffer* vb, iRenderBuffer* nb, 
    iRenderBuffer* litColor, iRenderBuffer* specColor = 0) const = 0;

  /**
   * Compute lighting, multiply lit colors with destination colors.
   * \copydoc CalculateLighting 
   */
  virtual void CalculateLightingMul (const csLightProperties& light,
    const csVector3& eyePos, float shininess,
    size_t numvert, iRenderBuffer* vb, iRenderBuffer* nb, 
    iRenderBuffer* litColor, iRenderBuffer* specColor = 0) const = 0;
};

/**
 * iVertexLightCalculator implementation that takes one of csPointLightProc,
 * csDirectionalLightProc or csSpotLightProc for \a LightProc to compute 
 * lighting for a light of the respective type.
 */
template<class LightProc>
class csVertexLightCalculator : public iVertexLightCalculator
{
  struct OpAssign
  {
    OpAssign (csColor& d, const csColor& x) { d = x; }
  };
  struct OpAdd
  {
    OpAdd (csColor& d, const csColor& x) { d += x; }
  };
  struct OpMul
  {
    OpMul (csColor& d, const csColor& x) { d *= x; }
  };
  
#include "csutil/custom_new_disable.h"

  template<typename T, bool B>
  class ConditionalAlloc
  {
    char _data[(sizeof(T) + sizeof(void*) - 1) / sizeof(char)];
    CS_FORCEINLINE_TEMPLATEMETHOD char* Data()
    {
      uintptr_t p = reinterpret_cast<uintptr_t> (_data);
      const int align = sizeof(void*);
      p = (p + align - 1) & ~(align - 1);
      return reinterpret_cast<char*> (p);
    }
  public:
    template<typename P1>
    ConditionalAlloc (const P1& a)
    {
      if (B) new (Data()) T (a);
    }
    ~ConditionalAlloc()
    {
      if (B) GetObject().~T();
    }
  
    T& GetObject()
    {
      CS_ASSERT(B);
      return *(reinterpret_cast<T*> (Data()));
    }
  };
  
#include "csutil/custom_new_enable.h"

  template<typename Op, bool zeroDest, bool diffuse, bool specular>
  void CalculateLightingODS (const csLightProperties& light,
    const csVector3& eyePos, float shininess,
    size_t numvert, iRenderBuffer* vb, iRenderBuffer* nb, 
    iRenderBuffer* litColor, iRenderBuffer* specColor) const
  {
    if (!diffuse && !specular) return;

    // setup the light calculator
    LightProc lighter (light);
    csVertexListWalker<float, csVector3> vbLock (vb, 3);
    csVertexListWalker<float, csVector3> nbLock (nb, 3);
    ConditionalAlloc<csRenderBufferLock<csColor, iRenderBuffer*>,
      diffuse> color (litColor);
    ConditionalAlloc<csRenderBufferLock<csColor, iRenderBuffer*>,
      specular> spec (specColor);

    for (size_t i = 0; i < numvert; i++)
    {
      const csVector3 v (*vbLock);
      const csVector3 n (*nbLock);
      typename LightProc::PerVertex pv (lighter, v, n);
      if (pv.IsLit())
      {
        if (diffuse)
        {
          Op op (color.GetObject()[i], pv.DiffuseAttenuated() * light.color);
        }
        if (specular)
        {
	  csVector3 vertToEye = eyePos - v;
	  csVector3 halfvec = pv.LightDirection() * pv.LightInvDistance();
	  halfvec += vertToEye.Unit();
	  float specDP = halfvec.Unit() * n;
          Op op (spec.GetObject()[i], pow (specDP, shininess) * light.specular * pv.Attenuation());
        }
      }
      else if (zeroDest)
      {
        csColor nullColor (0.0f, 0.0f, 0.0f);
        if (diffuse)
        {
          Op op (color.GetObject()[i], nullColor);
	}
        if (specular)
        {
          Op op (spec.GetObject()[i],  nullColor);
	}
      }
      ++vbLock; ++nbLock;
    }
  }
  template<typename Op, bool zeroDest, bool diffuse>
  void CalculateLightingOD (const csLightProperties& light,
    const csVector3& eyePos, float shininess,
    size_t numvert, iRenderBuffer* vb, iRenderBuffer* nb, 
    iRenderBuffer* litColor, iRenderBuffer* specColor) const
  {
    if (specColor != 0)
      CalculateLightingODS<Op, zeroDest, diffuse, true> (light, eyePos,
        shininess, numvert, vb, nb, litColor, specColor);
    else
      CalculateLightingODS<Op, zeroDest, diffuse, false> (light, eyePos,
        shininess, numvert, vb, nb, litColor, specColor);
  }
  template<typename Op, bool zeroDest>
  void CalculateLightingO (const csLightProperties& light,
    const csVector3& eyePos, float shininess,
    size_t numvert, iRenderBuffer* vb, iRenderBuffer* nb, 
    iRenderBuffer* litColor, iRenderBuffer* specColor) const
  {
    if (litColor != 0)
      CalculateLightingOD<Op, zeroDest, true> (light, eyePos, shininess,
        numvert, vb, nb, litColor, specColor);
    else
      CalculateLightingOD<Op, zeroDest, false> (light, eyePos, shininess,
        numvert, vb, nb, litColor, specColor);
  }
public:
  virtual void CalculateLighting (const csLightProperties& light,
    const csVector3& eyePos, float shininess,
    size_t numvert, iRenderBuffer* vb, iRenderBuffer* nb, 
    iRenderBuffer* litColor, iRenderBuffer* specColor = 0) const
  {
    CalculateLightingO<OpAssign, true> (light, eyePos, shininess, 
      numvert, vb, nb, litColor, specColor);
  }

  virtual void CalculateLightingAdd (const csLightProperties& light,
    const csVector3& eyePos, float shininess,
    size_t numvert, iRenderBuffer* vb, iRenderBuffer* nb, 
    iRenderBuffer* litColor, iRenderBuffer* specColor = 0) const
  {
    CalculateLightingO<OpAdd, false> (light, eyePos, shininess, numvert, 
      vb, nb, litColor, specColor);
  }

  virtual void CalculateLightingMul (const csLightProperties& light,
    const csVector3& eyePos, float shininess,
    size_t numvert, iRenderBuffer* vb, iRenderBuffer* nb, 
    iRenderBuffer* litColor, iRenderBuffer* specColor = 0) const
  {
    CalculateLightingO<OpMul, false> (light, eyePos, shininess, numvert,
      vb, nb, litColor, specColor);
  }
};

#endif //__CS_VERTEXLIGHT_H__
