/*
    Copyright (c) 2000 Andrew Zabolotny

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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "cssysdef.h"
#include "cssys/sysdriv.h"
#include "video/canvas/common/graph2d.h"
#include "csutil/util.h"

#include "gl2d_font.h"

#if defined (OPENGL_BITMAP_FONT)

struct GLGlyphSet : public GLProtoGlyphSet
{
  // All the glyphs
  uint8 **glyph;
  // Max character height
  int maxheight;
  // Per-glyph info
  struct
  {
    // Glyph width and height
    unsigned char w, h, bw;
  } glyphsize [256];

  // Create the glyph set
  GLGlyphSet (iFont *Font);
  // Destroy the glyph set
  ~GLGlyphSet ();

private:
  // Fetch all the characters of the font
  void Load ();
};
  
GLGlyphSet::GLGlyphSet (iFont *Font)
{
  font = Font;
  Load ();
}

GLGlyphSet::~GLGlyphSet ()
{
  for (int c = 0; c < 256; c++)
    delete [] glyph [c];
  delete [] glyph;
}

void GLGlyphSet::Load ()
{
  glyph = new uint8 * [256];
  maxheight = 0;
  for (int c = 0; c < 256; c++)
  {
    int width, height;
    // points to location of the font source data
    uint8 *bmp = font->GetGlyphBitmap (c, width, height);
    int bytes = (width + 7) / 8;

    if (height > maxheight)
      maxheight = height;

    glyphsize [c].w = width;
    glyphsize [c].h = height;
    glyphsize [c].bw = bytes;

    // Now we'll have to reverse character lines from bottom up
    uint8 *data = new unsigned char [bytes * height];
    for (int line = 0; line < height; line++)
      memcpy (data + bytes * (height - 1 - line), bmp + bytes * line, bytes);
    glyph [c] = data;
  }
}

//-----------------------------------------------------// GLGlyphVector //----//

GLFontCache::GLGlyphVector::~GLGlyphVector ()
{
  DeleteAll ();
}

bool GLFontCache::GLGlyphVector::FreeItem (csSome Item)
{
  delete (GLGlyphSet *)Item;
  return true;
}

//-------------------------------------------------------// GLFontCache //----//

struct FontDeleteNotify : public iFontDeleteNotify
{
  void* glyphset;
  SCF_DECLARE_IBASE;
  FontDeleteNotify () { SCF_CONSTRUCT_IBASE (NULL); }
  virtual ~FontDeleteNotify () { }
  virtual void BeforeDelete (iFont* font);
};

SCF_IMPLEMENT_IBASE (FontDeleteNotify)
  SCF_IMPLEMENTS_INTERFACE (iFontDeleteNotify)
SCF_IMPLEMENT_IBASE_END

void FontDeleteNotify::BeforeDelete (iFont *font)
{
  GLFontCache *This = (GLFontCache *)glyphset;
  This->CacheFree (font);
}

GLFontCache::GLFontCache (iFontServer *fs) : FontCache (8, 8)
{
  int i = 0;
  delete_callback = new FontDeleteNotify ();
  ((FontDeleteNotify*)delete_callback)->glyphset = this;
  iFont *font;
  while ((font = fs->GetFont (i++)))
    CacheFont (font);
}

GLFontCache::~GLFontCache ()
{
  // Remove deletion callbacks to avoid being deleted later -
  // when the font cache object will be already deleted
  for (int i = 0; i < FontCache.Length (); i++)
    FontCache.Get (i)->font->RemoveDeleteCallback (delete_callback);
  delete_callback->DecRef ();
}

GLGlyphSet *GLFontCache::CacheFont (iFont *font)
{
  // See if we have any instances of this font in the cache
  int i;
  for (i = FontCache.Length () - 1; i >= 0; i--)
    if (FontCache.Get (i)->font == font)
      break;
  if (i < 0)
    // Tell the font to notify us when it is freed
    font->AddDeleteCallback (delete_callback);

  GLGlyphSet *gs = new GLGlyphSet (font);
  FontCache.Push (gs);
  return gs;
}

void GLFontCache::CacheFree (iFont *font)
{
  for (int i = FontCache.Length () - 1; i >= 0; i--)
  {
    GLGlyphSet *gs = FontCache.Get (i);
    if (gs->font == font)
      FontCache.Delete (i);
  }
  font->RemoveDeleteCallback (delete_callback);
}

void GLFontCache::Write (iFont *font, int x, int y, const char *text)
{
  if (!text || !*text) return;

  int idx = FontCache.FindKey (font);
  GLGlyphSet *gs = idx >= 0 ? FontCache.Get (idx) : CacheFont (font);
  if (!gs) return;

  y = y - gs->maxheight;
  if (y >= ClipY2) return;

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glRasterPos2i (x < ClipX1 ? ClipX1 : x, y < ClipY1 ? ClipY1 : y);

  int dy = (y < ClipY1) ? ClipY1 - y : 0;
  if (dy >= gs->maxheight)
    return;

  while (*text)
  {
    unsigned char c = *text++;
    int w = gs->glyphsize [c].w;
    int h = gs->glyphsize [c].h;

    // Perform clipping
    if (x >= ClipX2)
      return;

    int dx = 0;
    if (x < ClipX1)
    {
      dx = ClipX1 - x;
      if (w <= dx) { x += w; continue; }
      x = ClipX1; w -= dx;
    }
    if ((h -= dy) <= 0)
      continue;

    if (x + w >= ClipX2)
      w = ClipX2 - x;
    if (y + h >= ClipY2)
      h = ClipY2 - y;

    glBitmap (w, h, dx, 0, w, 0, gs->glyph [c] + gs->glyphsize [c].bw * dy);
    x += w;
  }
}

#endif
