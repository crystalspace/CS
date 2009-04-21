/*
    Copyright (C) 2006 by Seth Yastrov

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

#ifndef __CS_IMAP_SAVERFILE_H__
#define __CS_IMAP_SAVERFILE_H__

/**\file
 * Saving multiple files.
 */

#include "csutil/scf.h"

struct iObject;

//@{
/**
 * Saver file types
 */
enum
{
  /// World file
  CS_SAVER_FILE_WORLD = 0,
  /// Library file
  CS_SAVER_FILE_LIBRARY = 1,
  /// Meshfact file
  CS_SAVER_FILE_MESHFACT = 2,
  /// Addon params file
  CS_SAVER_FILE_PARAMS = 3
};
//@}

/**
 * This interface represents a CS file to save to.
 * Attach engine objects to this to save them to this file.
 * This is useful to the saver to support saving to multiple files.
 */
struct iSaverFile : public virtual iBase
{
  SCF_INTERFACE (iSaverFile, 0, 0, 1);
  
  /**
   * Get the file name of the saver file.
   */
  virtual const char* GetFile () const = 0;

  /**
   * Get the type of the saver file.
   */
  virtual int GetFileType () const = 0;

  /**
   * Get the iObject for this object.
   */
  virtual iObject* QueryObject () = 0;
}; 

#endif // __CS_IMAP_SAVERFILE_H__
