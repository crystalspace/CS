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

#ifndef __FUR_MESH_H__
#define __FUR_MESH_H__

#include <imesh/furmesh.h>

#include "furmeshfactory.h"
#include "furdata.h"
#include "furmeshproperties.h"

#include "crystalspace.h"
#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  class FurMesh : public 
    scfImplementationExt2<FurMesh, csMeshObject, FurMeshState, CS::Mesh::iFurMesh>, 
    public FurMeshGeometry
  {
  public:
    CS_LEAKGUARD_DECLARE(FurMesh);
    FurMesh (iEngine* engine, iObjectRegistry* object_reg, 
      iMeshObjectFactory* object_factory);
    virtual ~FurMesh ();

    //-- csMeshObject
    virtual iMeshObjectFactory* GetFactory () const;

    virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
      csVector3& isect, float* pr, int* polygon_idx,
      iMaterialWrapper** baseMaterial, iMaterialArray* materials);

    virtual void NextFrame (csTicks current_time,const csVector3& pos,
      uint currentFrame);
    
    virtual bool SetMaterialWrapper (iMaterialWrapper* );
    virtual iMaterialWrapper* GetMaterialWrapper () const;

    virtual void UpdateObjectBoundingBox();

    virtual CS::Graphics::RenderMesh** GetRenderMeshes (int& num, iRenderView*, 
      iMovable*, uint32);

    // LOD function
    virtual void SetIndexRange (uint indexstart, uint indexend);
    
    //-- iFurMesh
    virtual void GenerateGeometry (iView* view, iSector *room);
    virtual void SetGuideLOD(float guideLOD);
    virtual void SetStrandLOD(float strandLOD);
    virtual void SetControlPointsLOD(float controlPointsLOD);
    virtual void SetLOD(float lod);

    virtual void SetAnimatedMesh(CS::Mesh::iAnimatedMesh* animesh);
    virtual void SetAnimationControl (CS::Animation::iFurAnimationControl* physicsControl);
    virtual void StartAnimationControl ( );
    virtual void StopAnimationControl ( );
    virtual void EnableMesh ( );
    virtual void ResetMesh ( );
    virtual void DisableMesh ( );

    virtual void SetMeshFactory ( CS::Mesh::iAnimatedMeshFactory* meshFactory);
    virtual void SetMeshFactorySubMesh 
      ( CS::Mesh::iAnimatedMeshSubMeshFactory* meshFactorySubMesh );

    virtual void SetFurMeshProperties
      ( CS::Mesh::iFurMeshMaterialProperties* hairMeshProperties);
    virtual CS::Mesh::iFurMeshMaterialProperties* GetFurMeshProperties( ) const;

  private:
    // Common data
    csRef<iMaterialWrapper> materialWrapper;
    csDirtyAccessArray<csRenderMesh*> renderMeshes;
    iObjectRegistry* object_reg;
    iMeshObjectFactory* object_factory;
    iEngine* engine;
    csRef<CS::Mesh::iFurMeshState> state;
    // Fur data
    bool isEnabled;
    csRef<iView> view;
    csArray<csFurStrand> furStrands;
    csArray<csGuideFur> guideFurs;
    csArray<csTriangle> guideFursTriangles;
    csArray<csGuideFurLOD> guideFursLOD;
    CS::Mesh::iAnimatedMesh* animesh;
    csRef<CS::Animation::iFurAnimationControl> physicsControl;
    csRef<CS::Mesh::iFurMeshMaterialProperties> hairMeshProperties;
    csVector3* furStrandShift;
    csVector3* positionShift;
    csRandomGen *rng;
    float controlPointsLOD;
    size_t offsetIndex;
    size_t offsetVertex;
    size_t endVertex;
    float guideLOD;
    float previousGuideLOD;
    float strandLOD;
    size_t furStrandsLODSize;
    bool physicsControlEnabled;
    bool isReset;
    uint startFrame;
    float controlPointsDeviation;
    // External data
    csRef<CS::Mesh::iAnimatedMeshFactory> meshFactory;
    csRef<CS::Mesh::iAnimatedMeshSubMeshFactory> meshFactorySubMesh;
    csRef<iShaderVarStringSet> svStrings;
    // Density and height maps
    csTextureRGBA densitymap;
    csTextureRGBA heightmap;
    float strandWidthLOD;
    // Render mesh data
    csRef<csRenderBufferHolder> bufferholder;
    csRef<csShaderVariableContext> svContext;
    uint indexstart, indexend;
    // Private functions
    size_t GetControlPointsCount(float controlPointsLOD) const;
    void SetRigidBody (iRigidBody* rigidBody);
    void GenerateGuideFurs();
    void SynchronizeGuideHairs();
    void GenerateGuideFursLOD();
    void GenerateFurStrands();
    void TriangleAreaDensity(const csTriangle& triangle, float &area, 
      float &density, csGuideFur& A, csGuideFur& B, csGuideFur& C);
    void RegenerateGeometry();
    // For debug
    void SaveUVImage();
    // Update
    void Update();
    void UpdateGuideHairs();
  };

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

#endif // __FUR_MESH_H__
