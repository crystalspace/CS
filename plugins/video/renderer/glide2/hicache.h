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


#include "csutil/scf.h"
#include "ipolygon.h"
#include "itexture.h"
#include "ilghtmap.h"
#include "cs3d/common/texmem.h"
#include <glide.h>

typedef struct
{
  GrChipID_t tmu_id;	/* TMU number */
  int memory_size;
  FxU32 minAddress;	/* Base Address */
  FxU32 maxAddress;	/* Max Address */
} TMUInfo;

typedef struct
{
  TMUInfo *tmu;		/* TMU Where it is loaded */
  GrTexInfo info;		/* Info about Texture */
  FxU32 loadAddress;	/* Memory Position in TMU */
  FxU32 size;		/* Memory size needed */
  bool loaded;          /* 1 if loaded in tmu, 0 if not */
  //int LastCounter;	/* Last used */
  float width,height;
}  TextureHandler;


///
struct csGlideCacheData
{
  /// size this takes up.
  long lSize;
  /// QueryInterface: ITextureMM successful if texture, iLightMap * if lightmap.
  iBase *pSource;
  /// internal cache data
  TextureHandler texhnd;
  /// linked list
  csGlideCacheData *next, *prev;
  /// tracks the position in texture memory of the board
  textMemSpace mempos;
};

///
enum HIGHCOLOR_TYPE
{
  HIGHCOLOR_TEXCACHE, HIGHCOLOR_LITCACHE, HIGHCOLOR_ALPHA
};

///
class HighColorCache
{
protected:

  HIGHCOLOR_TYPE type;
  /// the head and tail of the cache data
  csGlideCacheData *head, *tail;

protected:
  /// the maximum size of the cache
  long cache_size;
  /// number of items
  int num;
  /// the total size of the cache
  long total_size;

public:
  /// takes the maximum size of the cache
  HighColorCache (int max_size, HIGHCOLOR_TYPE type, int bpp, TextureMemoryManager * tm);
  ///
  virtual ~HighColorCache();

  ///
  void Add (iTextureHandle *texture);
  ///
  void Add (iPolygonTexture *polytex);
  ///
  void Clear ();

  ///
  virtual void Dump () = 0;

protected:
  ///
  int bpp;
  /// Manages the boards texture memory
  TextureMemoryManager * manager;

  ///
  virtual void Load (csGlideCacheData *d) = 0;
  ///
  virtual void Unload (csGlideCacheData *d) = 0;
  /// Calculate AspectRatio, LOD numbers and final width and size
  bool CalculateTexData( int width, int height, float wfak, float hfak, GrLOD_t *lod, int nLod, 
                        csGlideCacheData *d );
};

#endif


