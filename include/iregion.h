/*
    Crystal Space 3D engine
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

#ifndef __IREGION_H__
#define __IREGION_H__

#include "csutil/scf.h"

struct iSector;
struct iThing;
struct iSprite;
struct iSpriteTemplate;
struct iMaterialWrapper;

SCF_VERSION (iRegion, 0, 1, 0);

/**
 * A region. A region is basically a collection of objects in the
 * 3D engine that can be treated as a unit.
 */
struct iRegion : public iBase
{
  /**
   * Clear this region without removing the entities in it. The entities
   * will simply get unconnected from this region.
   */
  virtual void Clear () = 0;

  /**
   * Delete all entities in this region.
   */
  virtual void DeleteAll () = 0;

  /**
   * Prepare all objects in this region. This has to be called
   * directly after loading new objects.
   */
  virtual bool Prepare () = 0;

  /// Find a sector in this region by name.
  virtual iSector *FindSector (const char *iName) = 0;
  /// Find a thing in this region by name
  virtual iThing *FindThing (const char *iName) = 0;
  /// Find a sky thing in this region by name
  virtual iThing *FindSky (const char *iName) = 0;
  /// Find a thing template in this region by name
  virtual iThing *FindThingTemplate (const char *iName) = 0;
  /// Find a sprite in this region by name
  virtual iSprite *FindSprite (const char *iName) = 0;
  /// Find a sprite template in this region by name
  virtual iSpriteTemplate *FindSpriteTemplate (const char *iName) = 0;
  /// Find a material in this region by name
  virtual iMaterialWrapper *FindMaterial (const char *iName) = 0;
};

#endif // __IREGION_H__
