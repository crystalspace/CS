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

#include "IGraph3d.h"

#include "cs3d/direct3d61/d3d_g3d.h"
#include "cs3d/direct3d61/d3d_hicache.h"

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
  float ratio_width;
  float ratio_height;
};

///
class D3DTextureCache: public HighColorCache
{
private:
  ///
  bool m_bHardware;
  bool m_bMipMapping;
  ///
  LPDIRECTDRAW4 m_lpDD;
  ///
  LPDIRECT3DDEVICE3 m_lpD3dDevice;

  G3D_CAPS* m_pRendercaps;
  int       m_MaxAspectRatio;

public:
  ///
  D3DTextureCache(int nMaxSize, bool bHardware, LPDIRECTDRAW4 pDDraw, 
                  LPDIRECT3DDEVICE3 pDevice, int nBpp, bool bMipmapping,
		  G3D_CAPS* pRendercaps, int MaxAspectRatio);
  ///
  virtual void Dump();
  
protected:
  ///
  virtual void Load(HighColorCache_Data *d);
  ///
  virtual void Unload(HighColorCache_Data *d);

  unsigned char m_GammaCorrect[256];
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
  LPDIRECTDRAW4 m_lpDD;
  ///
  //@@@ some operations use Direct3DDevice3 some do use Direct3DDevice2,
  //due a missing implementation of DirectTexture3
  LPDIRECT3DDEVICE3 m_lpD3dDevice;
public:
  ///
  D3DLightMapCache(int nMaxSize, bool bHardware, LPDIRECTDRAW4 pDDraw, LPDIRECT3DDEVICE3 pDevice, int nBpp);
  ///
  virtual void Dump();
  
protected:
  ///
  virtual void Load(HighColorCache_Data *d);
  ///
  virtual void Unload(HighColorCache_Data *d);

  unsigned char m_GammaCorrect[256];
private:
  ///
  void LoadIntoVRAM(D3DLightCache_Data *tex);
};

#endif
