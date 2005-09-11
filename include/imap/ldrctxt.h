/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_IMAP_LDRCTXT_H__
#define __CS_IMAP_LDRCTXT_H__

/**\file
 */
#include "csutil/scf.h"

/**\addtogroup loadsave
 * @{ */
struct iMaterialWrapper;
struct iMeshFactoryWrapper;
struct iMeshWrapper;
struct iSector;
struct iRegion;
struct iTextureWrapper;
struct iLight;
struct iShader;

/**
 * This interface gives the context for the loader.
 * It basically gives loading plugins a way to find materials,
 * meshes, and sectors. In all these cases region-qualified
 * names can be used (i.e. 'regionname/objectname') or normal
 * object names.
 * <p>
 * WARNING! When a context is created it should not be modified
 * afterwards. Some loader plugins will keep a reference to the
 * context when they support delayed loading. In that case they
 * still need the original contents in the context. So a loader
 * that creates a context should create a new one every time.
 */
struct iLoaderContext : public virtual iBase
{
  SCF_INTERFACE(iLoaderContext,2,0,0);
  /// Find a sector.
  virtual iSector* FindSector (const char* name) = 0;
  
  /** Find a material.  If not found, try to use the filename supplied
   *  to find a matching texture.  If that isn't found, try to load
   *  the texture using the filename.  If loaded then it is also
   *  Prepared.
   */
  virtual iMaterialWrapper* FindMaterial (const char* filename) = 0;

  /**
   * Same as FindMaterial but there is no assumption that the name
   * and the filename are the same.  This is useful if the filename
   * has /'s in it (a pathname), since region searching will mean that
   * names with /'s will never be found correctly.
   */
  virtual iMaterialWrapper* FindNamedMaterial (const char* name,
  	const char *filename) = 0;
  /// Find a mesh factory.
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name) = 0;
  /// Find a mesh object.
  virtual iMeshWrapper* FindMeshObject (const char* name) = 0;
  
  /**
   * Find a texture.  If not found, attempt to load and prepare the
   * texture using the supplied filename as the name.
   */
  virtual iTextureWrapper* FindTexture (const char* filename) = 0;

  /**
   * Find a texture with the given name.  If not found, attempt to load
   * the supplied filename and prepare the texture using the supplied name.
   */
  virtual iTextureWrapper* FindNamedTexture (const char* name,
  	const char *filename) = 0;

  /// Find a light
  virtual iLight* FindLight (const char* name) = 0;

  /// Find a shader.
  virtual iShader* FindShader (const char* name) = 0;

  /**
   * Return true if we check for dupes (to avoid objects with same name
   * being loaded again.
   */
  virtual bool CheckDupes () const = 0;

  /**
   * Return a region if we only want to load in that region.
   * 0 otherwise. If not 0 then all objects will be created in the region.
   */
  virtual iRegion* GetRegion () const = 0;

  /**
   * Return true if we only want to look for objects in the region
   * given by GetRegion().
   */
  virtual bool CurrentRegionOnly () const = 0;
};

/** @} */

#endif // __CS_IMAP_LDRCTXT_H__

