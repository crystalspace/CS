/*
    Copyright (C) 2000 by Norman Krämer
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

#ifndef _CSFONT_H_
#define _CSFONT_H_

#include "ifntrndr.h"
#include "police.fnt"
#include "courier.fnt"	// font (C) Andrew Zabolotny
#include "tiny.fnt"	// font (C) Andrew Zabolotny
#include "italic.fnt"	// font (C) Andrew Zabolotny

class csDefaultFontRender : public iFontRender
{
  struct FontDef {
    int Width;
    int Height;
    int BytesPerChar;
    unsigned char *IndividualWidth;
    unsigned char *FontBitmap;
  };

  static FontDef FontList[];

 protected:
  int nFonts;

 public:
  DECLARE_IBASE;

  csDefaultFontRender (iBase *pParent);

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
   * Return true if chage was successfull. If not, the renderer returns false, decides whats the next
   * best possible thing to this propertym, returns this new value in invalue and if wanted also applies
   * this next best thing.
   */
  virtual bool SetFontProperty (int fontId, CS_FONTPROPERTY propertyId, long& property, bool autoApply );

  /**
   * Get a font property. Returns true if the property could be retrieved.
   */
  virtual bool GetFontProperty (int fontId, CS_FONTPROPERTY propertyId, long& property);

  /**
   * Return a pointer to a bitmap containing a rendered character (by current font). NULL if error occured.
   */
  virtual unsigned char* GetCharBitmap (int fontId, unsigned char c);

  /**
   * Return width and height of a char. Returns false if values could not be determined.
   */
  virtual int GetCharWidth (int fontId, unsigned char c);

  virtual int GetCharHeight (int fontId, unsigned char /*c*/);

  virtual int GetMaximumHeight (int fontId);

  /**
   * Return minimal boundings of text.
   */
  virtual void GetTextDimensions (int fontId, const char* text, int& width, int& height);

  virtual int GetFontCount (){ return nFonts; }
};

#endif
