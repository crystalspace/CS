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

#ifndef _FREEFONT_H_
#define _FREEFONT_H_

#include "isystem.h"
#include "ifntrndr.h"
#include "csutil/csvector.h"
#include "csutil/util.h"

class csFreeTypeRender : public iFontRender
{
  struct CharDef {
    int Width; // in pixel
    int Height; // in pixel
    unsigned char *Bitmap;
  };

  struct FTDef {
    CharDef outlines[256];
    int size;
  };

  class csFontDefVector : public csVector {
  public:
    FTDef* Get (int n) { return (FTDef*)csVector::Get (n); }
    virtual int Compare (csSome Item1, csSome Item2, int /*Mode*/) const
      { int id1 = ((FTDef*)Item1)->size, id2 = ((FTDef*)Item2)->size; return id1 - id2; }

    virtual int CompareKey (csSome Item1, csConstSome Key, int /*Mode*/) const
      { int id1 = ((FTDef*)Item1)->size, id2 = (int)Key; return id1 - id2; }
  };
  
  class FT {
  public:
    FT (const char *name, int fontId){ this->name = strnew (name); this->fontId = fontId; }
    ~FT () 
      { 
	for(int i=0; i<cache.Length(); i++){
	  for(int j=0; j<256; j++) delete [] cache.Get(i)->outlines[j].Bitmap;
	  TT_Close_Face (face);
	  delete cache.Get (i);
	}
      }

    FTDef* GetFontDef (int size){ int i=cache.FindKey ((csConstSome)size); return (i==-1?NULL:cache.Get (i));}
    int GetFontIdx (int size){ return cache.FindKey ((csConstSome)size);}

    char *name;
    int fontId;
    int current;
    TT_Face face;
    TT_Instance instance;
    TT_Face_Properties prop;
    TT_UShort pID, eID;
    TT_CharMap charMap;
    csFontDefVector cache;
  };

  class csFontVector : public csVector {
  public:
    FT* Get (int n) { return (FT*)csVector::Get (n); }
    virtual int Compare (csSome Item1, csSome Item2, int /*Mode*/) const
    { int id1 = ((FT*)Item1)->fontId, id2 = ((FT*)Item2)->fontId; return id1 - id2; }

    virtual int CompareKey (csSome Item1, csConstSome Key, int Mode) const
      { 
	// compare the font ids
	if (Mode==0){ int id1 = ((FT*)Item1)->fontId, id2 = (int)Key; return id1 - id2; }
	// compare the font names
	else{ char *id1 = ((FT*)Item1)->name; const char *id2 = (const char*)Key; return strcmp (id1,id2); }
      }
  };

 protected:
  csFontVector fonts;
  TT_Engine engine;
  TT_UShort platformID, encodingID;
  int defaultSize;
  bool bInit;

  iSystem *pSystem;

  bool CreateGlyphBitmaps (FT *font, int size);

 public:
  DECLARE_IBASE;

  csFreeTypeRender (iBase *pParent);
  virtual ~csFreeTypeRender ();

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

  virtual int GetCharHeight (int fontId, unsigned char c);

  virtual int GetMaximumHeight (int fontId);

  /**
   * Return minimal boundings of text.
   */
  virtual void GetTextDimensions (int fontId, const char* text, int& width, int& height);

  virtual int GetFontCount (){ return fonts.Length(); }

};

#endif
