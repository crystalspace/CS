/*
    Copyright (C) 2000 by Norman Krämer

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

#ifndef __IVIDEO_FONTSERV_H__
#define __IVIDEO_FONTSERV_H__

#include "csutil/scf.h"

/*
 * Some alias names for basic fonts.
 * Any font server should provide these fonts, since most
 * programs expect they to be available. Other fonts may or
 * may be not available but these should be always available.
 * Default font names always start with "*" to avoid confusion
 * with real file names.
 */

/// Thick and relatively large font
#define CSFONT_LARGE		"*large"
/// Thick italic relatively large font
#define CSFONT_ITALIC		"*italic"
/// Thin courier-like relatively large font
#define CSFONT_COURIER		"*courier"
/// Very small font (smallest font that is still readable)
#define CSFONT_SMALL		"*small"

struct iFont;

SCF_VERSION (iFontDeleteNotify, 0, 0, 1);

/**
 * Called before a font is deleted.
 * You can insert any number of callback routines into the font
 * so that when the font will be destroyed all of them will be
 * called in turn. This can be used by canvas driver, for example,
 * if the canvas driver does some kind of caching for fonts,
 * e.g. OpenGL driver pre-caches the font on a texture, it needs
 * some mechanism to be notified when the font is destroyed to free
 * the cache texture associated with the font.
 */
struct iFontDeleteNotify : public iBase
{
  /// Before delete.
  virtual void BeforeDelete (iFont* font) = 0;
};

SCF_VERSION (iFont, 2, 0, 1);

/**
 * A font object.
 * Objects of this class are used by canvas driver to paint glyphs.
 */
struct iFont : public iBase
{
  /**
   * Set the size for this font.
   * All other methods will change their behaviour as soon as you call
   * this method; but not all font managers supports rescalable fonts
   * in which case this method will be unimplemented.
   */
  virtual void SetSize (int iSize) = 0;

  /**
   * Query current font size. If server does not support rescalable
   * fonts, this method returns 0.
   */
  virtual int GetSize () = 0;

  /**
   * Return the maximum width and height of a single glyph.
   * Return -1 if it could not be determined.
   */
  virtual void GetMaxSize (int &oW, int &oH) = 0;

  /**
   * Return character size in pixels.
   * Returns false if values could not be determined.
   */
  virtual bool GetGlyphSize (uint8 c, int &oW, int &oH) = 0;

  /**
   * Return character width, height, advance, x- and y-bearing in pixels.
   * Returns false if values could not be determined.
   */
  virtual bool GetGlyphSize (uint8 c, int &oW, int &oH, int &adv, int &left, int &top) = 0;

  /**
   * Return a pointer to a bitmap containing a rendered character.
   * Returns NULL if error occured. The oW and oH parameters are
   * filled with bitmap width and height.
   */
  virtual uint8 *GetGlyphBitmap (uint8 c, int &oW, int &oH) = 0;

  /**
   * Return a pointer to a bitmap containing a rendered character.
   * Returns NULL if error occured. The oW and oH parameters are
   * filled with bitmap width and height. adv holds the advance in x-direction, left and top hold the
   * x- and y-bearing.
   */
  virtual uint8 *GetGlyphBitmap (uint8 c, int &oW, int &oH, int &adv, int &left, int &top) = 0;

  /**
   * Return the width and height of text written with this font.
   */
  virtual void GetDimensions (const char *text, int &oW, int &oH) = 0;

  /**
   * Return the width and height of text written with this font. desc gives the maximum descender.
   */
  virtual void GetDimensions (const char *text, int &oW, int &oH, int &desc) = 0;

  virtual void GetDimensions (const char *text, int &oW, int &oH, int &, int &, int &) = 0;

  /**
   * Determine how many characters from this string can be written
   * without exceeding given width (in pixels)
   */
  virtual int GetLength (const char *text, int maxwidth) = 0;

  /**
   * Add a font delete notification callback routine.
   * This routine will be called from font destructor,
   * with the font instance being passed as argument.
   * Another parameter is provided to supply additional data.
   */
  virtual void AddDeleteCallback (iFontDeleteNotify* func) = 0;

  /**
   * Remove a font delete notification callback.
   */
  virtual bool RemoveDeleteCallback (iFontDeleteNotify* func) = 0;
};

SCF_VERSION (iFontServer, 2, 0, 0);

/**
 * A font server interface.
 * Font server can load fonts and create iFont objects.
 * In fact user does not care whenever fonts are built-in
 * the font server or are on disk; thus some font servers
 * may contain the fonts hardcoded; in this case the
 * font path is really a identifier.
 */
struct iFontServer : public iBase
{
  /**
   * Load a font by name.
   * Returns a new iFont object or NULL on failure.
   */
  virtual iFont *LoadFont (const char *filename) = 0;

  /**
   * Get number of loaded fonts.
   */
  virtual int GetFontCount () = 0;

  /**
   * Get Nth loaded font or NULL.
   * You can query all loaded fonts with this method, by looping
   * through all indices starting from 0 until you get NULL.
   * Note that the returned font is NOT IncRef'd: do it yourself
   * if you store the pointer for long-term use.
   */
  virtual iFont *GetFont (int iIndex) = 0;
};

#endif // __IVIDEO_FONTSERV_H__
