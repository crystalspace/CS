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


#ifndef GL_TEXTURECACHE_H
#define GL_TEXTURECACHE_H

#include "cs3d/opengl/ogl_hicache.h"



///
class OpenGLTextureCache: public HighColorCache
{
private:
  bool rstate_bilinearmap;

public:
  ///
  virtual void Dump();
  ///
  OpenGLTextureCache(int size, int bitdepth);
  ///
  virtual ~OpenGLTextureCache ();

  ///
  void SetBilinearMapping (bool m) { rstate_bilinearmap = m; }
  ///
  bool GetBilinearMapping () { return rstate_bilinearmap; }

protected:
  ///
  virtual void Load(HighColorCache_Data *d);
  ///
  virtual void Unload(HighColorCache_Data *d);
};

///
class OpenGLLightmapCache: public HighColorCache
{
private:
public:
  ///
  OpenGLLightmapCache (int size, int bitdepth);
  ///
  virtual ~OpenGLLightmapCache ();
  ///
  virtual void Dump ();

protected:
  ///
  virtual void Load (HighColorCache_Data *d);
  ///
  virtual void Unload (HighColorCache_Data *d);
};

#endif // GL_TEXTURECACHE_H
