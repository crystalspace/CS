/*
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

#include "csgfxldr/rgbpixel.h"
#include "csobject/csobject.h"
#include "csobject/pobject.h"
#include "csobject/nobjvec.h"
#include "imater.h"

class csTextureWrapper;
struct iTextureManager;

/**
 * A material class.
 */
class csMaterial : public iMaterial
{
private:
  /// flat shading color
  csRGBcolor flat_color;
  /// the texture of the material (can be NULL)
  csTextureWrapper *texture;

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
  csMaterial ();
  /**
   * create a material with only the texture given.
   */
  csMaterial (csTextureWrapper *txt);

  /**
   * destroy material
   */
  virtual ~csMaterial ();

  /// Get the flat shading color
  inline csRGBcolor& GetFlatColor () { return flat_color; }
  /// Set the flat shading color
  inline void SetFlatColor (const csRGBcolor& col) { flat_color = col; }

  /// Get the texture (if none NULL is returned)
  inline csTextureWrapper *GetTextureWrapper () const { return texture; }
  /// Set the texture (pass NULL to set no texture)
  inline void SetTextureWrapper (csTextureWrapper *tex) { texture = tex; }

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
  /// Get flat color.
  virtual void GetFlatColor (csRGBpixel &oColor);
  /// Get reflection values (diffuse, ambient, reflection).
  virtual void GetReflection (float &oDiffuse, float &oAmbient,
    float &oReflection);
};

/**
 * csMaterialWrapper represents a texture and its link
 * to the iMaterialHandle as returned by iTextureManager.
 */
class csMaterialWrapper : public csPObject
{
private:
  /// The corresponding iMaterial.
  iMaterial* material;
  /// The handle as returned by iTextureManager.
  iMaterialHandle* handle;

public:
  /// Construct a material handle given a material.
  csMaterialWrapper (iMaterial* Image);

  /**
   * Construct a csMaterialWrapper from a pre-registered AND prepared material
   * handle. The engine takes over responsibility for destroying the material
   * handle. To prevent this IncRef () the material handle.
   */
  csMaterialWrapper (iMaterialHandle *ith);

  /// Copy constructor
  csMaterialWrapper (csMaterialWrapper &th);
  /// Release material handle
  virtual ~csMaterialWrapper ();

  /// Get the material handle.
  iMaterialHandle* GetMaterialHandle () { return handle; }

  /// Change the base material
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

  CSOBJTYPE;
  DECLARE_IBASE;

  //------------------- iMaterialWrapper implementation -----------------------
  struct MaterialWrapper : public iMaterialWrapper
  {
    DECLARE_EMBEDDED_IBASE (csMaterialWrapper);
    virtual iMaterialHandle* GetMaterialHandle ()
    {
      return scfParent->GetMaterialHandle ();
    }
  } scfiMaterialWrapper;
};

/**
 * This class is used to hold a list of materials.
 */
class csMaterialList : public csNamedObjVector
{
public:
  /// Initialize the array
  csMaterialList () : csNamedObjVector (16, 16)
  { }
  /// Destroy every material in the list
  virtual ~csMaterialList ();

  /// Create a new material.
  csMaterialWrapper* NewMaterial (iMaterial* material);

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed.
   */
  csMaterialWrapper* NewMaterial (iMaterialHandle *ith);

  /// Return material by index
  csMaterialWrapper *Get (int idx)
  { return (csMaterialWrapper *)csNamedObjVector::Get (idx); }

  /// Find a material by name
  csMaterialWrapper *FindByName (const char* iName)
  { return (csMaterialWrapper *)csNamedObjVector::FindByName (iName); }
};

#endif // __CS_MATERIAL_H__
