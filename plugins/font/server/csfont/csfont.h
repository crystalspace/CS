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

#ifndef __CS_CSFONT_H__
#define __CS_CSFONT_H__

#include "ifontsrv.h"
#include "csutil/csvector.h"

struct iSystem;

struct csFontDef
{
  char *Name;
  int Width;
  int Height;
  int BytesPerChar;
  unsigned char *FontBitmap;
  unsigned char *IndividualWidth;
};

/**
 * Default font server.
 */
class csDefaultFontServer : public iFontServer
{
private:
  iSystem *System;

  // A list of csFontDef pointers.
  csBasicVector fonts;

  // Read a .fnt file from VFS.  Return a csFontDef if ok, or NULL on error.
  csFontDef* ReadFntFile(const char *file);

  // Get the fontDef for a certain font ID
  csFontDef *GetFontDef(int id) {return (csFontDef*)fonts[id];}

public:
  DECLARE_IBASE;

  /// Create the plugin object
  csDefaultFontServer (iBase *pParent);
  /// destroy it
  virtual ~csDefaultFontServer ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /**
   * Load a font by name.
   * Returns a font id by which the font can be referenced further.
   * RETURN:
   * -1 ... loading failed
   * >=0 ... the font id
   */
  virtual int LoadFont (const char* name, const char* filename);

  /**
   * Set font property.
   * Return true if chage was successfull.  If not, the server returns false,
   * decides whats the next best possible thing to this propertym, returns
   * this new value in invalue and if wanted also applies this next best
   * thing.
   */
  virtual bool SetFontProperty (int fontId, CS_FONTPROPERTY propertyId,
    long& property, bool autoApply );

  /// Get a font property. Returns true if the property could be retrieved.
  virtual bool GetFontProperty (int fontId, CS_FONTPROPERTY propertyId,
    long& property);

  /**
   * Return a pointer to a bitmap containing a rendered character (by current
   * font).  NULL if error occured.
   */
  virtual unsigned char* GetGlyphBitmap (int fontId, unsigned char c,
    int &oW, int &oH);

  /**
   * Return character size in pixels.  Returns false if values could not be
   * determined.
   */
  virtual bool GetGlyphSize (int fontId, unsigned char c, int &oW, int &oH);

  /// Get maximal font height
  virtual int GetMaximumHeight (int fontId);

  /// Return minimal boundings of text.
  virtual void GetTextDimensions (int fontId, const char* text,
    int& width, int& height);

  /// Return font count.
  virtual int GetFontCount ()
  { return fonts.Length(); }
};

#endif // __CS_CSFONT_H__
