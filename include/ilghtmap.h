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

struct csHighColorCacheData;

SCF_VERSION (iLightMap, 0, 0, 1);

/**
 * The lightmap interface.
 * A lightmap is similar to a texture but contains just intensities,
 * 256 levels from 0 to 255. This map is overlaid onto the texture
 * to obtain darker or brighter portions of texture.
 */
struct iLightMap : public iBase
{
  ///
  virtual unsigned char *GetMap (int nMap) = 0;
  ///
  virtual int GetWidth () = 0;
  ///
  virtual int GetHeight () = 0;
  ///
  virtual int GetRealWidth () = 0;
  ///
  virtual int GetRealHeight () = 0;
  ///
  virtual bool IsCached () = 0;
  ///
  virtual csHighColorCacheData *GetHighColorCache () = 0;
  ///
  virtual void SetInCache (bool bVal) = 0;
  ///
  virtual void SetHighColorCache (csHighColorCacheData* pVal) = 0;
  ///
  virtual void GetMeanLighting (int& r, int& g, int& b) = 0;
};

#endif
