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

#ifndef __CSISO_MATERIAL_H__
#define __CSISO_MATERIAL_H__

#include "csgfx/rgbpixel.h"
#include "csobject/csobject.h"
#include "csobject/pobject.h"
#include "csobject/nobjvec.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "ivaria/iso.h"

class csTextureWrapper;
struct iTextureManager;

/**
 * A material class.
 */
class csIsoMaterial : public iMaterial
{
private:
  /// flat shading color
  csRGBcolor flat_color;
  /// the texture of the material (can be NULL)
  iTextureHandle *texture;

  /// The diffuse reflection value of the material
  float diffuse;
  /// The ambient lighting of the material
  float ambient;
  /// The reflectiveness of the material
  float reflection;

public:
  /**
   * create an empty material
   */
  csIsoMaterial ();
  /**
   * create a material with only the texture given.
   */
  csIsoMaterial (iTextureHandle *txt);

  /**
   * destroy material
   */
  virtual ~csIsoMaterial ();

  /// Get the flat shading color
  inline csRGBcolor& GetFlatColor () { return flat_color; }
  /// Set the flat shading color
  inline void SetFlatColor (const csRGBcolor& col) { flat_color = col; }

  /// Get the texture (if none NULL is returned)
  inline iTextureHandle *GetTextureHandle () const { return texture; }
  /// Set the texture (pass NULL to set no texture)
  inline void SetTextureHandle (iTextureHandle *tex) { texture = tex; }

  /// Get diffuse reflection constant for the material
  inline float GetDiffuse () const { return diffuse; }
  /// Set diffuse reflection constant for the material
  inline void SetDiffuse (float val) { diffuse = val; }

  /// Get ambient lighting for the material
  inline float GetAmbient () const { return ambient; }
  /// Set ambient lighting for the material
  inline void SetAmbient (float val) { ambient = val; }

  /// Get reflection of the material
  inline float GetReflection () const { return reflection; }
  /// Set reflection of the material
  inline void SetReflection (float val) { reflection = val; }

  DECLARE_IBASE;

  //--------------------- iMaterial implementation ---------------------

  /// Get texture.
  virtual iTextureHandle* GetTexture ();
  /// Number of texture layers.
  virtual int GetNumTextureLayers () { return 0; }
  /// Get a texture layer.
  virtual csTextureLayer* GetTextureLayer (int) { return NULL; }
  /// Get flat color.
  virtual void GetFlatColor (csRGBpixel &oColor);
  /// Get reflection values (diffuse, ambient, reflection).
  virtual void GetReflection (float &oDiffuse, float &oAmbient,
    float &oReflection);
};

/**
 * csIsoMaterialWrapper represents a texture and its link
 * to the iMaterialHandle as returned by iTextureManager.
 */
class csIsoMaterialWrapper : public csPObject
{
private:
  /// The corresponding iMaterial.
  iMaterial* material;
  /// The handle as returned by iTextureManager.
  iMaterialHandle* handle;
  /// the material number (index in the materiallist)
  int index;

public:
  /// Construct a material handle given a material.
  csIsoMaterialWrapper (iMaterial* Image);

  /**
   * Construct a csIsoMaterialWrapper from a pre-registered AND prepared material
   * handle. The engine takes over responsibility for destroying the material
   * handle. To prevent this IncRef () the material handle.
   */
  csIsoMaterialWrapper (iMaterialHandle *ith);

  /// Copy constructor
  csIsoMaterialWrapper (csIsoMaterialWrapper &th);
  /// Release material handle
  virtual ~csIsoMaterialWrapper ();

  /// Get the material handle.
  iMaterialHandle* GetMaterialHandle () { return handle; }
  /// Set the material handle.
  void SetMaterialHandle (iMaterialHandle *hdl);

  /// Change the base material - you must also change the index.
  void SetMaterial (iMaterial* material);
  /// Get the material.
  iMaterial* GetMaterial () { return material; }

  /// Register the material with the texture manager
  void Register (iTextureManager *txtmng);

  /**
   * Visit this material. This should be called by the engine right
   * before using the material. It will call Visit() on all textures
   * that are used.
   */
  void Visit ();

  /// Get the material index
  int GetIndex() const {return index;}
  /// Set the material index
  void SetIndex(int i) {index = i;}

  CSOBJTYPE;
  DECLARE_IBASE_EXT (csPObject);

  //------------------- iMaterialWrapper implementation -----------------------
  struct MaterialWrapper : public iMaterialWrapper
  {
    DECLARE_EMBEDDED_IBASE (csIsoMaterialWrapper);
    //// @@@ cast is wrong! It is an csIsoMaterialWrapper
    virtual csMaterialWrapper* GetPrivateObject ()
    {
      return (csMaterialWrapper*)scfParent;
    }
    virtual void SetMaterialHandle (iMaterialHandle* m)
    {
      scfParent->SetMaterialHandle (m);
    }
    virtual iMaterialHandle* GetMaterialHandle ()
    {
      return scfParent->GetMaterialHandle ();
    }
    virtual iObject* QueryObject ()
    {
      return scfParent;
    }
    virtual void Visit () { scfParent->Visit (); }
    virtual void Register (iTextureManager *mng) { scfParent->Register (mng); }
    virtual void SetMaterial (iMaterial* m)
    {
      scfParent->SetMaterial (m);
    }
    virtual iMaterial* GetMaterial ()
    {
      return scfParent->GetMaterial ();
    }
  } scfiMaterialWrapper;

  //------------------- iIsoMaterialWrapperIndex implementation ------------
  struct IsoMaterialWrapperIndex : public iIsoMaterialWrapperIndex
  {
    DECLARE_EMBEDDED_IBASE (csIsoMaterialWrapper);
    virtual int GetIndex() const {return scfParent->GetIndex();}
    virtual void SetIndex(int i) {scfParent->SetIndex(i);}
  } scfiIsoMaterialWrapperIndex;
};

/**
 * This class is used to hold a list of materials.
 */
class csIsoMaterialList : public csNamedObjVector
{
  /// the last possibly free index
  int lastindex;
  /// get an unused index (NULL), expands the array if necessary
  int GetNewIndex();
public:
  /// Initialize the array
  csIsoMaterialList ();
  /// Destroy every material in the list
  virtual ~csIsoMaterialList ();

  /// Create a new material.
  csIsoMaterialWrapper* NewMaterial (iMaterial* material);

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed.
   */
  csIsoMaterialWrapper* NewMaterial (iMaterialHandle *ith);

  /// Return material by index
  csIsoMaterialWrapper *Get (int idx)
  { return (csIsoMaterialWrapper *)csNamedObjVector::Get (idx); }

  /// Find a material by name
  csIsoMaterialWrapper *FindByName (const char* iName)
  { return (csIsoMaterialWrapper *)csNamedObjVector::FindByName (iName); }

  /// remove 'index' from the list. Does not decref/delete.
  void RemoveIndex(int i);

  DECLARE_IBASE;

  //------------------- iMaterialList implementation -----------------------
  struct MaterialList : public iMaterialList
  {
    DECLARE_EMBEDDED_IBASE (csIsoMaterialList);
    virtual iMaterialWrapper* NewMaterial (iMaterial* material)
    {
      csIsoMaterialWrapper* mw = scfParent->NewMaterial (material);
      if (mw) return &(mw->scfiMaterialWrapper);
      else return NULL;
    }
    virtual iMaterialWrapper* NewMaterial (iMaterialHandle *ith)
    {
      csIsoMaterialWrapper* mw = scfParent->NewMaterial (ith);
      if (mw) return &(mw->scfiMaterialWrapper);
      else return NULL;
    }
    virtual int GetNumMaterials ()
    {
      return scfParent->Length ();
    }
    virtual iMaterialWrapper* Get (int idx)
    {
      return &(scfParent->Get (idx)->scfiMaterialWrapper);
    }
    virtual iMaterialWrapper* FindByName (const char* iName)
    {
      csIsoMaterialWrapper* mw = scfParent->FindByName (iName);
      if (mw) return &(mw->scfiMaterialWrapper);
      else return NULL;
    }
  } scfiMaterialList;
};

#endif // __CSISO_MATERIAL_H__
