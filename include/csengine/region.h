/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef __CS_REGION_H__
#define __CS_REGION_H__

#include "csobject/csobject.h"
class csWorld;

/**
 * A region. A region is basically a collection of objects in the
 * 3D engine that can be treated as a unit.
 */
class csRegion : public csObjectNoDel
{
  friend class Dumper;

private:
  csWorld* world;

public:
  /**
   * Initialize an empty region.
   */
  csRegion (csWorld* world);

  /**
   * Delete the region without deleting the entities in it. The entities
   * in this region will simply get unconnected.
   */
  virtual ~csRegion ();

  /**
   * Clear this region without removing the entities in it. The entities
   * will simply get unconnected from this region.
   */
  void Clear ();

  /**
   * Delete all entities in this region.
   */
  void DeleteAll ();

  /**
   * Prepare all objects in this region. This has to be called
   * directly after loading new objects.
   */
  bool Prepare ();

  CSOBJTYPE;
};

#endif // __CS_REGION_H__
