/*
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

#ifndef __ILIGHTMAP_H__
#define __ILIGHTMAP_H__

#include "csutil/scf.h"

SCF_VERSION (iLightMap, 0, 0, 1);

/**
 * The lightmap interface.
 * A lightmap is similar to a texture but contains just intensities,
 * 256 levels from 0 to 255. This map is overlaid onto the texture
 * to obtain darker or brighter portions of texture.
 */
struct iLightMap : public iBase
{
  /// Get light map by index; 0 - red, 1 - green, 2 - blue
  virtual unsigned char *GetMap (int nMap) = 0;
  /// Get lightmap width (could be adjusted to power of two)
  virtual int GetWidth () = 0;
  /// Get lightmap height (could be adjusted to power of two)
  virtual int GetHeight () = 0;
  /// Get real lightmap width (could be less than returned by GetWidth())
  virtual int GetRealWidth () = 0;
  /// Get real lightmap height (could be less than returned by GetHeight())
  virtual int GetRealHeight () = 0;
  /// Get data used internally by texture cache
  virtual void *GetCacheData () = 0;
  /// Set data used internally by texture cache
  virtual void SetCacheData (void *d) = 0;
  /// Get mean color for the lightmaps
  virtual void GetMeanLighting (int& r, int& g, int& b) = 0;
  /// Get size of one lightmap
  virtual long GetSize () = 0;
};

#endif
