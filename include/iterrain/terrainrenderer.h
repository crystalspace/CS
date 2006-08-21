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

#ifndef __CS_ITERRAIN_TERRAINRENDERER_H__
#define __CS_ITERRAIN_TERRAINRENDERER_H__

#include "csutil/scf.h"
#include "csutil/array.h"
#include "csutil/refarr.h"

struct iRenderView;
struct iTerrainCellRenderProperties;
struct iMovable;
struct iTerrainCell;
struct iMaterialWrapper;

class csRect;
class csColor;
struct csRenderMesh;

/// Provides an interface for custom rendering
struct iTerrainRenderer : public virtual iBase
{
  SCF_INTERFACE (iTerrainRenderer, 1, 0, 0);

  /**
   * Create an object that implements iTerrainCellCollisionProperties
   * This object will be stored in the cell. This function gets invoked
   * at cells creation.
   *
   * \return properties object
   */
  virtual csPtr<iTerrainCellRenderProperties> CreateProperties () = 0;

  /**
   * Render the visible cells
   *
   * \param n - output value, that will contain the size of the resulting
   * mesh array
   * \param rview - view that was used for rendering
   * \param movable - the terrain object
   * \param frustum_mask - frustum mask
   * \param cells - array with visible cells
   * \param cell_count - number of visible cells
   *
   * \return array of render meshes
   */
  virtual csRenderMesh** GetRenderMeshes (int& n, iRenderView* rview,
                                   iMovable* movable, uint32 frustum_mask,
                                   iTerrainCell** cells, int cell_count) = 0;

  
  /**
   * Indicates that the material palette has been changed, and that the
   * renderer should update its internal structures to reflect the changes.
   *
   * \param material_palette - new material palette
   */
  virtual void OnMaterialPaletteUpdate (const csRefArray<iMaterialWrapper>&
                                        material_palette) = 0;

  /**
   * Indicates that the cell height data has been changed (while unlocking
   * the cell height data - either by a feeder or by a user-provided
   * functions), and that the renderer should update its internal structures
   * to reflect the changes.
   *
   * \param cell - cell with the changed data
   * \param rectangle - rectangle that was updated
   * \param data - height data
   * \param pitch - data pitch
   */
  virtual void OnHeightUpdate (iTerrainCell* cell, const csRect& rectangle,
                               const float* data, unsigned int pitch) = 0;

  /**
   * Indicates that the cell's material mask has been changed (while
   * unlocking the cell material map data - either by a feeder or by a user-
   * provided functions - or while setting the new mask with the respective
   * functions), and that the renderer should update its internal structures
   * to reflect the changes.
   *
   * \param cell - cell with the changed data
   * \param material - material index
   * \param rectangle - rectangle that was updated
   * \param data - height data
   * \param pitch - data pitch
   */
  virtual void OnMaterialMaskUpdate (iTerrainCell* cell, unsigned int material,
                               const csRect& rectangle, const unsigned char*
                               data, unsigned int pitch) = 0;
  
  /**
   * Indicates that the cell color data has been changed (while computing
   * terrain lighting), and that the renderer should update its internal
   * structures to reflect the changes.
   *
   * \param cell - cell with the changed data
   * \param data - color data
   * \param res - color data resolution
   */
  virtual void OnColorUpdate (iTerrainCell* cell, const csColor* data,
                               unsigned int res) = 0;
};

#endif // __CS_ITERRAIN_TERRAINRENDERER_H__
