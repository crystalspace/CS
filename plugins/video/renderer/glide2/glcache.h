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
#include "cs3d/glide2/hicache.h"
#include "itexture.h"

class csGraphics3DGlide;

///
class GlideTextureCache: public HighColorCache
{
private:
  TMUInfo *m_tmu;

public:
  ///
  GlideTextureCache(TMUInfo *tmu,int bpp,TextureMemoryManager*man);
  ///
  virtual void Dump();
  
  /// Load a halo texture data
  csGlideCacheData * LoadHalo(char *data);
  /// Unload halo texture
  void UnloadHalo(csGlideCacheData *d);

protected:
  ///
  virtual void Load(csGlideCacheData *d);
  ///
  virtual void Unload(csGlideCacheData *d);
};

///
class GlideLightmapCache: public HighColorCache
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
  virtual void Load(csGlideCacheData *d);
  ///
  virtual void Unload(csGlideCacheData *d);
};

#define GLIDECACHE_H_INCLUDED
#endif // GLIDECACHE_H_INCLUDED
