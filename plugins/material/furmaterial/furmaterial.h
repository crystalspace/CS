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

#ifndef __FUR_MATERIAL_H__
#define __FUR_MATERIAL_H__

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
  class FurMaterialType : public 
    scfImplementation2<FurMaterialType,iFurMaterialType,iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(FurMaterialType);

    FurMaterialType (iBase* parent);
    virtual ~FurMaterialType ();

    // From iComponent	
    virtual bool Initialize (iObjectRegistry*);

    // From iFurMaterialType
    virtual void ClearFurMaterials ();
    virtual void RemoveFurMaterial (const char *name, iFurMaterial* furMaterial);
    virtual iFurMaterial* CreateFurMaterial (const char *name);
    virtual iFurMaterial* FindFurMaterial (const char *name) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<iFurMaterial>, csString> furMaterialHash;
  };

  struct csGuideHairReference
  {
    size_t index;
    float distance;
  };

  struct csHairStrand
  {
    csVector3 *controlPoints;
    size_t controlPointsCount;

    csGuideHairReference *guideHairs;
    size_t guideHairsCount;
  };

  struct csGuideHair
  {
    csVector3 *controlPoints;
    size_t controlPointsCount;
  };

  class FurMaterial : public scfImplementation2<FurMaterial,
    scfFakeInterface<iShaderVariableContext>,
    iFurMaterial>, public CS::ShaderVariableContextImpl
  {
    friend class FurAnimationControl;

  public:
    CS_LEAKGUARD_DECLARE(FurMaterial);
    FurMaterial (FurMaterialType* manager, const char *name,
      iObjectRegistry* object_reg);
    virtual ~FurMaterial ();

    // From iFurMaterial
    virtual void GenerateGeometry (iView* view, iSector *room);
    virtual void SetPhysicsControl (iFurPhysicsControl* physicsControl);
    // Temporary - Set Mesh and Submesh
    virtual void SetMeshFactory ( iAnimatedMeshFactory* meshFactory);
    virtual void SetMeshFactorySubMesh ( iAnimatedMeshFactorySubMesh* 
      meshFactorySubMesh );
    // Set Material
    virtual void SetMaterial ( iMaterial* material );
    // Set FurStrandMaterial
    virtual void SetFurMaterialWrapper( iFurStrandMaterial* furStrandMaterial);
    // Get FurStrandMaterial
    virtual iFurStrandMaterial* GetFurMaterialWrapper( );

    // From iMaterial
    /// Associate a shader with a shader type
    virtual void SetShader (csStringID type, iShader* shader);
    /// Get shader associated with a shader type
    virtual iShader* GetShader (csStringID type);

    /// Get all Shaders
    const csHash<csRef<iShader>, csStringID>& GetShaders() const
    { return shaders; }

    /// Get texture.
    virtual iTextureHandle* GetTexture ();
    /// Get a texture from the material.
    virtual iTextureHandle* GetTexture (CS::ShaderVarStringID name);

    virtual iShader* GetFirstShader (const csStringID* types, size_t numTypes);

  protected:
    FurMaterialType* manager;
    csString name;

  private:
    iObjectRegistry* object_reg;
    /// Shader associated with material
    csHash<csRef<iShader>, csStringID> shaders;
    /// Fur geometry
    csRef<iGeneralFactoryState> factoryState;
    csRef<iView> view;
    csArray<csHairStrand> hairStrands;
    csArray<csGuideHair> guideHairs;
    csArray<csTriangle> guideHairsTriangles;
    csRef<iFurPhysicsControl> physicsControl;
    csRef<iFurStrandMaterial> furStrandMaterial;
    /// Temp fur geometry
    csRef<iAnimatedMeshFactory> meshFactory;
    csRef<iAnimatedMeshFactorySubMesh> meshFactorySubMesh;
    iTextureHandle* densitymap;
    iTextureHandle* heightmap;
    csRef<iMaterial> material;
    csRef<iShaderVarStringSet> svStrings;
    float strandWidth;
    /// Model
    csRef<iEngine> engine;
    csRef<iLoader> loader;
    float displaceEps;
    /// functions
    void GenerateGuidHairs(iRenderBuffer* indices, iRenderBuffer* vertexes,
      iRenderBuffer* normals);
    void SynchronizeGuideHairs();
    void GenerateHairStrands(iRenderBuffer* indices, iRenderBuffer* vertexes);
    void SynchronizeHairsStrands();
    void SetDensitymap();
    void SetHeightmap();
    void SetStrandWidth();
    void SetColor(csColor color);
    void SetDisplaceEps();
  };

  class FurAnimationControl : public scfImplementation1 
    <FurAnimationControl, iGenMeshAnimationControl>
  {
  public:
    CS_LEAKGUARD_DECLARE(FurAnimationControl);

    FurAnimationControl (FurMaterial* furMaterial);
    virtual ~FurAnimationControl ();

    //-- iGenMeshAnimationControl
    virtual bool AnimatesColors () const;
    virtual bool AnimatesNormals () const;
    virtual bool AnimatesTexels () const;
    virtual bool AnimatesVertices () const;
    virtual void Update (csTicks current, int num_verts, uint32 version_id);
    virtual const csColor4* UpdateColors (csTicks current, const csColor4* colors,
      int num_colors, uint32 version_id);
    virtual const csVector3* UpdateNormals (csTicks current, const csVector3* normals,
      int num_normals, uint32 version_id);
    virtual const csVector2* UpdateTexels (csTicks current, const csVector2* texels,
      int num_texels, uint32 version_id);
    virtual const csVector3* UpdateVertices (csTicks current, const csVector3* verts,
      int num_verts, uint32 version_id);

  private:
    csWeakRef<iMeshObject> mesh;
    csTicks lastTicks;
    FurMaterial* furMaterial;

    // functions
    virtual void UpdateHairStrand(csHairStrand* hairStrand);
  };

  class FurPhysicsControl : public scfImplementation2 <FurPhysicsControl, 
    iFurPhysicsControl, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(FurPhysicsControl);

    FurPhysicsControl (iBase* parent);
    virtual ~FurPhysicsControl ();

    // From iComponent	
    virtual bool Initialize (iObjectRegistry*);

    //-- iFurPhysicsControl
    virtual void SetRigidBody (iRigidBody* rigidBody);
    virtual void SetBulletDynamicSystem (iBulletDynamicSystem* bulletDynamicSystem);
    // Initialize the strand with the given ID
    virtual void InitializeStrand (size_t strandID, const csVector3* coordinates,
      size_t coordinatesCount);
    // Animate the strand with the given ID
    virtual void AnimateStrand (size_t strandID, csVector3* coordinates, size_t
      coordinatesCount);
    virtual void RemoveStrand (size_t strandID);
    virtual void RemoveAllStrands ();

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<iBulletSoftBody>, size_t > guideRopes;
    csRef<iRigidBody> rigidBody;
    csRef<iBulletDynamicSystem> bulletDynamicSystem;
  };

  class MarschnerConstants
  {
  public:         
    MarschnerConstants()
    {
      // Surface properties
      aR = -10;
      aTT = - (-10) / 2;
      aTRT = - 3 * (-10) / 2;

      bR = 5;
      bTT = (5) / 2;
      bTRT = 2 * (5);

      // Fiber properties
      eta = 1.55;
      absorption = 0.2;
      eccentricity = 0.85;

      // Glints
      kG = 0.5;
      wc = 10;
      Dh0 = 0.2;
      DhM = 0.5;
    }

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

  class FurStrandMaterial : public scfImplementation2 <FurStrandMaterial, 
    iFurStrandMaterial, iComponent> 
  {
  public:
    CS_LEAKGUARD_DECLARE(FurStrandMaterial);

    FurStrandMaterial (iBase* parent);
    virtual ~FurStrandMaterial ();

    // From iComponent	
    virtual bool Initialize (iObjectRegistry*);

    // From iFurStrandMaterial
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

  // Gaussian distribution - http://en.wikipedia.org/wiki/Normal_distribution
  static float GaussianDistribution(float sigma, float x_mu)
  {
    return ((1.0f / (fabs(sigma) * sqrt(2.0f * PI))) *
      exp(-(x_mu * x_mu) / (2.0f * sigma * sigma)));
  }

  // Bravais - Miller index - http://en.wikipedia.org/wiki/Miller_index
  static float BravaisIndex(float theta, float eta)
  {
    float sinTheta = sin(theta);
    return (sqrt(eta * eta - sinTheta * sinTheta) / cos(theta));
  }

  // Fresnel Parallel - http://en.wikipedia.org/wiki/Fresnel_equations
  static float FresnelParallel(float n2, float angle)
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
  static float FresnelPerpendicular(float n2, float angle)
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
  static double Fresnel(float etaPerpendicular, float etaParallel, float angle)
  {
    return 0.5f * (FresnelPerpendicular(etaPerpendicular, angle) + 
      FresnelParallel(etaParallel, angle));
  }
};

class EquationsSolver
{
public:
  // Solve a * x + b = 0
  static csVector4 LinearSolver(float a, float b)
  {
    csVector4 roots = csVector4();

    if (fabs(a) > EPSILON)
    {
      roots[0] = -b / a;
      roots[3] = 1;
    }
    else
      roots[3] = 0;

    return roots;
  }

  // Solve a * x ^ 2 + b * x + c = 0
  static csVector4 QuadraticSolver(float a, float b, float c)
  {
    csVector4 roots;

    if (fabs(a) < EPSILON)
      return LinearSolver(b, c);
    else
    {
      roots = csVector4();

      float D = b * b - 4 * a * c;

      if (fabs(D) < EPSILON)
      {
        roots[0] = -b / (2 * a);
        roots[1] = -b / (2 * a);
        roots[3] = 2;
      }
      else if (D > 0)
      {
        float delta = sqrt(D);
        roots[0] = (-b + delta) / (2 * a);
        roots[1] = (-b - delta) / (2 * a);
        roots[3] = 2;
      }
      else
        roots[3] = 0;
    }

    return roots;
  }

  // Solve x ^ 3 + A * x ^ 2 + B * x + C = 0 - http://en.wikipedia.org/wiki/Cubic_function
  static csVector4 NormalizedCubicSolver(float A, float B, float C)
  {
    csVector4 roots;

    if (fabs(C) < EPSILON)	//	x = 0 solution
    {
      roots = QuadraticSolver(1, A, B);
      roots[ (int)roots.w ] = 0;
      roots.w ++;
    }
    else
    {
      roots = csVector4();

      float Q = (3 * B - A * A) / 9;
      float R = (9 * A * B - 27 * C - 2 * A * A * A) / 54;
      float D = Q * Q * Q + R * R;

      if (D > 0)	// 1 root
      {
        float sqrtD = sqrt(D);
        float s = SIGN(R + sqrtD) * pow(fabs(R + sqrtD), 1.0f / 3.0f);
        float t = SIGN(R - sqrtD) * pow(fabs(R - sqrtD), 1.0f / 3.0f);

        roots[0] = (-A / 3 + (s + t));
        roots[3] = 1;
      }
      else	// 3 roots
      {
        float theta = acos(R / sqrt(-(Q * Q * Q)));
        float sqrtQ = sqrt(-Q);
        roots[0] = (2 * sqrtQ * cos(theta / 3) - A / 3);
        roots[1] = (2 * sqrtQ * cos((theta + 2 * PI) / 3) - A / 3);
        roots[2] = (2 * sqrtQ * cos((theta + 4 * PI) / 3) - A / 3);
        roots[3] = 3;
      }
    }

    return roots;
  }

  // Solve a * x ^ 3 + b * x ^ 2 + c * x  + d = 0 - http://en.wikipedia.org/wiki/Cubic_function
  static csVector4 CubicSolver(float a, float b, float c, float d)
  {
    csVector4 roots;

    if (fabs(a) < EPSILON)
      roots = QuadraticSolver(b, c, d);
    else
      roots = NormalizedCubicSolver(b / a, c / a, d / a);

    return roots;
  }

  // Solve o(p,y) - phi = 0
  static csVector4 Roots(float p, float etaPerpendicular, float phi)
  {

    float c = asin(1 / etaPerpendicular);
    return CubicSolver(-8 * (p * c / (PI * PI * PI)), 0, 
      (6 * p * c / PI - 2), p * PI - phi);
  }

  // computes the derivative of the polynomial relative to h.
  static float InverseFirstDerivate(float p, float etaPerpendicular, float h)
  {
    float gammaI = asin(h);
    float c = asin( 1 / etaPerpendicular );
    float dGamma = (6 * p * c / PI - 2) - 
      3 * 8 * (p * c / (PI * PI * PI)) * gammaI * gammaI;

    return sqrt(1 - h * h) / dGamma;
  }
};

#endif // __FUR_MATERIAL_H__
