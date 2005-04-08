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
#include "csutil/hash.h"
#include "csutil/hashhandlers.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/refcount.h"
#include "csutil/util.h"
#include "csutil/cfgacc.h"
#include "csutil/weakref.h"

class csFreeType2Server;

/**
 * Wrapper so a freetype face can be shared between fonts
 */
struct csFt2FaceWrapper : public csRefCount
{
  FT_Face face;
  char* faceName;
  csFreeType2Server* owner;
  csRef<iDataBuffer> data;

  csFt2FaceWrapper (csFreeType2Server* owner, iDataBuffer* data,
    char* faceName);
  virtual ~csFt2FaceWrapper ();
};

/**
 * A FreeType font object
 */
class csFreeType2Font : public iFont
{
protected:
  FT_Glyph glyph;

public:
  csRef<csFreeType2Server> server;
  // font filename (for identification)
  char *name;
  // The font id, used to identify a face/size pair
  char* fontid;
  // Size of this font
  float fontSize;
  // The list of delete callbacks
  csRefArray<iFontDeleteNotify> DeleteCallbacks;
  /*
    @@@ Somewhat ugly. The glyph metrics will be stored twice, in the font and
    the font cache itself.
   */
  csHash<csGlyphMetrics, utf32_char> glyphMetrics;
  //
  csRef<csFt2FaceWrapper> face;
  FT_Size size;

  SCF_DECLARE_IBASE;

  /// Constructor
  csFreeType2Font (csFreeType2Server* server, char* fontid, 
    csFt2FaceWrapper* face, float iSize);

  /// Destructor
  virtual ~csFreeType2Font ();

  /**
   * Query current font size. If server does not support rescalable
   * fonts, this method returns 0.
   */
  virtual float GetSize ();

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

  /** 
   * Gets the default baseline to baseline distance between 
   * two lines of text using this font.
   */
  virtual int GetTextHeight ();

  /**
   * When displaying or rendering underlined text, this 
   * value corresponds to the vertical position, relative 
   * to the baseline, of the underline bar. It is positive 
   * if the underline it is below the baseline. The position
   * returned is to the top of the underline bar/rectagle.
   */
  virtual int GetUnderlinePosition ();

  /**
   * When displaying or rendering underlined text, this value 
   * corresponds to the vertical thickness of the underline
   * bar/rectangle.
   */
  virtual int GetUnderlineThickness ();

};

/**
 * FreeType font server.
 */
class csFreeType2Server : public iFontServer
{
public:
  FT_Library library;
  iObjectRegistry *object_reg;
  csConfigAccess ftconfig;
  csRef<iVFS> VFS;
  const char *fontset;
  bool freetype_inited;
  csHash<csFt2FaceWrapper*, const char*, csConstCharHashKeyHandler> ftfaces;
  csHash<iFont*, const char*, csConstCharHashKeyHandler> fonts;

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

  /// Unregister a FT face. Called by face's dtor
  void RemoveFT2Face (csFt2FaceWrapper* face, char* faceName);
  /// Unregister a font. Called by font's dtor
  void RemoveFont (iFont* font, char* fontId);

  /**
   * Load a font by name.
   * Returns a new iFont object or 0 on failure.
   */
  virtual csPtr<iFont> LoadFont (const char *filename, float size = 10.0f);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFreeType2Server);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __CS_FREEFONT2_H__
