/*
    Copyright (C) 2002 by Norman Kraemer

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

#ifndef __CS_FREEFONT2_H__
#define __CS_FREEFONT2_H__

#include "ivideo/fontserv.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/util.h"
#include "csutil/cfgacc.h"

class csFreeType2Server;

/**
 * A FreeType font object
 */
class csFreeType2Font : public iFont
{
protected:
  FT_Glyph glyph;
  csRef<iDataBuffer> fontdata;

public:
  csFreeType2Server* server;
  // font filename (for identification)
  char *name;
  // Size of this font
  int fontSize;
  // The list of delete callbacks
  csRefArray<iFontDeleteNotify> DeleteCallbacks;
  //
  FT_Face face;
  FT_Library instance;
  FT_UShort pID, eID;
  FT_CharMap charMap;
  
  SCF_DECLARE_IBASE;

  /// Constructor
  csFreeType2Font (const char *filename, csFreeType2Server* server);

  /// Destructor
  virtual ~csFreeType2Font ();

  /// Load the font
  bool Load (iVFS *pVFS);

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
  virtual bool GetGlyphMetrics (utf32_char c, csGlyphMetrics& metrics);

  /**
   * Return a pointer to a bitmap containing a rendered character.
   * Returns 0 if error occured. The oW and oH parameters are
   * filled with bitmap width and height.
   */
  virtual csPtr<iDataBuffer> GetGlyphBitmap (utf32_char c, 
    csBitmapMetrics& metrics);

  virtual csPtr<iDataBuffer> GetGlyphAlphaBitmap (utf32_char c,
    csBitmapMetrics& metrics);

  /**
   * Return the width and height of text written with this font.
   */
  virtual void GetDimensions (const char *text, int &oW, int &oH, int &desc);
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

  /**
   * Get the font's descent in pixels.
   */
  virtual int GetDescent (); 

  /**
   * Get the font's ascent in pixels.
   */
  virtual int GetAscent (); 

  /**
   * Returns whether a specific glyph is present in this font.
   */
  virtual bool HasGlyph (utf32_char c); 
};

/**
 * FreeType font server.
 */
class csFreeType2Server : public iFontServer
{
  class csFontVector : public csRefArray<csFreeType2Font>
  {
  public:
    static int CompareKey (csFreeType2Font* const& Item1, void* Key)
    {
      // compare the font names
      const char *id1 = Item1->name;
      const char *id2 = (const char *)Key;
      return strcmp (id1, id2);
    }
  } fonts;

public:
  FT_Library library;
  int defaultSize;
  iObjectRegistry *object_reg;
  csConfigAccess ftconfig;
  csRef<iVFS> VFS;
  const char *fontset;
  bool freetype_inited;

  SCF_DECLARE_IBASE;

  csFreeType2Server (iBase *iParent);
  virtual ~csFreeType2Server ();

  virtual bool Initialize (iObjectRegistry *Sys);
  void Report (int severity, const char* msg, ...);
  void ReportV (int severity, const char* msg, va_list arg);

  const char* GetErrorDescription(int code);

  bool FreetypeError (int errorCode, int reportSeverity,
    const char* message, ...);
  bool FreetypeError (int errorCode, const char* message,
    ...);

  /**
   * Load a font by name.
   * Returns a new iFont object or 0 on failure.
   */
  virtual csPtr<iFont> LoadFont (const char *filename);

  /**
   * Get number of loaded fonts.
   */
  virtual int GetFontCount ()
  { return fonts.Length(); }

  /**
   * Get Nth loaded font or 0.
   * You can query all loaded fonts with this method, by looping
   * through all indices starting from 0 until you get 0.
   */
  virtual iFont *GetFont (int iIndex);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFreeType2Server);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __CS_FREEFONT2_H__
