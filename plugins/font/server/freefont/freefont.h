/*
    Copyright (C) 2000 by Norman Kramer

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

#ifndef __CS_FREEFONT_H__
#define __CS_FREEFONT_H__

#include "ivideo/fontserv.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/csvector.h"
#include "csutil/util.h"
#include "csutil/cfgacc.h"

class csFreeTypeServer;

/**
 * A FreeType font object
 */
class csFreeTypeFont : public iFont
{
  // A single glyph bitmap
  struct GlyphBitmap
  {
    // The width and height of the glyph bitmap (in pixels)
    int w, h;
    // The bitmap itself (or NULL if the glyph has not been rendered yet)
    uint8 *bitmap;

    // Destructor
    ~GlyphBitmap ()
    { delete [] bitmap; }
  };

  // A set of glyphs with same point size
  struct GlyphSet
  {
    int size;
    int maxW, maxH;
    GlyphBitmap glyphs [256];
  };

  // An array with (numerous) sets of glyphs of different sizes
  class csFontDefVector : public csVector
  {
  public:
    virtual ~csFontDefVector ()
    { DeleteAll (); }
    virtual bool FreeItem (csSome Item)
    { delete (GlyphSet *)Item; return true; }
    GlyphSet *Get (int n)
    { return (GlyphSet *)csVector::Get (n); }
    virtual int Compare (csSome Item1, csSome Item2, int /*Mode*/) const
    {
      int id1 = ((GlyphSet*)Item1)->size, id2 = ((GlyphSet*)Item2)->size;
      return id1 - id2;
    }
    virtual int CompareKey (csSome Item1, csConstSome Key, int /*Mode*/) const
    { int id1 = ((GlyphSet*)Item1)->size; return id1 - (int)Key; }
  } cache;

  GlyphSet *FindGlyphSet (int size)
  {
    int idx = cache.FindKey ((csConstSome)size);
    return (idx == -1 ? NULL : cache.Get (idx));
  }

  bool CreateGlyphBitmaps (int size);

public:
  // font filename (for identification)
  char *name;
  // current glyph set
  GlyphSet *current;
  // The list of delete callbacks
  csVector DeleteCallbacks;
  //
  TT_Face face;
  TT_Instance instance;
  TT_Face_Properties prop;
  TT_UShort pID, eID;
  TT_CharMap charMap;

  SCF_DECLARE_IBASE;

  /// Constructor
  csFreeTypeFont (const char *filename);

  /// Destructor
  virtual ~csFreeTypeFont ();

  /// Load the font
  bool Load (csFreeTypeServer *server);

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
 * FreeType font server.
 */
class csFreeTypeServer : public iFontServer
{
  class csFontVector : public csVector
  {
  public:
    void Put (csFreeTypeFont *Font)
    { Font->IncRef (); csVector::Push (Font); }
    virtual bool FreeItem (csSome Item)
    { ((csFreeTypeFont *)Item)->DecRef (); return true; }
    csFreeTypeFont *Get (int n)
    { return (csFreeTypeFont *)csVector::Get (n); }
    virtual int Compare (csSome Item1, csSome Item2, int /*Mode*/) const
    { return strcmp (((csFreeTypeFont *)Item1)->name,
                     ((csFreeTypeFont *)Item2)->name); }
    virtual int CompareKey (csSome Item1, csConstSome Key, int /*Mode*/) const
    {
      // compare the font names
      const char *id1 = ((csFreeTypeFont *)Item1)->name;
      const char *id2 = (const char *)Key;
      return strcmp (id1, id2);
    }
  } fonts;

public:
  TT_Engine engine;
  TT_UShort platformID, encodingID;
  int defaultSize;
  iObjectRegistry *object_reg;
  csConfigAccess ftconfig;
  iVFS *VFS;
  const char *fontset;

  SCF_DECLARE_IBASE;

  csFreeTypeServer (iBase *iParent);
  virtual ~csFreeTypeServer ();

  virtual bool Initialize (iObjectRegistry *Sys);
  void Report (int severity, const char* msg, ...);

  /**
   * Load a font by name.
   * Returns a new iFont object or NULL on failure.
   */
  virtual iFont *LoadFont (const char *filename);

  /**
   * Get number of loaded fonts.
   */
  virtual int GetFontCount ()
  { return fonts.Length(); }

  /**
   * Get Nth loaded font or NULL.
   * You can query all loaded fonts with this method, by looping
   * through all indices starting from 0 until you get NULL.
   */
  virtual iFont *GetFont (int iIndex);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFreeTypeServer);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __CS_FREEFONT_H__
