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

#if !defined(HICACHEANDMANAGE_H_INCLUDED)

#include "cs3d/glide2/hicache.h"
#include "cs3d/common/texmem.h"

///
struct HighColorCacheAndManage_Data : public csHighColorCacheData
{
  /// texture memory position
  textMemSpace mempos;
};

///
class HighColorCacheAndManage : public HighColorCache
{
public:
  /// takes the maximum size of the cache
  HighColorCacheAndManage(int max_size, HIGHCOLOR_TYPE type, int bpp,TextureMemoryManager * tm);
  ///
  virtual ~HighColorCacheAndManage();

  ///
  virtual void Add (iPolygonTexture *polytex);
  ///
  virtual void Add (iTextureHandle *texture);

  ///
  virtual void Clear();

  ///
  virtual void Dump() = 0;

protected:
  TextureMemoryManager * manager;
  ///
  virtual void Load (HighColorCacheAndManage_Data *d) = 0;
  ///
  virtual void Unload (HighColorCacheAndManage_Data *d) = 0;
  ///
  virtual void Load (csHighColorCacheData *d) {
	  Load ((HighColorCacheAndManage_Data *)d);
  }
  ///
  virtual void Unload (csHighColorCacheData *d) {
	  Unload ((HighColorCacheAndManage_Data *)d);
  }
};

#define HICACHEANDMANAGE_H_INCLUDED
#endif // !HICACHEANDMANAGE_H_INCLUDED
