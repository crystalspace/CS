/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#ifndef __CS_SOFTSHADER_SCANLINERENDERERS_H__
#define __CS_SOFTSHADER_SCANLINERENDERERS_H__

#include "scanline_base.h"

namespace cspluginSoftshader
{
  /* Instantiations of the different pixel format scanline renderers are
   * split over several source file to lessen the burden on the compiler. */
  extern ScanlineRendererBase* NewScanlineRendererARGB8888 (
    const csPixelFormat& pfmt);
  extern ScanlineRendererBase* NewScanlineRendererABGR8888 (
    const csPixelFormat& pfmt);
  extern ScanlineRendererBase* NewScanlineRendererARGBgen32 (
    const csPixelFormat& pfmt);
  extern ScanlineRendererBase* NewScanlineRendererABGRgen32 (
    const csPixelFormat& pfmt);
  
  extern ScanlineRendererBase* NewScanlineRendererRGB565 (
    const csPixelFormat& pfmt);
  extern ScanlineRendererBase* NewScanlineRendererBGR565 (
    const csPixelFormat& pfmt);
  extern ScanlineRendererBase* NewScanlineRendererRGB555 (
    const csPixelFormat& pfmt);
  extern ScanlineRendererBase* NewScanlineRendererBGR555 (
    const csPixelFormat& pfmt);
  extern ScanlineRendererBase* NewScanlineRendererRGBgen16 (
    const csPixelFormat& pfmt);
  extern ScanlineRendererBase* NewScanlineRendererBGRgen16 (
    const csPixelFormat& pfmt);
} // namespace cspluginSoftshader

#endif // __CS_SOFTSHADER_SCANLINERENDERERS_H__
