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

#include "furmeshfactory.h"
#include "crystalspace.h"
#include "csutil/scf_implementation.h"

struct iObjectRegistry;

#define GUIDE_HAIRS_COUNT 3

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
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

    /// some mesh properties
    virtual void SetMixMode (uint mode);
    virtual uint GetMixMode () const;
    virtual void SetPriority (uint priority);
    virtual uint GetPriority () const;
    virtual void SetZBufMode (csZBufMode z_buf_mode);
    virtual csZBufMode GetZBufMode () const;
    virtual void SetIndexRange (uint indexstart, uint indexend);
    
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
    /// render mesh data
    csRef<csRenderBufferHolder> bufferholder;
    csRef<csShaderVariableContext> svContext;
    uint mixmode;
    uint priority;
    csZBufMode z_buf_mode;
    uint indexstart, indexend;
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
