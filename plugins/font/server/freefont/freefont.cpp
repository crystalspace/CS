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

#include <freetype.h>
#include <ftxsbit.h>

#include "sysdef.h"
#include "csutil/inifile.h"
#include "csutil/csstrvec.h"
#include "freefont.h"

csFreeTypeRender::csFreeTypeRender (iBase *pParent)
{
  CONSTRUCT_IBASE (pParent);
  bInit = false;
}

csFreeTypeRender::~csFreeTypeRender ()
{
  // drop all fonts
  int i;
  for (i=0; i < fonts.Length(); i++) delete fonts.Get (i);
  if (pSystem) pSystem->DecRef ();
}

bool csFreeTypeRender::Initialize (iSystem *pSystem)
{
  if ( bInit ) return true;

  bool succ;
  this->pSystem = pSystem;
  pSystem->IncRef ();

  succ = TT_Init_FreeType (&engine) == 0;
  if (!succ)
    pSystem->Printf (MSG_FATAL_ERROR, "Could not create a TrueType engine instance !\n");
  else{
    int error = TT_Init_SBit_Extension (engine);
    if (error)
      pSystem->Printf(MSG_WARNING, "SBit extension not initialized: error %d\n", error);
    //    iVFS *v= pSystem->GetVFS ();
    csIniFile *ftini = new csIniFile ( "data/config/freetype.cfg");
    defaultSize = ftini->GetInt ("Default", "size", 10);
    platformID = ftini->GetInt ("Default", "platformid", 3);
    encodingID = ftini->GetInt ("Default", "encodingid", 1);

    csStrVector vFonts;
    const char *p = ftini->GetStr ("Default", "preload");
    ftini->EnumData (p, &vFonts);

    int i;
    for (i=0; i<vFonts.Length(); i++ ){
      pSystem->Printf (MSG_INITIALIZATION, "Load font %s\n",(const char*)vFonts[i]);
      LoadFont ((const char*)vFonts[i], ftini->GetStr (p, (const char*)vFonts[i]));
    }
    succ = (vFonts.Length() == 0 || fonts.Length() > 0 );
    //    v->DecRef ();
    delete ftini;
  }

  return (bInit=succ);
}

int csFreeTypeRender::LoadFont (const char* name, const char* filename)
{
  // first see if we already loaded that font
  int key = fonts.FindKey (name, 1);
  int fontid=-1;

  if (key==-1){
    // not yet loaded, so do it now
    FT *font = new FT (name, -1);
    font->current=0;
    int error = TT_Open_Face (engine, filename, &font->face);
    if (error)
      pSystem->Printf(MSG_WARNING, "Font %s could not be loaded from file %s!\n", name, filename);
    else {
      error = TT_Get_Face_Properties (font->face, &font->prop);
      if (error){
	pSystem->Printf(MSG_WARNING, "Get_Face_Properties: error %d.\n", error );
	delete font;
	return -1;
      }
      error = TT_New_Instance (font->face, &font->instance);
      if (error)
	pSystem->Printf(MSG_WARNING, "Could not create an instance of Font %s."
			" The font is probably broken!\n", name, filename);
      else {
	// we do not change the default values of the new instance ( 96 dpi, 10 pt. size, no trafo flags )
	
	// next we scan the charmap table if there is an encoding that matches the requested platform and
	// encoding ids
	TT_UShort i=0;
	font->pID=platformID-1;
	font->eID=encodingID-1;
	while (i < font->prop.num_CharMaps && platformID != font->pID && encodingID != font->eID){
	  error = TT_Get_CharMap_ID (font->face, i++, &font->pID, &font->eID);
	  if (error) pSystem->Printf(MSG_WARNING, "Get_CharMap_ID: error %d.\n", error );
	}

	i--;

	if (platformID != font->pID || encodingID != font->eID){
	  // encoding scheme not found
	  pSystem->Printf(MSG_INITIALIZATION, "Font %s does not contain encoding %d for platform %d.\n",
			  name, encodingID, platformID );
	  
	  error = TT_Get_CharMap_ID (font->face, 0, &font->pID, &font->eID);
	  if (error) pSystem->Printf(MSG_WARNING, "Get_CahrMap_ID: error %d.\n", error );
	  pSystem->Printf(MSG_INITIALIZATION, "Will instead use encoding %d for platform %d.\n", 
			  font->eID, font->pID);
	  i=0;
	}
	
	// ok. now lets retrieve a handle the the charmap
	error = TT_Get_CharMap (font->face, i, &font->charMap);
	if (error) pSystem->Printf(MSG_WARNING, "Get_CharMap: error %d.\n", error );
	
	// now we create the bitmap of all glyphs in the face
	if ( CreateGlyphBitmaps (font, defaultSize) ){
	  fontid = font->fontId = fonts.Length ();
	  fonts.Push (font);
	}
      }
    }
  }else
    fontid = fonts.Get (key)->fontId;
  
  return fontid;
}

bool csFreeTypeRender::CreateGlyphBitmaps (FT *font, int size)
{
  bool succ;
  FTDef *fontdef = font->GetFontDef (size);
  succ = fontdef != NULL;
  if (!succ){
    int error = TT_Set_Instance_CharSize (font->instance, size*64);
    if (error){
      pSystem->Printf(MSG_WARNING, "Set_Instance_CharSize: error %d.\n", error );
      return false;
    }
    // first get the instance metrics
    TT_Instance_Metrics im;
    error = TT_Get_Instance_Metrics (font->instance, &im);
    if (error){
      pSystem->Printf (MSG_WARNING, "Get_Instance_Metrics: error %d.\n", error);
      return false;
    }
    // does not exist, so we create it now
    TT_UShort iso_char;
    fontdef = new FTDef;
    fontdef->size = size;
    memset (fontdef->outlines, 0, 256*sizeof (CharDef) );
    
    // compute the maximum height a glyph from this font would have including linegap
    int maxY;
    int maxDesc;
    int lineGap;
    if (font->prop.os2->version == 0xffff){
      maxDesc = font->prop.horizontal->Descender;
      lineGap = font->prop.horizontal->Line_Gap;
      maxY= font->prop.horizontal->Ascender - maxDesc + lineGap;
    }else{
      lineGap = font->prop.os2->sTypoLineGap;
      /// @@@ EVIL: i found that some fonts incorrectly define descends as positive 
      /// numbers while they should be negative - so i force the numbers being negative
      maxDesc = -ABS(font->prop.os2->sTypoDescender);
      maxY = font->prop.os2->sTypoAscender - maxDesc + lineGap;
    }

    lineGap=(lineGap * im.y_scale)/0x10000;
    maxDesc=(maxDesc * im.y_scale)/0x10000;
    maxY   =(maxY * im.y_scale)/0x10000;

    lineGap=lineGap/64;
    maxY   =(maxY+32)/64;

    //    printf("* gap=%d desc=%d maxY=%d\n", lineGap, maxDesc/64, maxY);

    for (iso_char=0; iso_char < 256; iso_char++){
      TT_Glyph glyph;
      int error = TT_New_Glyph (font->face, &glyph);
      if (error){
	pSystem->Printf (MSG_WARNING, "Could not create glyph in font %s, error %d.\n", error, font->name);
	continue;
      }
      TT_UShort glyphindex = TT_Char_Index (font->charMap, iso_char);
      error = TT_Load_Glyph (font->instance, glyph, glyphindex, TTLOAD_DEFAULT);
      if (!error){
	// now we are going to render the glyphs outline into a bitmap
	TT_Raster_Map bitmap;
	TT_Big_Glyph_Metrics m;
	CharDef *cd = &fontdef->outlines[ iso_char ];
	
	// compute the extend
	error = TT_Get_Glyph_Big_Metrics (glyph, &m);
	if (error) pSystem->Printf(MSG_WARNING, "Get_Glyph_Metrics: error %d.\n", error );
	// grid fit ( numbers are 26.6 fixed point )
	/*
	m.bbox.xMin &= -64;
	m.bbox.yMin &= -64;
	m.bbox.xMax = (m.bbox.xMax+63) & -64;
	m.bbox.yMax = (m.bbox.yMax+63) & -64;
	*/
	// prepare the bitmap
	bitmap.rows  = maxY;
	//	bitmap.width = (m.bbox.xMax - m.bbox.xMin)/64;
	bitmap.width = MAX( m.horiAdvance, (m.horiBearingX + m.bbox.xMax - m.bbox.xMin)) / 64;
	bitmap.cols  = (bitmap.width+7)/8;
	bitmap.flow  = TT_Flow_Down;
	bitmap.size  = bitmap.rows * bitmap.cols;

	cd->Width = bitmap.width;
	cd->Height = bitmap.rows;
	cd->Bitmap = new unsigned char[bitmap.size];
	
	if (cd->Bitmap == NULL)
	  pSystem->Printf (MSG_WARNING, "Could not allocate memory to render character %c (dec:%d)"
			   " in font %s.\n", (char)iso_char, iso_char, font->name);
	else{
	  bitmap.bitmap = cd->Bitmap;
	  // zero out memory
	  memset (bitmap.bitmap, 0, bitmap.size);
	  // and finally render it into bitmap
	  error = TT_Get_Glyph_Bitmap ( glyph, &bitmap, 0/*-m.bbox.xMin*/, -maxDesc);
	  if (error) pSystem->Printf(MSG_WARNING, "Get_Outline_Bitmap: error %d.\n", error );
	}
	
      }else
	pSystem->Printf (MSG_WARNING, "Could not load glyph for character %c (dec:%d) in"
			 " font %s, error %d.\n", (char)iso_char, iso_char, font->name, error );
      error = TT_Done_Glyph (glyph);
      if (error) pSystem->Printf(MSG_WARNING, "Done_Glyph_: error %d.\n", error );
    }
    // our fontdescriptor is now filled
    succ = true;
    font->cache.Push (fontdef);
  }
  return succ;
}

bool csFreeTypeRender::SetFontProperty (int fontId, CS_FONTPROPERTY propertyId, long& property, 
					bool /*autoApply*/ )
{
  bool succ = false;
  if ( fontId >= 0 && fontId < fonts.Length() ){
    FT *font = fonts.Get (fontId);
    if (propertyId == CS_FONTSIZE) {
      FTDef *fontdef = font->GetFontDef (property);
      succ = (fontdef!=NULL);
      if (!succ){
	// the size does not yet exist, so we create it
	succ = CreateGlyphBitmaps (font, property);
      }
      if (succ) font->current = MAX( 0, font->GetFontIdx (property));
    }else
      pSystem->Printf (MSG_WARNING, "Unknown fontproperty %d requested.\n", (long)propertyId);
  }else
    pSystem->Printf (MSG_WARNING, "Unknown fontid %d requested, currently %d fonts are known.\n",
		     fontId, fonts.Length() );
  return succ;
}

bool csFreeTypeRender::GetFontProperty (int fontId, CS_FONTPROPERTY propertyId, long& property)
{
  bool succ = false;
  if ( fontId >= 0 && fontId < fonts.Length() ){
    FT *font = fonts.Get (fontId);
    if (propertyId == CS_FONTSIZE) {
      succ = true;
      property = font->cache.Get (font->current)->size;
    }else
      pSystem->Printf (MSG_WARNING, "Unknown fontproperty %d requested.\n", (long)propertyId);
  }else
    pSystem->Printf (MSG_WARNING, "Unknown fontid %d requested, currently %d fonts are known.\n",
		     fontId, fonts.Length() );  
  return succ;
}

unsigned char* csFreeTypeRender::GetCharBitmap (int fontId, unsigned char c)
{
  unsigned char *bm=NULL;
  if ( fontId >= 0 && fontId < fonts.Length() ){
    FT *font = fonts.Get (fontId);
    bm = font->cache.Get (font->current)->outlines[ c ].Bitmap;
  }else
    pSystem->Printf (MSG_WARNING, "Unknown fontid %d requested, currently %d fonts are known.\n",
		     fontId, fonts.Length() );
  return bm;
}

int csFreeTypeRender::GetCharWidth (int fontId, unsigned char c)
{
  int width=0;
  if ( fontId >= 0 && fontId < fonts.Length() ){
    FT *font = fonts.Get (fontId);
    width = font->cache.Get (font->current)->outlines[ c ].Width;
  }else
    pSystem->Printf (MSG_WARNING, "Unknown fontid %d requested, currently %d fonts are known.\n",
		     fontId, fonts.Length() );
  return width;
}

int csFreeTypeRender::GetCharHeight (int fontId, unsigned char c)
{
  int height=0;
  if ( fontId >= 0 && fontId < fonts.Length() ){
    FT *font = fonts.Get (fontId);
    height = font->cache.Get (font->current)->outlines[ c ].Height;
  }else
    pSystem->Printf (MSG_WARNING, "Unknown fontid %d requested, currently %d fonts are known.\n",
		     fontId, fonts.Length() );
  return height;
}

int csFreeTypeRender::GetMaximumHeight (int fontId)
{
  // all chars have same heigth so return any
  int height=-1;
  if ( fontId >= 0 && fontId < fonts.Length() ){
    FT *font = fonts.Get (fontId);
    height = font->cache.Get (font->current)->outlines[ 'T' ].Height;
  }else
    pSystem->Printf (MSG_WARNING, "Unknown fontid %d requested, currently %d fonts are known.\n",
		     fontId, fonts.Length() );
  return height;

}

void csFreeTypeRender::GetTextDimensions (int fontId, const char* text, int& width, int& height)
{
  width = height = 0;
  if ( fontId >= 0 && fontId < fonts.Length() ){
    const char *p=text;
    FT *font = fonts.Get (fontId);
    FTDef *fontdef = font->cache.Get (font->current);
    while (*p){
      width += fontdef->outlines[*p].Width;
      height = MAX (height, fontdef->outlines[*p].Height);
      p++;
    }
  }else
    pSystem->Printf (MSG_WARNING, "Unknown fontid %d requested, currently %d fonts are known.\n",
		     fontId, fonts.Length() );
}

IMPLEMENT_IBASE (csFreeTypeRender)
  IMPLEMENTS_INTERFACE (iFontRender)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csFreeTypeRender)

EXPORT_CLASS_TABLE (freefont)
  EXPORT_CLASS (csFreeTypeRender, "crystalspace.font.render.freetype", 
		"CrystalSpace FreeType font renderer" )
EXPORT_CLASS_TABLE_END
