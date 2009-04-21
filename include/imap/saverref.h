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

#ifndef __CS_IMAP_SAVERREF_H__
#define __CS_IMAP_SAVERREF_H__

/**\file
 * Saver references.
 */

#include "csutil/scf.h"

struct iObject;

/**
 * This interface represents a reference to a plugin.
 */
struct iPluginReference : public virtual iBase
{
  SCF_INTERFACE (iPluginReference, 0, 0, 1);
  
  /**
   * Get the name of the plugin reference.
   */
  virtual const char* GetName () const = 0;
  
  /**
   * Get the Class ID of the plugin.
   */
  virtual const char* GetClassID () const = 0;
  
  /**
   * Get the iObject for this interface.
   */
  virtual iObject* QueryObject () = 0;
};

/**
 * This interface represents a reference to a library file.
 */
struct iLibraryReference : public virtual iBase
{
  SCF_INTERFACE (iLibraryReference, 0, 0, 1);
  
  /**
   * Get the file name of the library reference.
   */
  virtual const char* GetFile () const = 0;
  
  /**
   * Get the path of the library reference.
   */
  virtual const char* GetPath () const = 0;
  
  /**
   * Get whether we should check dupes.
   */
  virtual bool GetCheckDupes () const = 0;

  /**
   * Get the iObject for this interface.
   */
  virtual iObject* QueryObject () = 0;
}; 

/**
 * This interface represents a reference to an addon.
 */
struct iAddonReference : public virtual iBase
{
  SCF_INTERFACE (iAddonReference, 0, 0, 1);
  
  /**
   * Get the SCF name of the plugin.
   */
  virtual const char* GetPlugin () const = 0;

  /**
   * Get the file name of the params file.
   */
  virtual const char* GetParamsFile () const = 0;
  
  /**
   * Get the object that the addon references.
   */
  virtual iBase* GetAddonObject () const = 0;

  /**
   * Get the iObject for this interface.
   */
  virtual iObject* QueryObject () = 0;
};

#endif // __CS_IMAP_SAVERREF_H__
