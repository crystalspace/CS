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

#include "cssysdef.h"
#include "csfont.h"
#include "ivfs.h"
#include "isystem.h"
#include "csutil/util.h"
#include "csutil/csvector.h"

#include "police.fnt"
#include "courier.fnt"	// font (C) Andrew Zabolotny
#include "tiny.fnt"	// font (C) Andrew Zabolotny
#include "italic.fnt"	// font (C) Andrew Zabolotny

struct csFontDef
{
  char *Name;
  int Width;
  int Height;
  int BytesPerChar;
  uint8 *FontBitmap;
  uint8 *IndividualWidth;
};

static csFontDef const FontList [] =
{
  { CSFONT_LARGE,	8, 8, 8,	font_Police,	width_Police	},
  { CSFONT_ITALIC,	8, 8, 8,	font_Italic,	width_Italic	},
  { CSFONT_COURIER,	7, 8, 8,	font_Courier,	width_Courier	},
  { CSFONT_SMALL,	4, 6, 8,	font_Tiny,	width_Tiny	}
};

int const FontListCount = sizeof (FontList) / sizeof (csFontDef);

IMPLEMENT_IBASE (csDefaultFontServer)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iFontServer)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csDefaultFontServer)

EXPORT_CLASS_TABLE (csfont)
  EXPORT_CLASS (csDefaultFontServer, "crystalspace.font.server.default", 
    "Crystal Space default font server")
EXPORT_CLASS_TABLE_END

csDefaultFontServer::csDefaultFontServer (iBase *pParent) : System(0)
{
  CONSTRUCT_IBASE (pParent);
}

csDefaultFontServer::~csDefaultFontServer()
{
  if (System)
    System->DecRef();
}

bool csDefaultFontServer::Initialize (iSystem* sys)
{
  (System = sys)->IncRef ();
  return true;
}

iFont *csDefaultFontServer::LoadFont (const char *filename)
{
  // First of all, look for an already loaded font
  for (int i = 0; i < fonts.Length (); i++)
  {
    csDefaultFont *font = fonts.Get (i);
    if (!strcmp (filename, font->Name))
    {
      font->IncRef ();
      return font;
    }
  }

  // Now check the list of static fonts
  if (filename [0] == '*')
  {
    for (int i = 0; i < FontListCount; i++)
      if (!strcmp (filename, FontList [i].Name))
        return new csDefaultFont (this, FontList [i].Name,
          FontList [i].Width, FontList [i].Height, FontList [i].BytesPerChar,
          FontList [i].FontBitmap, FontList [i].IndividualWidth);
  }
  else
  {
    // Otherwise try to load the font as a .csf file
    //@@todo
  }

  return NULL;
}

iFont *csDefaultFontServer::GetFont (int iIndex)
{
  if ((iIndex >= 0) && (iIndex < fonts.Length ()))
  {
    iFont *font = fonts.Get (iIndex);
    if (font) { font->IncRef (); return font; }
  }
  return NULL;
}

void csDefaultFontServer::NotifyCreate (csDefaultFont *font)
{
  fonts.Push (font);
}

void csDefaultFontServer::NotifyDelete (csDefaultFont *font)
{
  int iIndex = fonts.Find (font);
  if (iIndex >= 0)
  {
    // Set the cell to 0 to avoid the font being freed twice
    fonts.Get (iIndex) = NULL;
    fonts.Delete (iIndex);
  }
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//- The font object --//--//

IMPLEMENT_IBASE (csDefaultFont)
  IMPLEMENTS_INTERFACE (iFont)
IMPLEMENT_IBASE_END

csDefaultFont::csDefaultFont (csDefaultFontServer *parent, const char *name,
  int width, int height, int bytesperchar, uint8 *bitmap, uint8 *individualwidth)
  : DeleteCallbacks (4, 4)
{
  CONSTRUCT_IBASE (NULL);
  Parent = parent;
  Parent->NotifyCreate (this);
  if (name [0] != '*')
    Name = strnew (name);
  else
    Name = CONST_CAST(char*)(name);
  Width = width;
  Height = height;
  BytesPerChar = bytesperchar;
  FontBitmap = bitmap;
  IndividualWidth = individualwidth;
  if (IndividualWidth)
  {
    MaxWidth = 0;
    for (int i = 0; i < 256; i++)
      if (MaxWidth < IndividualWidth [i])
        MaxWidth = IndividualWidth [i];
  }
  else
    MaxWidth = Width;
}

csDefaultFont::~csDefaultFont ()
{
  for (int i = DeleteCallbacks.Length () - 2; i >= 0; i -= 2)
    ((DeleteNotify)DeleteCallbacks.Get (i)) (this, DeleteCallbacks.Get (i + 1));

  Parent->NotifyDelete (this);
  if (Name [0] != '*')
  {
    delete [] Name;
    delete [] FontBitmap;
    delete [] IndividualWidth;
  }
}

void csDefaultFont::SetSize (int iSize)
{
  (void)iSize;
}

int csDefaultFont::GetSize ()
{
  return 0;
}

void csDefaultFont::GetMaxSize (int &oW, int &oH)
{
  oW = MaxWidth;
  oH = Height; 
}

bool csDefaultFont::GetGlyphSize (uint8 c, int &oW, int &oH)
{
  oW = IndividualWidth ? IndividualWidth [c] : Width;
  oH = Height;
  return true;
}

uint8 *csDefaultFont::GetGlyphBitmap (uint8 c, int &oW, int &oH)
{
  oW = IndividualWidth ? IndividualWidth [c] : Width;
  oH = Height;
  return FontBitmap + c * BytesPerChar;
}

void csDefaultFont::GetDimensions (const char *text, int &oW, int &oH)
{
  oH = Height;
  oW = 0;

  const int n = strlen (text);
  if (!IndividualWidth)
    oW = n * Width;
  else
    for (int i = 0; i < n; i++)
      oW += IndividualWidth [*(uint8 *)text++];
}

int csDefaultFont::GetLength (const char *text, int maxwidth)
{
  if (!IndividualWidth)
    return (strlen (text) * Width) / maxwidth;

  int n = 0;
  while (*text)
  {
    int w = IndividualWidth [*(uint8 *)text];
    if (maxwidth < w)
      return n;
    maxwidth -= w;
    text++; n++;
  }
  return n;
}

void csDefaultFont::AddDeleteCallback (DeleteNotify func, void *databag)
{
  DeleteCallbacks.Push ((void *)func);
  DeleteCallbacks.Push (databag);
}

bool csDefaultFont::RemoveDeleteCallback (DeleteNotify func, void *databag)
{
  for (int i = DeleteCallbacks.Length () - 2; i >= 0; i -= 2)
    if ((DeleteCallbacks.Get (i) == (void *)func)
     && (DeleteCallbacks.Get (i + 1) == databag))
    {
      DeleteCallbacks.Delete (i);
      DeleteCallbacks.Delete (i);
      return true;
    }
  return false;
}
