/*
    Copyright (C) 2001-2006 by Jorrit Tyberghein
    Copyright (C) 2000 by W.C.A. Wijngaards

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_MATERIAL_H__
#define __CS_MATERIAL_H__

#include "csgfx/rgbpixel.h"
#include "csutil/scf_implementation.h"
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "csutil/hash.h"
#include "iutil/selfdestruct.h"
#include "csutil/leakguard.h"
#include "ivideo/material.h"
#include "iengine/material.h"

#include "plugins/engine/3d/engine.h"

struct iTextureWrapper;
struct iTextureManager;

#include "csutil/deprecated_warn_off.h"

/**
 * A material class.
 */
class csMaterial :
  public scfImplementation3<csMaterial, 
                            iMaterial, 
                            iMaterialEngine,
                            scfFakeInterface<iShaderVariableContext> >,
  public CS::ShaderVariableContextImpl
{
private:
  friend class csEngine;

  /// Shader associated with material
  csHash<csRef<iShader>, csStringID> shaders;
  csEngine* engine;
  csShaderVariable* GetVar (CS::ShaderVarStringID name, bool create = false);

  struct SVNamesHolder
  {
    CS::ShaderVarName diffuseTex;
  };
  CS_DECLARE_STATIC_CLASSVAR_REF(svNames, SVNames, SVNamesHolder);

  void SetupSVNames();
public:
  CS_LEAKGUARD_DECLARE (csMaterial);

  /**
   * create an empty material
   */
  csMaterial (csEngine* engine);
  /**
   * create a material with only the texture given.
   */
  csMaterial (csEngine* engine,
    iTextureWrapper *txt);

  /**
   * destroy material
   */
  virtual ~csMaterial ();

  /// Get the base diffuse texture (if none 0 is returned)
  iTextureWrapper* GetTextureWrapper ()
  {
    return GetTextureWrapper (SVNames().diffuseTex);
  }

  /// Set the base diffuse texture (pass 0 to set no texture)
  void SetTextureWrapper (iTextureWrapper* tex);

  /// Set a texture (pass 0 to set no texture)
  void SetTextureWrapper (CS::ShaderVarStringID name, iTextureWrapper* tex);

  /// Get a texture (if none 0 is returned)
  iTextureWrapper* GetTextureWrapper (CS::ShaderVarStringID name);

  /**\name iMaterial implementation
   * @{ */
  /// Associate a shader with a shader type
  virtual void SetShader (csStringID type, iShader* shader);
  /// Get shader associated with a shader type
  virtual iShader* GetShader (csStringID type);

  /// Get all Shaders
  const csHash<csRef<iShader>, csStringID>& GetShaders() const
  { return shaders; }

  /// Get texture.
  virtual iTextureHandle* GetTexture ();
  /**
   * Get a texture from the material.
   */
  virtual iTextureHandle *GetTexture (CS::ShaderVarStringID name);

  virtual iShader* GetFirstShader (const csStringID* types,
    size_t numTypes);

  /** @} */

  /**
   * Visit all textures.
   */
  void Visit ();
  bool IsVisitRequired () const;
};

#include "csutil/deprecated_warn_on.h"

/**
 * csMaterialWrapper represents a texture and its link
 * to the iMaterialHandle as returned by iTextureManager.
 */
class csMaterialWrapper : public scfImplementationExt2<csMaterialWrapper,
						       csObject,
						       iMaterialWrapper,
						       iSelfDestruct>
{
private:
  /// The corresponding iMaterial.
  csRef<iMaterial> material;
  /// The corresponding iMaterialEngine.
  csRef<iMaterialEngine> matEngine;

  iMaterialList* materials;

protected:
  virtual void InternalRemove() { SelfDestruct(); }

private:
  /// Release material handle
  virtual ~csMaterialWrapper ();

public:
  CS_LEAKGUARD_DECLARE (csMaterialWrapper);

  /// Construct a material handle given a material.
  csMaterialWrapper (iMaterialList* materials, iMaterial* Image);

  /**
   * Change the base material. Note: The changes will not be visible until
   * you re-register the material.
   */
  virtual void SetMaterial (iMaterial* material);
  /// Get the original material.
  virtual iMaterial* GetMaterial () { return material; }

  /**
   * Visit this material. This should be called by the engine right
   * before using the material. It will call Visit() on all textures
   * that are used.
   */
  virtual void Visit ();
  virtual bool IsVisitRequired () const;

  //------------------- iMaterialWrapper implementation -----------------------

  virtual iObject *QueryObject() { return (iObject*)this; }

  //------------------- iSelfDestruct implementation -----------------------

  virtual void SelfDestruct ();
};

/**
 * This class is used to hold a list of materials.
 */
class csMaterialList : public scfImplementation1<csMaterialList,
                                                 iMaterialList>
{
private:
  csRefArrayObject<iMaterialWrapper> list;
  csHash<iMaterialWrapper*, csString> mat_hash;
  mutable CS::Threading::ReadWriteMutex matLock;

  class NameChangeListener : public scfImplementation1<NameChangeListener,
  	iObjectNameChangeListener>
  {
  private:
    csWeakRef<csMaterialList> list;

  public:
    NameChangeListener (csMaterialList* list) : scfImplementationType (this),
  	  list (list)
    {
    }
    virtual ~NameChangeListener () { }

    virtual void NameChanged (iObject* obj, const char* oldname,
  	  const char* newname)
    {
      if (list)
        list->NameChanged (obj, oldname, newname);
    }
  };
  csRef<NameChangeListener> listener;

public:
  /// Initialize the array
  csMaterialList ();
  virtual ~csMaterialList ();

  void NameChanged (iObject* object, const char* oldname,
  	const char* newname);

  virtual iMaterialWrapper* NewMaterial (iMaterial* material,
  	const char* name);
  virtual csPtr<iMaterialWrapper> CreateMaterial (iMaterial* material,
  	const char* name);

  virtual int GetCount () const;
  virtual iMaterialWrapper* Get (int n) const;
  virtual int Add (iMaterialWrapper *obj);
  void AddBatch (csRef<iMaterialLoaderIterator> itr);
  virtual bool Remove (iMaterialWrapper *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iMaterialWrapper *obj) const;
  virtual iMaterialWrapper *FindByName (const char *Name) const;
};

#endif // __CS_MATERIAL_H__
