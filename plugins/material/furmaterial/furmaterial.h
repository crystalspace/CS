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
#include <imaterial/furinterf.h>
#include <csgfx/shadervarcontext.h>

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

  class FurMaterial : public scfImplementation2<FurMaterial,
	  scfFakeInterface<iShaderVariableContext>,iFurMaterial>,
	  public CS::ShaderVariableContextImpl
  {
  public:
    CS_LEAKGUARD_DECLARE(FurMaterial);
    FurMaterial (FurMaterialType* manager, const char *name);
    virtual ~FurMaterial ();

    // From iFurMaterial.
    virtual void DoSomething (int param, const csVector3&);
    virtual int GetSomething () const;

    virtual void SetLength (float len);
    virtual float GetLength () const;

    virtual void SetColor (const csColor4& color);
    virtual csColor4 GetColor () const;
	
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
	  csVector3 store_v;
	  /// Shader associated with material
	  csHash<csRef<iShader>, csStringID> shaders;
  };

}
CS_PLUGIN_NAMESPACE_END(FurMaterial)

#endif // __FUR_MATERIAL_H__
