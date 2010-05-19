/*
  Copyright (C) 2010 by Jelle Hellemans

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

#ifndef __CS_IMESH_MODIFIABLETERRAIN_H__
#define __CS_IMESH_MODIFIABLETERRAIN_H__

/**\file
 * Modifiable Terrain interfaces
 */

#include "csutil/scf_interface.h"
#include "iutil/array.h"
#include "ivideo/shader/shader.h"
#include "imesh/object.h"
#include "imesh/terrain2.h"

class csVector2;
class csVector3;

class csBox2;

/**
 * Terrain modifier class. .
 */
struct iTerrainModifier : public virtual iBase
{
  SCF_INTERFACE (iTerrainModifier, 1, 0, 0);

  /// Displace the affected area in the terrain cell
  virtual void Displace(iTerrainCell* cell, float intensity) const = 0;

  /// Check if this modifier affects the given area
  virtual bool InBounds(const csBox2& bb) const = 0;

  /**
   * Return ths type of the modifier
   * - Rectangle Flatten
   * - Circle Displacement
   * - Brush Displacement
   */
  virtual const char* GetType() const = 0;
};


/**
 * Modifiable Terrain data feederclass.
 */
struct iModifiableDataFeeder : public virtual iBase
{
  SCF_INTERFACE (iModifiableDataFeeder, 1, 0, 0);

  /**
   * Add a Rectangle Flatten modifier.
   *
   * \param center center of the modifier in object space
   * \param width width of the rectangle
   * \param height of the rectangle
   *
   * \return the created and added terrain modifier
   */
  virtual iTerrainModifier* AddModifier (const csVector3& center, float width, float height) = 0;

  /**
   * Remove a given terrain modifier.
   *
   * \param modifier the terrain modifier to remove
   */
  virtual void RemoveModifier (iTerrainModifier* modifier) = 0;

};

#endif
