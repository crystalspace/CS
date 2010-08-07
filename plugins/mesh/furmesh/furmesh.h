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
  
    /// geometry access
    virtual void SetVertexCount (uint n);
    virtual void SetTriangleCount (uint n);
  
    virtual uint GetVertexCount();
    virtual uint GetIndexCount();

    virtual iRenderBuffer* GetIndices ();
    virtual bool SetIndices (iRenderBuffer* renderBuffer);
    virtual iRenderBuffer* GetVertices ();
    virtual bool SetVertices (iRenderBuffer* renderBuffer);
    virtual iRenderBuffer* GetTexCoords ();
    virtual bool SetTexCoords (iRenderBuffer* renderBuffer);
    virtual iRenderBuffer* GetNormals ();
    virtual bool SetNormals (iRenderBuffer* renderBuffer);
    virtual iRenderBuffer* GetTangents ();
    virtual bool SetTangents (iRenderBuffer* renderBuffer);
    virtual iRenderBuffer* GetBinormals ();
    virtual bool SetBinormals (iRenderBuffer* renderBuffer);

  protected:
    uint indexCount;
    uint vertexCount;
    csRef<iRenderBuffer> indexBuffer;
    csRef<iRenderBuffer> vertexBuffer;
    csRef<iRenderBuffer> texcoordBuffer;
    csRef<iRenderBuffer> normalBuffer;
    csRef<iRenderBuffer> tangentBuffer;
    csRef<iRenderBuffer> binormalBuffer;
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
  public:
    CS_LEAKGUARD_DECLARE(FurMesh);
    FurMesh (iEngine* engine, iObjectRegistry* object_reg, 
      iMeshObjectFactory* object_factory);
    virtual ~FurMesh ();

    // From iMeshObject
    virtual iMeshObjectFactory* GetFactory () const;

    virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
      csVector3& isect, float* pr, int* polygon_idx,
      iMaterialWrapper** material, iMaterialArray* materials);

    virtual void NextFrame (csTicks current_time,const csVector3& pos,
      uint currentFrame);
    
    virtual bool SetMaterialWrapper (iMaterialWrapper* );

    virtual iMaterialWrapper* GetMaterialWrapper () const;

    virtual void UpdateObjectBoundingBox();

    virtual CS::Graphics::RenderMesh** GetRenderMeshes (int& num, iRenderView*, 
      iMovable*, uint32);

    /// From iRenderBufferAccessor
    virtual void PreGetBuffer(csRenderBufferHolder* holder, 
      csRenderBufferName buffer);

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

  private:
    csRef<iMaterialWrapper> materialWrapper;
    csDirtyAccessArray<csRenderMesh*> renderMeshes;
    iObjectRegistry* object_reg;
    iMeshObjectFactory* object_factory;
    csRef<iFurMeshFactory> factory;
    /// Model
    iEngine* engine;
    csRef<iLoader> loader;
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
    /// update
    void Update();
    void UpdateGuideHairs();
    void UpdateControlPoints(csVector3 *controlPoints, size_t controlPointsCount, 
      csGuideHairReference guideHairs[GUIDE_HAIRS_COUNT]);
  };

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

#endif // __FUR_MESH_H__
