/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_IMESH_TERRBIG_H__
#define __CS_IMESH_TERRBIG_H__

#include "csutil/scf.h"

struct iFile;
struct iImageIO;
struct iMaterialWrapper;

SCF_VERSION (iTerrBigState, 0, 0, 1);

/**
 * This class associates a file created using the terrbig conversion
 * to the terrbig object.
 */
struct iTerrBigState : public iBase
{
  /// Memory maps the file on disk and associates it with the terrain
  virtual bool LoadHeightMapFile (const char *hm) = 0;
  /// Sets the scale factor on the map x = horz, y = vert, z = height
  virtual void SetScaleFactor (const csVector3 &scale) = 0;
  /**
   * Sets the error tolerance.  This should be in pixels, meaning a
   * tolerance of 1 will allow gaps in the hieghtmap of 1 pixel, a 
   * tolerance of 0 means the heightmap will perform no LOD
   */
  virtual void SetErrorTolerance (float tolerance) = 0;
  /** 
   * Converts an image to a height map file usable by LoadHeighMapFile()
   * and saves it into hm.  It will also load the file.
   */
  virtual bool ConvertImageToMapFile (iFile *image, iImageIO *imageio,
  	const char *hm) = 0;
  /**
   * Same as ConvertImageToMapFile but works from a raw floating point array
   * the incoming float should be size width * width.  Width should be a value
   * 2^n+1
   */
  virtual bool ConvertArrayToMapFile (float *data, int width,
  	const char *hm) = 0;
  /// Set the materials list, copies the passed in list.
  virtual void SetMaterialsList(iMaterialWrapper **matlist,
  	unsigned int nMaterials) = 0;
};

#endif // __CS_IMESH_TERRBIG_H__
