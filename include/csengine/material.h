/*
    Copyright (C) 2001 by Jorrit Tyberghein
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
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "ivideo/effects/efdef.h"

#ifdef CS_USE_NEW_RENDERER
  #include "csengine/engine.h"
  #include "csutil/symtable.h"
#endif

struct iTextureWrapper;
struct iTextureManager;

SCF_VERSION (csMaterialWrapper, 0, 0, 1);

/**
 * A hash that stores ref-counted objects, exactly one
 * object per hash key.
 */
template <class T>
class csRefHash : csHashMap
{
public:
  csRefHash ()
  { }

  ~csRefHash ()
  {
    csGlobalHashIterator it (this);

    while (it.HasNext ())
    {
      T* obj = (T*)it.Next ();
      if (obj != 0) obj->DecRef ();
    }
  }

  void Put (csHashKey key, T* object)
  {
    T* oldobj = (T*)csHashMap::Get (key);
    if (oldobj != 0) oldobj->DecRef ();
    Delete (key, (csHashObject)oldobj);
    if (object != 0) object->IncRef ();
    csHashMap::Put (key, (csHashObject)object);
  }

  T* Get (csHashKey key) const
  {
    return (T*)csHashMap::Get (key);
  }
};

/**
 * A material class.
 */
class csMaterial : public iMaterial
{
private:
  friend class csEngine;

  /// flat shading color
  csRGBcolor flat_color;
#ifndef CS_USE_NEW_RENDERER
  /// the texture of the material (can be 0)
  csRef<iTextureWrapper> texture;
  /// Number of texture layers (currently maximum 4).
  int num_texture_layers;
  /// Optional texture layer.
  csTextureLayer texture_layers[4];
  /// Texture wrappers for texture layers.
  csRef<iTextureWrapper> texture_layer_wrappers[4];

  /// The diffuse reflection value of the material
  float diffuse;
  /// The ambient lighting of the material
  float ambient;
  /// The reflectiveness of the material
  float reflection;
  
  /// The effect associated with this material
  iEffectDefinition* effect;
#endif

#ifdef CS_USE_NEW_RENDERER
  /// Shader assoiciated with material
  csHashMap* shaders;

  csSymbolTable symtab;

  csEngine* engine;

  csStringID nameDiffuseParam;
  csStringID nameAmbientParam;
  csStringID nameReflectParam;
  csStringID nameFlatColorParam;
  csStringID nameDiffuseTexture;

  csShaderVariable* GetVar (csStringID name, bool create = false);
#endif

public:
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

  /// Get the flat shading color
  csRGBcolor& GetFlatColor ();

  /// Get diffuse reflection constant for the material
  float GetDiffuse ();
  /// Set diffuse reflection constant for the material
  void SetDiffuse (float val);

  /// Get ambient lighting for the material
  float GetAmbient ();
  /// Set ambient lighting for the material
  void SetAmbient (float val);

  /// Get reflection of the material
  float GetReflection ();
  /// Set reflection of the material
  void SetReflection (float val);

#ifdef CS_USE_NEW_RENDERER
  /// Get the base diffuse texture (if none 0 is returned)
  iTextureWrapper* GetTextureWrapper ()
  {
    return GetTextureWrapper (nameDiffuseTexture);
  }
#else
  /// Get the base diffuse texture (if none 0 is returned)
  iTextureWrapper* GetTextureWrapper () const { return texture; }
#endif
  /// Set the base diffuse texture (pass 0 to set no texture)
  void SetTextureWrapper (iTextureWrapper* tex);

#ifndef CS_USE_NEW_RENDERER
  /// Add a texture layer (currently only one supported).
  void AddTextureLayer (iTextureWrapper* txtwrap, uint mode,
        float uscale, float vscale, float ushift, float vshift);
#else
  /// Get a texture (if none 0 is returned)
  iTextureWrapper* GetTextureWrapper (csStringID name);
  /// Set a texture (pass 0 to set no texture)
  void SetTextureWrapper (csStringID name, iTextureWrapper* tex);
#endif

  //--------------------- iMaterial implementation ---------------------

#ifdef CS_USE_NEW_RENDERER
  /// Associate a shader with a shader type
  virtual void SetShader (csStringID type, iShaderWrapper* shader);
  /// Get shader associated with a shader type
  virtual iShaderWrapper* GetShader (csStringID type);

  virtual void AddChild (iShaderBranch *c) {
    csRef<iShaderWrapper> w = SCF_QUERY_INTERFACE (c, iShaderWrapper);
    if (w) w->SelectMaterial (this);
    symtab.AddChild (c->GetSymbolTable ());
  }
  virtual void AddVariable (csShaderVariable *v)
    { symtab.SetSymbol (v->GetName (), v); }
  virtual csShaderVariable* GetVariable (csStringID name)
    { return (csShaderVariable *) symtab.GetSymbol (name); }
  virtual csSymbolTable* GetSymbolTable () { return & symtab; }
  virtual csSymbolTable* GetSymbolTable (int i) { return & symtab; }
  virtual void SelectSymbolTable (int i) {}
#endif

#ifndef CS_USE_NEW_RENDERER
  /// Set effect.
  virtual void SetEffect (iEffectDefinition *ed);
  /// Get effect.
  virtual iEffectDefinition *GetEffect ();
#endif
  /// Get texture.
  virtual iTextureHandle* GetTexture ();
#ifdef CS_USE_NEW_RENDERER
  /**
   * Get a texture from the material.
   */
  virtual iTextureHandle *GetTexture (csStringID name);
  /**
   * Set a texture of the material.
   */
  virtual void SetTexture (csStringID name, iTextureHandle* texture);
#endif
#ifndef CS_USE_NEW_RENDERER
  /// Get num texture layers.
  virtual int GetTextureLayerCount ();
  /// Get a texture layer.
  virtual csTextureLayer* GetTextureLayer (int idx);
#endif
  /// Get flat color.
  virtual void GetFlatColor (csRGBpixel &oColor, bool useTextureMean = true);
  /// Set the flat shading color
  virtual void SetFlatColor (const csRGBcolor& col);
  /// Get reflection values (diffuse, ambient, reflection).
  virtual void GetReflection (float &oDiffuse, float &oAmbient,
    float &oReflection);
  /// Set reflection values (diffuse, ambient, reflection).
  virtual void SetReflection (float oDiffuse, float oAmbient,
    float oReflection);

  /**
   * Visit all textures.
   */
  void Visit ();
  bool IsVisitRequired () const;

  SCF_DECLARE_IBASE;

  /// iMaterialEngine implementation
  struct MaterialEngine : public iMaterialEngine
  {
    SCF_DECLARE_EMBEDDED_IBASE (csMaterial);
    virtual iTextureWrapper *GetTextureWrapper ()
    {
#ifdef CS_USE_NEW_RENDERER
      return scfParent->GetTextureWrapper (
	scfParent->nameDiffuseTexture);
#else
      return scfParent->GetTextureWrapper ();
#endif
    }
#ifndef CS_USE_NEW_RENDERER
    virtual iTextureWrapper* GetTextureWrapper (int idx)
    {
      return scfParent->texture_layer_wrappers[idx];
    }
#else
    virtual iTextureWrapper* GetTextureWrapper (csStringID name)
    {
      return scfParent->GetTextureWrapper (name);
    }
#endif
    virtual void Visit ()
    {
      scfParent->Visit ();
    }
    virtual bool IsVisitRequired () const
    {
      return scfParent->IsVisitRequired ();
    }
  } scfiMaterialEngine;
  friend struct MaterialEngine;
};

/**
 * csMaterialWrapper represents a texture and its link
 * to the iMaterialHandle as returned by iTextureManager.
 */
class csMaterialWrapper : public csObject
{
private:
  /// The corresponding iMaterial.
  csRef<iMaterial> material;
  /// The corresponding iMaterialEngine.
  csRef<iMaterialEngine> matEngine;
  /// The handle as returned by iTextureManager.
  csRef<iMaterialHandle> handle;

private:
  /// Release material handle
  virtual ~csMaterialWrapper ();

public:
  /// Construct a material handle given a material.
  csMaterialWrapper (iMaterial* Image);
  /// Construct a csMaterialWrapper from a pre-registered material handle.
  csMaterialWrapper (iMaterialHandle *ith);
  /// Copy constructor
  csMaterialWrapper (csMaterialWrapper &);

  /**
   * Change the material handle. Note: This will also change the base
   * material to 0.
   */
  void SetMaterialHandle (iMaterialHandle *mat);
  /// Get the material handle.
  iMaterialHandle* GetMaterialHandle () { return handle; }

  /**
   * Change the base material. Note: The changes will not be visible until
   * you re-register the material.
   */
  void SetMaterial (iMaterial* material);
  /// Get the original material.
  iMaterial* GetMaterial () { return material; }

  /// Register the material with the texture manager
  void Register (iTextureManager *txtmng);

  /**
   * Visit this material. This should be called by the engine right
   * before using the material. It will call Visit() on all textures
   * that are used.
   */
  void Visit ();
  bool IsVisitRequired () const;

  SCF_DECLARE_IBASE_EXT (csObject);

  //------------------- iMaterialWrapper implementation -----------------------
  struct MaterialWrapper : public iMaterialWrapper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csMaterialWrapper);
    virtual iObject *QueryObject()
    { return scfParent; }
    virtual iMaterialWrapper *Clone () const
    { return &(new csMaterialWrapper (*scfParent))->scfiMaterialWrapper; }
    virtual void SetMaterialHandle (iMaterialHandle *mat)
    { scfParent->SetMaterialHandle (mat); }
    virtual iMaterialHandle* GetMaterialHandle ()
    { return scfParent->GetMaterialHandle (); }
    virtual void SetMaterial (iMaterial* material)
    { scfParent->SetMaterial (material); }
    virtual iMaterial* GetMaterial ()
    { return scfParent->GetMaterial (); }
    virtual void Register (iTextureManager *txtmng)
    { scfParent->Register (txtmng); }
    virtual void Visit ()
    { scfParent->Visit (); }
    virtual bool IsVisitRequired () const
    {
      return scfParent->IsVisitRequired ();
    }
  } scfiMaterialWrapper;
  friend struct MaterialWrapper;
};

/**
 * This class is used to hold a list of materials.
 */
class csMaterialList : public csRefArrayObject<iMaterialWrapper>
{
public:
  /// Initialize the array
  csMaterialList ();
  virtual ~csMaterialList () { }

  /// Create a new material.
  iMaterialWrapper* NewMaterial (iMaterial* material);

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed.
   */
  iMaterialWrapper* NewMaterial (iMaterialHandle *ith);

  SCF_DECLARE_IBASE;

  //------------------- iMaterialList implementation -----------------------
  class MaterialList : public iMaterialList
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csMaterialList);

    virtual iMaterialWrapper* NewMaterial (iMaterial* material);
    virtual iMaterialWrapper* NewMaterial (iMaterialHandle *ith);
    virtual int GetCount () const;
    virtual iMaterialWrapper *Get (int n) const;
    virtual int Add (iMaterialWrapper *obj);
    virtual bool Remove (iMaterialWrapper *obj);
    virtual bool Remove (int n);
    virtual void RemoveAll ();
    virtual int Find (iMaterialWrapper *obj) const;
    virtual iMaterialWrapper *FindByName (const char *Name) const;
  } scfiMaterialList;
};

#endif // __CS_MATERIAL_H__
