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

#ifndef __FUR_MESH_H__
#define __FUR_MESH_H__

#include <iutil/comp.h>
#include <csgeom/vector3.h>
#include <imesh/furmesh.h>
#include <csgfx/shadervarcontext.h>
#include <imesh/genmesh.h>

#include "crystalspace.h"

#include "csutil/scf_implementation.h"

struct iObjectRegistry;

#define GUIDE_HAIRS_COUNT 3

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  class FurMeshFactory : public scfImplementationExt1<FurMeshFactory, 
    csMeshFactory, iFurMeshFactory>
  {
  public:
    CS_LEAKGUARD_DECLARE(FurMeshFactory);

    FurMeshFactory (iEngine *e, iObjectRegistry* reg, iMeshObjectType* type);
    virtual ~FurMeshFactory ();

    virtual csPtr<iMeshObject> NewInstance ();
    virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  };

  class FurMeshType : public 
    scfImplementation2<FurMeshType,iFurMeshType,iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(FurMeshType);

    FurMeshType (iBase* parent);
    virtual ~FurMeshType ();

    // From iComponent	
    virtual bool Initialize (iObjectRegistry*);

    // From iMeshObjectType
    virtual csPtr<iMeshObjectFactory> NewFactory ();

  private:
    iObjectRegistry* object_reg;
    /// pointer to the engine if available.
    iEngine *Engine;
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
    float tipRatio;

    csGuideHairReference guideHairs[GUIDE_HAIRS_COUNT];
  };

  struct csGuideHair
  {
    csVector3 *controlPoints;
    size_t controlPointsCount;
    csVector2 uv;
  };

  struct csGuideHairLOD : csGuideHair
  {
    csGuideHairReference guideHairs[GUIDE_HAIRS_COUNT];
    bool isActive;  //  ropes vs interpolate
  };

  struct TextureData
  {
    iTextureHandle* handle;
    int width, height;
    uint8* data;
  };

  class FurMesh : public scfImplementationExt1<FurMesh, csMeshObject, iFurMesh>
  {
    friend class FurAnimationControl;

  public:
    CS_LEAKGUARD_DECLARE(FurMesh);
    FurMesh (iObjectRegistry* object_reg);
    virtual ~FurMesh ();

    virtual iMeshObjectFactory* GetFactory () const {return 0;}
    virtual CS::Graphics::RenderMesh** GetRenderMeshes (
      int& num, iRenderView* rview, iMovable* movable,
      uint32 frustum_mask) {return 0;}

    virtual bool HitBeamOutline (const csVector3& start,
      const csVector3& end, csVector3& isect, float* pr) {return false;}

    virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
      csVector3& isect, float* pr, int* polygon_idx,
      iMaterialWrapper** material, iMaterialArray* materials) {return false;}

    // From iFurMesh
    virtual void GenerateGeometry (iView* view, iSector *room);
    virtual void SetGuideLOD(float guideLOD);
    virtual void SetStrandLOD(float strandLOD);
    virtual void SetLOD(float lod);

    virtual void SetPhysicsControl (iFurPhysicsControl* physicsControl);
    virtual void StartPhysicsControl ( );
    virtual void StopPhysicsControl ( );

    // Temporary - Set Mesh and Submesh
    virtual void SetMeshFactory ( iAnimatedMeshFactory* meshFactory);
    virtual void SetMeshFactorySubMesh ( iAnimatedMeshFactorySubMesh* 
      meshFactorySubMesh );
    // Set Material
    virtual void SetMaterial ( iMaterial* material );
    // Set HairStrandGenerator
    virtual void SetFurStrandGenerator( iFurStrandGenerator* hairStrandGenerator);
    // Get HairStrandGenerator
    virtual iFurStrandGenerator* GetFurStrandGenerator( );

  protected:
    FurMeshType* manager;
    csString name;

  private:
    iObjectRegistry* object_reg;
    /// Fur geometry
    csRef<iGeneralFactoryState> factoryState;
    csRef<iView> view;
    csArray<csHairStrand> hairStrands;
    csArray<csGuideHair> guideHairs;
    csArray<csTriangle> guideHairsTriangles;
    csArray<csGuideHairLOD> guideHairsLOD;
    csRef<iFurPhysicsControl> physicsControl;
    csRef<iFurStrandGenerator> hairStrandGenerator;
    csVector3* positionShift;
    csRandomGen *rng;
    float guideLOD;
    float strandLOD;
    size_t hairStrandsLODSize;
    bool physicsControlEnabled;
    /// Temp fur geometry
    csRef<iAnimatedMeshFactory> meshFactory;
    csRef<iAnimatedMeshFactorySubMesh> meshFactorySubMesh;
    csRef<iMaterial> material;
    csRef<iShaderVarStringSet> svStrings;
    /// Density & Height maps
    TextureData densitymap;
    float densityFactorGuideHairs;
    float densityFactorHairStrands;
    TextureData heightmap;
    float heightFactor;
    float displaceDistance;
    float strandWidth;
    float strandWidthLOD;
    float controlPointsDistance;
    float positionDeviation;
    int growTangents;
    /// Model
    csRef<iEngine> engine;
    csRef<iLoader> loader;
    /// functions
    void SetRigidBody (iRigidBody* rigidBody);
    void GenerateGuideHairs(iRenderBuffer* indices, iRenderBuffer* vertexes,
      iRenderBuffer* normals, iRenderBuffer* texCoords);
    void SynchronizeGuideHairs();
    void GenerateGuideHairsLOD();
    void GenerateHairStrands();
    float TriangleDensity(csGuideHair A, csGuideHair B, csGuideHair C);
    /// debug
    void GaussianBlur(TextureData texture);
    void SaveUVImage();
    void SaveImage(uint8* buf, const char* texname,int width, int height);
    /// setters
    void SetGrowTangents();
    void SetDensitymap();
    void SetHeightmap();
    void SetStrandWidth();
    void SetColor(csColor color);
    void SetDisplaceDistance();
  };

  class FurAnimationControl : public scfImplementation1 
    <FurAnimationControl, iGenMeshAnimationControl>
  {
  public:
    CS_LEAKGUARD_DECLARE(FurAnimationControl);

    FurAnimationControl (FurMesh* furMesh);
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
    FurMesh* furMesh;
    // functions
    void UpdateGuideHairs();
    void UpdateControlPoints(csVector3 *controlPoints, size_t controlPointsCount, 
      csGuideHairReference guideHairs[GUIDE_HAIRS_COUNT]);
  };

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

#endif // __FUR_MESH_H__
