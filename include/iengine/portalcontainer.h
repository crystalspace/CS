/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_PORTALCONTAINER_H__
#define __CS_IENGINE_PORTALCONTAINER_H__

/**\file
 */
/**
 * \addtogroup engine3d
 * @{ */

#include "csutil/scf.h"

SCF_VERSION (iPortalContainer, 0, 0, 1);

/**
 * A container for portals.
 */
struct iPortalContainer : public iBase
{
  /**
   * Get the iObject for this mesh object. This can be used to get the
   * name of the mesh wrapper and also to attach other user objects
   * to this mesh (like for collision detection or game data).
   */
  virtual iObject *QueryObject () = 0;
};

SCF_VERSION (iPortalContainerList, 0, 0, 1);

/**
 * A list of portal containers.
 */
struct iPortalContainerList : public iBase
{
  /// Return the number of portal containers in this list.
  virtual int GetCount () const = 0;

  /// Return a portal container by index.
  virtual iPortalContainer *Get (int n) const = 0;

  /// Add a portal container.
  virtual int Add (iPortalContainer *obj) = 0;

  /// Remove a portal container.
  virtual bool Remove (iPortalContainer *obj) = 0;

  /// Remove the nth portal container.
  virtual bool Remove (int n) = 0;

  /// Remove all portal containers.
  virtual void RemoveAll () = 0;

  /// Find a portal container and return its index.
  virtual int Find (iPortalContainer *obj) const = 0;

  /**
   * Find a portal container by name.
   */
  virtual iPortalContainer *FindByName (const char *Name) const = 0;
};


/** @} */

#endif // __CS_IENGINE_PORTALCONTAINER_H__

