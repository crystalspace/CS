/*
    Copyright (C) 2004 by Anders Stenberg, Daniel Duhprey

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

#ifndef __CS_IVARIA_SIMPLEFORMER_H__
#define __CS_IVARIA_SIMPLEFORMER_H__

#include "csgeom/vector3.h"

struct iImage;

SCF_VERSION (iSimpleFormerState, 0, 0, 1);

/**
 * iSimpleFormerState exposes implementation specific methods
 * for the SimpleTerraformer plugin
 */
struct iSimpleFormerState : public iBase
{
  /**
   * Set a heightmap to be used. The heightmap will by default be
   * covering a region from -1..1 along X and Z, and areas outside
   * this will return a height of 0
   */
  virtual void SetHeightmap (iImage *heightmap) = 0;

  /**
   * Set a scaling factor to be applied to the heightmap region (X, Z)
   * and height (Y)
   */
  virtual void SetScale (csVector3 scale) = 0;

  /**
   * Set a offset to be applied to the heightmap region (X, Z)
   * and height (Y)
   */
  virtual void SetOffset (csVector3 scale) = 0;
};

#endif // __CS_IVARIA_SIMPLEFORMER_H__
