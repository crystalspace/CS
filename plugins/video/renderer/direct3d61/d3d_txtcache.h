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

#ifndef D3DCACHE_H
#define D3DCACHE_H

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <d3dcaps.h>

#include "d3d_g3d.h"
#include "ivideo/igraph3d.h"

struct iTextureHandle;
struct iPolygonTexture;
///
struct csD3DCacheData
{
  /// size this takes up.
  long lSize;
  /// The source iTextureHandle/iLightMap
  void* pSource;
  /// Internal texture cache data
  union
  {
    ///
    struct
    {
      LPDIRECTDRAWSURFACE4 lpsurf;	// texture data surface
      LPDIRECT3DTEXTURE2   lptex;	// texture interface
      LPDIRECTDRAWPALETTE  lpddpal;	// texture palette
    } Texture;
    ///
    struct
    {
      LPDIRECTDRAWSURFACE4 lpsurf;	// texture data surface
      LPDIRECT3DTEXTURE2   lptex;	// texture interface
      float ratio_width;
      float ratio_height;
    } LightMap;
  };
  /// linked list
  csD3DCacheData *next, *prev;
};

///
enum csCacheType
{
  CS_TEXTURE, CS_LIGHTMAP
};

///
class D3DCache
{
protected:
  csCacheType type;
  /// the head and tail of the cache data
  csD3DCacheData *head, *tail;

protected:
  /// the maximum size of the cache
  long cache_size;
  /// number of items
  int num;
  /// the total size of the cache
  long total_size;

public:
  /// takes the maximum size of the cache
  D3DCache (int max_size, csCacheType type, int bpp);

  ///
  void cache_texture (iTextureHandle *texture);
  ///
  void cache_lightmap (iPolygonTexture *polytex);
  ///
  void Clear ();

  ///
  virtual void Dump () = 0;

protected:
  ///
  int bpp;

  ///
  virtual void Load (csD3DCacheData *d) = 0;
  ///
  virtual void Unload (csD3DCacheData *d) = 0;
};

///
class D3DTextureCache: public D3DCache
{
private:
  ///
  bool m_bHardware;
  bool m_bMipMapping;
  ///
  LPDIRECTDRAW4 m_lpDD;
  ///
  LPDIRECT3DDEVICE3 m_lpD3dDevice;

  csGraphics3DCaps *m_pRendercaps;

public:
  ///
  D3DTextureCache(int nMaxSize, bool bHardware, LPDIRECTDRAW4 pDDraw, 
                  LPDIRECT3DDEVICE3 pDevice, int nBpp, bool bMipmapping,
		  csGraphics3DCaps *pRendercaps);
  virtual ~D3DTextureCache ()
  { Clear (); }

  ///
  virtual void Dump();
  
protected:
  ///
  virtual void Load(csD3DCacheData *cached_texture);
  ///
  virtual void Unload(csD3DCacheData *cached_texture);

private:
  ///
  void LoadIntoVRAM(csD3DCacheData *cached_texture);
};

///
class D3DLightMapCache: public D3DCache
{
private:
  ///
  bool m_bHardware;
  ///
  LPDIRECTDRAW4 m_lpDD;
  ///
  //@@@ some operations use Direct3DDevice3 some do use Direct3DDevice2,
  //due a missing implementation of DirectTexture3
  LPDIRECT3DDEVICE3 m_lpD3dDevice;
public:
  ///
  D3DLightMapCache(int nMaxSize, bool bHardware, LPDIRECTDRAW4 pDDraw, LPDIRECT3DDEVICE3 pDevice, int nBpp);
  ///
  virtual ~D3DLightMapCache ()
  { Clear (); }
  ///
  virtual void Dump();
  
protected:
  ///
  virtual void Load(csD3DCacheData *cached_lightmap);
  ///
  virtual void Unload(csD3DCacheData *cached_lightmap);
private:
  ///
  void LoadIntoVRAM(csD3DCacheData *cached_lightmap);
};

#endif
