/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef __CS_TEXTURE_H__
#define __CS_TEXTURE_H__

#include "cstypes.h"
#include "csobject/csobject.h"
#include "csobject/pobject.h"
#include "csobject/nobjvec.h"
#include "ivideo/graph2d.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"

class csTextureWrapper;
struct iTextureManager;
struct iTextureHandle;
struct iImage;

/**
 * csTextureWrapper represents a texture and its link
 * to the iTextureHandle as returned by iTextureManager.
 */
class csTextureWrapper : public csPObject
{
private:
  /// The corresponding iImage.
  iImage* image;
  /// The handle as returned by iTextureManager.
  iTextureHandle* handle;
  // key color
  int key_col_r, key_col_g, key_col_b;
  // The callback which is called just before texture is used.
  csTextureCallback* use_callback;
  // User-data for the callback.
  void* use_data;

public:
  /// Texture registration flags
  int flags;

  /// Construct a texture handle given a image file
  csTextureWrapper (iImage* Image);

  /**
   * Construct a csTextureWrapper from a pre-registered AND prepared texture 
   * handle. The engine takes over responsibility for destroying the texture
   * handle. To prevent this IncRef () the texture handle.
   */
  csTextureWrapper (iTextureHandle *ith);

  /// Copy constructor
  csTextureWrapper (csTextureWrapper &th);
  /// Release texture handle
  virtual ~csTextureWrapper ();

  /// Get the texture handle.
  iTextureHandle* GetTextureHandle () { return handle; }

  /// Change the base iImage
  void SetImageFile (iImage *Image);
  /// Get the iImage.
  iImage* GetImageFile () { return image; }

  /// Set the transparent color.
  void SetKeyColor (int red, int green, int blue);

  /// Query the transparent color.
  void GetKeyColor (int &red, int &green, int &blue)
  { red = key_col_r; green = key_col_g; blue = key_col_b; }

  /// Register the texture with the texture manager
  void Register (iTextureManager *txtmng);

  /**
   * Set a callback which is called just before the texture is used.
   * This is mainly useful for procedural textures which can then
   * choose to update their image.
   */
  void SetUseCallback (csTextureCallback* callback, void* data)
  { use_callback = callback; use_data = data; }

  /**
   * Get the use callback. If there are multiple use callbacks you can
   * use this function to chain.
   */
  csTextureCallback* GetUseCallback () { return use_callback; }

  /**
   * Get the use data.
   */
  void* GetUseData () { return use_data; }

  /**
   * Visit this texture. This should be called by the engine right
   * before using the texture. It is responsible for calling the use
   * callback if there is one.
   */
  void Visit () { if (use_callback) use_callback (&scfiTextureWrapper, use_data); }

  CSOBJTYPE;
  DECLARE_OBJECT_INTERFACE;
  DECLARE_IBASE;

  //-------------------- iTextureWrapper implementation -----------------------
  struct TextureWrapper : public iTextureWrapper
  {
    DECLARE_EMBEDDED_IBASE (csTextureWrapper);

    virtual csTextureWrapper *GetPrivateObject() const;
    virtual iObject *QueryObject();
    virtual iTextureHandle *GetTextureHandle() const;
    virtual void SetKeyColor (int red, int green, int blue);
    virtual void GetKeyColor (int &red, int &green, int &blue) const;
    virtual void Register (iTextureManager *txtmng);
    virtual void SetUseCallback (csTextureCallback* callback, void* data);
    virtual csTextureCallback* GetUseCallback () const;
    virtual void* GetUseData () const;
  } scfiTextureWrapper;
};

/**
 * This class is used to hold a list of textures.
 */
class csTextureList : public csNamedObjVector
{
public:
  /// Initialize the array
  csTextureList ();
  /// Destroy every texture in the list
  virtual ~csTextureList ();

  /// Create a new texture.
  csTextureWrapper *NewTexture (iImage *image);

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed
   */
  csTextureWrapper *NewTexture (iTextureHandle *ith);

  /// Return texture by index
  csTextureWrapper *Get (int idx)
  { return (csTextureWrapper *)csNamedObjVector::Get (idx); }

  /// Find a texture by name
  csTextureWrapper *FindByName (const char* iName)
  { return (csTextureWrapper *)csNamedObjVector::FindByName (iName); }

  DECLARE_IBASE;

  //-------------------- iTextureList implementation -------------------------
  struct TextureList : public iTextureList
  {
    DECLARE_EMBEDDED_IBASE (csTextureList);

    virtual iTextureWrapper *NewTexture (iImage *image);
    virtual iTextureWrapper *NewTexture (iTextureHandle *ith);
    virtual long GetNumTextures () const;
    virtual iTextureWrapper *Get (int idx) const;
    virtual iTextureWrapper *FindByName (const char* iName) const;
  } scfiTextureList;
};

#endif // __CS_TEXTURE_H__
