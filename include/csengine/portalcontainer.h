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

#include "iengine/portalcontainer.h"
#include "cstool/meshobjtmpl.h"

/**
 * This is a container class for portals.
 */
class csPortalContainer : public csMeshObject, public iPortalContainer
{
protected:
  /**
   * Destructor.  This is private in order to force clients to use DecRef()
   * for object destruction.
   */
  virtual ~csPortalContainer ();

public:
  /// Constructor.
  csPortalContainer (iEngine* engine);

  SCF_DECLARE_IBASE_EXT (csMeshObject);

  //-------------------For iPortalContainer ----------------------------//

  //--------------------- For iMeshObject ------------------------------//
  virtual iMeshObjectFactory* GetFactory () const { return 0; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable,
  	csZBufMode zbufMode);
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }
  virtual bool HitBeamOutline (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);
  virtual int GetPortalCount () const { return 0; /*@@@*/ }
  virtual iPortal* GetPortal (int idx) const { return 0; /*@@@*/ }
};

#endif // __CS_PORTALCONTAINER_H__

