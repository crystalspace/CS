/*
    Copyright (C) 2001-2002 by Jorrit Tyberghein

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

struct iCacheManager;
struct iDynLight;
struct iStatLight;

SCF_VERSION (iLightingInfo, 0, 1, 3);

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
   */
  virtual bool ReadFromCache (iCacheManager* cache_mgr) = 0;

  /**
   * Write the lighting information to the cache. Returns false if there
   * was a problem. This function will write the data to the current VFS
   * dir.
   */
  virtual bool WriteToCache (iCacheManager* cache_mgr) = 0;

  /**
   * Finally prepare the lighting for use. This function must be called
   * last.
   */
  virtual void PrepareLighting () = 0;

  /**
   * Sets dynamic ambient light for this object.
   */
  virtual void SetDynamicAmbientLight (const csColor& color) = 0;

  /**
   * Get dynamic ambient light.
   */
  virtual const csColor& GetDynamicAmbientLight () = 0;

  /**
   * Get dynamic ambient light version to test if needs to be recalculated.
   */
  virtual uint32 GetDynamicAmbientVersion () const = 0;

  /**
   * Indicate that some dynamic light has changed. This function will
   * be called by the lighting system whenever a dynamic light that
   * affects this mesh is changed in some way.
   */
  virtual void DynamicLightChanged (iDynLight* dynlight) = 0;

  /**
   * Indicate that some dynamic light no longer affects this mesh.
   */
  virtual void DynamicLightDisconnect (iDynLight* dynlight) = 0;

  /**
   * Indicate that some pseudo-dynamic light has changed color.
   */
  virtual void StaticLightChanged (iStatLight* statlight) = 0;
};

#endif // __IMESH_LIGHTING_H__

