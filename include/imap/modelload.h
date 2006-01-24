/*
    Copyright (C) 2006 by Jorrit Tyberghein

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

#ifndef __CS_IMAP_MODELLOAD_H__
#define __CS_IMAP_MODELLOAD_H__

#include "csutil/scf.h"

/**\file
 * Loader plugins
 */
/**\addtogroup loadsave
 * @{ */

struct iMeshFactoryWrapper;
struct iDataBuffer;

/**
 * Some loader plugins implement this as an easier way to load a factory from
 * code.
 *
 * Main creators of instances implementing this interface:
 * - gmesh3ds plugin (crystalspace.mesh.loader.factory.genmesh.3ds)
 * - spr3md2 plugin (crystalspace.mesh.loader.factory.sprite.3d.md2)
 * 
 * Main ways to get pointers to this interface:
 * - csLoadPlugin()
 */
struct iModelLoader : public virtual iBase
{
  SCF_INTERFACE(iModelLoader, 1,0,0);

  /**
   * Create a mesh factory and load the given model into it.
   * \param factname The name of the factory.
   * \param filename The VFS name of the file.
   * \return a mesh factory wrapper on success or otherwise 0. The error
   * will be reported to the reporter.
   */
  virtual iMeshFactoryWrapper* Load (const char* factname,
    const char* filename) = 0;

  /**
   * Create a mesh factory and load the given model into it.
   * \param factname The name of the factory.
   * \param buffer The buffer containing the file.
   * \return a mesh factory wrapper on success or otherwise 0. The error
   * will be reported to the reporter.
   */
  virtual iMeshFactoryWrapper* Load (const char* factname,
    iDataBuffer* buffer) = 0;

  /**
   * Test if the model is recognized by this model loader.
   */
  virtual bool IsRecognized (const char* filename) = 0;

  /**
   * Test if the model is recognized by this model loader.
   */
  virtual bool IsRecognized (iDataBuffer* buffer) = 0;
};

/** @} */

#endif // __CS_IMAP_MODELLOAD_H__

