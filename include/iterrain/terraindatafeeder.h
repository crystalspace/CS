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

#ifndef __CS_ITERRAIN_TERRAINDATAFEEDER_H__
#define __CS_ITERRAIN_TERRAINDATAFEEDER_H__

#include "csutil/scf.h"

struct iTerrainCell;

/// Provides an interface for reading cell data
struct iTerrainDataFeeder : public virtual iBase
{
  SCF_INTERFACE (iTerrainDataFeeder, 1, 0, 0);

  /**
   * Start cell data preloading (in case of threaded/async loading). This is
   * triggered by TerrainSystem::PreLoadCells, which is either called by user
   * or called automatically while rendering terrain.
   *
   * \param cell - cell to start preloading for
   *
   * \return preloading success flag
   */
  virtual bool PreLoad (iTerrainCell* cell) = 0;
  
  /**
   * Load cell data. After the completion of this call the cell should have
   * all necessary information.
   *
   * \param cell - cell to load
   *
   * \return loading success flag
   */
  virtual bool Load (iTerrainCell* cell) = 0;
  
  /**
   * Set feeder-dependent parameter
   *
   * \param param - parameter name
   * \param value - parameter value
   */
  virtual void SetParam(const char* param, const char* value) = 0;
};

#endif // __CS_ITERRAIN_TERRAINDATAFEEDER_H__
