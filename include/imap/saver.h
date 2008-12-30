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

struct iDocumentNode;
struct iString;

/**
 * This interface is used to serialize the engine
 * contents.
 */ 
struct iSaver : public virtual iBase
{
  SCF_INTERFACE (iSaver, 3, 0, 0);

  /**\name Whole world saving
   * @{ */
  /// Save the current engine contents to the filename.
  virtual bool SaveMapFile(const char *filename) = 0;
  /// Return the current engine contents as a string.
  virtual csRef<iString> SaveMapFile() = 0;
  /// Save map to DocumentNode
  virtual bool SaveMapFile(csRef<iDocumentNode> &root) = 0;
  
  /**
   * Save all collections to their respective files.
   * Collections that do not have any iSaverFile attached will not be saved.
   */
  virtual bool SaveAllCollections() = 0;
  
  /// Save a region to the file attached to it using iSaverFile.
  virtual bool SaveCollectionFile(iCollection* collection, const char* filename,
    int filetype) = 0;
  
  /// Return the region contents as a string.
  virtual csRef<iString> SaveCollection(iCollection* collection, int filetype) = 0;
  
  /// Save region to the document node.
  virtual bool SaveCollection(iCollection* collection, int filetype,
    csRef<iDocumentNode>& root) = 0;
  /** @} */
  
  /**\name Fine-grained saving
   * @{ */
  /**
   * Save a portal to the given document node.
   * Will create a '<tt>portal</tt>' node below \p parent.
   */
  virtual bool SavePortal (iPortal *portal, iDocumentNode *parent) = 0;

  // TODO: Add more, as needed
  /** @} */
};

#include "csutil/deprecated_warn_on.h"

#endif // __CS_IMAP_SAVER_H__
