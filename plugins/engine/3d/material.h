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
#include "csutil/leakguard.h"
#include "ivideo/material.h"
#include "iengine/material.h"

#include "plugins/engine/3d/engine.h"

struct iTextureWrapper;
struct iTextureManager;

SCF_VERSION (csMaterialWrapper, 0, 0, 1);

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
#endif

  /// Shader associated with material
  csHash<csRef<iShader>, csStringID> shaders;
  csEngine* engine;

#ifdef CS_USE_NEW_RENDERER
  csShaderVariable* GetVar (csStringID name, bool create = false);
#endif

  static csStringID nameDiffuseParam;
  static csStringID nameAmbientParam;
  static csStringID nameReflectParam;
  static csStringID nameFlatColorParam;
  static csStringID nameDiffuseTexture;

  static csStringID nameTextureLayer1;
  static csStringID nameTextureLayer2;
  static csStringID nameTextureLayer3;
  static csStringID nameTextureLayer4;

  csShaderVariableContext svcontext;

public:
  CS_LEAKGUARD_DECLARE (csMaterial);

  /**
   * @@@ Slight hack: when the engine creates a material, it implicitly sets
   * the "OR compatibility" shader. When later a shader is set and 
   * shadersCustomized is false, the "OR compatibility" shader will be set to
   * 0. This is done so users can "unset" the OR compar shader.
   */
  bool shadersCustomized;

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
  /// Set a texture (pass 0 to set no texture)
  void SetTextureWrapper (csStringID name, iTextureWrapper* tex);
#endif
  /// Get a texture (if none 0 is returned)
  iTextureWrapper* GetTextureWrapper (csStringID name);

  //--------------------- iMaterial implementation ---------------------

  /// Associate a shader with a shader type
  virtual void SetShader (csStringID type, iShader* shader);
  /// Get shader associated with a shader type
  virtual iShader* GetShader (csStringID type);

  /// Get texture.
  virtual iTextureHandle* GetTexture ();
  /**
   * Get a texture from the material.
   */
  virtual iTextureHandle *GetTexture (csStringID name);
#ifndef CS_USE_NEW_RENDERER
  /// Get num texture layers.
  virtual int GetTextureLayerCount ();
  /// Get a texture layer.
  virtual csTextureLayer* GetTextureLayer (int idx);
#else
  /// Get num texture layers. OR only.
  virtual int GetTextureLayerCount () { return 0; }
  /// Get a texture layer. OR only.
  virtual csTextureLayer* GetTextureLayer (int) { return 0; }
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
      return scfParent->GetTextureWrapper ();
    }
    virtual iTextureWrapper* GetTextureWrapper (csStringID name)
    {
      return scfParent->GetTextureWrapper (name);
    }
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


  //=================== iShaderVariableContext ================//

  /// Add a variable to this context
  void AddVariable (csShaderVariable *variable)
  { svcontext.AddVariable (variable); }

  /// Get a named variable from this context
  csShaderVariable* GetVariable (csStringID name) const
  { return svcontext.GetVariable (name); }

  /**
  * Push the variables of this context onto the variable stacks
  * supplied in the "stacks" argument
  */
  void PushVariables (csShaderVarStack &stacks) const
  { svcontext.PushVariables (stacks); }

  /**
  * Pop the variables of this context off the variable stacks
  * supplied in the "stacks" argument
  */
  void PopVariables (csShaderVarStack &stacks) const
  { svcontext.PopVariables (stacks); }
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
  CS_LEAKGUARD_DECLARE (csMaterialWrapper);

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
  virtual ~csMaterialList ();

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
