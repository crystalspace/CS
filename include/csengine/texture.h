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
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "ivideo/graph2d.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "igraphic/image.h"

class csTextureWrapper;
struct iTextureManager;
struct iTextureHandle;
struct iImage;


SCF_VERSION (csTextureWrapper, 0, 0, 1);

/**
 * csTextureWrapper represents a texture and its link
 * to the iTextureHandle as returned by iTextureManager.
 */
class csTextureWrapper : public csObject
{
private:
  /// The corresponding iImage.
  iImage* image;
  /// The handle as returned by iTextureManager.
  iTextureHandle* handle;
  // key color
  int key_col_r, key_col_g, key_col_b;
  /// Texture registration flags
  int flags;

  // The callback which is called just before texture is used.
  iTextureCallback* use_callback;

  // update our key color with that from the handle
  void UpdateKeyColorFromHandle ()
  {
    if (handle && handle->GetKeyColor ())
    {
      UByte r, g, b;
      handle->GetKeyColor (r, g, b);
      SetKeyColor ((int)r, (int)g, (int)b);
    }
    else
      key_col_r = -1;
  }

  // update our key color with that from the image
  void UpdateKeyColorFromImage ()
  {
    if(image->HasKeycolor ())
      image->GetKeycolor( key_col_r, key_col_g, key_col_b );
    else
      key_col_r = -1;
  }

private:
  /// Release texture handle
  virtual ~csTextureWrapper ();

public:

  /// Construct a texture handle given a image file
  csTextureWrapper (iImage* Image);
  /// Construct a csTextureWrapper from a pre-registered texture
  csTextureWrapper (iTextureHandle *ith);

  /**
   * Change the base iImage. The changes will not be visible until the
   * texture is registered again.
   */
  void SetImageFile (iImage *Image);
  /// Get the iImage.
  iImage* GetImageFile () { return image; }

  /**
   * Change the texture handle. The changes will immediatly be visible. This
   * will also change the key color and registration flags to those of
   * the new texture and the iImage to NULL.
   */
  void SetTextureHandle (iTextureHandle *tex);
  /// Get the texture handle.
  iTextureHandle* GetTextureHandle () { return handle; }

  /// Set the transparent color.
  void SetKeyColor (int red, int green, int blue);
  /// Query the transparent color.
  void GetKeyColor (int &red, int &green, int &blue)
  { red = key_col_r; green = key_col_g; blue = key_col_b; }

  /// Set the flags which are used to register the texture
  void SetFlags (int flags) { csTextureWrapper::flags = flags; }
  /// Return the flags which are used to register the texture
  int GetFlags () { return flags; }

  /// Register the texture with the texture manager
  void Register (iTextureManager *txtmng);

  /**
   * Set a callback which is called just before the texture is used.
   * This is mainly useful for procedural textures which can then
   * choose to update their image.
   */
  void SetUseCallback (iTextureCallback* callback)
  {
    SCF_SET_REF (use_callback, callback);
  }

  /**
   * Get the use callback. If there are multiple use callbacks you can
   * use this function to chain.
   */
  iTextureCallback* GetUseCallback ()
  {
    return use_callback;
  }

  /**
   * Visit this texture. This should be called by the engine right
   * before using the texture. It is responsible for calling the use
   * callback if there is one.
   */
  void Visit ()
  {
    if (use_callback)
      use_callback->UseTexture (&scfiTextureWrapper);
  }

  SCF_DECLARE_IBASE_EXT (csObject);

  //-------------------- iTextureWrapper implementation -----------------------
  struct TextureWrapper : public iTextureWrapper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTextureWrapper);

    virtual csTextureWrapper *GetPrivateObject()
    {return scfParent;}

    virtual iObject *QueryObject();
    virtual void SetImageFile (iImage *Image);
    virtual iImage* GetImageFile ();
    virtual void SetTextureHandle (iTextureHandle *tex);
    virtual iTextureHandle* GetTextureHandle ();
    virtual void SetKeyColor (int red, int green, int blue);
    virtual void GetKeyColor (int &red, int &green, int &blue);
    virtual void SetFlags (int flags);
    virtual int GetFlags ();
    virtual void Register (iTextureManager *txtmng);
    virtual void SetUseCallback (iTextureCallback* callback);
    virtual iTextureCallback* GetUseCallback ();
    virtual void Visit ();
  } scfiTextureWrapper;
  friend struct TextureWrapper;
};

// helper for the texture list
CS_DECLARE_OBJECT_VECTOR (csTextureListHelper, iTextureWrapper);

/**
 * This class is used to hold a list of textures.
 */
class csTextureList : public csTextureListHelper
{
public:
  /// Initialize the array
  csTextureList ();

  /// Create a new texture.
  iTextureWrapper *NewTexture (iImage *image);

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed
   */
  iTextureWrapper *NewTexture (iTextureHandle *ith);

  SCF_DECLARE_IBASE;

  //-------------------- iTextureList implementation -------------------------
  struct TextureList : public iTextureList
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTextureList);

    virtual iTextureWrapper *NewTexture (iImage *image);
    virtual iTextureWrapper *NewTexture (iTextureHandle *ith);
    virtual long GetTextureCount () const;
    virtual iTextureWrapper *Get (int idx) const;
    virtual iTextureWrapper *FindByName (const char* iName) const;
  } scfiTextureList;
};

#endif // __CS_TEXTURE_H__
