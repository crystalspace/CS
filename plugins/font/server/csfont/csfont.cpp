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

static csFontDef const FontList [] =
{
  { "Police",		8, 8, 8,	font_Police,	width_Police	},
  { "Police.fixed",	8, 8, 8,	font_Police,	NULL		},
  { "Italic",		8, 8, 8,	font_Italic,	width_Italic	},
  { "Italic.fixed",	8, 8, 8,	font_Italic,	NULL		},
  { "Courier",		7, 8, 8,	font_Courier,	width_Courier	},
  { "Courier.fixed",	8, 8, 8,	font_Courier,	NULL		},
  { "Tiny",		4, 6, 8,	font_Tiny,	width_Tiny	},
  { "Tiny.fixed",	6, 6, 8,	font_Tiny,	NULL		}
};
int const FontListCount = sizeof(FontList) / sizeof(FontList[0]);

IMPLEMENT_IBASE (csDefaultFontServer)
  IMPLEMENTS_INTERFACE (iFontServer)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csDefaultFontServer)

EXPORT_CLASS_TABLE (csfont)
  EXPORT_CLASS (csDefaultFontServer, "crystalspace.font.server.csfont", 
		"CrystalSpace default font server" )
EXPORT_CLASS_TABLE_END

csDefaultFontServer::csDefaultFontServer (iBase *pParent) : System(0)
{
  CONSTRUCT_IBASE (pParent);
  for (int i = 0; i < FontListCount; i++)
    fonts.Push(CONST_CAST(csFontDef*)(FontList + i));
}

csDefaultFontServer::~csDefaultFontServer()
{
  for (int i = fonts.Length(); i-- > FontListCount; )
  {
    csFontDef* f = GetFontDef(i);
    if (f->Name) delete[] f->Name;
    if (f->FontBitmap) delete[] f->FontBitmap;
    if (f->IndividualWidth) delete[] f->IndividualWidth;
    delete f;
  }
  if (System)
    System->DecRef();
}

bool csDefaultFontServer::Initialize (iSystem* sys)
{
  System = sys;
  System->IncRef();
  return true;
}

csFontDef* csDefaultFontServer::ReadFntFile(const char *file)
{
  // read a .fnt file, originally meant for use as #include into CS.
  iVFS *VFS = QUERY_PLUGIN(System, iVFS);
  iDataBuffer *fntfile = VFS->ReadFile(file);
  VFS->DecRef();
  if(!fntfile)
  {
    System->Printf(MSG_WARNING, "Could not read file %s.\n", file);
    return 0;
  }
  
  /// the new fontdef to store info into
  csFontDef* fontdef = new csFontDef;
  fontdef->Name = 0;
  fontdef->Width = 0;
  fontdef->Height = 0;
  fontdef->BytesPerChar = 0;
  fontdef->FontBitmap = 0;
  fontdef->IndividualWidth = 0;

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
        fontdef->Name = strnew(name);
      }
      else if(strcmp(option, "Size:")==0)
      {
        sscanf(arg, "%d", &val);
	FontSize = val;
      }
      else if(strcmp(option, "MaxWidth:")==0)
      {
        sscanf(arg, "%d", &val);
        fontdef->Width = val;
      }
      else if(strcmp(option, "MaxHeight:")==0)
      {
        sscanf(arg, "%d", &val);
        fontdef->Height = val;
      }
      else if(strcmp(option, "BytesPerChar:")==0)
      {
        sscanf(arg, "%d", &val);
        fontdef->BytesPerChar = val;
      }
      //// ignore unknown options...
    }
    // shift cp a little so that the current //XX is not found again
    cp += 2;
  }
  
  int bitmaplen = 256*fontdef->BytesPerChar;
  if (fontdef->Width  <= 0 || fontdef->Height <= 0 || bitmaplen <= 0)
  {
      System->Printf(MSG_WARNING, "File %s contains invalid font metrics.\n",
        file);
      goto error_exit;
  }

#if defined(CS_DEBUG)
  printf("Reading Font %s, size %d, width %d, height %d, bytesperchar %d.\n",
    fontdef->Name, FontSize, fontdef->Width, fontdef->Height, 
    fontdef->BytesPerChar);
#endif

  /// alloc
  fontdef->FontBitmap = new unsigned char [bitmaplen];
  fontdef->IndividualWidth = new unsigned char [256];
  
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
    memcpy(fontdef->FontBitmap, cp, bitmaplen);
    cp += bitmaplen;
    for(int i=0; i<256; i++)
      fontdef->IndividualWidth[i] = cp[i];
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
      fontdef->FontBitmap[i] = byte;
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
        fontdef->IndividualWidth[i] = byte;
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
  return fontdef;

error_exit:
   if (fontdef->Name)
     delete[] fontdef->Name;
   if (fontdef->FontBitmap)
     delete[] fontdef->FontBitmap;
   if (fontdef->IndividualWidth)
     delete[] fontdef->IndividualWidth;
   delete fontdef;
   fntfile->DecRef();
   return 0;
}


int csDefaultFontServer::LoadFont (const char* name, const char* filename)
{
  for (int i = fonts.Length(); i-- > 0; )
    if (!strcmp (GetFontDef(i)->Name, name))
      return i;

  /// if it is a .fnt file we can load it
  int len = strlen(filename);
  if ((len > 4) && (strcmp(filename+len-4, ".fnt" )==0))
  {
    csFontDef *fontdef = ReadFntFile(filename);
    if (fontdef)
    {
      /// add to registered fonts
      if (fontdef->Name == 0 || strcmp(fontdef->Name, name) != 0)
      {
        // user wants to load file under a different name
        if (fontdef->Name)
          delete [] fontdef->Name;
        fontdef->Name = strnew(name);
      }
      fonts.Push(fontdef);
      return fonts.Length() - 1;
    }
  }
  return -1;
}

bool csDefaultFontServer::SetFontProperty (int /*fontId*/,
  CS_FONTPROPERTY propertyId, long& property, bool /*autoApply*/)
{
  bool succ = (propertyId == CS_FONTSIZE);
  if (succ && property != 1) property = 1;
  return succ;
}

bool csDefaultFontServer::GetFontProperty (int fontId,
  CS_FONTPROPERTY propertyId, long& property)
{
  (void)fontId;
  bool succ = (propertyId == CS_FONTSIZE);
  if (succ) property = 1;
  return succ;
}

unsigned char *csDefaultFontServer::GetGlyphBitmap (
  int fontId, unsigned char c, int &oW, int &oH)
{ 
  csFontDef const* f = GetFontDef(fontId);
  oW = f->IndividualWidth ? f->IndividualWidth [c] : f->Width;
  oH = f->Height;
  return f->FontBitmap + c * f->BytesPerChar;
}

bool csDefaultFontServer::GetGlyphSize (
  int fontId, unsigned char c, int &oW, int &oH)
{
  csFontDef const* f = GetFontDef(fontId);
  oW = f->IndividualWidth ? f->IndividualWidth [c] : f->Width;
  oH = f->Height;
  return true;
}

int csDefaultFontServer::GetMaximumHeight (int fontId)
{
  return GetFontDef (fontId)->Height; 
}

void csDefaultFontServer::GetTextDimensions (
  int fontId, const char* text, int& width, int& height)
{
  csFontDef const* f = GetFontDef(fontId);
  height = f->Height;
  width = 0;

  int const n = strlen (text);
  if (!f->IndividualWidth)
    width = n * f->Width;
  else
    for (int i = 0; i < n; i++)
      width += f->IndividualWidth [*(unsigned char *)text++];
}
