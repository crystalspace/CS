/*
    Copyright (C) 2002 by Norman Kraemer

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
#include "csutil/sysfunc.h"
#include "csutil/csuctransform.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "iutil/cfgfile.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_MODULE_H
#include "freefnt2.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csFreeType2Server)

SCF_IMPLEMENT_IBASE (csFreeType2Server)
  SCF_IMPLEMENTS_INTERFACE (iFontServer)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFreeType2Server::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csFreeType2Server::csFreeType2Server (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  library = 0;
  freetype_inited = false;
}

csFreeType2Server::~csFreeType2Server ()
{
  fonts.DeleteAll();
  if (freetype_inited) 
  {
    FT_Done_FreeType (library);
  }
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

void csFreeType2Server::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  ReportV (severity, msg, arg);
  va_end (arg);
}

void csFreeType2Server::ReportV (int severity, const char* msg, va_list arg)
{
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.font.freefont2", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
}

const char* csFreeType2Server::GetErrorDescription(int code)
{
  #undef __FTERRORS_H__
  #define FT_ERROR_START_LIST		    switch (code) {
  #define FT_ERRORDEF(Error, Value, String) case Value: return String;
  #define FT_ERROR_END_LIST		    default: return "unknown error"; }
  #include FT_ERRORS_H
}

bool csFreeType2Server::FreetypeError (int errorCode, int reportSeverity, 
				       const char* message, ...)
{
  if (errorCode != 0)
  {
    va_list arg;
    va_start (arg, message);
    csString msg;
    msg.FormatV (message, arg);
    va_end (arg);

    Report (reportSeverity,
      "%s: %s (%d)", 
      msg.GetData (), GetErrorDescription (errorCode), errorCode);
    return true;
  }
  else
  {
    return false;
  }
}

bool csFreeType2Server::FreetypeError (int errorCode, const char* message, 
				       ...)
{
  if (errorCode != 0)
  {
    va_list arg;
    va_start (arg, message);
    csString msg;
    msg.FormatV (message, arg);
    va_end (arg);

    Report (CS_REPORTER_SEVERITY_WARNING,
      "%s: %s (%d)", 
      msg.GetData (), GetErrorDescription (errorCode), errorCode);
    return true;
  }
  else
  {
    return false;
  }
}

bool csFreeType2Server::Initialize (iObjectRegistry *object_reg)
{
  csFreeType2Server::object_reg = object_reg;

  freetype_inited = !FreetypeError (FT_Init_FreeType (&library),
    CS_REPORTER_SEVERITY_ERROR,
    "Could not create a FreeType engine instance");

  if (!freetype_inited)
  {
    return false;
  }

  VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  ftconfig.AddConfig(object_reg, "config/freetype.cfg");

  defaultSize = ftconfig->GetInt ("Freetype2.Settings.Size", 10);

  fontset = ftconfig->GetStr ("Freetype2.Settings.FontSet", 0);

  return true;
}

csPtr<iFont> csFreeType2Server::LoadFont (const char *filename)
{
  // First of all look for an alias in config file
  if (ftconfig && fontset)
  {
    csString Keyname;
    Keyname << "Freetype2.Fonts.";
    if (fontset) Keyname << fontset << '.';
    Keyname << filename;
    const char *s = ftconfig->GetStr (Keyname, 0);
    if (s) filename = s;
  }

  // see if we already loaded that font
  int idx = fonts.FindKey ((void*)filename, fonts.CompareKey);
  if (idx >= 0)
  {
    csFreeType2Font *font = fonts.Get (idx);
    font->IncRef ();
    return font;
  }

  // not yet loaded, so do it now
  csFreeType2Font *font = new csFreeType2Font (filename, this);
  if (!font->Load (VFS))
  {
    delete font;
    return 0;
  }
  fonts.Push (font);
  return font;
}

iFont *csFreeType2Server::GetFont (int iIndex)
{
  if (iIndex >= 0 && iIndex < fonts.Length ())
  {
    csFreeType2Font *font = fonts.Get (iIndex);
    if (font) return font;
  }
  return 0;
}

//-------------------------------------------// A FreeType font object //----//

SCF_IMPLEMENT_IBASE (csFreeType2Font)
  SCF_IMPLEMENTS_INTERFACE (iFont)
SCF_IMPLEMENT_IBASE_END

csFreeType2Font::csFreeType2Font (const char *filename, 
				  csFreeType2Server* server) : 
  DeleteCallbacks (4, 4)
{
  SCF_CONSTRUCT_IBASE (0);
  name = csStrNew (filename);
  face = 0;
  fontdata = 0;
  csFreeType2Font::server = server;
}

csFreeType2Font::~csFreeType2Font ()
{
  for (int i = DeleteCallbacks.Length () - 1; i >= 0; i--)
  {
    iFontDeleteNotify* delnot = DeleteCallbacks[i];
    delnot->BeforeDelete (this);
  }
  if (face)
  {
    FT_Done_Face (face);
  }
  fontdata = 0;
  delete [] name;
  SCF_DESTRUCT_IBASE();
}

void csFreeType2Font::SetSize (int iSize)
{
  fontSize = iSize;

  if (server->FreetypeError (FT_Set_Char_Size (face, 0, 
    iSize * 64, 96, 96), 
    "Could not set character dimensions for %s",
    name))
  {
    server->FreetypeError (FT_Set_Pixel_Sizes (face, 0, 
      iSize), 
      "Could not set character pixel dimensions for %s",
      name);
  }
}

int csFreeType2Font::GetSize ()
{
  return fontSize;
}

void csFreeType2Font::GetMaxSize (int &oW, int &oH)
{
  int maxrows = (face->size->metrics.height + 63) >> 6;
  oW = (face->size->metrics.max_advance + 63) >> 6;
  oH = maxrows;
}

bool csFreeType2Font::GetGlyphMetrics (utf32_char c, csGlyphMetrics& metrics)
{
  FT_UInt ci = (c == CS_FONT_DEFAULT_GLYPH) ? 0 : 
   FT_Get_Char_Index (face, (FT_ULong)c);
  if ((c != CS_FONT_DEFAULT_GLYPH) && (ci == 0)) return 0;

  if (server->FreetypeError (FT_Load_Glyph (face, ci, FT_LOAD_DEFAULT),
    "Could not load glyph %d for %s", ci, name))
  {
    return false;
  }
  
  metrics.advance = face->glyph->advance.x >> 6;

  return true;
}

csPtr<iDataBuffer> csFreeType2Font::GetGlyphBitmap (utf32_char c,
						    csBitmapMetrics& metrics)
{
  FT_UInt ci = (c == CS_FONT_DEFAULT_GLYPH) ? 0 : 
    FT_Get_Char_Index (face, (FT_ULong)c);
  if ((c != CS_FONT_DEFAULT_GLYPH) && (ci == 0)) return 0;

  if (server->FreetypeError (FT_Load_Glyph (face, ci, 
    FT_LOAD_RENDER | FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO),
    "Could not load glyph %d for %s", ci, name))
  {
    return 0;
  }

  int stride = (face->glyph->bitmap.width + 7) / 8;
  int maxrows = (face->size->metrics.height + 63) >> 6;
  int bitmapsize = maxrows*stride;
  uint8* bitmap = new uint8 [bitmapsize];
  memset (bitmap, 0, bitmapsize);

  int descend = (-face->size->metrics.descender + 63) >> 6;;

  int startrow = maxrows - (descend + face->glyph->bitmap_top);

  int endrow = startrow + face->glyph->bitmap.rows;

  if (startrow < 0) startrow = 0;
  if (endrow > maxrows) endrow = maxrows;

  int n, i;
  for (n = 0, i = startrow; i < endrow; i++, n++)
    memcpy (bitmap + stride*i, 
            face->glyph->bitmap.buffer + n * face->glyph->bitmap.pitch,
            MIN(stride, face->glyph->bitmap.pitch));

  metrics.width = face->glyph->bitmap.width;
  metrics.height = maxrows;
  metrics.left = face->glyph->bitmap_left;
  metrics.top = maxrows - descend;

  return (csPtr<iDataBuffer> (new csDataBuffer ((char*)bitmap, bitmapsize, 
    true)));
}

csPtr<iDataBuffer> csFreeType2Font::GetGlyphAlphaBitmap (utf32_char c, 
						 csBitmapMetrics& metrics)
{
  FT_UInt ci = (c == CS_FONT_DEFAULT_GLYPH) ? 0 : 
    FT_Get_Char_Index (face, (FT_ULong)c);
  if ((c != CS_FONT_DEFAULT_GLYPH) && (ci == 0)) return 0;

  if (server->FreetypeError (FT_Load_Glyph (face, ci, 
    FT_LOAD_RENDER | FT_RENDER_MODE_NORMAL),
    "Could not load glyph %d for %s", ci, name))
  {
    return 0;
  }

  int stride = face->glyph->bitmap.width;
  int maxrows = (face->size->metrics.height + 63) >> 6;
  int bitmapsize = maxrows * stride;
  // malloc at least 1 byte (malloc 0 bytes is undefined).
  uint8* bitmap = (bitmapsize > 0) ? new uint8 [bitmapsize] : new uint8[1];
  memset (bitmap, 0, bitmapsize);

  int descend = (-face->size->metrics.descender + 63) >> 6;

  int startrow = maxrows - (descend + face->glyph->bitmap_top);

  int endrow = startrow + face->glyph->bitmap.rows;

  if (startrow < 0) startrow = 0;
  if (endrow > maxrows) endrow = maxrows;

  int n, i;
  for (n = 0, i = startrow; i < endrow; i++, n++)
    memcpy (bitmap + stride*i, 
            face->glyph->bitmap.buffer + n * face->glyph->bitmap.pitch,
            MIN(stride, face->glyph->bitmap.pitch));

  metrics.width = face->glyph->bitmap.width;
  metrics.height = maxrows;
  metrics.left = face->glyph->bitmap_left;
  metrics.top = maxrows - descend;

  return (csPtr<iDataBuffer> (new csDataBuffer ((char*)bitmap, bitmapsize, 
    true)));
}

void csFreeType2Font::GetDimensions (const char *text, int &oW, int &oH, int &desc)
{
  if (!text)
  {
    oW = oH = desc = 0;
    return;
  }

  int defW = 0;
  if (!server->FreetypeError (FT_Load_Glyph (face, 0, 
    FT_LOAD_DEFAULT),
    "Could not load glyph %d for %s", 0, name))
  {
    defW = (face->glyph->advance.x >> 6);
  }

  oW = 0; 
  oH = (face->size->metrics.height + 63) >> 6; 
  desc = (-face->size->metrics.descender + 63) >> 6;
  int textLen = strlen ((char*)text);
  while (textLen > 0)
  {
    utf32_char glyph;
    int skip = csUnicodeTransform::UTF8Decode ((utf8_char*)text, textLen, glyph, 0);
    if (skip == 0) break;

    text += skip;
    textLen -= skip;

    FT_UInt ci = FT_Get_Char_Index (face, (FT_ULong)glyph);

    if (!server->FreetypeError (FT_Load_Glyph (face, ci, 
      FT_LOAD_DEFAULT),
      "Could not load glyph %d for %s", ci, name))
    {
      oW += (face->glyph->advance.x >> 6);
    }
    else
    {
      oW += defW;
    }
  }
}

void csFreeType2Font::GetDimensions (const char *text, int &oW, int &oH)
{
  int dummy;
  GetDimensions (text, oW, oH, dummy);
}

int csFreeType2Font::GetLength (const char *text, int maxwidth)
{
  // @@@ Improve.
  if (!text)
    return 0;

  int defW = 0;
  if (!server->FreetypeError (FT_Load_Glyph (face, 0, 
    FT_LOAD_DEFAULT),
    "Could not load glyph %d for %s", 0, name))
  {
    defW = (face->glyph->advance.x >> 6);
  }

  int count = 0;
  int textLen = strlen ((char*)text);
  while (textLen > 0)
  {
    utf32_char glyph;
    int skip = csUnicodeTransform::UTF8Decode ((utf8_char*)text, textLen, glyph, 0);
    if (skip == 0) break;

    text += skip;
    textLen -= skip;

    FT_UInt ci = FT_Get_Char_Index (face, (FT_ULong)glyph);

    int glyphW = defW;
    if (!server->FreetypeError (FT_Load_Glyph (face, ci, 
      FT_LOAD_DEFAULT),
      "Could not load glyph %d for %s", ci, name))
    {
      glyphW = face->glyph->advance.x >> 6;
    }
    if (maxwidth < glyphW)
      break;
    count += skip;
    maxwidth -= glyphW;
  }
  return count;
}

bool csFreeType2Font::Load (iVFS *pVFS)
{
  csRef<iFile> file (pVFS->Open (name, VFS_FILE_READ));
  if (file)
  {
    size_t size = file->GetSize ();
    if (size)
    {
      fontdata = file->GetAllData ();

      // @@@ kludge: don't report error on CSF files(or?)
      if ((size >= 3) && (strncmp (fontdata->GetData (), "CSF", 3) == 0))
      {
	fontdata = 0;
	return false;
      }
      if (server->FreetypeError (FT_New_Memory_Face (server->library, 
	(FT_Byte*)fontdata->GetData (), size, 0, &face),
	"Font file %s could not be loaded", name))
      {
	fontdata = 0;
        return false;
      }
    }
    else
    {
      server->Report (CS_REPORTER_SEVERITY_WARNING,
                      "Could not determine filesize for fontfile %s!", name);
      return false;
    }
  }
  else
  {
    server->Report (CS_REPORTER_SEVERITY_WARNING,
                    "Could not open fontfile %s!", name);
    return false;
  }

  // we do not change the default values of the new instance

  // Attempt to select an Unicode charmap
  if (server->FreetypeError (FT_Select_Charmap (face, FT_ENCODING_UNICODE),
    "Could not select an Unicode charmap for %s", name))
  {
    return false;
  }

  SetSize (server->defaultSize);

  return true;
}

void csFreeType2Font::AddDeleteCallback (iFontDeleteNotify* func)
{
  DeleteCallbacks.Push (func);
}

bool csFreeType2Font::RemoveDeleteCallback (iFontDeleteNotify* func)
{
  int i;
  for (i = DeleteCallbacks.Length () - 1; i >= 0; i--)
  {
    iFontDeleteNotify* delnot = DeleteCallbacks[i];
    if (delnot == func)
    {
      DeleteCallbacks.DeleteIndex (i);
      return true;
    }
  }
  return false;
}

int csFreeType2Font::GetAscent ()
{
  return (face->size->metrics.ascender + 63) >> 6;
}

int csFreeType2Font::GetDescent ()
{
  return -((face->size->metrics.descender + 63) >> 6);
}

bool csFreeType2Font::HasGlyph (utf32_char c)
{
  if (c == CS_FONT_DEFAULT_GLYPH) return true;

  return (FT_Get_Char_Index (face, (FT_ULong)c) != 0);
}

