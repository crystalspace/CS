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
#include "cssysdef.h"
#include "qsqrt.h"
#include "csgeom/sphere.h"
#include "igeom/objmodel.h"
#include "csengine/portalcontainer.h"
#include "csgeom/transfrm.h"

// ---------------------------------------------------------------------------
// csMeshWrapper
// ---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csPortalContainer)
  SCF_IMPLEMENTS_INTERFACE (iPortalContainer)
SCF_IMPLEMENT_IBASE_EXT_END

csPortalContainer::csPortalContainer (iEngine* engine)
	: csMeshObject (engine)
{
}

csPortalContainer::~csPortalContainer ()
{
}

//--------------------- For iMeshObject ------------------------------//

bool csPortalContainer::DrawTest (iRenderView* rview, iMovable* movable)
{
  (void)rview;
  (void)movable;
  return false;
}

bool csPortalContainer::Draw (iRenderView* rview, iMovable* movable,
  	csZBufMode zbufMode)
{
  (void)rview;
  (void)movable;
  (void)zbufMode;
  return false;
}

void csPortalContainer::HardTransform (const csReversibleTransform& t)
{
  (void)t;
}

bool csPortalContainer::HitBeamOutline (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr)
{
  (void)start;
  (void)end;
  (void)isect;
  (void)pr;
  return false;
}

bool csPortalContainer::HitBeamObject (const csVector3& start,
	const csVector3& end, csVector3& isect, float* pr)
{
  (void)start;
  (void)end;
  (void)isect;
  (void)pr;
  return false;
}

