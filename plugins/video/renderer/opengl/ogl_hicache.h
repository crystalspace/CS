/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles
  
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

// HICACHE.H
// A texture/lightmap class for hi-color rasterizers
// Written by Dan Ogles

#ifndef HICACHE_H
#define HICACHE_H


#include "cscom/com.h"

///
struct HighColorCache_Data
{
  /// size this takes up.
  long lSize;
  /// QueryInterface: ITextureHandle successful if texture, ILightMap * if lightmap.
  IUnknown *pSource;
  /// internal cache data
  void *pData;
  /// linked list
  HighColorCache_Data *next, *prev;
};

///
enum HIGHCOLOR_TYPE
{
  HIGHCOLOR_TEXCACHE, HIGHCOLOR_LITCACHE
};

interface ITextureHandle;
interface IPolygonTexture;

///
class HighColorCache
{
private:

  HIGHCOLOR_TYPE type;
  /// the head and tail of the cache data
  HighColorCache_Data *head, *tail;

protected:
  /// the maximum size of the cache
  long cache_size;
  /// number of items
  int num;
  /// the total size of the cache
  long total_size;

public:
  /// takes the maximum size of the cache
  HighColorCache (int max_size, HIGHCOLOR_TYPE type, int bpp);
  ///
  virtual ~HighColorCache ();

  ///
  void Add (ITextureHandle *texture);
  ///
  void Add (IPolygonTexture *polytex);
  ///
  void Clear ();

  ///
  virtual void Dump () = 0;

protected:
  ///
  int bpp;

  ///
  virtual void Load (HighColorCache_Data *d) = 0;
  ///
  virtual void Unload (HighColorCache_Data *d) = 0;
};

#endif
