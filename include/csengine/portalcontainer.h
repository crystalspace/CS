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

#ifndef __CS_PORTALCONTAINER_H__
#define __CS_PORTALCONTAINER_H__

#include "csengine/sectorobj.h"

/**
 * This is a container class for portals.
 */
class csPortalContainer : public csSectorObject
{
  virtual void MoveToSector (iSector*) { }
  virtual void RemoveFromSectors () { }
  virtual void UpdateMove () { }

protected:
  /**
   * Destructor.  This is private in order to force clients to use DecRef()
   * for object destruction.
   */
  virtual ~csPortalContainer ();

public:
  /// Constructor.
  csPortalContainer (iMeshWrapper* theParent);

  /**
   * Pure abstract function to return the object model associated with
   * this object (for iVisibilityObject).
   */
  virtual iObjectModel* GetObjectModel () { return 0; }

  // For iVisibilityObject:
  virtual iMeshWrapper* GetMeshWrapper () const { return 0; }

  //--------------------- SCF stuff follows ------------------------------//
  SCF_DECLARE_IBASE_EXT (csSectorObject);
};

#endif // __CS_PORTALCONTAINER_H__

