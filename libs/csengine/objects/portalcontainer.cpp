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


// ---------------------------------------------------------------------------
// csMeshWrapper
// ---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csPortalContainer)
SCF_IMPLEMENT_IBASE_EXT_END

csPortalContainer::csPortalContainer (iMeshWrapper *theParent) :
    csSectorObject (theParent)
{
}

csPortalContainer::~csPortalContainer ()
{
}

