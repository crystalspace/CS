/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

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

#include <cssysdef.h>

#include "furmesh.h"
#include "hairstrandgenerator.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /************************
  *  HairStrandGenerator
  ************************/  

  SCF_IMPLEMENT_FACTORY (HairStrandGenerator)

    CS_LEAKGUARD_IMPLEMENT(HairStrandGenerator);	

  HairStrandGenerator::HairStrandGenerator (iBase* parent)
    : scfImplementationType (this, parent), object_reg(0), material(0), 
    g3d(0), svStrings(0), M(256, 256), N(256, 256), gauss_matrix(0), mc(0)
  {
  }

  HairStrandGenerator::~HairStrandGenerator ()
  {
    if (mc)
      delete mc;
    if (M.data)
      delete M.data;
    if (gauss_matrix)
      delete gauss_matrix;
    if (N.data)
      delete N.data;
  }

  bool HairStrandGenerator::Initialize (iObjectRegistry* r)
  {
    object_reg = r;

    svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
      object_reg, "crystalspace.shader.variablenameset");
  
    if (!svStrings) 
    {
      csPrintfErr ("No SV names string set!\n");
      return false;
    }

    g3d = csQueryRegistry<iGraphics3D> (object_reg);
    
    if (!g3d) 
    {
      csPrintfErr ("No g3d found!\n");
      return false;
    }

    mc = new MarschnerConstants();

    if (!mc) 
    {
      csPrintfErr ("No MarschnerConstants found!\n");
      return false;
    }

    return true;
  }

  iMaterial* HairStrandGenerator::GetMaterial() const
  {
    return material;
  }

  void HairStrandGenerator::SetMaterial(iMaterial* material)
  {
    this->material = material;
    Invalidate();
  }

  // Data need to be recomputed
  void HairStrandGenerator::Invalidate()
  {
      UpdateConstans();
      UpdateM();
      UpdateN();
  }

  // Nothing to be updated 
  void HairStrandGenerator::Update()
  {
  }

  void HairStrandGenerator::UpdateConstans()
  {
    if(!M.handle)
    {
      // Surface properties
      CS::ShaderVarName aR (svStrings, "aR");	
      material->GetVariableAdd(aR)->SetValue(mc->aR);

      CS::ShaderVarName bR (svStrings, "bR");	
      material->GetVariableAdd(bR)->SetValue(mc->bR);

      // Fiber properties
      CS::ShaderVarName absorption (svStrings, "absorption");	
      material->GetVariableAdd(absorption)->SetValue(mc->absorption);

      CS::ShaderVarName eccentricity (svStrings, "eccentricity");	
      material->GetVariableAdd(eccentricity)->SetValue(mc->eccentricity);

      // Glints
      CS::ShaderVarName kG (svStrings, "kG");	
      material->GetVariableAdd(kG)->SetValue(mc->kG);

      CS::ShaderVarName wc (svStrings, "wc");	
      material->GetVariableAdd(wc)->SetValue(mc->wc);

      CS::ShaderVarName Dh0 (svStrings, "Dh0");	
      material->GetVariableAdd(Dh0)->SetValue(mc->Dh0);
    }

    // Surface properties
    CS::ShaderVarName aR (svStrings, "aR");	
    material->GetVariableAdd(aR)->GetValue(mc->aR);
    mc->aTT = -mc->aR/2;
    mc->aTRT = -3 * mc->aR/2;

    CS::ShaderVarName bR (svStrings, "bR");	
    material->GetVariableAdd(bR)->GetValue(mc->bR);
    mc->bTT = mc->bR/2;
    mc->bTRT = 2 * mc->bR;

    // Fiber properties
    CS::ShaderVarName absorption (svStrings, "absorption");	
    material->GetVariableAdd(absorption)->GetValue(mc->absorption);

    CS::ShaderVarName eccentricity (svStrings, "eccentricity");	
    material->GetVariableAdd(eccentricity)->GetValue(mc->eccentricity);

    // Glints
    CS::ShaderVarName kG (svStrings, "kG");	
    material->GetVariableAdd(kG)->GetValue(mc->kG);

    CS::ShaderVarName wc (svStrings, "wc");	
    material->GetVariableAdd(wc)->GetValue(mc->wc);

    CS::ShaderVarName Dh0 (svStrings, "Dh0");	
    material->GetVariableAdd(Dh0)->GetValue(mc->Dh0);
  }

  // Marschner specific methods
  void HairStrandGenerator::UpdateM()
  {
    if(!M.handle)
    {
      CS::ShaderVarName strandWidthName (svStrings, "tex M");	
      csRef<csShaderVariable> shaderVariable = 
        material->GetVariableAdd(strandWidthName);

      if(!M.Create(g3d))
      {
        csPrintfErr ("Failed to create M texture!\n");
        return;
      }

      shaderVariable->SetValue(M.handle);

      M.data = new uint8 [M.width * M.height * 4];
      gauss_matrix = new float [M.width * M.height];

      for( int x = 0 ; x < M.width ; x ++ )
        for (int y = 0 ; y < M.height; y ++)
        {
          M.Set(x, y, 0, 255); // red
          M.Set(x, y, 1, 0); // green
          M.Set(x, y, 2, 0); // blue
          M.Set(x, y, 3, 255); // alpha
        }
    }

    csVector3 constantsM;
    constantsM.x = ComputeM(mc->aR, mc->bR, 0) / 255.0f;
    constantsM.y = ComputeM(mc->aTT, mc->bTT, 1) / 255.0f;
    constantsM.z = ComputeM(mc->aTRT, mc->bTRT, 2) / 255.0f;

    // alpha is qd
    for( int x = 0 ; x < M.width ; x ++ )
      for (int y = 0 ; y < M.width; y ++)
      {
        float sin_thI = -1.0f + (x * 2.0f) / (M.width - 1);
        float sin_thR = -1.0f + (y * 2.0f) / (M.width - 1);
        float thI = asin(sin_thI);
        float thR = asin(sin_thR);
        float cos_thD = cos( (thI - thR) / 2.0f );
        M.Set(x, y, 3, (uint8)( ( 255.0f * (cos_thD + 1.0f) ) / 2.0f) );
      }

    CS::ShaderVarName constantsMName (svStrings, "constants M");	
    material->GetVariableAdd(constantsMName)->SetValue(constantsM);

    // send buffer to texture
    M.Write();

    M.SaveImage(object_reg, "/data/hairtest/debug/M_debug.png");
  }

  float HairStrandGenerator::ComputeM(float a, float b, int channel)
  {
    float max = 0;
    
    // find max
    for (int x = 0; x < M.width; x++)
      for (int y = 0; y < M.height; y++)    
      {
        float sin_thI = -1.0f + (x * 2.0f) / (M.width - 1);
        float sin_thR = -1.0f + (y * 2.0f) / (M.height - 1);
        float thI = (180 * asin(sin_thI) / PI);
        float thR = (180 * asin(sin_thR) / PI);
        float thH = (thR + thI) / 2;
        float thH_a = thH - a;

        float gauss = MarschnerHelper::GaussianDistribution(b, thH_a);
        gauss_matrix[x + y * M.width] = gauss;

        if (255 * gauss > max)
          max = 255 * gauss;
      }

    // normalize
    for (int x = 0; x < M.width; x++)
      for (int y = 0; y < M.height; y++)
      {
        float gauss = gauss_matrix[x + y * M.width];
        M.Set(x, y, channel, (uint8)(255 * 255 * gauss / max) );
      }

    return max;
  }

  void HairStrandGenerator::UpdateN()
  {
    if(!N.handle)
    {
      CS::ShaderVarName strandWidthName (svStrings, "tex N");	
      csRef<csShaderVariable> shaderVariable = 
        material->GetVariableAdd(strandWidthName);

      N.Create(g3d);

      if(!N.handle)
      {
        csPrintfErr ("Failed to create N texture!\n");
        return;
      }

      shaderVariable->SetValue(N.handle);

      N.data = new uint8 [N.width * N.height * 4];

      for( int x = 0 ; x < N.width ; x ++ )
        for (int y = 0 ; y < N.height; y ++)
        {
          N.Set(x, y, 0, 0); // red
          N.Set(x, y, 1, 0); // green
          N.Set(x, y, 2, 0); // blue
          N.Set(x, y, 3, 255); // alpha
        }
    }

    for( int x = 0 ; x < N.width ; x ++ )
      for (int y = 0 ; y < N.height; y ++)
      {
        float cos_phiD = -1.0f + (x * 2.0f) / (N.width - 1);
        float cos_thD = -1.0f + (y * 2.0f) / (N.height - 1);
        float phiD = acos(cos_phiD);
        float thD = acos(cos_thD);
        N.Set(x, y, 0, (uint8)(255 * ComputeNP(0, phiD, thD)) ); // red
        N.Set(x, y, 1, (uint8)(255 * ComputeNP(1, phiD, thD)) ); // green
        N.Set(x, y, 2, (uint8)(255 * ComputeNP(2, phiD, thD)) ); // blue
      }

    // send buffer to texture
    N.Write();

    N.SaveImage(object_reg, "/data/hairtest/debug/N_debug.png");
  }

  float HairStrandGenerator::ComputeT(float absorption, float gammaT, int p) const
  {
    float l = 2 * cos(gammaT);	// l = ls / cos qt = 2r cos h0t / cos qt
    return exp(absorption * l * p);
  }

  float HairStrandGenerator::ComputeA(float absorption, int p, float h, 
    float refraction, float etaPerpendicular, float etaParallel) const
  {
    float gammaI = asin(h);

    // A(0; h) = F(h0; h00; gi)
    if (p == 0)
      return MarschnerHelper::Fresnel(etaPerpendicular, etaParallel, gammaI);

    // A(p; h) = ( (1 - F(h0; h00; gi) ) ^ 2 ) * 
    //           ( F(1 / h0; 1 / h00; gi) ^ (p - 1) ) * ( T(s0a; h) ^ p )
    float gammaT = asin(h / etaPerpendicular);	// h0 sin gt = h

    float fresnel = 
      MarschnerHelper::Fresnel(etaPerpendicular, etaParallel, gammaI);
    float invFresnel = 
      MarschnerHelper::Fresnel(1 / etaPerpendicular, 1 / etaParallel, gammaT);
    float t = ComputeT(absorption, gammaT, p);

    fresnel = (1 - fresnel) * (1 - invFresnel);
    if (p > 1)
      fresnel = fresnel * invFresnel;

    return fresnel * t;
  }

  float HairStrandGenerator::ComputeNP(int p, float phi, float thD) const
  {
    float refraction = mc->eta;
    float absorption = mc->absorption;

    float etaPerpendicular = MarschnerHelper::BravaisIndex(thD, refraction);
    float etaParallel = (refraction * refraction) / etaPerpendicular;

    CubicSolution roots = EquationsSolver::Roots(p, etaPerpendicular, phi);
    float result = 0;

    for (size_t index = 0; index < roots.count; index++ )
    {
      float gammaI = roots[index];

      float h = sin(gammaI);
      float finalAbsorption = ComputeA(absorption, p, h, refraction, 
        etaPerpendicular, etaParallel);
      float inverseDerivateAngle = 
        EquationsSolver::InverseFirstDerivate(p, etaPerpendicular, h);

      result += finalAbsorption * 0.5f * fabs(inverseDerivateAngle);
    }

    return csMin(1.0f, result);
  }

  float HairStrandGenerator::SimpleNP(float phi, float thD ) const
  {
    float refraction = mc->eta;

    float etaPerpendicular = MarschnerHelper::BravaisIndex(thD, refraction);
    float etaParallel = (refraction * refraction) / etaPerpendicular;
    float gammaI = -phi / 2.0f;

    float h = sin(gammaI);

    float result = (sqrt(1 - h * h));

    result *= MarschnerHelper::Fresnel(etaPerpendicular, etaParallel, gammaI);

    return csMin(1.0f, result);
  }

  /************************
  *  MarschnerConstants
  ************************/  

  CS_LEAKGUARD_IMPLEMENT(MarschnerConstants);	
  
  MarschnerConstants::MarschnerConstants()
  {
    // Surface properties
    aR = -5;
    aTT = - aR / 2;
    aTRT = - 3 * aR / 2;

    bR = 5;
    bTT = bTT / 2;
    bTRT = 2 * bTT;

    // Fiber properties
    eta = 1.55f;
    absorption = 0.2f;
    eccentricity = 0.85f;

    // Glints
    kG = 0.5f;
    wc = 10;
    Dh0 = 0.2f;
    DhM = 0.5f;
  }
}
CS_PLUGIN_NAMESPACE_END(FurMesh)

/*******************
*  MarschnerHelper
********************/  

CS_LEAKGUARD_IMPLEMENT(MarschnerHelper);	

// Gaussian distribution - http://en.wikipedia.org/wiki/Normal_distribution
float MarschnerHelper::GaussianDistribution(float sigma, float x_mu)
{
  return ((1.0f / (fabs(sigma) * sqrt(2.0f * PI))) *
    exp(-(x_mu * x_mu) / (2.0f * sigma * sigma)));
}

// Bravais - Miller index - http://en.wikipedia.org/wiki/Miller_index
float MarschnerHelper::BravaisIndex(float theta, float eta)
{
  float sinTheta = sin(theta);
  return (sqrt(eta * eta - sinTheta * sinTheta) / cos(theta));
}

// Fresnel Parallel - http://en.wikipedia.org/wiki/Fresnel_equations
float MarschnerHelper::FresnelParallel(float n2, float angle)
{
  float R = 1;
  float n1 = 1;
  float cos_gammaI = cos(angle);
  float a = ((n1 / n2) * sin(angle));
  float b = a * a;

  if (b > 1)
    return R;

  float cos_gammaT = sqrt(1 - b);

  R = (n2 * cos_gammaI - n1 * cos_gammaT) / (n2 * cos_gammaI + n1 * cos_gammaT);

  return csMin(1.0f, R * R);
}

// Fresnel Perpendicular - http://en.wikipedia.org/wiki/Fresnel_equations
float MarschnerHelper::FresnelPerpendicular(float n2, float angle)
{
  float R = 1;
  float n1 = 1;
  float cos_gammaI = cos(angle);
  float a = ((n1 / n2) * sin(angle));
  float b = a * a;

  if (b > 1)
    return R;

  float cos_gammaT = sqrt(1 - b);

  R = (n1 * cos_gammaI - n2 * cos_gammaT) / (n1 * cos_gammaI + n2 * cos_gammaT);

  return csMin(1.0f, R * R);
}

// Fresnel Equation - http://en.wikipedia.org/wiki/Fresnel_equations
double MarschnerHelper::Fresnel
  (float etaPerpendicular, float etaParallel, float angle)
{
  return 0.5f * (FresnelPerpendicular(etaPerpendicular, angle) + 
    FresnelParallel(etaParallel, angle));
}


/*******************
*  EquationsSolver
********************/  

CS_LEAKGUARD_IMPLEMENT(EquationsSolver);	

// Solve a * x + b = 0
CubicSolution EquationsSolver::LinearSolver(float a, float b)
{
  CubicSolution roots;

  if (fabs(a) > SMALL_EPSILON)
  {
    roots.X1 = -b / a;
    roots.count = 1;
  }
  else
    roots.count = 0;

  return roots;
}

// Solve a * x ^ 2 + b * x + c = 0
CubicSolution EquationsSolver::QuadraticSolver(float a, float b, float c)
{
  if (fabs(a) < SMALL_EPSILON)
    return LinearSolver(b, c);

  CubicSolution roots;

  float D = b * b - 4 * a * c;

  if (fabs(D) < SMALL_EPSILON)
  {
    roots.X1 = -b / (2 * a);
    roots.X2 = roots.X1;
    roots.count = 2;
  }
  else if (D > 0)
  {
    float delta = sqrt(D);
    roots.X1 = (-b + delta) / (2 * a);
    roots.X2 = (-b - delta) / (2 * a);
    roots.count = 2;
  }
  else
    roots.count = 0;

  return roots;
}

// Solve x ^ 3 + A * x ^ 2 + B * x + C = 0 - 
//  http://en.wikipedia.org/wiki/Cubic_function
CubicSolution EquationsSolver::NormalizedCubicSolver(float A, float B, float C)
{
  CubicSolution roots;

  if (fabs(C) < SMALL_EPSILON)	// + x = 0 solution
  {
    roots = QuadraticSolver(1, A, B);
    roots[ roots.count ] = 0;
    roots.count ++;
  }
  else
  {
    float Q = (3 * B - A * A) / 9;
    float R = (9 * A * B - 27 * C - 2 * A * A * A) / 54;
    float D = Q * Q * Q + R * R;

    if (D > 0)	// 1 real root
    {
      float sqrtD = sqrt(D);
      float s = SIGN(R + sqrtD) * pow(fabs(R + sqrtD), 1.0f / 3.0f);
      float t = SIGN(R - sqrtD) * pow(fabs(R - sqrtD), 1.0f / 3.0f);

      roots.X1 = (-A / 3 + (s + t));
      roots.count = 1;
    }
    else	// 3 real roots
    {
      float theta = acos(R / sqrt(-(Q * Q * Q)));
      float sqrtQ = sqrt(-Q);
      roots.X1 = (2 * sqrtQ * cos(theta / 3) - A / 3);
      roots.X2 = (2 * sqrtQ * cos((theta + 2 * PI) / 3) - A / 3);
      roots.X3 = (2 * sqrtQ * cos((theta + 4 * PI) / 3) - A / 3);
      roots.count = 3;
    }
  }

  return roots;
}

// Solve a * x ^ 3 + b * x ^ 2 + c * x  + d = 0 - 
//  http://en.wikipedia.org/wiki/Cubic_function
CubicSolution EquationsSolver::CubicSolver(float a, float b, float c, float d)
{
  CubicSolution roots;

  if (fabs(a) < SMALL_EPSILON)
    roots = QuadraticSolver(b, c, d);
  else
    roots = NormalizedCubicSolver(b / a, c / a, d / a);

  return roots;
}

// Solve o(p,y) - phi = 0
CubicSolution EquationsSolver::Roots(float p, float etaPerpendicular, float phi)
{
  float c = asin(1 / etaPerpendicular);
  return CubicSolver(-8 * (p * c / (PI * PI * PI)), 0, 
    (6 * p * c / PI - 2), p * PI - phi);
}

// computes the derivative of the polynomial relative to h.
float EquationsSolver::InverseFirstDerivate(float p, float etaPerpendicular, 
  float h)
{
  float gammaI = asin(h);
  float c = asin( 1 / etaPerpendicular );
  float dGamma = (6 * p * c / PI - 2) - 
    3 * 8 * (p * c / (PI * PI * PI)) * gammaI * gammaI;

  return sqrt(1 - h * h) / dGamma;
}
