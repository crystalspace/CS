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

#ifndef __CS_ISO_MATERIAL_H__
#define __CS_ISO_MATERIAL_H__

#include "csgfx/rgbpixel.h"
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "iutil/strset.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
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
  /// the texture of the material (can be 0)
  csRef<iTextureHandle> texture;

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

  /// Get the texture (if none 0 is returned)
  inline iTextureHandle *GetTextureHandle () const { return texture; }
  /// Set the texture (pass 0 to set no texture)
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

  SCF_DECLARE_IBASE;

  //--------------------- iMaterial implementation ---------------------

  /// Get texture.
  virtual iTextureHandle* GetTexture ();
  virtual iTextureHandle* GetTexture (csStringID name);
  /// Number of texture layers.
  virtual int GetTextureLayerCount () { return 0; }
  /// Get a texture layer.
  virtual csTextureLayer* GetTextureLayer (int) { return 0; }
  /// Set the flat shading color
  virtual void SetFlatColor (const csRGBcolor& col) { flat_color = col; }
  /// Get flat color.
  virtual void GetFlatColor (csRGBpixel &oColor, bool useTextureMean=1);
  /// Get reflection values (diffuse, ambient, reflection).
  virtual void GetReflection (float &oDiffuse, float &oAmbient,
    float &oReflection);
  /// Set reflection values (diffuse, ambient, reflection).
  virtual void SetReflection (float oDiffuse, float oAmbient,
    float oReflection)
  {
    diffuse = oDiffuse;
    ambient = oAmbient;
    reflection = oReflection;
  }

  // @@@NR@@@
  virtual void SetShader (csStringID, iShader*) {}
  virtual iShader* GetShader (csStringID) { return 0; }
  virtual void AddVariable (csShaderVariable *variable) {}
  virtual csShaderVariable* GetVariable (csStringID name) const { return 0; }
  virtual void PushVariables (csShaderVarStack &stacks) const {}
  virtual void PopVariables (csShaderVarStack &stacks) const {}
};

/**
 * csIsoMaterialWrapper represents a texture and its link
 * to the iMaterialHandle as returned by iTextureManager.
 */
class csIsoMaterialWrapper : public csObject
{
private:
  /// The corresponding iMaterial.
  csRef<iMaterial> material;
  /// The handle as returned by iTextureManager.
  csRef<iMaterialHandle> handle;
  /// the material number (index in the materiallist)
  int index;

public:
  /// Construct a material handle given a material.
  csIsoMaterialWrapper (iMaterial* Image);

  /**
   * Construct a csIsoMaterialWrapper from a pre-registered AND prepared
   * material handle. The engine takes over responsibility for destroying
   * the material handle. To prevent this IncRef () the material handle.
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
  bool IsVisitRequired () const { return false; }

  /// Get the material index
  int GetIndex() const {return index;}
  /// Set the material index
  void SetIndex(int i) {index = i;}

  SCF_DECLARE_IBASE_EXT (csObject);

  //------------------- iMaterialWrapper implementation -----------------------
  struct MaterialWrapper : public iMaterialWrapper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csIsoMaterialWrapper);
    csIsoMaterialWrapper* GetPrivateObject () { return scfParent; }
    virtual iMaterialWrapper *Clone () const
    {
      return &(new csIsoMaterialWrapper (*scfParent))->scfiMaterialWrapper;
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
    virtual bool IsVisitRequired () const
    {
      return scfParent->IsVisitRequired ();
    }
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
    SCF_DECLARE_EMBEDDED_IBASE (csIsoMaterialWrapper);
    virtual int GetIndex() const {return scfParent->GetIndex();}
    virtual void SetIndex(int i) {scfParent->SetIndex(i);}
  } scfiIsoMaterialWrapperIndex;
};

/**
 * This class is used to hold a list of materials.
 */
class csIsoMaterialList : public csRefArray<csIsoMaterialWrapper>
{
  /// the last possibly free index
  int lastindex;
  /// get an unused index (0), expands the array if necessary
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

  /// Find a material by name
  csIsoMaterialWrapper *FindByName (const char* iName);

  /// remove 'index' from the list. Does DecRef().
  void RemoveIndex(int i);

  SCF_DECLARE_IBASE;

  //------------------- iMaterialList implementation -----------------------
  struct MaterialList : public iMaterialList
  {
    SCF_DECLARE_EMBEDDED_IBASE (csIsoMaterialList);
    virtual iMaterialWrapper* NewMaterial (iMaterial* material)
    {
      csIsoMaterialWrapper* mw = scfParent->NewMaterial (material);
      if (mw) return &(mw->scfiMaterialWrapper);
      else return 0;
    }
    virtual iMaterialWrapper* NewMaterial (iMaterialHandle *ith)
    {
      csIsoMaterialWrapper* mw = scfParent->NewMaterial (ith);
      if (mw) return &(mw->scfiMaterialWrapper);
      else return 0;
    }
    virtual int Add (iMaterialWrapper *imw)
    {
      csIsoMaterialWrapper* mw = scfParent->NewMaterial (
      	imw->GetMaterialHandle ());
      if (mw) return Find (&(mw->scfiMaterialWrapper));
      return -1;
    }
    virtual bool Remove (int idx)
    {
      scfParent->RemoveIndex (idx);
      return true;
    }

    virtual bool Remove (iMaterialWrapper *imw)
    {
      int idx = Find (imw);
      if (idx != -1)
	Remove (idx);
      return idx != -1;
    }

    virtual void RemoveAll ()
    {
      for (int i=GetCount ()-1; i>=0; i--)
	scfParent->RemoveIndex (i);
    }
    virtual int GetCount () const
    {
      return scfParent->Length ();
    }
    virtual iMaterialWrapper* Get (int idx) const
    {
      CS_ASSERT (idx >= 0 && idx < GetCount ());
      return &(scfParent->Get (idx)->scfiMaterialWrapper);
    }
    virtual iMaterialWrapper* FindByName (const char* iName) const
    {
      csIsoMaterialWrapper* mw = scfParent->FindByName (iName);
      if (mw) return &(mw->scfiMaterialWrapper);
      else return 0;
    }
    virtual int Find (iMaterialWrapper *imw) const
    {
      return scfParent->Find (((csIsoMaterialWrapper::MaterialWrapper*)imw)->
      	GetPrivateObject ());
    }
  } scfiMaterialList;
};

#endif // __CS_ISO_MATERIAL_H__
