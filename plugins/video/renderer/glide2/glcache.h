/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles.

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

// GLIDECACHE.H
// GlideTextureCache and GlideLightmapCache declarations
// Written by xtrochu and Nathaniel

#if !defined(GLIDECACHE_H_INCLUDED)

#include <glide.h>
#include "cs3d/glide2/hicache2.h"
#include "itexture.h"

class csGraphics3DGlide;

typedef struct
{
  GrChipID_t tmu_id;		/* TMU number */
  int memory_size;
	FxU32 minAddress;	/* Base Address */
	FxU32 maxAddress;	/* Max Address */
} TMUInfo;

typedef struct
{
	TMUInfo *tmu;		/* TMU Where it is loaded */
	GrTexInfo info;		/* Info about Texture */
	FxU32 loadAddress;	/* Memory Position in TMU */
	FxU32 size;			/* Memory size needed */
//	int isActuallyLoaded; /* 1 if loaded in tmu, 0 if not */
//	int LastCounter;	/* Last used */
	float width,height;
}  TextureHandler;

///
class GlideTextureCache: public HighColorCacheAndManage
{
private:
  TMUInfo *m_tmu;

public:
  ///
  GlideTextureCache(TMUInfo *tmu,int bpp,TextureMemoryManager*man);
  ///
  virtual void Dump();
  
  /// Load a halo texture data
  HighColorCacheAndManage_Data * LoadHalo(char *data);
  /// Unload halo texture
  void UnloadHalo(HighColorCacheAndManage_Data *d);

protected:
  ///
  virtual void Load(HighColorCacheAndManage_Data *d);
  ///
  virtual void Unload(HighColorCacheAndManage_Data *d);
};

///
class GlideLightmapCache: public HighColorCacheAndManage
{
private:
  TMUInfo *m_tmu;
public:
  ///
  GlideLightmapCache(TMUInfo *tmu,TextureMemoryManager*man);
  ///
  virtual void Dump();
  
protected:
  ///
  virtual void Load(HighColorCacheAndManage_Data *d);
  ///
  virtual void Unload(HighColorCacheAndManage_Data *d);
};

#define GLIDECACHE_H_INCLUDED
#endif // GLIDECACHE_H_INCLUDED
