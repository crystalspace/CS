/*
    Copyright (C) 2000 by Norman Kramer
    Copyright (C) 2000 by W.C.A. Wijngaards
    original unplugged code and fonts by Andrew Zabolotny

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

#ifndef __CSFONT_H__
#define __CSFONT_H__

#include "ivideo/fontserv.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/typedvec.h"
#include "isys/plugin.h"

struct iObjectRegistry;
struct iPluginManager;
class csDefaultFontServer;

/**
 * Bitmap font
 */
class csDefaultFont : public iFont
{
public:
  char *Name;
  int First;
  int Glyphs;
  int Width;
  int MaxWidth;
  int Height;
  uint8 *FontBitmap;
  uint8 *IndividualWidth;
  uint8 **GlyphBitmap;
  csDefaultFontServer *Parent;
  csVector DeleteCallbacks;

  SCF_DECLARE_IBASE;

  /// Create the font object
  csDefaultFont (csDefaultFontServer *parent, const char *name, int first,
    int glyphs, int width, int height, uint8 *bitmap, uint8 *individualwidth);

  /// Destroy the font object
  virtual ~csDefaultFont ();

  /**
   * Set the size for this font.
   * All other methods will change their behaviour as soon as you call
   * this method; but not all font managers supports rescalable fonts
   * in which case this method will be unimplemented.
   */
  virtual void SetSize (int iSize);

  /**
   * Query current font size. If server does not support rescalable
   * fonts, this method returns 0.
   */
  virtual int GetSize ();

  /**
   * Return the maximum width and height of a single glyph.
   * Return -1 if it could not be determined.
   */
  virtual void GetMaxSize (int &oW, int &oH);

  /**
   * Return character size in pixels.
   * Returns false if values could not be determined.
   */
  virtual bool GetGlyphSize (uint8 c, int &oW, int &oH);

  /**
   * Return a pointer to a bitmap containing a rendered character.
   * Returns NULL if error occured. The oW and oH parameters are
   * filled with bitmap width and height.
   */
  virtual uint8 *GetGlyphBitmap (uint8 c, int &oW, int &oH);

  /**
   * Return the width and height of text written with this font.
   */
  virtual void GetDimensions (const char *text, int &oW, int &oH);

  /**
   * Determine how much characters from this string can be written
   * without exceeding given width (in pixels)
   */
  virtual int GetLength (const char *text, int maxwidth);

  /**
   * Add a call-on-font-delete callback routine.
   */
  virtual void AddDeleteCallback (iFontDeleteNotify* func);

  /**
   * Remove a font delete notification callback.
   */
  virtual bool RemoveDeleteCallback (iFontDeleteNotify* func);
};

/**
 * Default font server.
 */
class csDefaultFontServer : public iFontServer
{
private:
  iObjectRegistry* object_reg;
  iPluginManager* plugin_mgr;

  // A list of csDefaultFont pointers.
  CS_DECLARE_TYPED_VECTOR (csFontList, csDefaultFont) fonts;

  /// read a font file from vfs
  csDefaultFont *ReadFontFile(const char *file);

public:
  SCF_DECLARE_IBASE;

  /// Create the plugin object
  csDefaultFontServer (iBase *pParent);
  /// destroy it
  virtual ~csDefaultFontServer ();

  /**
   * Load a font by name.
   * Returns a new iFont object or NULL on failure.
   */
  virtual iFont *LoadFont (const char *filename);

  /**
   * Get number of loaded fonts.
   */
  virtual int GetFontCount ()
  { return fonts.Length (); }

  /**
   * Get Nth loaded font or NULL.
   * You can query all loaded fonts with this method, by looping
   * through all indices starting from 0 until you get NULL.
   */
  virtual iFont *GetFont (int iIndex);

  /// Called by child fonts to be added to font registry
  void NotifyCreate (csDefaultFont *font);

  /// This function is called by iFont objects when they are destroyed
  void NotifyDelete (csDefaultFont *font);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csDefaultFontServer);
    virtual bool Initialize(iObjectRegistry* p);
  } scfiComponent;
  friend struct eiComponent;
};

#endif // __CSFONT_H__
