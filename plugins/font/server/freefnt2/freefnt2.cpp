/*
    Copyright (C) 2002 by Norman Krämer

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
#include "cssys/sysfunc.h"
#include "csutil/csstrvec.h"
#include "csutil/csstring.h"
#include "iutil/cfgfile.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "freefnt2.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csFreeType2Server)

SCF_EXPORT_CLASS_TABLE (freefnt2)
  SCF_EXPORT_CLASS (csFreeType2Server, "crystalspace.font.server.freetype2",
    "Crystal Space FreeType2 font server" )
SCF_EXPORT_CLASS_TABLE_END

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
  VFS = NULL;
}

csFreeType2Server::~csFreeType2Server ()
{
  if (VFS) VFS->DecRef ();
}

void csFreeType2Server::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.font.freefont2", msg, arg);
    rep->DecRef ();
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csFreeType2Server::Initialize (iObjectRegistry *object_reg)
{
  csFreeType2Server::object_reg = object_reg;

  if (FT_Init_FreeType (&library))
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Could not create a TrueType engine instance !");
    return false;
  }

  VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  ftconfig.AddConfig(object_reg, "config/freetype.cfg");

  defaultSize = ftconfig->GetInt ("Freetype2.Settings.Size", 10);
  platform_id = ftconfig->GetInt ("Freetype2.Settings.PlatformID", 3);
  encoding_id = ftconfig->GetInt ("Freetype2.Settings.EncodingID", 1);

  fontset = ftconfig->GetStr ("Freetype2.Settings.FontSet", NULL);

  csString s;
  s << fontset << '.';
  iConfigIterator *fontenum = ftconfig->Enumerate (s);
  while (fontenum->Next ())
    if (fontenum->GetKey (true) [0] == '*')
      LoadFont (fontenum->GetKey (true));
  fontenum->DecRef ();

  return true;
}

iFont *csFreeType2Server::LoadFont (const char *filename)
{
  // First of all look for an alias in config file
  if (ftconfig && fontset)
  {
    csString Keyname;
    Keyname << fontset << '.' << filename;
    const char *s = ftconfig->GetStr (Keyname, NULL);
    if (s) filename = s;
  }
  // Okay, now convert the VFS path into a real path
  iDataBuffer *db = VFS->GetRealPath (filename);
  if (db) filename = (const char *)db->GetData ();

  // see if we already loaded that font
  int idx = fonts.FindKey (filename);
  if (idx >= 0)
  {
    if (db) db->DecRef ();
    csFreeType2Font *font = fonts.Get (idx);
    font->IncRef ();
    return font;
  }

  // not yet loaded, so do it now
  csFreeType2Font *font = new csFreeType2Font (filename);
  if (db) db->DecRef ();
  if (!font->Load (this))
  {
    delete font;
    return NULL;
  }
  fonts.Put (font);
  font->IncRef ();
  return font;
}

iFont *csFreeType2Server::GetFont (int iIndex)
{
  if (iIndex >= 0 && iIndex < fonts.Length ())
  {
    csFreeType2Font *font = fonts.Get (iIndex);
    if (font) return font;
  }
  return NULL;
}

//-------------------------------------------// A FreeType font object //----//

SCF_IMPLEMENT_IBASE (csFreeType2Font)
  SCF_IMPLEMENTS_INTERFACE (iFont)
SCF_IMPLEMENT_IBASE_END

csFreeType2Font::csFreeType2Font (const char *filename) : DeleteCallbacks (4, 4)
{
  SCF_CONSTRUCT_IBASE (NULL);
  name = csStrNew (filename);
  face = NULL;
  current = NULL;
}

csFreeType2Font::~csFreeType2Font ()
{
  int i;
  for (i = DeleteCallbacks.Length () - 1; i >= 0; i--)
  {
    iFontDeleteNotify* delnot = (iFontDeleteNotify*)(DeleteCallbacks.Get (i));
    delnot->BeforeDelete (this);
    delnot->DecRef ();
  }
  if (face)
  {
    FT_Done_Face (face);
  }
}

void csFreeType2Font::SetSize (int iSize)
{
  CreateGlyphBitmaps (iSize);
  current = FindGlyphSet (iSize);
}

int csFreeType2Font::GetSize ()
{
  return current ? current->size : 0;
}

void csFreeType2Font::GetMaxSize (int &oW, int &oH)
{
  if (current)
  {
    oW = current->maxW;
    oH = current->maxH;
  }
  else
    oW = oH = 0;
}

bool csFreeType2Font::GetGlyphSize (uint8 c, int &oW, int &oH)
{
  if (!current || !current->glyphs[c].isOk) return false;
  
  oW = ((FT_BitmapGlyph)current->glyphs[c].glyph)->bitmap.width;
  oH = ((FT_BitmapGlyph)current->glyphs[c].glyph)->bitmap.rows;
  return true;
}

uint8 *csFreeType2Font::GetGlyphBitmap (uint8 c, int &oW, int &oH)
{
  if (!current || !current->glyphs[c].isOk) return NULL;
  oW = ((FT_BitmapGlyph)current->glyphs[c].glyph)->bitmap.width;
  oH = ((FT_BitmapGlyph)current->glyphs[c].glyph)->bitmap.rows;
  return ((FT_BitmapGlyph)current->glyphs[c].glyph)->bitmap.buffer;
}

void csFreeType2Font::GetDimensions (const char *text, int &oW, int &oH)
{
  if (!text || !current)
  {
    oW = oH = 0;
    return;
  }

  oW = 0; oH = current->maxH;
  while (*text)
  {
    oW += ((FT_BitmapGlyph)current->glyphs[*(const uint8 *)text].glyph)->bitmap.width;
    text++;
  }
}

int csFreeType2Font::GetLength (const char *text, int maxwidth)
{
  if (!text || !current)
    return 0;

  int count = 0, w = 0;
  while (*text)
  {
    w += ((FT_BitmapGlyph)current->glyphs[*(const uint8 *)text].glyph)->bitmap.width;
    if (w > maxwidth)
      break;
    text++; count++;
  }
  return count;
}

bool csFreeType2Font::Load (csFreeType2Server *server)
{
  int error;
  if ((error=FT_New_Face (server->library, name, 0, &face)))
  {
    server->Report (CS_REPORTER_SEVERITY_WARNING,
      "Font file %s could not be loaded - FreeType2 errorcode for FT_New_Face = %d!", name, error);
    return false;
  }

  // we do not change the default values of the new instance
  // ( 96 dpi, 10 pt. size, no trafo flags )

  // next we scan the charmap table if there is an encoding
  // that matches the requested platform and encoding ids
  FT_UShort i = 0;
  FT_CharMap charmap = NULL;
  while (i < face->num_charmaps)
  {
    FT_CharMap cm = face->charmaps[i];
    if (server->platform_id == cm->platform_id && server->encoding_id == cm->encoding_id)
    {
      charmap = cm;
      break;
    }
    i++;
  }

  if (!charmap)
  {
    // encoding scheme not found
    server->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Font %s does not contain encoding %d for platform %d.",
      name, server->encoding_id, server->platform_id);

    charmap = face->charmaps[0];
    server->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Will instead use encoding %d for platform %d.", charmap->encoding_id, charmap->platform_id);
  }

  if ((error=FT_Set_Charmap (face, charmap)))
  {
    server->Report (CS_REPORTER_SEVERITY_WARNING,
      "Could not set CharMap - FreeType2 errorcode for FT_Set_CharMap = %d!", error);
    return false;
  }
  // now we create the bitmap of all glyphs in the face
  return CreateGlyphBitmaps (server->defaultSize);
}

bool csFreeType2Font::CreateGlyphBitmaps (int size)
{
  if (FindGlyphSet (size))
    return true;

  if (FT_Set_Char_Size (face, size * 64, size * 64, 0, 0))
    return false;

  // Create the glyphset
  GlyphSet *glyphset;
  glyphset = new GlyphSet;
  glyphset->size = size;
  glyphset->maxW = glyphset->maxH = 0;
  memset (glyphset->glyphs, 0, sizeof (glyphset->glyphs));
  cache.Push (glyphset);

  for (FT_UShort iso_char = 0; iso_char < 256; iso_char++)
  {
    FT_Glyph &glyph = glyphset->glyphs[iso_char].glyph;
    if (FT_Load_Char (face, iso_char, FT_LOAD_RENDER) || FT_Get_Glyph (face->glyph, &glyph))
      continue;

    if (glyph->format != ft_glyph_format_bitmap)
    {
      if (FT_Glyph_To_Bitmap (&glyph, ft_render_mode_mono, 0, 1)) continue;
      glyphset->glyphs[iso_char].isOk = true;
    }
  }

  current = glyphset;
  return true;
}

void csFreeType2Font::AddDeleteCallback (iFontDeleteNotify* func)
{
  DeleteCallbacks.Push ((void *)func);
  func->IncRef ();
}

bool csFreeType2Font::RemoveDeleteCallback (iFontDeleteNotify* func)
{
  int i;
  for (i = DeleteCallbacks.Length () - 1; i >= 0; i--)
  {
    iFontDeleteNotify* delnot = (iFontDeleteNotify*)(DeleteCallbacks.Get (i));
    if (delnot == func)
    {
      DeleteCallbacks.Delete (i);
      func->DecRef ();
      return true;
    }
  }
  return false;
}
