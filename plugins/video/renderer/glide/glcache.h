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
#include "iengine/polygon.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "iengine/lightmap.h"
#include "glalpha.h"
#include "video/renderer/common/texmem.h"
#include <glide.h>

/// supported texture types

#define CS_GLIDE_TEXCACHE   0x0001
#define CS_GLIDE_LITCACHE   0x0002
#define CS_GLIDE_ALPHACACHE 0x0004

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
  short type;
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
class csGlideTextureCache : public iBase
{
protected:
  /// TMUInfo
  TMUInfo *m_tmu;
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
  csGlideTextureCache ( TMUInfo *tmu, int bpp, TextureMemoryManager * tm);
  ///
  virtual ~csGlideTextureCache();
  
  SCF_DECLARE_IBASE;

  /// Add a texture to the texture memory
  void Add (iTextureHandle *texture, bool alpha);
  /// Add a lightmap to the texture memory
  void Add (iPolygonTexture *polytex);
  /// Clear cache
  void Clear ();

  void Remove (iTextureHandle *texture);

  ///
  virtual void Dump ();

protected:
  ///
  int bpp;
  /// Manages the boards texture memory
  TextureMemoryManager * manager;

  ///
  virtual void Load (csGlideCacheData *d, int nMM = 1);
  ///
  virtual void Unload (csGlideCacheData *d);
  /// Calculate AspectRatio, LOD numbers and final width and size
  bool CalculateTexData( int width, int height, float wfak, float hfak, GrLOD_t *lod, int nLod, 
                        csGlideCacheData *d );
  virtual void LoadTex (csGlideCacheData *d, int nMM);
  virtual void LoadLight (csGlideCacheData *d);
  virtual void LoadAlpha (csGlideCacheData *d);
  
};

#endif


