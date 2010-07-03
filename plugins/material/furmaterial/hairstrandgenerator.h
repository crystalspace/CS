/*
  Copyright (C) 2010 Alexandru - Teodor Voicu

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
*/

#ifndef __HAIR_STRAND_GENERATOR_H__
#define __HAIR_STRAND_GENERATOR_H__

#include <iutil/comp.h>
#include <csgeom/vector3.h>
#include <imaterial/furmaterial.h>
#include <csgfx/shadervarcontext.h>
#include <imesh/genmesh.h>

#include "crystalspace.h"

#include "csutil/scf_implementation.h"

struct iObjectRegistry;

CS_PLUGIN_NAMESPACE_BEGIN(FurMaterial)
{
  class MarschnerConstants
  {
  public:         
    CS_LEAKGUARD_DECLARE(MarschnerConstants);
    MarschnerConstants();
    virtual ~MarschnerConstants();

    // Surface properties
    float aR;
    float aTT;
    float aTRT;

    float bR;
    float bTT;
    float bTRT;

    // Fiber properties
    float eta;
    float absorption;
    float eccentricity;

    // Glints
    float kG;
    float wc;
    float Dh0;
    float DhM;
  };

  class HairStrandGenerator : public scfImplementation2 <HairStrandGenerator, 
    iFurStrandGenerator, iComponent> 
  {
  public:
    CS_LEAKGUARD_DECLARE(HairStrandGenerator);

    HairStrandGenerator (iBase* parent);
    virtual ~HairStrandGenerator ();

    // From iComponent	
    virtual bool Initialize (iObjectRegistry*);

    // From iFurStrandGenerator
    virtual iMaterial* GetMaterial();
    virtual void SetMaterial(iMaterial* material);
    virtual void Invalidate();
    virtual void Update();

  private:
    iObjectRegistry* object_reg;
    iMaterial* material;
    bool valid;
    // Shader
    csRef<iGraphics3D> g3d;
    csRef<iShaderVarStringSet> svStrings;
    int width, height;
    csRef<iTextureHandle> M;
    uint8* m_buf;
    float* gauss_matrix;
    csRef<iTextureHandle> N;
    uint8* n_buf;
    MarschnerConstants* mc;
    // Marschner specific functions
    void UpdateM();
    float ComputeM(float a, float b, int channel);
    void UpdateN();
    float SimpleNP(float phi, float thD );
    float ComputeT(float absorption, float gammaT);
    float ComputeA(float absorption, int p, float h, float refraction, 
      float etaPerpendicular, float etaParallel);
    float ComputeNP(int p, float phiD, float thD);
    // Marschner temp functions
    void SaveImage(uint8 *buf, const char* texname);
  };

}
CS_PLUGIN_NAMESPACE_END(FurMaterial)


class MarschnerHelper
{
public:
  CS_LEAKGUARD_DECLARE(MarschnerHelper);
  // Gaussian distribution - http://en.wikipedia.org/wiki/Normal_distribution
  static float GaussianDistribution(float sigma, float x_mu);

  // Bravais - Miller index - http://en.wikipedia.org/wiki/Miller_index
  static float BravaisIndex(float theta, float eta);

  // Fresnel Parallel - http://en.wikipedia.org/wiki/Fresnel_equations
  static float FresnelParallel(float n2, float angle);

  // Fresnel Perpendicular - http://en.wikipedia.org/wiki/Fresnel_equations
  static float FresnelPerpendicular(float n2, float angle);

  // Fresnel Equation - http://en.wikipedia.org/wiki/Fresnel_equations
  static double Fresnel(float etaPerpendicular, float etaParallel, float angle);
};

struct CubicSolution
{
  float X1, X2, X3;
  size_t count;

  inline float operator[] (size_t n) const 
  { return (n&2) ? X3 : ( (n&1) ? X2 : X1 ) ; }

  inline float & operator[] (size_t n) 
  { return (n&2) ? X3 : ( (n&1) ? X2 : X1 ) ; }
};

class EquationsSolver
{
public:
  CS_LEAKGUARD_DECLARE(EquationsSolver);
  // Solve a * x + b = 0
  static CubicSolution LinearSolver(float a, float b);

  // Solve a * x ^ 2 + b * x + c = 0
  static CubicSolution QuadraticSolver(float a, float b, float c);

  // Solve x ^ 3 + A * x ^ 2 + B * x + C = 0 - 
  //  http://en.wikipedia.org/wiki/Cubic_function
  static CubicSolution NormalizedCubicSolver(float A, float B, float C);

  // Solve a * x ^ 3 + b * x ^ 2 + c * x  + d = 0 - 
  //  http://en.wikipedia.org/wiki/Cubic_function
  static CubicSolution CubicSolver(float a, float b, float c, float d);

  // Solve o(p,y) - phi = 0
  static CubicSolution Roots(float p, float etaPerpendicular, float phi);

  // computes the derivative of the polynomial relative to h.
  static float InverseFirstDerivate(float p, float etaPerpendicular, float h);
};


#endif // __HAIR_STRAND_GENERATOR_H__
