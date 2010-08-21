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

#ifndef __FUR_MESH_PROPERTIES_H__
#define __FUR_MESH_PROPERTIES_H__

#include "crystalspace.h"

#include "furdata.h"
#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  class FurMeshState : public virtual CS::Mesh::iFurMeshState
  {
  public:
    CS_LEAKGUARD_DECLARE(FurMeshState);

    FurMeshState ();
    virtual ~FurMeshState ();

    virtual float GetStrandWidth() const;
    virtual void SetStrandWidth(float strandWidth);
    virtual float GetDisplacement() const;
    virtual void SetDisplacement(float displacement);
    virtual iTextureHandle* GetDensityMap() const;
    virtual void SetDensityMap(iTextureHandle* densityMap);
    virtual float GetDensityFactorGuideFurs() const;
    virtual void SetDensityFactorGuideFurs(float densityFactorGuideFurs);
    virtual float GetDensityFactorFurStrands() const;
    virtual void SetDensityFactorFurStrands(float densityFactorFurStrands);
    virtual iTextureHandle* GetHeightMap() const;
    virtual void SetHeightMap(iTextureHandle* heightMap);
    virtual float GetHeightFactor() const;
    virtual void SetHeightFactor(float heightFactor);
    virtual float GetControlPointsDistance() const;
    virtual void SetControlPointsDistance(float controlPointsDistance);
    virtual float GetPositionDeviation() const;
    virtual void SetPositionDeviation(float positionDeviation);
    virtual bool GetGrowTangent() const;
    virtual void SetGrowTangent(bool growTangent);
    virtual uint GetMixmode () const;
    virtual void SetMixmode (uint mode);
    virtual uint GetPriority () const;
    virtual void SetPriority (uint priority);
    virtual csZBufMode GetZBufMode () const;
    virtual void SetZBufMode (csZBufMode z_buf_mode);

  private:
    float strandWidth;
    float displacement;
    iTextureHandle* densityMap;
    float densityFactorGuideFurs;
    float densityFactorFurStrands;
    iTextureHandle* heightMap;
    float heightFactor;
    float controlPointsDistance;
    float positionDeviation;
    bool growTangents;
    uint mixmode;
    uint priority;
    csZBufMode z_buf_mode;
  };

  // Light Scattering from Human Hair Fibers
  // http://www.cs.cornell.edu/~srm/publications/SG03-hair.pdf
  class MarschnerConstants
  {
/*
  Surface properties
    aR longitudinal shift: R lobe -10 to -5
    aTT longitudinal shift: TT lobe -aR/2
    aTRT longitudinal shift: TRT lobe -3aR/2
 
    bR longitudinal width (stdev.): R lobe 5 to 10
    bTT longitudinal width (stdev.): TT lobe bR/2
    bTRT longitudinal width (stdev.): TRT lobe 2bR
  
  Fiber properties
	  eta: 1.55
	  absorption: 0.2 to inf
	  eccentricity: 0.85 to 1
 
  Glints
    kG glint scale factor: 0.5 to 5
    wc azimuthal width of caustic: 10 to 25
    Dh0 fade range for caustic merge: 0.2 to 0.4
    DhM caustic intensity limit: 0.5
*/
  public:         
    CS_LEAKGUARD_DECLARE(MarschnerConstants);
    // Synchronized shader variables with this class
    MarschnerConstants();

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

  class HairMeshProperties : public scfImplementation1 <HairMeshProperties, 
    CS::Mesh::iFurMeshMaterialProperties>
  {
  public:
    CS_LEAKGUARD_DECLARE(HairMeshProperties);

    HairMeshProperties (iObjectRegistry* object_reg);
    virtual ~HairMeshProperties ();

    //-- iFurMeshMaterialProperties
    virtual iMaterial* GetMaterial() const;
    virtual void SetMaterial(iMaterial* material);
    virtual void Invalidate();
    virtual void Update();

  private:
    iObjectRegistry* object_reg;
    iMaterial* material;
    // Shader
    csRef<iGraphics3D> g3d;
    csRef<iShaderVarStringSet> svStrings;
    csTextureRGBA M;
    csTextureRGBA N;
    float* gauss_matrix;
    // Marschner constants functions
    MarschnerConstants mc;
    void UpdateConstans();
    // Marschner specific functions
    void UpdateM();
    float ComputeM(float a, float b, int channel);
    void UpdateN();
    float SimpleNP(float phi, float thD ) const;
    float ComputeT(float absorption, float gammaT, int p) const;
    float ComputeA(float absorption, int p, float h, float refraction, 
      float etaPerpendicular, float etaParallel) const;
    float ComputeNP(int p, float phiD, float thD) const;
  };

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

// Various functions needed for the Marschner implementation
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

  CubicSolution()
  { count = 0; }

  inline float operator[] (size_t n) const 
  { return (n&2) ? X3 : ( (n&1) ? X2 : X1 ) ; }

  inline float & operator[] (size_t n) 
  { return (n&2) ? X3 : ( (n&1) ? X2 : X1 ) ; }
};

// Solve different equations
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


#endif // __FUR_MESH_PROPERTIES_H__
