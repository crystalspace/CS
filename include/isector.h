/*
    Crystal Space 3D engine
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

#ifndef __ISECTOR_H__
#define __ISECTOR_H__

#include "csutil/scf.h"

class csSector;
struct iThing;
struct iStatLight;
struct iVisibilityCuller;

SCF_VERSION (iSector, 0, 1, 3);

/**
 * The iSector interface is used to work with "sectors". A "sector"
 * is a convex polyhedron, that possibly contains portals, things,
 * sprites, lights and so on. Simply speaking, a "sector" is analogous
 * to an real-world room, rooms are interconnected with doors and windows
 * (e.g. portals), and rooms contain miscelaneous things, sprites
 * and lights.
 */
struct iSector : public iBase
{
  /// Used by the engine to retrieve internal sector object (ugly)
  virtual csSector *GetPrivateObject () = 0;
  /// Create the static BSP or octree for this sector.
  virtual void CreateBSP () = 0;

  /// Find a sky with the given name.
  virtual iThing *GetSkyThing (const char *name) = 0;
  /// Get the number of sky things in this sector.
  virtual int GetNumSkyThings () = 0;
  /// Get a sky thing by index
  virtual iThing *GetSkyThing (int iIndex) = 0;

  /// Find a thing with the given name.
  virtual iThing *GetThing (const char *name) = 0;
  /// Get the number of things in this sector.
  virtual int GetNumThings () = 0;
  /// Get a thing by index
  virtual iThing *GetThing (int iIndex) = 0;

  /// Add a static or pseudo-dynamic light to this sector.
  virtual void AddLight (iStatLight *light) = 0;
  /// Find a light with the given position and radius.
  virtual iStatLight *FindLight (float x, float y, float z, float dist) = 0;

  /**
   * Calculate the bounding box of all objects in this sector.
   * This function is not very efficient as it will traverse all objects
   * in the sector one by one and compute a bounding box from that.
   */
  virtual void CalculateSectorBBox (csBox3& bbox, bool do_things, bool do_meshes,
  	bool do_terrain) = 0;

  /**
   * Get the visibility culler that is used for this sector.
   * NULL if none.
   */
  virtual iVisibilityCuller* GetVisibilityCuller () = 0;
};

#endif // __ISECTOR_H__
