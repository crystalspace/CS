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
#include "csutil/scf_implementation.h"
#include "csutil/leakguard.h"
#include "ivideo/graph2d.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "igraphic/image.h"

class csTextureWrapper;
struct iTextureManager;
struct iTextureHandle;
struct iImage;

/**
 * csTextureWrapper represents a texture and its link
 * to the iTextureHandle as returned by iTextureManager.
 */
class csTextureWrapper : public scfImplementationExt1<csTextureWrapper,
                                                      csObject,
                                                      iTextureWrapper>
{
private:
  /// The corresponding iImage.
  csRef<iImage> image;
  /**
   * Keep image. If this flag is true the image will not be removed
   * after registering to the texture manager. This is important
   * if you want to be able to modify the texture later.
   * By default images are removed when they are no longer needed.
   */
  bool keep_image;
  /// The handle as returned by iTextureManager.
  csRef<iTextureHandle> handle;
  // key color
  int key_col_r, key_col_g, key_col_b;
  /// Texture registration flags
  int flags;
  char* texClass;

  // The callback which is called just before texture is used.
  csRef<iTextureCallback> use_callback;

  // update our key color with that from the handle
  void UpdateKeyColorFromHandle ()
  {
    if (handle && handle->GetKeyColor ())
    {
      uint8 r, g, b;
      handle->GetKeyColor (r, g, b);
      SetKeyColor ((int)r, (int)g, (int)b);
    }
    else
      key_col_r = -1;
  }

  // update our key color with that from the image
  void UpdateKeyColorFromImage ()
  {
    if (image->HasKeyColor ())
      image->GetKeyColor( key_col_r, key_col_g, key_col_b );
    else
      key_col_r = -1;
  }

public:
  CS_LEAKGUARD_DECLARE (csTextureWrapper);

  /// Construct a texture handle given a image file
  csTextureWrapper (iImage* Image);
  /// Construct a csTextureWrapper from a pre-registered texture
  csTextureWrapper (iTextureHandle *ith);
  /// Copy constructor
  csTextureWrapper (const csTextureWrapper &c);

  /// Release texture handle
  virtual ~csTextureWrapper ();


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
   * the new texture and the iImage to 0.
   */
  void SetTextureHandle (iTextureHandle *tex);
  /// Get the texture handle.
  iTextureHandle* GetTextureHandle () { return handle; }

  /// Set the transparent color.
  void SetKeyColor (int red, int green, int blue);
  /// Query the transparent color.
  void GetKeyColor (int &red, int &green, int &blue) const
  { red = key_col_r; green = key_col_g; blue = key_col_b; }

  /// Set the flags which are used to register the texture
  void SetFlags (int flags) { csTextureWrapper::flags = flags; }
  /// Return the flags which are used to register the texture
  int GetFlags () const { return flags; }

  /// Register the texture with the texture manager
  void Register (iTextureManager *txtmng);

  /**
   * Set a callback which is called just before the texture is used.
   * This is mainly useful for procedural textures which can then
   * choose to update their image.
   */
  void SetUseCallback (iTextureCallback* callback)
  {
    use_callback = callback;
  }

  /**
   * Get the use callback. If there are multiple use callbacks you can
   * use this function to chain.
   */
  iTextureCallback* GetUseCallback () const
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
      use_callback->UseTexture (this);
  }

  bool IsVisitRequired () const
  {
    return use_callback != 0;
  }

  void SetTextureClass (const char* className);
  const char* GetTextureClass ();



  //-------------------- iTextureWrapper implementation -----------------------
  virtual iObject *QueryObject() {return this; }
  virtual iTextureWrapper *Clone () const
  {
    return new csTextureWrapper (*this);
  }

  virtual void SetKeepImage (bool k) { keep_image = k; }
  virtual bool KeepImage () const { return keep_image; }

};

/**
 * This class is used to hold a list of textures.
 */
class csTextureList : public csRefArrayObject<iTextureWrapper>
{
public:
  /// Initialize the array
  csTextureList ();
  virtual ~csTextureList ();

  /// Create a new texture.
  iTextureWrapper *NewTexture (iImage *image);

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed
   */
  iTextureWrapper *NewTexture (iTextureHandle *ith);

  SCF_DECLARE_IBASE;

  //-------------------- iTextureList implementation -------------------------
  class TextureList : public iTextureList
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csTextureList);

    virtual iTextureWrapper *NewTexture (iImage *image);
    virtual iTextureWrapper *NewTexture (iTextureHandle *ith);
    virtual int GetCount () const;
    virtual iTextureWrapper *Get (int n) const;
    virtual int Add (iTextureWrapper *obj);
    virtual bool Remove (iTextureWrapper *obj);
    virtual bool Remove (int n);
    virtual void RemoveAll ();
    virtual int Find (iTextureWrapper *obj) const;
    virtual iTextureWrapper *FindByName (const char *Name) const;
  } scfiTextureList;
};

#endif // __CS_TEXTURE_H__
