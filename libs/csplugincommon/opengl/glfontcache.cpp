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

#include "csutil/csuctransform.h"
#include "csgfx/memimage.h"
#include "iutil/databuff.h"
#include "ivideo/fontserv.h"

#include "csplugincommon/opengl/glcommon2d.h"
#include "csplugincommon/opengl/glfontcache.h"

const GLenum fontFilterMode = GL_LINEAR;

//---------------------------------------------------------------------------

csGLFontCache::csGLFontCache (csGraphics2DGLCommon* G2D) : 
  cacheDataAlloc (512), verts2d (256, 256), texcoords (256, 256)
{
  csGLFontCache::G2D = G2D;
  statecache = G2D->statecache;

  usedTexs = 0;

  glyphAlign = 1;
  numFloats = 0;
  jobCount = 0;

  textWriting = false;
}

csGLFontCache::~csGLFontCache ()
{
  CleanupCache ();

  statecache->SetTexture (GL_TEXTURE_2D, 0);
  size_t tex;
  for (tex = 0; tex < textures.Length (); tex++)
  {
    glDeleteTextures (1, &textures[tex].handle);
    if (!(multiTexText || intensityBlendText))
      glDeleteTextures (1, &textures[tex].mirrorHandle);
  }
  glDeleteTextures (1, &texWhite);
  textures.DeleteAll ();
}

void csGLFontCache::Setup()
{
  GLint maxtex = 256;
  glGetIntegerv (GL_MAX_TEXTURE_SIZE, &maxtex);

  multiTexText = G2D->config->GetBool (
    "Video.OpenGL.FontCache.UseMultiTexturing", true) && G2D->useCombineTE;
  intensityBlendText = G2D->config->GetBool (
    "Video.OpenGL.FontCache.UseIntensityBlend", true);

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (G2D->object_reg, iVerbosityManager));
  bool do_verbose = false;
  if (verbosemgr) 
    do_verbose = verbosemgr->CheckFlag ("renderer", "fontcache");
  if (do_verbose)
  {
    int textMethod;
    if (multiTexText)
      textMethod = 0;
    else if (intensityBlendText)
      textMethod = 1;
    else
      textMethod = 2;
    static const char* textMethodStr[3] =
      {"Multitexturing",
       "GL_BLEND texenv with GL_INTENSITY texture",
       "GL_MODULATE, two-pass"};
    csReport (G2D->object_reg, CS_REPORTER_SEVERITY_NOTIFY,
      "crystalspace.canvas.openglcommon.fontcache",
      "Text drawing method: %s", textMethodStr[textMethod]);
  }
  
  texSize = G2D->config->GetInt ("Video.OpenGL.FontCache.TextureSize", 256);
  texSize = MAX (texSize, 64);
  texSize = MIN (texSize, maxtex);
  maxTxts = G2D->config->GetInt ("Video.OpenGL.FontCache.MaxTextureNum", 16);
  maxTxts = MAX (maxTxts, 1);
  maxTxts = MIN (maxTxts, 32);
  maxFloats = G2D->config->GetInt ("Video.OpenGL.FontCache.VertexCache", 128);
  maxFloats = ((maxFloats + 3) / 4) * 4;
  maxFloats = MAX (maxFloats, 4);

  glGenTextures (1, &texWhite);
  statecache->SetTexture (GL_TEXTURE_2D, texWhite);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fontFilterMode);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, fontFilterMode);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  csRGBpixel texPix (255, 255, 255, 0);

  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, 
    GL_RGBA, GL_UNSIGNED_BYTE, &texPix);
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

  int allocWidth = bmetrics.width;
  int allocHeight = bmetrics.height;
  int coordCorrect = 0;
  while ((allocWidth > texSize) || (allocHeight > texSize))
  {
    allocWidth = MAX ((allocWidth+1) / 2, 1);
    allocHeight = MAX ((allocHeight+1) / 2, 1);
    coordCorrect = 1;
  }
  /*if (glyphAlign != 1) // uncomment if glyphAlign gets != 1 someday
  {
    allocWidth = 
      ((bmetrics.width + glyphAlign - 1) / glyphAlign) * glyphAlign;
    allocHeight = 
      ((bmetrics.height + glyphAlign - 1) / glyphAlign) * glyphAlign;
  }*/
  size_t tex = 0;
  while (tex < textures.Length ())
  {
    sr = textures[tex].glyphRects->Alloc (allocWidth, allocHeight, 
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
    statecache->SetTexture (GL_TEXTURE_2D, textures[tex].handle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fontFilterMode);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, fontFilterMode);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    uint8* texImage = new uint8[texSize * texSize];
#ifdef CS_DEBUG
    uint8* p = texImage;
    for (int y = 0; y < texSize; y++)
    {
      for (int x = 0; x < texSize; x++)
      {
	const uint8 val = 0x7f + (((x ^ y) & 1) << 7);
	*p++ = val;
      }
    }
#endif
    // Alloc a pixel on that texture for background drawing.
    *texImage = multiTexText ? 0 : 255;
    textures[tex].glyphRects->Alloc (1, 1, texRect);

    glTexImage2D (GL_TEXTURE_2D, 0, 
      (multiTexText || intensityBlendText) ? GL_INTENSITY : GL_ALPHA, 
      texSize, texSize, 0, 
      (multiTexText || intensityBlendText) ? GL_LUMINANCE : GL_ALPHA, 
      GL_UNSIGNED_BYTE, texImage);
    
    if (!(multiTexText || intensityBlendText))
    {
      glGenTextures (1, &textures[tex].mirrorHandle);
      statecache->SetTexture (GL_TEXTURE_2D, textures[tex].mirrorHandle);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fontFilterMode);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, fontFilterMode);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      *texImage = 0;
      glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, texSize, texSize, 0, 
        GL_ALPHA, GL_UNSIGNED_BYTE, texImage);
    }
    else
      textures[tex].mirrorHandle = 0;
    delete[] texImage;

    statecache->SetTexture (GL_TEXTURE_2D, 0);

    sr = textures[tex].glyphRects->Alloc (allocWidth, allocHeight, 
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
    const float tsf = (float)texSize;
    // When using size-reduced glyphs, nudge the TCs slightly inward
    // to reduce leaking in of neighbouring glyphs.
    const float tccorrect = (float)((1 << coordCorrect) / 2) * (0.5f / tsf);
    cacheData->tx1 = (float)texRect.xmin / tsf + tccorrect;
    cacheData->ty1 = (float)texRect.ymin / tsf + tccorrect;
    cacheData->tx2 = (float)(texRect.xmax) / tsf - tccorrect;
    cacheData->ty2 = (float)(texRect.ymax) / tsf - tccorrect;
    cacheData->hasGlyph = true;

    CopyGlyphData (font->font, glyph, tex, bmetrics, texRect, bitmapData, 
      alphaData);

    return cacheData;
  }
  return 0;
}

void csGLFontCache::InternalUncacheGlyph (GlyphCacheData* cacheData)
{
  GLGlyphCacheData* glCacheData = (GLGlyphCacheData*)cacheData;
  const size_t texNum = glCacheData->texNum;
  if (usedTexs & (1 << texNum))
  {
    FlushArrays ();
    usedTexs &= ~(1 << texNum);
  }
  textures[texNum].glyphRects->Reclaim (glCacheData->subrect);
  cacheDataAlloc.Free (glCacheData);
}

/**
 * Shrink glyph data with a simple box filter.
 */
static void ShrinkGlyphData (uint8* glyph, int oldW, int oldH, int newW, int newH)
{
  const int boxX = (oldW + (newW - 1)) / newW;
  const int boxY = (oldH + (newH - 1)) / newH;
  CS_ALLOC_STACK_ARRAY (uint8, destLine, newW);

  for (int y = 0; y < newH; y++)
  {
    uint8* srcLine = glyph + y * boxY * oldW;
    for (int x = 0; x < newW; x++)
    {
      int val = 0; 
      int cnt = 0;
      uint8* box = srcLine + x * boxX;
      int by = MIN (boxY, oldH - y * boxY);
      while (by-- > 0)
      {
	int bx = MIN (boxX, oldW - x * boxX);
	while (bx-- > 0)
	{
	  val += box[bx];
	  cnt++;
	}
	box += oldW;
      }
      destLine[x] = (cnt > 0) ? (val / cnt) : 0;
    }
    memcpy (glyph + y * newW, destLine, newW);
  }
}

void csGLFontCache::CopyGlyphData (iFont* font, utf32_char glyph, size_t tex, 
				   const csBitmapMetrics& bmetrics, 
				   const csRect& texRect, iDataBuffer* bitmapDataBuf, 
				   iDataBuffer* alphaDataBuf)
{
  if ((texRect.Width () > 0) && (texRect.Height () > 0))
  {
    statecache->SetTexture (GL_TEXTURE_2D, textures[tex].handle);

    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

    uint8* intData = new uint8[MAX((texRect.Width () * texRect.Height ()),
      (bmetrics.width * bmetrics.height))];

    const uint8 valXor = multiTexText ? 0 : 0xff;
    const int padX = MAX (texRect.Width() - bmetrics.width, 0);
    if (alphaDataBuf)
    {
      uint8* alphaData = alphaDataBuf->GetUint8 ();
      uint8* dest = intData;
      uint8* src = alphaData;
      int x, y;
      for (y = 0; y < bmetrics.height; y++)
      {
	for (x = 0; x < bmetrics.width; x++)
	{
	  const uint8 val = *src++;
	  *dest++ = valXor ^ val;
	}
	dest += padX;
      }
    }
    else
    {
      if (bitmapDataBuf)
      {
	uint8* bitData = bitmapDataBuf->GetUint8 ();

	uint8* dest = intData;
	uint8* src = bitData;
	uint8 byte = *src++;
	int x, y;
	for (y = 0; y < bmetrics.height; y++)
	{
	  for (x = 0; x < bmetrics.width; x++)
	  {
	    const uint8 val = (byte & 0x80) ? 0xff : 0;
	    *dest++ = valXor ^ val;
	    if ((x & 7) == 7)
	    {
	      byte = *src++;
	    }
	    else
	    {
	      byte <<= 1;
	    }
	  }
	  if ((bmetrics.width & 7) != 0) byte = *src++;
	  dest += padX;
	}
      }
    }

    if ((texRect.Width() < bmetrics.width) || 
      (texRect.Height() < bmetrics.height))
    {
      ShrinkGlyphData (intData, bmetrics.width, bmetrics.height,
	texRect.Width(), texRect.Height());
    }

    glTexSubImage2D (GL_TEXTURE_2D, 0, texRect.xmin, texRect.ymin, 
      texRect.Width (), texRect.Height (), 
      (multiTexText || intensityBlendText) ? GL_LUMINANCE : GL_ALPHA, 
      GL_UNSIGNED_BYTE, intData);
    if (!(multiTexText || intensityBlendText))
    {
      int n = texRect.Width () * texRect.Height ();
      uint8* p = intData;
      while (n-- > 0)
      {
        *(p++) ^= 0xff;
      }
      statecache->SetTexture (GL_TEXTURE_2D, textures[tex].mirrorHandle);
      glTexSubImage2D (GL_TEXTURE_2D, 0, texRect.xmin, texRect.ymin, 
        texRect.Width (), texRect.Height (), 
        GL_ALPHA, GL_UNSIGNED_BYTE, intData);
    }
    delete[] intData;
  }
}

void csGLFontCache::FlushArrays ()
{
  if (jobCount == 0) return;

  bool texEnabled = statecache->enabled_GL_TEXTURE_2D;
  statecache->Enable_GL_TEXTURE_2D ();

  if (needStates)
  {
    if (multiTexText)
    {
      glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PRIMARY_COLOR);
      glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
      glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_CONSTANT_ARB);
      glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
      glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_TEXTURE);
      glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_ALPHA);
      glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
      glTexEnvi (GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

      glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PRIMARY_COLOR);
      glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
      glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_CONSTANT_ARB);
      glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);
      glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, GL_TEXTURE);
      glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, GL_SRC_ALPHA);
      glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_INTERPOLATE_ARB);
      glTexEnvi (GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
    }
    else if (intensityBlendText)
    {
      glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    }
    else 
    {
      glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    statecache->SetBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    statecache->Enable_GL_BLEND ();

    static float envTransparent[4] = {1.0f, 1.0f, 1.0f, 0.0f};
    glTexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envTransparent);
    envColor = -1;
    needStates = false;
  }
  statecache->SetTexCoordPointer (2, GL_FLOAT, sizeof (float) * 2, texcoords.GetArray ());
  statecache->SetVertexPointer (2, GL_FLOAT, sizeof (float) * 2, verts2d.GetArray ());
  for (size_t j = 0; j < jobCount; j++)
  {
    const TextJob& job = jobs[j];
    const bool doFG = (job.vertCount != 0);
    const bool doBG = (job.bgVertCount != 0);
    if (doFG || doBG)
    {
      if (multiTexText || intensityBlendText)
      {
        statecache->SetTexture (GL_TEXTURE_2D, job.texture);
        if (envColor != job.bg)
        {
          float bgRGB[4];
          G2D->DecomposeColor (job.bg, bgRGB[0], bgRGB[1], bgRGB[2], bgRGB[3]);
          glTexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, bgRGB);
          envColor = job.bg;
        }
        if (doBG)
        {
          glDrawArrays(GL_QUADS, (GLsizei)job.bgVertOffset, 
            (GLsizei)job.bgVertCount);
        }
  
        if (doFG)
        {
          G2D->setGLColorfromint (job.fg);
          glDrawArrays (GL_QUADS, (GLsizei)job.vertOffset, (GLsizei)job.vertCount);
        }
      }
      else
      {
        GLubyte fR, fG, fB, fA, bR, bG, bB, bA;
        G2D->DecomposeColor (job.bg, bR, bG, bB, bA);
        if ((doBG || doFG) && (bA != 0))
        {
          statecache->SetTexture (GL_TEXTURE_2D, job.texture);
          G2D->setGLColorfromint (job.bg);
          glDrawArrays(GL_QUADS, (GLsizei)job.bgVertOffset, 
            (GLsizei)job.bgVertCount);
          glDrawArrays (GL_QUADS, (GLsizei)job.vertOffset, (GLsizei)job.vertCount);
        }
        G2D->DecomposeColor (job.fg, fR, fG, fB, fA);
        if (doFG && (fA != 0))
        {
          statecache->SetTexture (GL_TEXTURE_2D, job.mirrorTexture);
          G2D->setGLColorfromint (job.fg);
          glDrawArrays (GL_QUADS, (GLsizei)job.vertOffset, (GLsizei)job.vertCount);
        }
      }
    }
  }
  jobCount = 0;
  numFloats = 0;

  if (!texEnabled)
    statecache->Disable_GL_TEXTURE_2D ();
}

csGLFontCache::TextJob& csGLFontCache::GetJob (int fg, int bg, 
					       GLuint texture, 
                                               GLuint mirrorTexture,
					       size_t bgOffset)
{
  TextJob& newJob = jobs.GetExtend (jobCount);
  jobCount++;
  newJob.ClearRanges ();
  newJob.vertOffset = numFloats / 2;
  newJob.bgVertOffset = (numFloats + bgOffset) / 2;
  newJob.fg = fg; newJob.bg = bg;
  newJob.texture = texture;
  newJob.mirrorTexture = mirrorTexture;
  return newJob;
}

void csGLFontCache::WriteString (iFont *font, int pen_x, int pen_y, 
				 int fg, int bg, const utf8_char* text,
				 uint flags)
{
  if (!text || !*text) return;

  int bgTrans;
  {
    GLubyte oR, oG, oB, oA;
    G2D->DecomposeColor (fg, oR, oG, oB, oA);
    bgTrans = G2D->FindRGB (oR, oG, oB, 0);
    G2D->DecomposeColor (bg, oR, oG, oB, oA);
    if (oA == 0)
      bg = bgTrans;
  }
  BeginText();

  if (!(flags & CS_WRITE_BASELINE)) pen_y += font->GetAscent ();

  int maxwidth, maxheight;
  font->GetMaxSize (maxwidth, maxheight);

  KnownFont* knownFont = GetCachedFont (font);
  if (knownFont == 0) knownFont = CacheFont (font);

  if (pen_y <= ClipY1) return;
  pen_y = G2D->Height - pen_y/* - maxheight*/;

  size_t textLen = strlen ((char*)text);

  if (bg != -1)
  {
    texcoords.GetExtend (numFloats + textLen * 16);
    verts2d.GetExtend (numFloats + textLen * 16);
  }
  else
  {
    texcoords.GetExtend (numFloats + textLen * 8);
    verts2d.GetExtend (numFloats + textLen * 8);
  }
  size_t bgVertOffset = (textLen + 1) * 8;
  float* tcPtr = 0;
  float* vertPtr = 0;
  float* bgTcPtr = 0;
  float* bgVertPtr = 0;

  float x1 = pen_x;
  float x2 = x1, y1 = 0, y2 = 0;
  int advance = 0;
  bool firstchar = true;
  float oldH = 0.0f;

  TextJob* job = 0;

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
      cacheData = (GLGlyphCacheData*)CacheGlyphUnsafe (knownFont, glyph, 
	flags);
    }
    if (!cacheData->hasGlyph)
    {
      // fall back to the default glyph (CS_FONT_DEFAULT_GLYPH)
      cacheData = (GLGlyphCacheData*)CacheGlyph (knownFont, 
	CS_FONT_DEFAULT_GLYPH, flags);
      if (!cacheData->hasGlyph) continue;
    }

    const size_t newTexNum = cacheData->texNum;
    const GLuint newHandle = textures[newTexNum].handle;
    if (!job || (job->texture != newHandle) || !(usedTexs & (1 << newTexNum)))
    {
      job = &GetJob (fg, bg, newHandle, textures[newTexNum].mirrorHandle, 
        bgVertOffset);
      // Fetch pointers again, the vertex cache might've been flushed
      tcPtr = texcoords.GetArray() + numFloats;
      vertPtr = verts2d.GetArray() + numFloats;
      bgTcPtr = texcoords.GetArray() + numFloats + bgVertOffset;
      bgVertPtr = verts2d.GetArray() + numFloats + bgVertOffset;
    }
    usedTexs |= (1 << newTexNum);

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

    y1 = cacheData->bmetrics.top + pen_y;
    y2 = y1 - cacheData->bmetrics.height;

    bool needBgJob = false;
    if (bg != -1)
    {
      float bx1 = x2;
      float bx2 = x2; //x2;
      float by1 = y1;
      float by2 = y2;

      if (advance > 0)
      {
	bx2 += (float)advance;
	// The texcoords are irrelevant for the BG
	*bgTcPtr++ = 0.0f;
	*bgTcPtr++ = 0.0f;
	*bgVertPtr++ = bx1;
	*bgVertPtr++ = by1;
	*bgTcPtr++ = 0.0f;
	*bgTcPtr++ = 0.0f;
	*bgVertPtr++ = bx2;
	*bgVertPtr++ = by1;
	*bgTcPtr++ = 0.0f;
	*bgTcPtr++ = 0.0f;
	*bgVertPtr++ = bx2;
	*bgVertPtr++ = by2;
	*bgTcPtr++ = 0.0f;
	*bgTcPtr++ = 0.0f;
	*bgVertPtr++ = bx1;
	*bgVertPtr++ = by2;
	job->bgVertCount += 4;
	bgVertOffset += 8;
      }
      else if (advance < 0)
      {
	/*
	  Negative advance slightly complicates things. This character 
	  overlaps with the last one. So we add the overlapping BG to the
	  current job, but create a new job for the next char with a 
	  transparent bg.
	 */
	bx1 = x1;
	bx2 = bx1 + (float)(cacheData->bmetrics.left + 
	  cacheData->bmetrics.width);
	*bgTcPtr++ = 0.0f;
	*bgTcPtr++ = 0.0f;
	*bgVertPtr++ = bx1;
	*bgVertPtr++ = by1;
	*bgTcPtr++ = 0.0f;
	*bgTcPtr++ = 0.0f;
	*bgVertPtr++ = bx2;
	*bgVertPtr++ = by1;
	*bgTcPtr++ = 0.0f;
	*bgTcPtr++ = 0.0f;
	*bgVertPtr++ = bx2;
	*bgVertPtr++ = by2;
	*bgTcPtr++ = 0.0f;
	*bgTcPtr++ = 0.0f;
	*bgVertPtr++ = bx1;
	*bgVertPtr++ = by2;
	job->bgVertCount += 4;
	bgVertOffset += 8;

	// The glyph needs to be drawn transparently, so fetch another job
	job = &GetJob (fg, bgTrans, job->texture, job->mirrorTexture, bgVertOffset);
	// Later, fetch a BG job again
	needBgJob = true;
      }
      advance = 0;

    }

    float x_left = x1;
    x1 = x1 + cacheData->bmetrics.left;
    x2 = x1 + cacheData->bmetrics.width;
    float tx1, tx2, ty1, ty2;

    tx1 = cacheData->tx1;
    tx2 = cacheData->tx2;
    ty1 = cacheData->ty1;
    ty2 = cacheData->ty2;

    *tcPtr++ = tx1;
    *tcPtr++ = ty1;
    *vertPtr++ = x1;
    *vertPtr++ = y1;
    *tcPtr++ = tx2;
    *tcPtr++ = ty1;
    *vertPtr++ = x2;
    *vertPtr++ = y1;
    *tcPtr++ = tx2;
    *tcPtr++ = ty2;
    *vertPtr++ = x2;
    *vertPtr++ = y2;
    *tcPtr++ = tx1;
    *tcPtr++ = ty2;
    *vertPtr++ = x1;
    *vertPtr++ = y2;
    numFloats += 8;
    bgVertOffset -= 8;
    job->vertCount += 4;

    advance += cacheData->glyphMetrics.advance - 
      (cacheData->bmetrics.width + cacheData->bmetrics.left);

    if (needBgJob)
    {
      // Just in case fetched a transparent job for negative advance
      job = &GetJob (fg, bg, job->texture, job->mirrorTexture, bgVertOffset);
    }

    x1 = x_left + cacheData->glyphMetrics.advance;
    oldH = y2 - y1;
  }

  // "Trailing" background
  if ((bg != -1) & (advance > 0))
  {
    float bx1 = x2;
    float bx2 = bx1 + (float)advance;
    float by1 = y1;
    float by2 = y2;

    *bgTcPtr++ = 0.0f;
    *bgTcPtr++ = 0.0f;
    *bgVertPtr++ = bx1;
    *bgVertPtr++ = by1;
    *bgTcPtr++ = 0.0f;
    *bgTcPtr++ = 0.0f;
    *bgVertPtr++ = bx2;
    *bgVertPtr++ = by1;
    *bgTcPtr++ = 0.0f;
    *bgTcPtr++ = 0.0f;
    *bgVertPtr++ = bx2;
    *bgVertPtr++ = by2;
    *bgTcPtr++ = 0.0f;
    *bgTcPtr++ = 0.0f;
    *bgVertPtr++ = bx1;
    *bgVertPtr++ = by2;
    job->bgVertCount += 4;
    bgVertOffset += 8;
  }

  if (bg != -1)
  {
    // Make sure the next data added to the cached comes in after the
    // BG stuff
    numFloats = (job->bgVertOffset + job->bgVertCount) * 2;
  }
  if (numFloats > maxFloats) FlushArrays();
}

void csGLFontCache::BeginText ()
{
  if (textWriting) return;

  vaEnabled = statecache->enabled_GL_VERTEX_ARRAY;
  tcaEnabled = statecache->enabled_GL_TEXTURE_COORD_ARRAY;
  caEnabled = statecache->enabled_GL_COLOR_ARRAY;

  statecache->SetActiveTU (0);
  statecache->Enable_GL_VERTEX_ARRAY();
  statecache->Enable_GL_TEXTURE_COORD_ARRAY();
  statecache->Disable_GL_COLOR_ARRAY();

  textWriting = true;
  needStates = true;
}

void csGLFontCache::FlushText ()
{
  if (!textWriting) return;

  FlushArrays ();

  if (!vaEnabled) statecache->Disable_GL_VERTEX_ARRAY();
  if (!tcaEnabled) statecache->Disable_GL_TEXTURE_COORD_ARRAY();
  if (caEnabled) statecache->Enable_GL_COLOR_ARRAY();

  if (G2D->useCombineTE)
  {
    if (!multiTexText)
      glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
    glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
    glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR);
    glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
    glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
    glTexEnvi (GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

    glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
    glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
    glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PRIMARY_COLOR);
    glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);
    glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
    glTexEnvi (GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
  }
  else
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  statecache->SetBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  PurgeEmptyPlanes ();

  textWriting = false;
}

void csGLFontCache::DumpFontCache (csRefArray<iImage>& pages)
{
  for (size_t t = 0; t < textures.Length(); t++)
  {
    csRef<csImageMemory> page;
    page.AttachNew (new csImageMemory (texSize, texSize, 
      CS_IMGFMT_PALETTED8));
    csRGBpixel* pal = page->GetPalettePtr ();
    for (int i = 0; i < 256; i++)
    {
      pal[i].Set (i, i, i);
    }

    statecache->SetTexture (GL_TEXTURE_2D, textures[t].handle);
    glGetTexImage (GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
      page->GetImagePtr ());
    pages.Push (page);
  }
}
