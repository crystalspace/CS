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

#ifndef __CS_IMAP_SAVER_H__
#define __CS_IMAP_SAVER_H__

/**\file
 * Engine contents saving
 */

#include "csutil/scf_interface.h"

#include "csutil/deprecated_warn_off.h"

struct iCameraPosition;
struct iDocumentNode;
struct iString;

/**
 * This interface is used to serialize the engine
 * contents.
 */ 
struct iSaver : public virtual iBase
{
  SCF_INTERFACE (iSaver, 3, 0, 1);

  /**\name Whole world saving
   * @{ */
  /// Save the current engine contents to the file with VFS name \p filename.
  virtual bool SaveMapFile(const char *filename) = 0;
  /// Return the current engine contents as a string.
  virtual csRef<iString> SaveMapFile() = 0;
  /// Save the current engine contents as a world file in a DocumentNode
  virtual bool SaveMapFile(csRef<iDocumentNode> &root) = 0;
  
  /**
   * Save all collections to their respective files.
   * Collections that do not have any iSaverFile attached will not be saved.
   */
  virtual bool SaveAllCollections() = 0;
  
  /**
   * Save a collection to the given file.
   * \param collection The collection to save
   * \param filename The VFS name of the file where to save the collection
   * \param filetype The type of CS file to be saved. It can be one of
   * CS_SAVER_FILE_WORLD, CS_SAVER_FILE_LIBRARY, CS_SAVER_FILE_MESHFACT and
   * CS_SAVER_FILE_PARAMS.
   */
  virtual bool SaveCollectionFile(iCollection* collection, const char* filename,
    int filetype) = 0;
  
  /**
   * Return the collection contents as a string.
   * \param collection The collection to save
   * \param filename The VFS name of the file where to save the collection
   * \param filetype The type of CS file to be saved. It can be one of
   * CS_SAVER_FILE_WORLD, CS_SAVER_FILE_LIBRARY, CS_SAVER_FILE_MESHFACT and
   * CS_SAVER_FILE_PARAMS.
   */
  virtual csRef<iString> SaveCollection(iCollection* collection, int filetype) = 0;
  
  /**
   * Save the collection to the given document node.
   * \param collection The collection to save
   * \param filetype The type of CS file to be saved. It can be one of
   * CS_SAVER_FILE_WORLD, CS_SAVER_FILE_LIBRARY, CS_SAVER_FILE_MESHFACT and
   * CS_SAVER_FILE_PARAMS.
   * \param root The document node where the description of the collection will be added
   */
  virtual bool SaveCollection(iCollection* collection, int filetype,
    csRef<iDocumentNode>& root) = 0;
  /** @} */
  
  /**\name Fine-grained saving
   * @{ */
  /**
   * Save a portal to the given document node.
   * This will create a '<tt>portal</tt>' node below \p parent.
   * \param portal The portal to save
   * \param parent The document node where the description of the portal will be added
   */
  virtual bool SavePortal (iPortal *portal, iDocumentNode *parent) = 0;

  /**
   * Save a camera position to the given document node.
   * This will create a '<tt>start</tt>' node below \p parent.
   * \param portal The camera position to save
   * \param parent The document node where the description of the camera position will be added
   */
  virtual bool SaveCameraPosition (iCameraPosition *position, iDocumentNode *parent) = 0;

  // TODO: Add more, as needed
  /** @} */
};

#include "csutil/deprecated_warn_on.h"

#endif // __CS_IMAP_SAVER_H__
