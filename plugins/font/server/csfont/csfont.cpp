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

#include "cssysdef.h"
#include "csfont.h"

#include "police.fnt"
#include "courier.fnt"	// font (C) Andrew Zabolotny
#include "tiny.fnt"	// font (C) Andrew Zabolotny
#include "italic.fnt"	// font (C) Andrew Zabolotny

csFontDef csDefaultFontRender::FontList [] =
{
  { "Police",		8, 8, 8,	font_Police,	width_Police	},
  { "Police.fixed",	8, 8, 8,	font_Police,	NULL		},
  { "Italic",		8, 8, 8,	font_Italic,	width_Italic	},
  { "Italic.fixed",	8, 8, 8,	font_Italic,	NULL		},
  { "Courier",		7, 8, 8,	font_Courier,	width_Courier	},
  { "Courier.fixed",	8, 8, 8,	font_Courier,	NULL		},
  { "Tiny",		4, 6, 8,	font_Tiny,	width_Tiny	},
  { "Tiny.fixed",	6, 6, 8,	font_Tiny,	NULL		},
  { NULL,               0, 0, 0,        NULL,           NULL            }
};

IMPLEMENT_IBASE (csDefaultFontRender)
  IMPLEMENTS_INTERFACE (iFontRender)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csDefaultFontRender)

EXPORT_CLASS_TABLE (csfont)
  EXPORT_CLASS (csDefaultFontRender, "crystalspace.font.render.csfont", 
		"CrystalSpace default font renderer" )
EXPORT_CLASS_TABLE_END

csDefaultFontRender::csDefaultFontRender (iBase *pParent)
{
  CONSTRUCT_IBASE (pParent);
  FontCount = sizeof(FontList) / sizeof(FontList[0]) - 1;
}

bool csDefaultFontRender::Initialize (iSystem*)
{
  return true;
}

int csDefaultFontRender::LoadFont (const char* name, const char* /*filename*/)
{
  for (int i = 0; FontList [i].Name; i++)
    if (!strcmp (FontList [i].Name, name))
      return i;
  return -1;
}

bool csDefaultFontRender::SetFontProperty (int /*fontId*/,
  CS_FONTPROPERTY propertyId, long& property, bool /*autoApply*/)
{
  bool succ = (propertyId == CS_FONTSIZE);
  if (succ && property != 1) property = 1;
  return succ;
}

bool csDefaultFontRender::GetFontProperty (int fontId,
  CS_FONTPROPERTY propertyId, long& property)
{
  (void)fontId;
  bool succ = (propertyId == CS_FONTSIZE);
  if (succ) property = 1;
  return succ;
}

unsigned char *csDefaultFontRender::GetCharBitmap (int fontId, unsigned char c)
{ 
  return &(FontList [fontId].FontBitmap [c * FontList [fontId].BytesPerChar]); 
}

int csDefaultFontRender::GetCharWidth (int fontId, unsigned char c)
{
  int width;
  if (FontList [fontId].IndividualWidth)
    width = FontList [fontId].IndividualWidth [c];
  else
    width = FontList [fontId].Width;

  return width;
}

int csDefaultFontRender::GetCharHeight (int fontId, unsigned char /*c*/)
{ 
  return FontList [fontId].Height; 
}

int csDefaultFontRender::GetMaximumHeight (int fontId)
{
  return FontList [fontId].Height; 
}

void csDefaultFontRender::GetTextDimensions (int fontId, const char* text,
  int& width, int& height)
{
  int i, n = strlen (text);

  width = 0;
  height = 0;

  if (FontList [fontId].IndividualWidth)
    for (i = 0; i < n; i++)
      width += FontList [fontId].IndividualWidth [*(unsigned char *)text++];
  else
    width = n * FontList [fontId].Width;
  height = FontList [fontId].Height;
}
