/*
    Crystal Space 3D engine
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

#ifndef __IWORLD_H__
#define __IWORLD_H__

#include "csutil/scf.h"
#include "iplugin.h"

class csWorld;
class csColor;

SCF_VERSION (iWorld, 0, 1, 0);

/**
 * This interface is the main interface to the 3D engine.
 * The engine is responsible for creating new engine-specific objects
 * such as sectors, things, sprites and so on.
 */
struct iWorld : public iPlugIn
{
  /// KLUDGE: this shouldn't be used when iWorld interface will be complete
  virtual csWorld *GetCsWorld () = 0;
  /// Query the format to load textures (usually this depends on texture manager)
  virtual int GetTextureFormat () = 0;

  /**
   * Create or select a new object library (name can be NULL for world).
   * All new objects will be marked as belonging to this library.
   * You can then delete a whole library at once, for example.
   */
  virtual void SelectLibrary (const char *iName) = 0;
  /// Delete a whole library (all objects that are part of library)
  virtual bool DeleteLibrary (const char *iName) = 0;
  /// Delete all libraries (clear the world)
  virtual void DeleteAll () = 0;

  /// Register a texture to be loaded during Prepare()
  virtual bool RegisterTexture (const char *iName, const char *iFileName,
    csColor *iTransp, int iFlags) = 0;
};

#endif // __IWORLD_H__
