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

#ifndef __CS_ITERRAIN_TERRAINCELLRENDERPROPERTIES_H__
#define __CS_ITERRAIN_TERRAINCELLRENDERPROPERTIES_H__

#include "csutil/scf.h"

/**
 * This is a base class for per-cell renderer-specific properties.
 * The classes which hold the render-related data that is to be
 * customized by user should implement this interface.
 */
struct iTerrainCellRenderProperties : public virtual iBase
{
  SCF_INTERFACE (iTerrainCellRenderProperties, 1, 0, 0);

  /**
   * Get visibility flag (if it is not set, the cell does not get rendered)
   * 
   * \return visibility flag
   */
  virtual bool GetVisible () const = 0;
  
  /**
   * Set visibility flag
   * 
   * \param value - new flag value
   */
  virtual void SetVisible (bool value) = 0;
};

#endif // __CS_ITERRAIN_TERRAINCELLRENDERPROPERTIES_H__
