/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __SCENE_H__
#define __SCENE_H__

#include "csutil/hash.h"
#include "csutil/refarr.h"

#include "lightmesh.h"
#include "kdtree.h"

CS_SPECIALIZE_TEMPLATE
class csHashComputer<iSector*> : public csHashComputerIntegral<iSector*> {};

/**
 * Topmost scene for lighting.
 * A scene consists of one or several CS worlds which should be lit togheter.
 */
struct csScene 
{
  /**
   * Information about a single CS sector 
   */
  struct SectorInformation
  {
    iSector *sector;
    csRefArray<csLightingMesh> meshes;
    csKDTree *kdtree;
  };

  csHash<SectorInformation*, iSector*> sectors;

  /**
   * Add an object to the scene, in correct sector 
   */
  void AddObject (csLightingMesh *object, iSector *sector)
  {
    SectorInformation *sinf = sectors.Get (sector, 0);
    if (sinf == 0)
    {
      sinf = new SectorInformation;
      sinf->sector = sector;
      sectors.Put (sector, sinf);
    }
    sinf->meshes.Push (object);
  }
};

#endif
