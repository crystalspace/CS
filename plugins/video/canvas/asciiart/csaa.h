/*
    ASCII art rendering support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSAA_H__
#define __CSAA_H__

#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cssys/common/system.h"
#include "cssys/os2/icsos2.h"
#include "aalib.h"

extern const CLSID CLSID_AAGraphics2D;

/// IGraphics2DFactory interface implementation
class csGraphics2DFactoryAA : public IGraphics2DFactory
{
public:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DFactoryAA)

  STDMETHOD (CreateInstance) (REFIID riid, ISystem* piSystem, void** ppv);
  STDMETHOD (LockServer) (COMBOOL bLock);
};

/**
 * The Ascii Art driver. This is a cross-platform graphics driver which
 * implements drawing using characters and (depending on platform)
 * different intensities.
 */
class csGraphics2DAA : public csGraphics2D
{
  /// The configuration file
  csIniFile* config;
  /// Pointer to system driver interface
  ISystem* System;
  /// Use native mouse cursor, if possible?
  bool HardwareCursor;
  /// The AAlib context
  aa_context *context;
  /// The palette
  aa_palette palette;

public:
  csGraphics2DAA (ISystem* piSystem);
  virtual ~csGraphics2DAA ();

  virtual void Initialize ();
  virtual bool Open (char *Title);
  virtual void Close ();

  virtual void Print (csRect *area = NULL);

  virtual void SetRGB (int i, int r, int g, int b);

  /* These procedures locks/unlocks DIVE buffer */
  virtual bool BeginDraw ();
  virtual void FinishDraw ();

  virtual bool SetMousePosition (int x, int y);
  virtual bool SetMouseCursor (int iShape, ITextureHandle *hBitmap);

protected:
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics2DAA)
};

#endif // __CSAA_H__
