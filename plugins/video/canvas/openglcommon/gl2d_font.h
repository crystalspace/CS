/*
    Copyright (C) 1999 by Gary Haussmann
    Accelerated by Samuel Humphreys

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

#ifndef __CS_GL2D_FONT_H__
#define __CS_GL2D_FONT_H__

#include <GL/gl.h>
#include "csutil/csvector.h"
#include "ivideo/fontserv.h"

// Define the following macro to use glBitmap() text drawing approach
// rather than default "font-on-a-texture" approach.
// Currently this is defined in csosdefs.h for platforms with
// OpenGL's that prefer glBitmap() method.
//#define OPENGL_BITMAP_FONT

struct GLGlyphSet;

/**
 * The parent class for csGlyphSet
 * (common for both implementations of csGlyphSet).
 */
struct GLProtoGlyphSet
{
  // The font
  iFont *font;
  // Font size
  int size;
};

/**
 * This class will "cache" the bitmap font so that the font could be
 * painted somehow accelerated. There are two different implementation
 * of the font cache class, one "mainstream" suitable for most OpenGL
 * implementations using a texture (the font is cached on a texture),
 * and the second version which uses glBitmap() for text drawing, which
 * is meant for OpenGL's with either buggy or slow alpha-textures support,
 * such as OS/2 software OpenGL or MacOS OpenGL. To use the second version
 * of font cache, you should #define OPENGL_BITMAP_FONT in the cssysdef.h
 * header file for the respective platform.
 */
class GLFontCache
{
protected:
  // The font cache array, sorted by iFont + size
  class GLGlyphVector : public csVector
  {
  public:
    GLGlyphVector (int limit, int threshold) : csVector (limit, threshold) {}
    virtual ~GLGlyphVector ();
    virtual bool FreeItem (csSome Item);
    virtual int Compare (csSome Item1, csSome Item2, int Mode) const
    {
      (void)Mode;
      GLProtoGlyphSet *gs1 = (GLProtoGlyphSet *)Item1;
      GLProtoGlyphSet *gs2 = (GLProtoGlyphSet *)Item2;
      if (gs1->font < gs2->font)
        return -1;
      else if (gs1->font > gs2->font)
        return +1;
      else if (gs1->size < gs2->size)
        return -1;
      else if (gs1->size > gs2->size)
        return +1;
      else
        return 0;
    }
    virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const
    {
      (void)Mode;
      GLProtoGlyphSet *gs = (GLProtoGlyphSet *)Item;
      iFont *font = (iFont *)Key;
      if (gs->font < font)
        return -1;
      else if (gs->font > font)
        return +1;
      int size = font->GetSize ();
      if (gs->size < size)
        return -1;
      else if (gs->size > size)
        return +1;
      else
        return 0;
    }
    GLGlyphSet *Get (int iIndex)
    { return (GLGlyphSet *)csVector::Get (iIndex); }
  } FontCache;
  
  /// the current clipping rect
  int ClipX1, ClipY1, ClipX2, ClipY2;

#ifndef OPENGL_BITMAP_FONT
  // Auxiliary routine for "font-on-a-texture" approach
  bool ClipRect (float x, float y,
    float &x1, float &y1, float &x2, float &y2, 
    float &tx1, float &ty1, float &tx2, float &ty2);
#endif
public:
  /// Initialize the font cache and load all the static fonts
  GLFontCache (iFontServer *fs);

  /// Clean up all remaining font cache items
  ~GLFontCache ();

  /**
   * Draw a string using OpenGL at x,y. It is assumed you have
   * set up the render state using glColor.
   */
  void Write (iFont *font, int x, int y, const char *text);

  /**
   * This is called from iGraphics2D::SetClipRect() to cache
   * clipping rectangle inside the font cache object in OpenGL
   * format, e.g. switched bottom-up.
   */
  void SetClipRect (int x1, int y1, int x2, int y2)
  { ClipX1 = x1; ClipY1 = y1; ClipX2 = x2; ClipY2 = y2; }   

  /**
   * Cache a font. If the font is already cached, just returns the
   * corresponding GLGlyphSet object.
   */
  GLGlyphSet *CacheFont (iFont *font);

  /**
   * Free all the font cache items associated with this font.
   */
  void CacheFree (iFont *font);
};

#endif // __CS_GL2D_FONT_H__
