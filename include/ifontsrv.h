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

#ifndef __IFNTRNDR_H__
#define __IFNTRNDR_H__

#include "csutil/scf.h"
#include "iplugin.h"

enum CS_FONTPROPERTY
{
  CS_FONTSIZE
};

SCF_VERSION (iFontServer, 1, 0, 0);

/**
 * A font server interface.  Serves up fonts.
 */
struct iFontServer : public iPlugIn
{
  /**
   * Load a font by name.
   * Returns a font id by which the font can be referenced further.
   * RETURN:
   * -1 ... loading failed
   * >=0 ... the font id
   */
  virtual int LoadFont (const char* name, const char* filename) = 0;

  /**
   * Set font property.<p>
   * Return true if chage was successful.  If not, the server returns false,
   * decides whats the next best possible thing to this property, returns this
   * new value in 'property' and if wanted also applies this next best thing.
   */
  virtual bool SetFontProperty (int fontId, CS_FONTPROPERTY propertyId,
    long& property, bool autoApply) = 0;

  /// Get a font property. Returns true if the property could be retrieved.
  virtual bool GetFontProperty (int fontId, CS_FONTPROPERTY propertyId,
    long& property) = 0;

  /**
   * Return a pointer to a bitmap containing a rendered character (by current
   * font).  NULL if error occured.
   */
  virtual unsigned char* GetGlyphBitmap (int fontId, unsigned char c,
    int &oW, int &oH) = 0;

  /// Return character size in pixels. Returns false if values could not be determined.
  virtual bool GetGlyphSize (int fontId, unsigned char c, int &oW, int &oH) = 0;

  /**
   * Return the maximum height of the glyphs.  Return -1 if it could not be
   * determined.
   */
  virtual int GetMaximumHeight (int fontId) = 0;

  /// Return minimal bounding box of text.
  virtual void GetTextDimensions (int fontId, const char* text,
    int& width, int& height) = 0;

  /// Return number of fonts served.
  virtual int GetFontCount () = 0;
};

#endif // __IFNTRNDR_H__
