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

#ifndef __CS_TXTMGR_H__
#define __CS_TXTMGR_H__

/**\file
 * Texture manager base implementation
*/

#include "csextern.h"
#include "csgfx/rgbpixel.h"
#include "csutil/parray.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakrefarr.h"
#include "iengine/material.h"
#include "iengine/texture.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "ivideo/shader/shader.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

class csTexture;
class csTextureManager;
struct iImage;
struct iConfigFile;
struct iGraphics2D;
struct iObjectRegistry;

/**
 * This class is the top-level representation of a texture.
 * It contains a number of csTexture objects that represents
 * each a single image. A csTexture object is created for
 * each mipmap and for the 2D texture. This class is responsible
 * for creating these textures and filling them with the correct
 * info. The csTextureHandle class is private to the 3D driver, the
 * driver clients see just the iTextureHandle interface.
 * <p>
 * The handle is initialized by giving the 3D driver a iImage object.
 * Later the renderer will create mipmaps and 2D textures are created.
 * The texture manager will release its reference to the image when no
 * longer needed.
 */
class CS_CRYSTALSPACE_EXPORT csTextureHandle : 
  public scfImplementation1<csTextureHandle, iTextureHandle>
{
protected:
  /// Parent texture manager
  csRef<csTextureManager> texman;

  /// Texture usage flags: 2d/3d/etc
  int flags;

  /// Does color 0 mean "transparent" for this texture?
  bool transp;
  /// The transparent color
  csRGBpixel transp_color;

  csStringID texClass;
  csAlphaMode::AlphaType alphaType;
public:
  ///
  csTextureHandle (csTextureManager* texman, int Flags);
  ///
  virtual ~csTextureHandle ();

  /**
   * Adjusts the textures size, to ensure some restrictions like
   * power of two dimension are met.
   */
  void AdjustSizePo2 (int width, int height, int depth,
    int& newwidth, int& newheight, int& newdepth);

  ///--------------------- iTextureHandle implementation ----------------------

  int GetFlags () const { return flags; }

  /// Enable transparent color
  virtual void SetKeyColor (bool Enable);

  /// Set the transparent color.
  virtual void SetKeyColor (uint8 red, uint8 green, uint8 blue);

  /**
   * Get the transparent status (false if no transparency, true if
   * transparency).
   */
  virtual bool GetKeyColor () const;

  /// Get the transparent color
  virtual void GetKeyColor (uint8 &r, uint8 &g, uint8 &b) const;

  /// Get the csTextureHandle object associated with the texture handle
  virtual void *GetPrivateObject ()
  { return (csTextureHandle *)this; }

  /**
   * Query if the texture has an alpha channel.<p>
   * This depends both on whenever the original image had an alpha channel
   * and of the fact whenever the renderer supports alpha maps at all.
   */
  virtual bool GetAlphaMap () 
  { return false; }

  /**
   * Given a texture width and height, it tries to 'guesstimate' the po2 size
   * that causes the least quality reduction: it calculates how many 
   * rows/columns would be added/removed when sizing up/down, and takes the 
   * one with the smaller number. In case of a tie, it'll size up. 
   */
  static void CalculateNextBestPo2Size (int flags, const int orgDim, 
    int& newDim);

  virtual csAlphaMode::AlphaType GetAlphaType () const
  { return alphaType; }
  virtual void SetAlphaType (csAlphaMode::AlphaType alphaType)
  { this->alphaType = alphaType; }

  virtual void SetTextureClass (const char* className);
  virtual const char* GetTextureClass ();
};

/**
 * General version of the texture manager.
 * Each 3D driver should derive a texture manager class from this one
 * and implement the missing functionality.
 */
class CS_CRYSTALSPACE_EXPORT csTextureManager : 
  public scfImplementation1<csTextureManager, iTextureManager>
{
protected:

  //typedef csArray<csTextureHandle*> csTexVector;
  typedef csWeakRefArray<csTextureHandle> csTexVector;

  /// List of textures.
  csTexVector textures;

  ///
  iObjectRegistry *object_reg;

  /// Read configuration values from config file.
  virtual void read_config (iConfigFile *config);
public:
  /// Pixel format.
  csPixelFormat pfmt;

  csStringID nameDiffuseTexture;

  csStringSet texClassIDs;

  /// Initialize the texture manager
  csTextureManager (iObjectRegistry* object_reg, iGraphics2D *iG2D);
  /// Destroy the texture manager
  virtual ~csTextureManager ();

  /// Clear (free) all textures
  virtual void Clear ()
  {
    textures.DeleteAll ();
  }
};

#endif // __CS_TXTMGR_H__
