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

#ifndef __CS_CANVAS_OPENGLCOMMON_GLFONTCACHE_H__
#define __CS_CANVAS_OPENGLCOMMON_GLFONTCACHE_H__

#include "csgeom/subrec2.h"
#include "csgeom/vector2.h"
#include "csutil/garray.h"

#include "../common/fontcache.h"

/*
  ATI Radeon9700P, Cat3.6 driver, exhibit the following problem:
  When uploading glyphs with glTexSubImage2D(), a number of the
  first drawn glyphs doesn't show up right (rather, instead of the
  expected texture content only the top left texel is drawn over the
  whole area of the glyph on-screen.) The problem does not appear if
  glTexImage2D() is used. Hence this ugly hack, where super texture
  is stored in RAM, glyphs are written to it in RAM and the whole
  thing is uploaded at once.
  [res]
  Update: As of Cat3.8, the problem seems to be fixed.
 */
//#define HACK_AROUND_WEIRD_ATI_TEXSUBIMAGE_PROBLEM

class csGraphics2DGLCommon;

class csGLFontCache : public csFontCache
{
  struct GLGlyphCacheData : public csFontCache::GlyphCacheData
  {
    csSubRect2* subrect;
    float tx1, ty1, tx2, ty2;
    int texNum;
    csBitmapMetrics bmetrics;
  };

  csGraphics2DGLCommon* G2D;

  int texSize, maxTxts;

  struct CacheTexture
  {
    GLuint handle;
    csSubRectangles2* glyphRects;
#ifdef HACK_AROUND_WEIRD_ATI_TEXSUBIMAGE_PROBLEM
    uint8* data;
#endif

    CacheTexture () 
    { 
      glyphRects = 0; 
#ifdef HACK_AROUND_WEIRD_ATI_TEXSUBIMAGE_PROBLEM
      data = 0;
#endif
    }
    ~CacheTexture () 
    { 
      delete glyphRects; 
#ifdef HACK_AROUND_WEIRD_ATI_TEXSUBIMAGE_PROBLEM
      delete [] data;
#endif
    }
    void InitRects (int size) 
    { 
      glyphRects = new csSubRectangles2 (csRect (0, 0, size, size));
    }
  };
  csArray<CacheTexture> textures;
  csBlockAllocator<GLGlyphCacheData> cacheDataAlloc;

  csDirtyAccessArray<csVector2> wtVerts2d;
  csDirtyAccessArray<csVector2> wtTexcoords;

  inline void FlushArrays (int& numverts, int bgVertsOffset, 
    int& numBgVerts, const int fg, const int bg);
protected:
  virtual GlyphCacheData* InternalCacheGlyph (KnownFont* font,
    utf32_char glyph, uint flags);
  virtual void InternalUncacheGlyph (GlyphCacheData* cacheData);

  void CopyGlyphData (iFont* font, utf32_char glyph, int tex,
    const csRect& rect, iDataBuffer* bitmapDataBuf, iDataBuffer* alphaDataBuf);

  bool ClipRect (float x, float y,
    float &x1, float &y1, float &x2, float &y2,
    float &tx1, float &ty1, float &tx2, float &ty2);
public:
  csGLFontCache (csGraphics2DGLCommon* G2D);
  virtual ~csGLFontCache ();

  virtual void SetClipRect (int x1, int y1, int x2, int y2);

  /**
   * Draw a string using OpenGL, where the baseline starts at x,y. 
   */
  virtual void WriteString (iFont *font, int pen_x, int pen_y, 
    int fg, int bg, const utf8_char* text, uint flags);
};

#endif // __CS_CANVAS_OPENGLCOMMON_GLFONTCACHE_H__
