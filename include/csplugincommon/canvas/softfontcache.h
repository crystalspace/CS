/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_CANVAS_COMMON_SOFTFONTCACHE_H__
#define __CS_CANVAS_COMMON_SOFTFONTCACHE_H__

#include "csextern.h"
#include "fontcache.h"

class csGraphics2D;

class CS_CSPLUGINCOMMON_EXPORT csSoftFontCache : public csFontCache
{
public:
  struct SoftGlyphCacheData : public csFontCache::GlyphCacheData
  {
    csRef<iDataBuffer> glyphDataBuf;
    csRef<iDataBuffer> glyphAlphaDataBuf;
    uint8* glyphData;
    uint8* glyphAlphaData;
    csBitmapMetrics bitmapMetrics;
    csBitmapMetrics alphaMetrics;
  };

  size_t cacheRemaining;
  csGraphics2D* G2D;

  /// Cache canvas-dependent information for a specific font/glyph pair.
  virtual GlyphCacheData* InternalCacheGlyph (KnownFont* font,
    utf32_char glyph, uint flags);
  /// Uncache canvas-dependent information.
  virtual void InternalUncacheGlyph (GlyphCacheData* cacheData);
public:
  csSoftFontCache (csGraphics2D* G2D);
  virtual ~csSoftFontCache ();
};

#endif // __CS_CANVAS_COMMON_SOFTFONTCACHE_H__
