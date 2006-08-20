/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#ifndef __CS_ITERRAIN_TERRAINFACTORY_H__
#define __CS_ITERRAIN_TERRAINFACTORY_H__

#include "csutil/scf.h"

struct iTerrainRenderer;
struct iTerrainCollider;
struct iTerrainDataFeeder;

struct iTerrainSystem;
struct iTerrainCell;

class csVector2;
class csVector3;

/// Provides an interface for creating terrain system
struct iTerrainFactory : public virtual iBase
{
  SCF_INTERFACE (iTerrainFactory, 1, 0, 0);

  /**
   * Set desired renderer (there is a single renderer for the whole terrain)
   *
   * \param renderer - new renderer
   */
  virtual void SetRenderer (iTerrainRenderer* renderer) = 0;
  
  /**
   * Set desired collider (there is a single collider for the whole terrain)
   *
   * \param collider - new collider
   */
  virtual void SetCollider (iTerrainCollider* collider) = 0;
  
  /**
   * Add cell to the terrain
   *
   * \param name - optional cell name
   * \param grid_width - grid width. It will be changed to match the grid
   * width requirements. See iTerrainCell::GetGridWidth
   * \param grid_height - grid height. It will be changed to match the grid
   * height requirements. See iTerrainCell::GetGridHeight
   * \param material_width - material map width
   * \param material_height - material map height
   * \param position - cell object-space position
   * \param size - cell object-space size and height scale
   * \param feeder - feeder that would be attached to the cell
   *
   * \return added cell
   */
  virtual iTerrainCell* AddCell (const char* name, int grid_width,
                        int grid_height, int material_width,
                        int material_height, const csVector2& position,
                        const csVector3& size, iTerrainDataFeeder* feeder) = 0;
};

#endif // __CS_ITERRAIN_TERRAINFACTORY_H__
