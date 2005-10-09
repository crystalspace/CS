/*
    Copyright (C) 2000 by Norman Kraemer

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

#ifndef __CS_IVIDEO_FONTSERV_H__
#define __CS_IVIDEO_FONTSERV_H__

#include "csutil/csunicode.h"
#include "csutil/scf_interface.h"
#include "csutil/ref.h"

/**\file
 * Font server interface
 */

/**
 * \addtogroup gfx2d
 * @{ */
 
/**\name Basic fonts alias names
 * Any font server should provide these fonts, since most
 * programs expect they to be available. Other fonts may or
 * may be not available but these should be always available.
 * Default font names always start with "*" to avoid confusion
 * with real file names.
 * @{ */
/// Thick and relatively large font
#define CSFONT_LARGE		"*large"
/// Thick italic relatively large font
#define CSFONT_ITALIC		"*italic"
/// Thin courier-like relatively large font
#define CSFONT_COURIER		"*courier"
/// Very small font (smallest font that is still readable)
#define CSFONT_SMALL		"*small"
/** @} */

/**
 * The default char, drawn in case a glyph wasn't present in the font.
 * The Unicode standard says that this will never be a valid code point -
 * so we just take as the "replacer" char.
 */
#define CS_FONT_DEFAULT_GLYPH	0xffff

struct iFont;
struct iDataBuffer;


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
struct iFontDeleteNotify : public virtual iBase
{
  SCF_INTERFACE(iFontDeleteNotify, 2,0,0);
  /// Before delete.
  virtual void BeforeDelete (iFont* font) = 0;
};

/**
 * Metrics for a glyph that are dependent from whether a simple
 * or antialiased image is used.
 */
struct csBitmapMetrics
{
  /// Width of the glyph image
  int width;
  /// Height of the glyph image
  int height;
  /// X offset of the image to the pen position
  int left;
  /// Y offset of the image to the pen position
  int top;
};

/**
 * Metrics for a glyph that are independent from whether a simple
 * or antialiased image is used.
 */
struct csGlyphMetrics
{
  /// Amount of pixels the pen needs to advance after the glyph
  int advance;
};

SCF_VERSION (iFont, 5, 1, 0);

/**
 * A font object.
 * Objects of this class are used by canvas driver to paint glyphs.
 *
 * Main creators of instances implementing this interface:
 * - iFontServer::LoadFont()
 */
struct iFont : public iBase
{
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
  
  /**
   * Query current font size in Point. If server does not support rescalable
   * fonts, this method returns 0.
   */
  virtual float GetSize () = 0;

  /**
   * Return the maximum width and height of a single glyph, in pixels.
   * Return -1 if it could not be determined.
   */
  virtual void GetMaxSize (int &oW, int &oH) = 0;

  /**
   * Return the metrics of a glyph.
   */
  virtual bool GetGlyphMetrics (utf32_char c, csGlyphMetrics& metrics) = 0;

  /**
   * Return a pointer to a bitmap containing a rendered character.
   * Returns 0 if the glyph can't be retrieved.
   */
  virtual csPtr<iDataBuffer> GetGlyphBitmap (utf32_char c,
    csBitmapMetrics& metrics) = 0;

  /**
   * Return a pointer to a bitmap containing the alpha bitmap for the
   * rendered character. 
   * Returns 0 if the glyph can't be retrieved.
   */
  virtual csPtr<iDataBuffer> GetGlyphAlphaBitmap (utf32_char c,
    csBitmapMetrics& metrics) = 0;

  /**
   * Return the width and height of text written with this font.
   */
  virtual void GetDimensions (const char *text, int &oW, int &oH) = 0;

  /**
   * Return the width and height of text written with this font. desc
   * gives the maximum descender.
   */
  virtual void GetDimensions (const char *text, int &oW, int &oH,
  	int &desc) = 0;

  /**
   * Determine how many characters from this string can be written
   * without exceeding given width (in pixels)
   */
  virtual int GetLength (const char *text, int maxwidth) = 0;
  
  /**
   * Get the font's descent in pixels.
   * Returns a value <0 if an error occured.
   * The sum of descent and ascent must not necessarily equal the 
   * maximum height.
   */
  virtual int GetDescent () = 0; 

  /**
   * Get the font's ascent in pixels.
   * Returns a value <0 if an error occured.
   * The sum of descent and ascent must not necessarily equal the 
   * maximum height.
   */
  virtual int GetAscent () = 0; 
  
  /**
   * Returns whether a specific glyph is present in this font.
   */
  virtual bool HasGlyph (utf32_char c) = 0; 

  /** 
   * Gets the default baseline to baseline distance between 
   * two lines of text using this font.
   */
  virtual int GetTextHeight () = 0;

  /**
   * When displaying or rendering underlined text, this 
   * value corresponds to the vertical position, relative 
   * to the baseline, of the underline bar. It is positive 
   * if the underline it is below the baseline. The position
   * returned is to the top of the underline bar/rectagle.
   */
  virtual int GetUnderlinePosition () = 0;

  /**
   * When displaying or rendering underlined text, this value 
   * corresponds to the vertical thickness of the underline
   * bar/rectangle.
   */
  virtual int GetUnderlineThickness () = 0;
};

SCF_VERSION (iFontServer, 3, 0, 0);

/**
 * A font server interface.
 * Font server can load fonts and create iFont objects.
 * In fact user does not care whenever fonts are built-in
 * the font server or are on disk; thus some font servers
 * may contain the fonts hardcoded; in this case the
 * font path is really a identifier.
 *
 * Main creators of instances implementing this interface:
 * - Font Multiplexer plugin (crystalspace.font.server.multiplexer)
 * - Standard Font server plugin (crystalspace.font.server.default)
 * - FreeType2 Font server plugin (crystalspace.font.server.freetype2)
 *
 * Main ways to get pointers to this interface:
 * - CS_QUERY_REGISTRY()
 *
 * Main users of this interface:
 * - iGraphics3D implementations (3D renderers).
 */
struct iFontServer : public iBase
{
  /**
   * Load a font by name.
   * Returns a new iFont object or 0 on failure.
   */
  virtual csPtr<iFont> LoadFont (const char* filename, 
    float size = 10.0f) = 0;
};

/** @} */

#endif // __CS_IVIDEO_FONTSERV_H__
