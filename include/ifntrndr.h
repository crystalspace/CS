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

#ifndef _IFNTRNDR_H_
#define _IFNTRNDR_H_

#include "csutil/scf.h"
#include "iplugin.h"

SCF_VERSION (iFontRender, 0, 0, 1);

enum CS_FONTPROPERTY
{
  CS_FONTSIZE
};

/**
 * @@@ Please document me using Doc++!
 */
struct iFontRender : public iPlugIn
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
   * Set font property.
   * Return true if chage was successfull. If not, the render returns false, decides whats the next
   * best possible thing to this propertym, returns this new value in invalue and if wanted also applies
   * this next best thing.
   */
  virtual bool SetFontProperty (int fontId, CS_FONTPROPERTY propertyId, long& property, bool autoApply ) = 0;

  /**
   * Get a font property. Returns true if the property could be retrieved.
   */
  virtual bool GetFontProperty (int fontId, CS_FONTPROPERTY propertyId, long& property) = 0;

  /**
   * Return a pointer to a bitmap containing a rendered character (by current font). NULL if error occured.
   */
  virtual unsigned char* GetCharBitmap (int fontId, unsigned char c) = 0;

  /**
   * Return width and height of a char. Returns false if values could not be determined.
   */
  virtual int GetCharWidth (int fontId, unsigned char c) = 0;
  virtual int GetCharHeight (int fontId, unsigned char c) = 0;

  /**
   * Return the maximum height of the glyphs. Return -1 if it could not be determined.
   */
  virtual int GetMaximumHeight (int fontId) = 0;

  /**
   * Return minimal bounding box of text.
   */
  virtual void GetTextDimensions (int fontId, const char* text, int& width, int& height) = 0;

  /**
   * Return number of fonts served.
   */
  virtual int GetFontCount () = 0;
};

#endif
