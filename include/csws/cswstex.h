/*
    Crystal Space Windowing System: Windowing System Texture class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSWSTEX_H__
#define __CS_CSWSTEX_H__

/**
 * \addtogroup csws
 * @{ */
 
#include "csextern.h"
 
#include "csutil/parray.h"
#include "igraphic/image.h"

struct iTextureHandle;
struct iTextureManager;

/**
 * Texture representation within the windowing system.
 * The application keeps an array of such objects, and they are registered
 * with the 3D and 2D drivers as soon as app->RegisterTextures() is called.
 */
class CS_CRYSTALSPACE_EXPORT csWSTexture
{
  // Reference count
  int RefCount;
  // texture image
  csRef<iImage> Image;
  // Will be this texture used for 3D and/or for 2D operations?
  int Flags;
  // Red,Green and Blue components of key color
  uint8 tr, tg, tb;
  // texture has key color?
  bool HasKey;
  // Has transparent color changed?
  bool KeyChanged;
  // texture handle for the 3D/2D driver
  csRef<iTextureHandle> Handle;
  // texture name
  char *Name;
  // VFS file name
  char *FileName;

public:
  /// Create the 2D texture
  csWSTexture (const char *iName, iImage *Image, int iFlags);
  /// Destroy the texture object
  ~csWSTexture ();
  /// Set texture transparent color
  void SetKeyColor (int iR, int iG, int iB);
  /// Set whenever texture has transparent holes or not
  void SetKeyColor (bool iEnable);
  /// Get texture transparent color
  void GetKeyColor (int &oR, int &oG, int &oB) const
  { oR = tr; oG = tg; oB = tb; }
  /// Query whenever texture has transparent areas
  bool HasKeyColor () const
  { return HasKey; }
  /// Register the texture with texture manager
  void Register (iTextureManager *iTexMan);
  /// Unregister the texture
  void Unregister ();
  /// Refresh the texture in video memory.
  void Refresh ();
  /// Define texture name
  void SetName (const char *iName);
  /// Get texture name
  const char *GetName () const
  { return Name; }
  /// CSWS uses its own reference counting system.
  iTextureHandle *GetHandle ()
  { return Handle; }
  /// Increment reference count to this texture
  void IncRef ()
  { RefCount++; }
  /// Delete a reference to this texture
  void DecRef ()
  { RefCount--; }
  /// Return reference count
  int GetRefCount () const
  { return RefCount; }
  /// Get texture file name
  const char *GetFileName () const
  { return FileName; }
  /// Set texture file name
  void SetFileName (const char *iFileName);
  /// Find nearest transparent color in image
  void FixKeyColor ();
  /// Tell texture that transparent color has already been fixed
  void DontFixKeyColor ()
  { KeyChanged = false; }
  /// Query texture width
  int GetWidth ();
  /// Query texture height
  int GetHeight ();
};

/// This class is a vector of csWSTexture's
class CS_CRYSTALSPACE_EXPORT csWSTexVector : public csPDelArray<csWSTexture>
{
public:
  /// Initialize the texture vector
  csWSTexVector ();
  /// Compare texture with name; used in FindKey ()
  static int CompareKey (csWSTexture* const&, char const* const& Key);
  /// Find a texture by name
  csWSTexture *FindTexture (const char *name) const
  {
    size_t idx = FindKey (csArrayCmp<csWSTexture*,char const*>(name, CompareKey));
    return idx != csArrayItemNotFound ? Get (idx) : (csWSTexture*)0;
  }
};

#endif // __CS_CSWSTEX_H__
