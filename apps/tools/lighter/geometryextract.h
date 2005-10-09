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

#ifndef __GEOMETRYEXTRACT_H__
#define __GEOMETRYEXTRACT_H__

#include "csutil/refcount.h"

struct iMeshObjectFactory;
class litLightingMeshFactory;
struct litScene;

/**
 * Baseclass for geometry extractors 
 */
class litGeometryExtractor : public csRefCount
{
public:
  /**
   * Extract the geometry from csmesh and save it into lightmesh. 
   */
  virtual bool ExtractGeometry (litScene *scene,
    iMeshObjectFactory *csfact, 
    litLightingMeshFactory *lightmeshfact);
};

class litGenmeshGeometryExtractor : public litGeometryExtractor
{
public:
  /**
   * Extract the geometry from csmesh and save it into lightmesh. 
   */
  virtual bool ExtractGeometry (litScene *scene,
    iMeshObjectFactory *csfact, 
    litLightingMeshFactory *lightmeshfact);
};

class litThingGeometryExtractor : public litGeometryExtractor
{
public:
  /**
  * Extract the geometry from csmesh and save it into lightmesh. 
  */
  virtual bool ExtractGeometry (litScene *scene,
    iMeshObjectFactory *csfact, 
    litLightingMeshFactory *lightmeshfact);
};


#endif
