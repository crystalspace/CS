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

// alpha.h: interface for the csAlphaHandle class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ALPHA_H
#define ALPHA_H
#include "types.h"
#include "csobject/csobj.h"
#include "csgfxldr/boxfilt.h"
#include "igraph2d.h"

class csWorld;
class AlphaMapFile;
interface IAlphaMapHandle;

/**
 * csTextureHandle represents a texture and its link
 * to the ITextureHandle as returned by ITextureManager.
 */
class csAlphaMapHandle : public csObject
{
private:
  /// The corresponding ImageFile.
	AlphaMapFile* afile;
  /// The handle as returned by ITextureManager.
  IAlphaMapHandle* alpha_handle;
  // Transparent color
public:
  /// Construct a texture handle given a image file
  csAlphaMapHandle (AlphaMapFile* alphamap);
  /// Copy contstructor
  csAlphaMapHandle (csAlphaMapHandle &ah);
  /// Release texture handle
  virtual ~csAlphaMapHandle ();

  /// Get the texture handle.
  IAlphaMapHandle* GetAlphaHandle () { return alpha_handle; }

  /// Set the texture handle.
  void SetAlphaHandle (IAlphaMapHandle* h);

  /// Get the ImageFile.
  AlphaMapFile* GetAlphaMapFile () { return afile; }

  CSOBJTYPE;
};


/**
 * This class maintains all named textures and their
 * corresponding handles.
 */
class csAlphaMapList
{
private:
  /// List of textures.
  csAlphaMapHandle** alphamaps;
  ///
  int num_alphamaps;
  ///
  int max_alphamaps;

private:
  ///
  int GetAlphaMapIdx (const char* name);

public:
  /// Add a texture
  void AddTexture (csAlphaMapHandle* tm);

public:
  ///
  csAlphaMapList ();
  ///
  virtual ~csAlphaMapList ();

  ///
  void Clear ();

  /// Create a new texture.
  csAlphaMapHandle* NewAlphaMap(AlphaMapFile* alphamap);

  /// Return number of textures
  int GetNumAlphaMaps () { return num_alphamaps; }

  /// Return texture by index
  csAlphaMapHandle* GetAlphaMapMM (int idx) { return alphamaps[idx]; }

  /// Find a texture given a name.
  csAlphaMapHandle* GetAlphaMapMM (const char* name);
};


#endif /*TEXTURE_H*/
