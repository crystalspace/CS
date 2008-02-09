/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CS_CSENGINE_LIGHTMGR_H__
#define __CS_CSENGINE_LIGHTMGR_H__

#include "csutil/scf_implementation.h"
#include "iengine/lightmgr.h"

/**
 * Engine implementation of the light manager.
 */
class csLightManager : public scfImplementation1<csLightManager,
                                                 iLightManager>
{
private:

public:
  csLightManager ();
  virtual ~csLightManager ();

  virtual void GetRelevantLights (iMeshWrapper* meshObject, 
    iLightInfluenceArray* lightArray, int maxLights, uint flags);
  virtual void GetRelevantLights (iMeshWrapper* meshObject, 
    iLightInfluenceCallback* lightCallback, int maxLights, 
    uint flags);

  virtual void GetRelevantLights (iSector* sector, 
    iLightInfluenceArray* lightArray, int maxLights, 
    uint flags);
  virtual void GetRelevantLights (iSector* sector, 
    iLightInfluenceCallback* lightCallback, int maxLights, 
    uint flags);

  virtual void GetRelevantLights (iSector* sector, const csBox3& boundingBox,
    iLightInfluenceArray* lightArray, int maxLights, 
    uint flags);
  virtual void GetRelevantLights (iSector* sector, const csBox3& boundingBox,
    iLightInfluenceCallback* lightCallback, int maxLights, 
    uint flags);

  virtual size_t GetRelevantLights (iSector* sector, 
    const csBox3& boundingBox, csLightInfluence* lightArray, size_t maxLights,
    uint flags);
};

/**
 * 
 */

#endif // __CS_CSENGINE_LIGHTMGR_H__

