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
#include "ddraw.h"
#include "d3d.h"
#include "d3dcaps.h"


#include "cs3d/direct3d6/g3d_d3d.h"
#include "cs3d/common/hicache.h"

///
struct D3DTextureCache_Data 
{
  LPDIRECTDRAWSURFACE4 lpsurf;		// texture data surface
  LPDIRECT3DTEXTURE2 lptex;		// texture interface
  
  LPDIRECTDRAWPALETTE lpddpal;	// texture palette
};

///
struct D3DLightCache_Data
{
  LPDIRECTDRAWSURFACE4 lpsurf;		// texture data surface
  LPDIRECT3DTEXTURE2 lptex;		// texture interface
};

///
class D3DTextureCache: public HighColorCache
{
private:
  ///
  bool m_bHardware;
  bool m_bMipMapping;
  ///
  IDirectDraw4 * m_lpDD;
  ///
  IDirect3DDevice3 * m_lpD3dDevice;
public:
  ///
  D3DTextureCache(int nMaxSize, bool bHardware, LPDIRECTDRAW4 pDDraw, LPDIRECT3DDEVICE3 pDevice, int nBpp, bool bMipmapping = true);
  ///
  virtual void Dump();
  
protected:
  ///
  virtual void Load(csHighColorCacheData *d);
  ///
  virtual void Unload(csHighColorCacheData *d);
private:
  ///
  void LoadIntoVRAM(D3DTextureCache_Data *tex);
};

///
class D3DLightMapCache: public HighColorCache
{
private:
  ///
  bool m_bHardware;
  ///
  IDirectDraw4 * m_lpDD;
  ///
  IDirect3DDevice3 * m_lpD3dDevice;
public:
  ///
  D3DLightMapCache(int nMaxSize, bool bHardware, LPDIRECTDRAW4 pDDraw, LPDIRECT3DDEVICE3 pDevice, int nBpp);
  ///
  virtual void Dump();
  
protected:
  ///
  virtual void Load(csHighColorCacheData *d);
  ///
  virtual void Unload(csHighColorCacheData *d);
private:
  ///
  void LoadIntoVRAM(D3DLightCache_Data *tex);
};

#endif
