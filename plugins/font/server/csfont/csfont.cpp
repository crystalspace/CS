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
    /// if it is a .fnt file we can load it
    int len = strlen(filename);
    if ( ((len > 4) && (strcmp(filename+len-4, ".fnt" )==0))
     || ((len > 4) && (strcmp(filename+len-4, ".csf" )==0)) )
    {
      csDefaultFont *fontdef = ReadFntFile(filename);
      if (fontdef)
      {
        /// add to registered fonts
        if (fontdef->Name) delete [] fontdef->Name;
        fontdef->Name = strnew(filename);
        fonts.Push(fontdef);
        return fontdef;
      }
    }
  }

  return NULL;
}

iFont *csDefaultFontServer::GetFont (int iIndex)
{
  if ((iIndex >= 0) && (iIndex < fonts.Length ()))
  {
    iFont *font = fonts.Get (iIndex);
    if (font) return font;
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


csDefaultFont* csDefaultFontServer::ReadFntFile(const char *file)
{
  // read a .fnt file, originally meant for use as #include into CS.
  // renamed to .csf file
  iVFS *VFS = QUERY_PLUGIN(System, iVFS);
  iDataBuffer *fntfile = VFS->ReadFile(file);
  VFS->DecRef();
  if(!fntfile)
  {
    System->Printf(MSG_WARNING, "Could not read file %s.\n", file);
    return 0;
  }
  
  /// the new fontdef to store info into
  csDefaultFont *resultfont = 0;
  csFontDef fontdef;
  fontdef.Name = 0;
  fontdef.Width = 0;
  fontdef.Height = 0;
  fontdef.BytesPerChar = 0;
  fontdef.FontBitmap = 0;
  fontdef.IndividualWidth = 0;

  /// parse file
  int skip = 0; 
  char *cp = **fntfile;
  int val = 0;
  int FontSize = 10;
  int span = 0;
  /// find the //XX parts
  while ((cp = strstr(cp, "//XX")) != 0)
  {
    // parse info
    char option[50];
    if(sscanf(cp, "//XX %50s %n", option, &skip)==1)
    {
      char *arg = cp+skip;
      if(strcmp(option, "Name:")==0)
      {
        char name[50];
        sscanf(arg, "%50s", name);
        fontdef.Name = strnew(name);
      }
      else if(strcmp(option, "Size:")==0)
      {
        sscanf(arg, "%d", &val);
        FontSize = val;
      }
      else if(strcmp(option, "MaxWidth:")==0)
      {
        sscanf(arg, "%d", &val);
        fontdef.Width = val;
      }
      else if(strcmp(option, "MaxHeight:")==0)
      {
        sscanf(arg, "%d", &val);
        fontdef.Height = val;
      }
      else if(strcmp(option, "BytesPerChar:")==0)
      {
        sscanf(arg, "%d", &val);
        fontdef.BytesPerChar = val;
      }
      //// ignore unknown options...
    }
    // shift cp a little so that the current //XX is not found again
    cp += 2;
  }
  
  int bitmaplen = 256*fontdef.BytesPerChar;
  if (fontdef.Width  <= 0 || fontdef.Height <= 0 || bitmaplen <= 0)
  {
      System->Printf(MSG_WARNING, "File %s contains invalid font metrics.\n",
        file);
      goto error_exit;
  }

#if defined(CS_DEBUG)
  printf("Reading Font %s, size %d, width %d, height %d, bytesperchar %d.\n",
    fontdef.Name, FontSize, fontdef.Width, fontdef.Height, 
    fontdef.BytesPerChar);
#endif

  /// alloc
  fontdef.FontBitmap = new unsigned char [bitmaplen];
  fontdef.IndividualWidth = new unsigned char [256];
  
  cp = **fntfile;

  /// if this is a binary fnt file we can do it faster
  if (strncmp (**fntfile, "FNTBINARY", 9) == 0)
  {
    const char * startstr = "STARTBINARY";
    cp = strstr(cp, startstr);
    if(!cp)
    {
      System->Printf(MSG_WARNING, "File %s has no binary part.\n", file);
      goto error_exit;
    }
    cp += strlen(startstr) +1; /// also skip \n
    if( fntfile->GetSize () - (cp - **fntfile) < (unsigned int)( bitmaplen + 256 ) )
    {
      System->Printf(MSG_WARNING, "File %s is too short.\n", file);
      goto error_exit;
    }
    memcpy(fontdef.FontBitmap, cp, bitmaplen);
    cp += bitmaplen;
    for(int i=0; i<256; i++)
      fontdef.IndividualWidth[i] = cp[i];
    goto success_exit;
  }

  /// read fontbitmaps
  /// find first 'unsigned char' then the '{' after that.
  cp = strstr(cp, "unsigned char");
  if(cp)
    cp = strchr(cp, '{');
  if(!cp) 
  {
    System->Printf(MSG_WARNING, "File %s has no font bitmap.\n", file);
    goto error_exit;
  }
  cp++; /// skip the {

  int i;
  int byte;
  for(i=0; i<bitmaplen; i++)
  {
    if(sscanf(cp, " %x%n", &byte, &skip)==1)
    {
      fontdef.FontBitmap[i] = byte;
      cp += skip;
    }
    else 
    {
      //printf("i = %d, cp %c%c%c%c\n", i, cp[0], cp[1], cp[2], cp[3]);
      printf("Could not read font bitmap byte\n");
      goto error_exit;
    }
    cp++; // go one further, to skip the ','
  }
  
  /// read widths
  cp = strstr(cp, "unsigned char");
  if(cp)
    cp = strchr(cp, '{');
  if(!cp) 
  {
    System->Printf(MSG_WARNING, "File %s has no font bitmap.\n", file);
    goto error_exit;
  }
  cp++;

  for(span=0; span<16; span++)
  {
    for(i=span*16; i<span*16+16; i++)
    {
      if(sscanf(cp, " %d%n", &byte, &skip)==1)
      {
        fontdef.IndividualWidth[i] = byte;
        cp += skip;
      }
      else 
      {
        //printf("i = %d, cp %c%c%c%c\n", i, cp[0], cp[1], cp[2], cp[3]);
        printf("Could not read font character width\n");
        goto error_exit;
      }
      cp ++; /// skip ,
    }
    // skip the comment to end of line
    cp = strchr(cp, '\n');
  }

success_exit:
  fntfile->DecRef ();
  resultfont = new csDefaultFont(this, fontdef.Name, fontdef.Width,
    fontdef.Height, fontdef.BytesPerChar, fontdef.FontBitmap, 
    fontdef.IndividualWidth);
  if (fontdef.Name)
    delete[] fontdef.Name;
  return resultfont ;

error_exit:
   if (fontdef.Name)
     delete[] fontdef.Name;
   if (fontdef.FontBitmap)
     delete[] fontdef.FontBitmap;
   if (fontdef.IndividualWidth)
     delete[] fontdef.IndividualWidth;
   fntfile->DecRef();
   return 0;
}


//--//--//--//--//--//--//--//--//--//--//--//--//--//- The font object --//--//

IMPLEMENT_IBASE (csDefaultFont)
  IMPLEMENTS_INTERFACE (iFont)
IMPLEMENT_IBASE_END

csDefaultFont::csDefaultFont (csDefaultFontServer *parent, const char *name,
  int width, int height, int bytesperchar, uint8 *bitmap, uint8 *individualwidth)
  : DeleteCallbacks (4, 4)
{
  CONSTRUCT_IBASE (parent);
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
