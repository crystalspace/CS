/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
	      (C) 1999 by Gary Haussmann
			  Samuel Humphreys

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

#include "cssysdef.h"

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "cssys/csuctransform.h"
#include "iutil/databuff.h"
#include "ivideo/fontserv.h"

#include "glcommon2d.h"
#include "glfontcache.h"

//---------------------------------------------------------------------------

csGLFontCache::csGLFontCache (csGraphics2DGLCommon* G2D) : cacheDataAlloc(512)
{
  csGLFontCache::G2D = G2D;

  texSize = G2D->config->GetInt ("Video.OpenGL.FontCache.TextureSize", 256);
  texSize = MAX (texSize, 64);
  maxTxts = G2D->config->GetInt ("Video.OpenGL.FontCache.MaxTextureNum", 16);
  maxTxts = MAX (maxTxts, 1);
}

csGLFontCache::~csGLFontCache ()
{
  CleanupCache ();

  G2D->statecache->SetTexture (GL_TEXTURE_2D, 0);
  int tex;
  for (tex = 0; tex < textures.Length (); tex++)
  {
    glDeleteTextures (1, &textures[tex].handle);
  }
  textures.DeleteAll ();
}

csGLFontCache::GlyphCacheData* csGLFontCache::InternalCacheGlyph (
  KnownFont* font, utf32_char glyph, uint flags)
{
  bool hasGlyph = font->font->HasGlyph (glyph);
  if (!hasGlyph)
  {
    GLGlyphCacheData* cacheData = cacheDataAlloc.Alloc ();
    memset (cacheData, 0, sizeof (GLGlyphCacheData));
    cacheData->font = font;
    cacheData->glyph = glyph;
    cacheData->hasGlyph = false;
    return cacheData;
  }
  csRect texRect;
  csSubRect2* sr = 0;

  csBitmapMetrics bmetrics;
  csRef<iDataBuffer> alphaData;
  if ((flags & CS_WRITE_NOANTIALIAS) == 0)
  {
    alphaData = font->font->GetGlyphAlphaBitmap (glyph, bmetrics);
  }
  csRef<iDataBuffer> bitmapData;
  if (!alphaData)
    bitmapData = font->font->GetGlyphBitmap (glyph, bmetrics);

  int tex = 0;
  while (tex < textures.Length ())
  {
    sr = textures[tex].glyphRects->Alloc (bmetrics.width, bmetrics.height, 
      texRect);
    if (sr != 0)
    {
      break;
    }
    tex++;
  }
  if ((sr == 0) && (textures.Length () < maxTxts))
  {
    tex = textures.Length ();
    textures.SetLength (textures.Length () + 1);

    textures[tex].InitRects (texSize);

    glGenTextures (1, &textures[tex].handle);
    G2D->statecache->SetTexture (GL_TEXTURE_2D, textures[tex].handle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    uint8* texImage = new uint8[texSize * texSize];
#ifdef CS_DEBUG
    uint8* p = texImage;
    for (int y = 0; y < texSize; y++)
    {
      for (int x = 0; x < texSize; x++)
      {
	*p++ = 0x7f + (((x ^ y) & 1) << 7);
      }
    }
#endif

#ifdef HACK_AROUND_WEIRD_ATI_TEXSUBIMAGE_PROBLEM
    textures[tex].data = texImage;  
#else
    glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, texSize, texSize, 0, 
      GL_ALPHA, GL_UNSIGNED_BYTE, texImage);
    delete[] texImage;
#endif

    G2D->statecache->SetTexture (GL_TEXTURE_2D, 0);

    sr = textures[tex].glyphRects->Alloc (bmetrics.width, bmetrics.height, 
      texRect);
  }
  if (sr != 0)
  {
    GLGlyphCacheData* cacheData = cacheDataAlloc.Alloc ();
    cacheData->subrect = sr;
    cacheData->texNum = tex;
    cacheData->font = font;
    cacheData->glyph = glyph;
    cacheData->flags = flags & RELEVANT_WRITE_FLAGS;
    cacheData->bmetrics = bmetrics;
    font->font->GetGlyphMetrics (glyph, cacheData->glyphMetrics);
    cacheData->tx1 = (float)texRect.xmin / (float)texSize;
    cacheData->ty1 = (float)texRect.ymin / (float)texSize;
    cacheData->tx2 = (float)texRect.xmax / (float)texSize;
    cacheData->ty2 = (float)texRect.ymax / (float)texSize;
    cacheData->hasGlyph = true;

    CopyGlyphData (font->font, glyph, tex, texRect, bitmapData, alphaData);

    return cacheData;
  }
  return 0;
}

void csGLFontCache::InternalUncacheGlyph (GlyphCacheData* cacheData)
{
  GLGlyphCacheData* glCacheData = (GLGlyphCacheData*)cacheData;
  textures[glCacheData->texNum].glyphRects->Reclaim (glCacheData->subrect);
  cacheDataAlloc.Free (glCacheData);
}

void csGLFontCache::CopyGlyphData (iFont* font, utf32_char glyph, int tex, 
				   const csRect& rect, iDataBuffer* bitmapDataBuf, 
				   iDataBuffer* alphaDataBuf)
{
  G2D->statecache->SetTexture (GL_TEXTURE_2D, textures[tex].handle);

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  if (alphaDataBuf)
  {
    uint8* alphaData = alphaDataBuf->GetUint8 ();
#ifdef HACK_AROUND_WEIRD_ATI_TEXSUBIMAGE_PROBLEM
    uint8* dest = textures[tex].data + (rect.ymin * texSize) + rect.xmin;
    uint8* src = alphaData;
    int y;
    for (y = 0; y < rect.Height (); y++)
    {
      memcpy (dest, src, rect.Width ());
      dest += texSize;
      src += rect.Width ();
    }
    glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, texSize, texSize, 0, 
      GL_ALPHA, GL_UNSIGNED_BYTE, textures[tex].data);
#else
    glTexSubImage2D (GL_TEXTURE_2D, 0, rect.xmin, rect.ymin, 
      rect.Width (), rect.Height (), GL_ALPHA, 
      GL_UNSIGNED_BYTE, alphaData);
#endif
  }
  else
  {
    if (bitmapDataBuf)
    {
      uint8* bitData = bitmapDataBuf->GetUint8 ();

#ifdef HACK_AROUND_WEIRD_ATI_TEXSUBIMAGE_PROBLEM
      uint8* dest = textures[tex].data + (rect.ymin * texSize) + rect.xmin;
      int ladd = texSize - rect.Width ();
#else
      uint8* alphaData = new uint8[rect.Width () * rect.Height ()];
      uint8* dest = alphaData;
#endif
      uint8* src = bitData;
      uint8 byte = *src++;
      int x, y;
      for (y = 0; y < rect.Height (); y++)
      {
	for (x = 0; x < rect.Width (); x++)
	{
	  *dest++ = (byte & 0x80) ? 0xff : 0;
	  if ((x & 7) == 7)
	  {
	    byte = *src++;
	  }
	  else
	  {
	    byte <<= 1;
	  }
	}
#ifdef HACK_AROUND_WEIRD_ATI_TEXSUBIMAGE_PROBLEM
	dest += ladd;
#endif
	if ((rect.Width () & 7) != 0) byte = *src++;
      }

#ifdef HACK_AROUND_WEIRD_ATI_TEXSUBIMAGE_PROBLEM
      glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, texSize, texSize, 0, 
	GL_ALPHA, GL_UNSIGNED_BYTE, textures[tex].data);
#else
      glTexSubImage2D (GL_TEXTURE_2D, 0, rect.xmin, rect.ymin, 
	rect.Width (), rect.Height (), 
	GL_ALPHA, GL_UNSIGNED_BYTE, alphaData);
      delete[] alphaData;
#endif
    }
  }
}

void csGLFontCache::SetClipRect (int x1, int y1, int x2, int y2)
{ 
  ClipX1 = x1; 
  ClipY1 = G2D->Height - y2; 
  ClipX2 = x2; 
  ClipY2 = G2D->Height - y1; 
}

bool csGLFontCache::ClipRect (float x, float y,
  float &x1, float &y1, float &x2, float &y2,
  float &tx1, float &ty1, float &tx2, float &ty2)
{
  float nx1 = x1 + x, ny1 = y1 + y, nx2 = x2 + x, ny2 = y2 + y;
  float ntx1 = tx1, nty1 = ty1, ntx2 = tx2, nty2 = ty2;

  if ((nx1 > float (ClipX2)) || (nx2 < float (ClipX1))
   || (ny1 > float (ClipY2)) || (ny2 < float (ClipY1)))
      return false;

  if (nx1 < ClipX1)
    tx1 += (ntx2 - ntx1) * (ClipX1 - nx1) / (nx2 - nx1), x1 = ClipX1 - x;
  if (nx2 > ClipX2)
    tx2 -= (ntx2 - ntx1) * (nx2 - ClipX2) / (nx2 - nx1), x2 = ClipX2 - x;
  if (tx2 <= tx1)
    return false;

  if (ny1 < ClipY1)
    ty2 -= (nty2 - nty1) * (ClipY1 - ny1) / (ny2 - ny1), y1 = ClipY1 - y;
  if (ny2 > ClipY2)
    ty1 += (nty2 - nty1) * (ny2 - ClipY2) / (ny2 - ny1), y2 = ClipY2 - y;
  if (ty2 <= ty1)
    return false;

  return true;
}

void csGLFontCache::FlushArrays (GLuint texture, int& numverts, 
				 int bgVertOffset, int& numBgVerts, 
				 const int fg, const int bg)
{
  if (numverts != 0)
  {
    if (bg >= 0)
    {
      G2D->statecache->Disable_GL_TEXTURE_2D ();
      G2D->setGLColorfromint (bg);
      glDrawArrays(GL_QUADS, bgVertOffset, numBgVerts);
      G2D->statecache->Enable_GL_TEXTURE_2D ();
      G2D->setGLColorfromint (fg);
    }

    G2D->statecache->SetTexture (GL_TEXTURE_2D, texture);
    glDrawArrays(GL_QUADS, 0, numverts);
    numverts = 0;
    numBgVerts = 0;
  }
}

void csGLFontCache::WriteString (iFont *font, int pen_x, int pen_y, 
				 int fg, int bg, const utf8_char* text,
				 uint flags)
{
  if (!text || !*text) return;

  if (!(flags & CS_WRITE_BASELINE)) pen_y += font->GetAscent ();

  bool gl_texture2d = G2D->statecache->IsEnabled_GL_TEXTURE_2D ();
  if (!gl_texture2d) G2D->statecache->Enable_GL_TEXTURE_2D ();
  if (bg < 0) G2D->setGLColorfromint (fg);

  int maxwidth, maxheight;
  font->GetMaxSize (maxwidth, maxheight);

  KnownFont* knownFont = GetCachedFont (font);
  if (knownFont == 0) knownFont = CacheFont (font);

  pen_y = G2D->Height - pen_y/* - maxheight*/;
  if (pen_y >= ClipY2) return;

  glPushMatrix ();
  glTranslatef (pen_x, pen_y, 0);

  G2D->statecache->Enable_GL_BLEND ();
  G2D->statecache->SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLuint lastTexture = 0;
  int numverts = 0, numBgVerts = 0;

  GLboolean vaenabled = glIsEnabled(GL_VERTEX_ARRAY);
  GLboolean tcaenabled = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
  GLboolean caenabled = glIsEnabled(GL_COLOR_ARRAY);

  if(vaenabled == GL_FALSE)
    glEnableClientState(GL_VERTEX_ARRAY);
  if(tcaenabled == GL_FALSE)
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  if(caenabled == GL_TRUE)
    glDisableClientState(GL_COLOR_ARRAY);

  int textLen = strlen ((char*)text);

  if (wtTexcoords.Length () < (textLen * 4)) wtTexcoords.SetLength (textLen * 4);
  if (wtVerts2d.Length () < (textLen * 12)) wtVerts2d.SetLength (textLen * 12);
  int bgVertOffset = textLen * 4;
  glTexCoordPointer(2, GL_FLOAT, sizeof (csVector2), wtTexcoords.GetArray ());
  glVertexPointer(2, GL_FLOAT, sizeof (csVector2), wtVerts2d.GetArray ());

  float x1 = 0.0;
  float x2, y1, y2;
  int advance = 0;
  bool firstchar = true;
  float oldH = 0.0f;

  while (textLen > 0)
  {
    utf32_char glyph;
    int skip = csUnicodeTransform::UTF8Decode (text, textLen, glyph, 0);
    if (skip == 0) break;

    text += skip;
    textLen -= skip;

    const GLGlyphCacheData* cacheData = 
      (GLGlyphCacheData*)GetCacheData (knownFont, glyph, flags);
    if (cacheData == 0)
    {
      FlushArrays (lastTexture, numverts, bgVertOffset, numBgVerts, fg, bg);
      cacheData = (GLGlyphCacheData*)CacheGlyphUnsafe (knownFont, glyph, 
	flags);
    }
    if (!cacheData->hasGlyph)
    {
      // fall back to the default glyph (CS_FONT_DEFAULT_GLYPH)
      FlushArrays (lastTexture, numverts, bgVertOffset, numBgVerts, fg, bg);
      cacheData = (GLGlyphCacheData*)CacheGlyph (knownFont, 
	CS_FONT_DEFAULT_GLYPH, flags);
      if (!cacheData->hasGlyph) continue;
    }

    GLuint newHandle = textures[cacheData->texNum].handle;
    if (lastTexture != newHandle) 
    {
      FlushArrays (lastTexture, numverts, bgVertOffset, numBgVerts, fg, bg);
      lastTexture = newHandle;
    }

    advance += cacheData->bmetrics.left;
    
    // Hack: in case the first char has a negative left bitmap offset,
    // some of the background isn't drawn. Fix that.
    if (firstchar)
    {
      if (advance < 0)
      {
	advance = 0;
      }
      firstchar = false;
    }
   
    float x_left = x1;
    x1 = x1 + cacheData->bmetrics.left;
    x2 = x1 + cacheData->bmetrics.width;
    float tx1, tx2, ty1, ty2;

    tx1 = cacheData->tx1;
    tx2 = cacheData->tx2;
    ty1 = cacheData->ty1;
    ty2 = cacheData->ty2;
    y2 = cacheData->bmetrics.top;
    y1 = y2 - cacheData->bmetrics.height;

    if (ClipRect (pen_x, pen_y, x1, y1, x2, y2, tx1, ty1, tx2, ty2))
    {
      wtTexcoords[numverts].x = tx1;
      wtTexcoords[numverts].y = ty1;
      wtVerts2d[numverts].x = x1;
      wtVerts2d[numverts].y = y2;
      numverts++;

      wtTexcoords[numverts].x = tx2;
      wtTexcoords[numverts].y = ty1;
      wtVerts2d[numverts].x = x2;
      wtVerts2d[numverts].y = y2;
      numverts++;

      wtTexcoords[numverts].x = tx2;
      wtTexcoords[numverts].y = ty2;
      wtVerts2d[numverts].x = x2;
      wtVerts2d[numverts].y = y1;
      numverts++;
      
      wtTexcoords[numverts].x = tx1;
      wtTexcoords[numverts].y = ty2;
      wtVerts2d[numverts].x = x1;
      wtVerts2d[numverts].y = y1;
      numverts++;
    }

    if (bg >= 0)
    {
      float bx1 = x_left;
      float bx2 = x2;
      float by1 = y1;
      float by2 = y2;

      if (advance > 0)
      {
	bx1 -= advance;
	advance = 0;
      }
      else if (advance < 0)
      {
	bx1 -= (float)advance;
	// The texcoords are irrelevant for the BG, but ClipRect() rejects 
	// null-width or height glyphs. But we still want a BG for them.
	tx1 = ty1 = 0.0f;
	tx2 = ty2 = 1.0f;
	if (ClipRect (pen_x, pen_y, bx1, by1, bx2, by2, tx1, ty1, tx2, ty2))
	{
	  wtVerts2d[bgVertOffset + numBgVerts].x = bx1;
	  wtVerts2d[bgVertOffset + numBgVerts].y = by2;
	  numBgVerts++;
	  wtVerts2d[bgVertOffset + numBgVerts].x = bx2;
	  wtVerts2d[bgVertOffset + numBgVerts].y = by2;
	  numBgVerts++;
	  wtVerts2d[bgVertOffset + numBgVerts].x = bx2;
	  wtVerts2d[bgVertOffset + numBgVerts].y = by1;
	  numBgVerts++;
	  wtVerts2d[bgVertOffset + numBgVerts].x = bx1;
	  wtVerts2d[bgVertOffset + numBgVerts].y = by1;
	  numBgVerts++;
	}

	if (oldH < (y2 - y1))
	{
	  bx2 = bx1;
	  bx1 += (float)advance;
	  by1 += oldH;
	}
	advance = 0;
      }

      tx1 = ty1 = 0.0f;
      tx2 = ty2 = 1.0f;
      if (ClipRect (pen_x, pen_y, bx1, by1, bx2, by2, tx1, ty1, tx2, ty2))
      {
	wtVerts2d[bgVertOffset + numBgVerts].x = bx1;
	wtVerts2d[bgVertOffset + numBgVerts].y = by2;
	numBgVerts++;
	wtVerts2d[bgVertOffset + numBgVerts].x = bx2;
	wtVerts2d[bgVertOffset + numBgVerts].y = by2;
	numBgVerts++;
	wtVerts2d[bgVertOffset + numBgVerts].x = bx2;
	wtVerts2d[bgVertOffset + numBgVerts].y = by1;
	numBgVerts++;
	wtVerts2d[bgVertOffset + numBgVerts].x = bx1;
	wtVerts2d[bgVertOffset + numBgVerts].y = by1;
	numBgVerts++;
      }
    }

    x1 = x_left + cacheData->glyphMetrics.advance;
    advance += cacheData->glyphMetrics.advance - 
      (cacheData->bmetrics.width + cacheData->bmetrics.left);
    oldH = y2 - y1;
  }

  // "Trailing" background
  if ((bg >= 0) & (advance > 0))
  {
    float bx1 = x1 - advance;
    float bx2 = x2;
    float by1 = y1;
    float by2 = y2;

    float tx1, ty1, tx2, ty2;
    tx1 = ty1 = 0.0f;
    tx2 = ty2 = 1.0f;
    if (ClipRect (pen_x, pen_y, bx1, by1, bx2, by2, tx1, ty1, tx2, ty2))
    {
      wtVerts2d[bgVertOffset + numBgVerts].x = bx1;
      wtVerts2d[bgVertOffset + numBgVerts].y = by2;
      numBgVerts++;
      wtVerts2d[bgVertOffset + numBgVerts].x = bx2;
      wtVerts2d[bgVertOffset + numBgVerts].y = by2;
      numBgVerts++;
      wtVerts2d[bgVertOffset + numBgVerts].x = bx2;
      wtVerts2d[bgVertOffset + numBgVerts].y = by1;
      numBgVerts++;
      wtVerts2d[bgVertOffset + numBgVerts].x = bx1;
      wtVerts2d[bgVertOffset + numBgVerts].y = by1;
      numBgVerts++;
    }
  }

  FlushArrays (lastTexture, numverts, bgVertOffset, numBgVerts, fg, bg);

  if(vaenabled == GL_FALSE)
    glDisableClientState(GL_VERTEX_ARRAY);
  if(tcaenabled == GL_FALSE)
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  if(caenabled == GL_TRUE)
    glEnableClientState(GL_COLOR_ARRAY);

  G2D->statecache->Disable_GL_BLEND ();
  glPopMatrix ();

  if (!gl_texture2d) G2D->statecache->Disable_GL_TEXTURE_2D ();
  
  PurgeEmptyPlanes (knownFont);
}
