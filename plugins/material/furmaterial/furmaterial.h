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

class FurMaterialFactory : public scfImplementation1<FurMaterialFactory, 
	iGenMeshAnimationControlFactory>
{
public:
	CS_LEAKGUARD_DECLARE(FurMaterialFactory);

	FurMaterialFactory ();

	//-- iGenMeshAnimationControlFactory
	virtual csPtr<iGenMeshAnimationControl> CreateAnimationControl (iMeshObject* mesh);
	virtual const char* Load (iDocumentNode* node);
	virtual const char* Save (iDocumentNode* parent);
};

class FurMaterial : public scfImplementation2<FurMaterial,
	scfFakeInterface<iShaderVariableContext>,
	iFurMaterial>, public CS::ShaderVariableContextImpl
{
  public:
    CS_LEAKGUARD_DECLARE(FurMaterial);
    FurMaterial (FurMaterialType* manager, const char *name,
	  iObjectRegistry* object_reg);
    virtual ~FurMaterial ();

    // From iFurMaterial.
    virtual void DoSomething (int param, const csVector3&);
    virtual int GetSomething () const;

	virtual void GenerateGeometry (iView *view,iSector *room, 
	  int controlPoints, int numberOfStrains, float length);
	virtual void GenerateGeometry (iView* view, iSector *room, 
	  csRefArray<iBulletSoftBody> hairStrands);

	/*
	// From iShaderVariableContext
	virtual void AddVariable (csShaderVariable *variable);
	virtual void Clear ();
	virtual const csRefArray <csShaderVariable>& GetShaderVariables () const;
	virtual csShaderVariable* GetVariable (CS::ShaderVarStringID name) const;
	csShaderVariable* GetVariableAdd (CS::ShaderVarStringID name);
	virtual bool IsEmpty () const;
	virtual void PushVariables (csShaderVariableStack &stack) const;
	virtual bool RemoveVariable (CS::ShaderVarStringID name);
	virtual bool RemoveVariable (csShaderVariable *variable);
	virtual void ReplaceVariable (csShaderVariable *variable);
    */

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
	csVector3 store_v;
	/// Shader associated with material
	csHash<csRef<iShader>, csStringID> shaders;
	/// Fur geometry
	csRef<iGeneralFactoryState> factoryState;
	csRef<iView> view;
};

class FurMaterialControl : public scfImplementation1 
    <FurMaterialControl, iGenMeshAnimationControl>
{
  public:
	CS_LEAKGUARD_DECLARE(FurMaterialAnimationControl);

    FurMaterialControl (iMeshObject* mesh);
    virtual ~FurMaterialControl ();

    //-- general factory state
	virtual void SetGeneralFactoryState (iGeneralFactoryState* factory);
	virtual void SetHairStrands (csRefArray<iBulletSoftBody> hairStrands);
	virtual void SetView (iView* view);

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
	/// Fur geometry
	csRef<iGeneralFactoryState> factoryState;
	csRef<iView> view;
	csRefArray<iBulletSoftBody> hairStrands;
};

}
CS_PLUGIN_NAMESPACE_END(FurMaterial)

#endif // __FUR_MATERIAL_H__
