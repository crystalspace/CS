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
	/// Temp fur geometry
	csRef<iAnimatedMeshFactory> meshFactory;
	csRef<iAnimatedMeshFactorySubMesh> meshFactorySubMesh;
	iTextureHandle* densitymap;
	iTextureHandle* heightmap;
	csRef<iMaterial> material;
	csRef<iShaderVarStringSet> svStrings;
	float strandWidth;

	/// functions
	void GenerateGuidHairs(iRenderBuffer* indices, iRenderBuffer* vertexes);
	void SynchronizeGuideHairs();
	void GenerateHairStrands(iRenderBuffer* indices, iRenderBuffer* vertexes);
	void SynchronizeHairsStrands();
	void SetDensitymap();
	void SetHeightmap();
	void SetStrandWidth();
	void SetColor(csColor color);
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
}
CS_PLUGIN_NAMESPACE_END(FurMaterial)


#endif // __FUR_MATERIAL_H__
