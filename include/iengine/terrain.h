/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Plug-In Written by Richard D Shank

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

#ifndef __IENGINE_TERRAIN_H__
#define __IENGINE_TERRAIN_H__

#include "csutil/scf.h"
#include "isys/plugin.h"

class csTerrainWrapper;
struct iLight;
struct iTerrainObject;
struct iTerrainObjectFactory;

SCF_VERSION (iTerrainWrapper, 0, 0, 1);

/**
 * This interface corresponds to the object in the engine
 * that holds reference to the real iTerrainObject.
 */
struct iTerrainWrapper : public iBase
{
  /// Ugly.
  virtual csTerrainWrapper* GetPrivateObject () = 0;

  /// Get the iTerrainObject.
  virtual iTerrainObject* GetTerrainObject () = 0;

  /**
   * Light object according to the given array of lights (i.e.
   * fill the vertex color array).
   * No shadow calculation will be done. This is assumed to have
   * been done earlier. This is a primitive lighting process
   * based on the lights which hit one point of the sprite (usually
   * the center). More elaborate lighting systems are possible
   * but this will do for now.
   */
  virtual void UpdateLighting (iLight** lights, int num_lights) = 0;
};

SCF_VERSION (iTerrainFactoryWrapper, 0, 0, 1);

/**
 * This interface corresponds to the object in the engine
 * that holds reference to the real iTerrainFactory.
 */
struct iTerrainFactoryWrapper : public iBase
{
  /// Get the iTerrainFactory.
  virtual iTerrainObjectFactory* GetTerrainObjectFactory () = 0;
};

#endif // __IENGINE_TERRAIN_H__
