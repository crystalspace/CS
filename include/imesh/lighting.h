/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __IMESH_LIGHTING_H__
#define __IMESH_LIGHTING_H__

#include "csutil/scf.h"

SCF_VERSION (iLightingInfo, 0, 0, 2);

/**
 * This interface is implemented by mesh objects that have some kind
 * of lighting system. It has features to initialize lighting, to read
 * it from a cache, ...
 */
struct iLightingInfo : public iBase
{
  /**
   * Initialize the lighting information to some default (mostly black).
   */
  virtual void InitializeDefault () = 0;

  /**
   * Read the lighting information from the cache. Call this instead
   * of InitializeDefault(). Returns false if there was a problem.
   * This function will read the data from the current VFS dir.
   * The id is used to uniquely identify the elements of this cache.
   */
  virtual bool ReadFromCache (int id) = 0;

  /**
   * Write the lighting information to the cache. Returns false if there
   * was a problem. This function will write the data to the current VFS
   * dir. The id is used to uniquely identify the elements of this cache.
   */
  virtual bool WriteToCache (int id) = 0;

  /**
   * Finally prepare the lighting for use. This function must be called
   * last.
   */
  virtual void PrepareLighting () = 0;

  /**
   * Set the name of the lightmap cache to use for this. If not given
   * then the 'id' will be used for uniqueness.
   */
  virtual void SetCacheName (const char* cachename) = 0;

  /**
   * Get the name of the lightmap cache used for this.
   * Returns NULL if none given.
   */
  virtual const char* GetCacheName () const = 0;
};

#endif // __IMESH_LIGHTING_H__

