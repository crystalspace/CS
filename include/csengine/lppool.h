/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef __CS_LPPOOL_H__
#define __CS_LPPOOL_H__

#include "csutil/objpool.h"
#include "csengine/light.h"


CS_DECLARE_OBJECT_POOL (csLightPatchPoolHelper, csLightPatch);

/**
 * This is an object pool which holds objects of type
 * csLightPatch. You can ask new instances from this pool.
 * If needed it will allocate one for you but ideally it can
 * give you one which was allocated earlier.
 */
class csLightPatchPool : public csLightPatchPoolHelper {
public:
  void Free (void* o)
  {
    csLightPatch* p = (csLightPatch*)o;
    p->RemovePatch ();
    csLightPatchPoolHelper::Free (p);
  }
};

#endif // __CS_LPPOOL_H__
