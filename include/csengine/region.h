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
#include "iregion.h"

class csEngine;

/**
 * A region. A region is basically a collection of objects in the
 * 3D engine that can be treated as a unit.
 */
class csRegion : public csObjectNoDel, public iRegion
{
  friend class Dumper;

private:
  csEngine* engine;

public:
  /**
   * Initialize an empty region.
   */
  csRegion (csEngine*);

  /**
   * Delete the region without deleting the entities in it. The entities
   * in this region will simply get unconnected.
   */
  virtual ~csRegion ();

  /**
   * Add an object to this region.
   */
  void AddToRegion (csObject* obj);

  /**
   * Release an object from this region.
   */
  void ReleaseFromRegion (csObject* obj);

  CSOBJTYPE;

  //--------------------- iRegion implementation ---------------------
  DECLARE_IBASE;

  /**
   * Clear this region without removing the entities in it. The entities
   * will simply get unconnected from this region.
   */
  virtual void Clear ();

  /**
   * Delete all entities in this region.
   */
  virtual void DeleteAll ();

  /**
   * Prepare all objects in this region. This has to be called
   * directly after loading new objects.
   */
  virtual bool Prepare ();
};

#endif // __CS_REGION_H__
