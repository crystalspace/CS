/*
    Copyright (C) 2000 by Jerry A. Segler, Jr. Based on csFont
    Major improvements by Andrew Zabolotny, <bit@eltech.ru>
    More enhancements 2003 by Frank Richter

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
#include "csqint.h"
#include "csutil/csuctransform.h"
#include "csutil/util.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/databuff.h"
#include "ivaria/reporter.h"
#include "fontplex.h"

CS_IMPLEMENT_PLUGIN

//---------------------------------------------------------------------------

csFontLoadOrderEntry::csFontLoadOrderEntry (iFontServer* server, 
					    const char* fontName, float scale)
{
  csFontLoadOrderEntry::server = server;
  csFontLoadOrderEntry::scale = scale;
  csFontLoadOrderEntry::fontName = csStrNew (fontName);
  loaded = false;
}

csFontLoadOrderEntry::csFontLoadOrderEntry (const csFontLoadOrderEntry& other)
{
  fontName = csStrNew (other.fontName);
  server = other.server;
  loaded = other.loaded;
  font = other.font;
  scale = other.scale;
}

csFontLoadOrderEntry::~csFontLoadOrderEntry ()
{
  delete[] fontName;
}

bool csFontLoadOrderEntry::operator== (const csFontLoadOrderEntry& e2)
{
  return ((strcmp (e2.fontName, fontName) == 0) && (e2.server == server));
}

iFont* csFontLoadOrderEntry::GetFont (csFontPlexer* parent)
{
  if (!loaded)
  {
    font = server->LoadFont (fontName,
      csQround ((float)parent->size * scale));
    loaded = true;
  }
  return font;
}

//---------------------------------------------------------------------------

void csFontLoaderOrder::AppendSmart (const csFontLoaderOrder& other)
{
  size_t i;
  for (i = 0; i < other.Length (); i++)
  {
    PushSmart (other[i]);
  }
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csFontServerMultiplexor)
  SCF_IMPLEMENTS_INTERFACE (iFontServer)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFontServerMultiplexor::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csFontServerMultiplexor)

csFontServerMultiplexor::FontServerMapEntry::FontServerMapEntry (
  const char* name, iFontServer* server)
{
  FontServerMapEntry::name = csStrNew (name);
  FontServerMapEntry::server = server;
}

csFontServerMultiplexor::FontServerMapEntry::FontServerMapEntry (
  const FontServerMapEntry& source)
{
  name = csStrNew (source.name);
  server = source.server;
}

csFontServerMultiplexor::FontServerMapEntry::~FontServerMapEntry ()
{
  delete[] name;
}

csFontServerMultiplexor::csFontServerMultiplexor (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFontServerMultiplexor::~csFontServerMultiplexor ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csFontServerMultiplexor::Initialize (iObjectRegistry *object_reg)
{
  csFontServerMultiplexor::object_reg = object_reg;

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (object_reg,
    iPluginManager);

  config.AddConfig(object_reg, "config/fontplex.cfg");
  fontset = config->GetStr ("Fontplex.Settings.FontSet", 0);
  csString mapKey;
  mapKey << "Fontplex.ServerMaps.";
  if (fontset) mapKey << fontset << '.';
  
  csRef<iConfigIterator> mapEnum (config->Enumerate (mapKey));
  while (mapEnum->Next ())
  {
    const char* pluginName = mapEnum->GetStr ();
    csRef<iFontServer> fs = CS_QUERY_PLUGIN_CLASS (plugin_mgr, pluginName, 
      iFontServer);

    if (fs)
    {
      const char* name = mapEnum->GetKey (true);

      FontServerMapEntry entry (name, fs);
      fontServerMap.Put (name, entry);
    }
  }

  // Query the auxiliary font servers in turn
  csString tag;
  int idx;
  int errorcount = 0;
  for (idx = 1; ; idx++)
  {
    tag.Format ("iFontServer.%d", idx);
    csRef<iBase> b (CS_QUERY_REGISTRY_TAG (object_reg, tag));
    if (!b) 
    {
      // in cases where just one entry in the server list doesn't work
      // but later ones would those are loaded, too
      errorcount++;
      if (errorcount == 2) break;
    }
    else
    {
      errorcount = 0;	
      csRef<iFontServer> fs (SCF_QUERY_INTERFACE (b, iFontServer));
      fontservers.Push (fs);
    }
  }
  if (!fontservers.Length ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.font.fontplex",
        "Font server multiplexor: WARNING, no slave font servers found!");
  }

  csString fallbackKey;
  fallbackKey << "Fontplex.Fonts.";
  if (fontset) fallbackKey << fontset << '.';
  fallbackKey << "*Fallback";

  ParseFontLoaderOrder (fallbackOrder, config->GetStr (fallbackKey, 0));

  return true;
}

void csFontServerMultiplexor::NotifyDelete (csFontPlexer* font, 
					    char* fontid)
{
  loadedFonts.Delete (fontid, font);
  delete[] fontid;
}

csPtr<iFont> csFontServerMultiplexor::LoadFont (const char *filename, 
						int size)
{
  csString fontid;
  fontid.Format ("%d:%s", size, filename);
  iFont* font = loadedFonts.Get ((const char*)fontid, 0);
  if (font != 0)
  {
    return csPtr<iFont> (csRef<iFont> (font));
  }

  csFontLoaderOrder* order = new csFontLoaderOrder;

  csString substKey;
  substKey << "Fontplex.Fonts.";
  if (fontset) substKey << fontset << '.';
  substKey << filename;

  const char* orderStr = config->GetStr (substKey, 0);
  if (orderStr)
  {
    ParseFontLoaderOrder (*order, orderStr);
  }
  else
  {
    size_t i;
    for (i = 0; i < fontservers.Length (); i++)
    {
      order->PushSmart (csFontLoadOrderEntry (fontservers[i], filename, 1.0f));
    }
  }

  order->AppendSmart (fallbackOrder);

  // The first font that could be loaded is the "primary" font.
  iFont* primary = 0;
  size_t i;
  for (i = 0; i < order->Length (); i++)
  {
    primary = (*order)[i].font = 
      (*order)[i].server->LoadFont ((*order)[i].fontName, size);
    (*order)[i].loaded = true;
    if (primary != 0) break;
  }
  if (primary == 0)
  {
    // Not a single font in the substitution list could be loaded?...
    delete order;
    return 0;
  }
  else
  {
    char* newFontId = csStrNew (fontid);
    iFont* newFont = new csFontPlexer (this, newFontId, primary, size, order);
    loadedFonts.Put (newFontId, newFont);
    return (newFont);
  }
}

void csFontServerMultiplexor::ParseFontLoaderOrder (
  csFontLoaderOrder& order, const char* str)
{
  while ((str != 0) && (*str != 0))
  {
    const char* comma = strchr (str, ',');
    size_t partLen = (comma ? (comma - str) : strlen (str));
    CS_ALLOC_STACK_ARRAY (char, part, partLen + 1);
    strncpy (part, str, partLen);
    part[partLen] = 0;

    char* fontName = part;
    char* newserver = 0;
    char* fontScale = 0;
    char* p;

    if ((p = strchr (fontName, ':')))
    {
      newserver = fontName;
      *p = 0;
      fontName = p + 1;
    }
    if ((p = strrchr (fontName, '@')))
    {
      fontScale = p + 1;
      *p = 0;
    }

    float scale;
    if ((!fontScale) || (sscanf (fontScale, "%f", &scale) <= 0))
    {
      scale = 1.0f;
    }

    if (newserver)
    {
      csRef<iFontServer> fs = ResolveFontServer (newserver);
      if (fs)
      {
	order.PushSmart (csFontLoadOrderEntry (fs, fontName, scale));
      }
    }
    size_t i;
    for (i = 0; i < fontservers.Length (); i++)
    {
      order.PushSmart (csFontLoadOrderEntry (fontservers[i], fontName, scale));
    }

    str = comma ? comma + 1 : 0;
  }
}

csPtr<iFontServer> csFontServerMultiplexor::ResolveFontServer (const char* name)
{
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (object_reg,
    iPluginManager);

  csRef<iFontServer> fs;
  if (iSCF::SCF->ClassRegistered (name))
  {
    fs = CS_QUERY_PLUGIN_CLASS (plugin_mgr, name, iFontServer);
  }
  if (fs == 0)
  {
    csHash<FontServerMapEntry, csStrKey, 
    csConstCharHashKeyHandler>::Iterator it = 
      fontServerMap.GetIterator (name);

    while (it.HasNext ())
    {
      const FontServerMapEntry& entry = it.Next ();
      if (strcmp (entry.name, name) == 0)
      {
	fs = entry.server;
	break;
      }
    }
  }
  if (fs == 0)
  {
    csString plugName;
    plugName << "crystalspace.font.server." << name;

    fs = CS_QUERY_PLUGIN_CLASS (plugin_mgr, plugName, iFontServer);
    if (fs == 0)
    {
      fs = CS_LOAD_PLUGIN (plugin_mgr, plugName, iFontServer);
    }
  }
  if (fs) fs->IncRef ();
  return ((iFontServer*)fs);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csFontPlexer)
  SCF_IMPLEMENTS_INTERFACE (iFont)
SCF_IMPLEMENT_IBASE_END

csFontPlexer::csFontPlexer (csFontServerMultiplexor* parent,
			    char* fontid, iFont* primary, 
			    int size, csFontLoaderOrder* order)
{
  SCF_CONSTRUCT_IBASE(0);

  csFontPlexer::order = order;
  primaryFont = primary;
  csFontPlexer::size = size;
  csFontPlexer::parent = parent;
  csFontPlexer::fontid = fontid;
}

csFontPlexer::~csFontPlexer ()
{
  parent->NotifyDelete (this, fontid);

  delete order;

  size_t i = DeleteCallbacks.Length ();
  while (i-- > 0)
  {
    iFontDeleteNotify* delnot = DeleteCallbacks[i];
    delnot->BeforeDelete (this);
  }

  SCF_DESTRUCT_IBASE();
}

int csFontPlexer::GetSize ()
{
  return size;
}

void csFontPlexer::GetMaxSize (int &oW, int &oH)
{
  primaryFont->GetMaxSize (oW, oH);
}

bool csFontPlexer::GetGlyphMetrics (utf32_char c, csGlyphMetrics& metrics)
{
  iFont* font;
  size_t i;
  for (i = 0; i < order->Length (); i++)
  {
    if ((font = (*order)[i].GetFont (this)))
    {
      if (font->GetGlyphMetrics (c, metrics))
	return true;
    }
  }
  return false;
}

csPtr<iDataBuffer> csFontPlexer::GetGlyphBitmap (utf32_char c, 
  csBitmapMetrics& metrics)
{
  iFont* font;
  size_t i;
  for (i = 0; i < order->Length (); i++)
  {
    if ((font = (*order)[i].GetFont (this)))
    {
      csRef<iDataBuffer> db (font->GetGlyphBitmap (c, metrics));
      if (db != 0) 
      {
	db->IncRef ();
	return ((iDataBuffer*)db);
      }
    }
  }
  return 0;
}

csPtr<iDataBuffer> csFontPlexer::GetGlyphAlphaBitmap (utf32_char c,
  csBitmapMetrics& metrics)
{
  iFont* font;
  size_t i;
  for (i = 0; i < order->Length (); i++)
  {
    if ((font = (*order)[i].GetFont (this)))
    {
      if (font->HasGlyph (c))
      {
	return font->GetGlyphAlphaBitmap (c, metrics);
      }
    }
  }
  return 0;
}

void csFontPlexer::GetDimensions (const char *text, int &oW, int &oH, int &desc)
{
  csGlyphMetrics defMetrics;

  oW = oH = desc = 0;
  if (!GetGlyphMetrics (CS_FONT_DEFAULT_GLYPH, defMetrics))
  {
    return;
  }

  int dummy;
  primaryFont->GetMaxSize (dummy, oH);
  desc = primaryFont->GetDescent ();

  size_t textLen = strlen ((char*)text);
  while (textLen > 0)
  {
    utf32_char glyph;
    int skip = csUnicodeTransform::UTF8Decode ((utf8_char*)text, textLen, glyph, 0);
    if (skip == 0) break;

    text += skip;
    textLen -= skip;

    csGlyphMetrics gMetrics = defMetrics;
    iFont* font;
    size_t i;
    for (i = 0; i < order->Length (); i++)
    {
      if ((font = (*order)[i].GetFont (this)))
      {
	if (font->HasGlyph (glyph)) 
	{
	  font->GetGlyphMetrics (glyph, gMetrics);
	  int fW, fH, fDesc = font->GetDescent ();;
	  font->GetMaxSize (fW, fH);

	  oH = MAX (oH, fH);
	  desc = MAX (desc, fDesc);
	  break;
	}
      }
    }

    oW += gMetrics.advance;
  }

}

void csFontPlexer::GetDimensions (const char *text, int &oW, int &oH)
{
  int dummy;
  GetDimensions (text, oW, oH, dummy);
}

int csFontPlexer::GetLength (const char *text, int maxwidth)
{
  csGlyphMetrics defMetrics;

  if (!GetGlyphMetrics (CS_FONT_DEFAULT_GLYPH, defMetrics))
  {
    return 0;
  }

  int n = 0;
  size_t textLen = strlen ((char*)text);
  while (textLen > 0)
  {
    utf32_char glyph;
    int skip = csUnicodeTransform::UTF8Decode ((utf8_char*)text, textLen, glyph, 0);
    if (skip == 0) break;

    text += skip;
    textLen -= skip;

    csGlyphMetrics gMetrics = defMetrics;
    iFont* font;
    size_t i;
    for (i = 0; i < order->Length (); i++)
    {
      if ((font = (*order)[i].GetFont (this)))
      {
	if (font->HasGlyph (glyph)) 
	{
	  font->GetGlyphMetrics (glyph, gMetrics);
	  break;
	}
      }
    }

    if (maxwidth < gMetrics.advance)
      break;
    n += skip;
    maxwidth -= gMetrics.advance;
  }
  return n;
}

void csFontPlexer::AddDeleteCallback (iFontDeleteNotify* func)
{
  DeleteCallbacks.Push (func);
}

bool csFontPlexer::RemoveDeleteCallback (iFontDeleteNotify* func)
{
  size_t i = DeleteCallbacks.Length ();
  while (i-- > 0)
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

int csFontPlexer::GetDescent ()
{
  return primaryFont->GetDescent ();
}
 
int csFontPlexer::GetAscent ()
{
  return primaryFont->GetAscent ();
}
 
bool csFontPlexer::HasGlyph (utf32_char c)
{
  iFont* font;
  size_t i;
  for (i = 0; i < order->Length (); i++)
  {
    if ((font = (*order)[i].GetFont (this)))
    {
      if (font->HasGlyph (c)) return true;
    }
  }
  return false;
}

int csFontPlexer::GetTextHeight ()
{
	return primaryFont->GetTextHeight();
}

int csFontPlexer::GetUnderlinePosition ()
{
	return primaryFont->GetUnderlinePosition();
}

int csFontPlexer::GetUnderlineThickness ()
{
	return primaryFont->GetUnderlineThickness();
}
