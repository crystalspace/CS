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
#include <limits.h>
#include "csqint.h"
#include "csutil/csuctransform.h"
#include "csutil/util.h"
#include "csutil/set.h"
#include "iutil/databuff.h"
#include "iutil/eventh.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"

#include "fontplex.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(FontPlex)
{

//---------------------------------------------------------------------------

csFontLoadOrderEntry::csFontLoadOrderEntry (csRefArray<iFontServer> servers, 
					    const char* fontName, float scale,
                                            bool fallback) : servers (servers),
                                              fallback (fallback), loaded (false), 
                                              scale (scale)
{
  csFontLoadOrderEntry::fontName = fontName;
}

csFontLoadOrderEntry::csFontLoadOrderEntry (const csFontLoadOrderEntry& other)
{
  fontName = other.fontName;
  servers = other.servers;
  loaded = other.loaded;
  font = other.font;
  scale = other.scale;
  fallback = other.fallback;
}

csFontLoadOrderEntry::~csFontLoadOrderEntry ()
{
}

bool csFontLoadOrderEntry::operator== (const csFontLoadOrderEntry& e2)
{
  return ((strcmp (e2.fontName, fontName) == 0) && (e2.servers == servers));
}

iFont* csFontLoadOrderEntry::GetFont (csFontPlexer* parent)
{
  if (!loaded)
  {
    loaded = true;
    for (size_t i = 0; i < servers.GetSize(); i++)
    {
      font = servers[i]->LoadFont (fontName, parent->size * scale);
      if (font.IsValid()) break;
    }
    if (!font.IsValid())
      parent->parent->ReportFontNotFound (fallback, fontName);
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

SCF_IMPLEMENT_FACTORY (csFontServerMultiplexer)

csFontServerMultiplexer::FontServerMapEntry::FontServerMapEntry (
  const char* name, iFontServer* server)
{
  FontServerMapEntry::name = name;
  FontServerMapEntry::server = server;
}

csFontServerMultiplexer::FontServerMapEntry::FontServerMapEntry (
  const FontServerMapEntry& source)
{
  name = source.name;
  server = source.server;
}

csFontServerMultiplexer::FontServerMapEntry::~FontServerMapEntry ()
{
}

csFontServerMultiplexer::csFontServerMultiplexer (iBase *pParent) :
  scfImplementationType (this, pParent), emitErrors (true)
{
}

csFontServerMultiplexer::~csFontServerMultiplexer ()
{
}

#define FSCLASSPREFIX "crystalspace.font.server."
#define FONTPLEX_CLASSNAME FSCLASSPREFIX "multiplexer"
#define CSFONT_CLASSNAME FSCLASSPREFIX "default"

bool csFontServerMultiplexer::Initialize (iObjectRegistry *object_reg)
{
  csFontServerMultiplexer::object_reg = object_reg;

  config.AddConfig(object_reg, "config/fontplex.cfg");
  fontset = config->GetStr ("Fontplex.Settings.FontSet", 0);
  csString mapKey;
  mapKey << "Fontplex.ServerMaps.";
  if (fontset) mapKey << fontset << '.';
  
  csRef<iConfigIterator> mapEnum (config->Enumerate (mapKey));
  while (mapEnum->Next ())
  {
    const char* pluginName = mapEnum->GetStr ();
    csRef<iFontServer> fs = csLoadPluginCheck<iFontServer> (object_reg, 
      pluginName);

    if (fs)
    {
      fs->SetWarnOnError (false);
      const char* name = mapEnum->GetKey (true);

      FontServerMapEntry entry (name, fs);
      fontServerMap.Put (name, entry);
    }
  }

  // Query the auxiliary font servers in turn
  csSet<csString> usedPlugins;
  usedPlugins.Add (FONTPLEX_CLASSNAME);
  csString tag;
  int idx;
  int errorcount = 0;
  for (idx = 1; ; idx++)
  {
    tag.Format ("iFontServer.%d", idx);
    csRef<iBase> b (csQueryRegistryTag (object_reg, tag));
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
      csRef<iFactory> fact = scfQueryInterface<iFactory> (b);
      const char* classId = fact->QueryClassID ();
      if (usedPlugins.Contains (classId)) continue;
      csRef<iFontServer> fs (scfQueryInterface<iFontServer> (b));
      if (!fs.IsValid()) continue;
      fs->SetWarnOnError (false);
      fontservers.Push (fs);
    }
  }

  // Small hack: forcibly add csfont as the last server
  usedPlugins.Add (CSFONT_CLASSNAME);
  csRef<iStringArray> scfClassList = 
    iSCF::SCF->QueryClassList (FSCLASSPREFIX);
  for (size_t i = 0; i < scfClassList->GetSize(); i++)
  {
    const char* scfClass = scfClassList->Get (i);
    if (!usedPlugins.Contains (scfClass))
    {
      csRef<iFontServer> fs = csLoadPluginCheck<iFontServer> (object_reg, 
        scfClass);
      if (fs.IsValid()) fontservers.Push (fs);
    }
  }
  {
    csRef<iFontServer> fs = csLoadPluginCheck<iFontServer> (object_reg, 
      CSFONT_CLASSNAME);
    if (fs.IsValid()) fontservers.Push (fs);
  }

  if (!fontservers.Length ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.font.fontplex",
        "Font server multiplexer: WARNING, no slave font servers found!");
  }

  csString fallbackKey;
  fallbackKey << "Fontplex.Fonts.";
  if (fontset) fallbackKey << fontset << '.';
  fallbackKey << "*Fallback";

  ParseFontLoaderOrder (fallbackOrder, config->GetStr (fallbackKey, 0), true);

  return true;
}

void csFontServerMultiplexer::ReportFontNotFound (bool fallback, const char* font)
{
  int oldSeverity = fontsNotFound.Get (font, INT_MAX);
  int newSeverity = fallback ? CS_REPORTER_SEVERITY_NOTIFY : GetErrorSeverity();
  if (oldSeverity > newSeverity)
  {
    csReport (object_reg,
      newSeverity,
      "crystalspace.font.server.multiplexer",
      "Could not load font \"%s\"",
      font);
    fontsNotFound.PutUnique (font, newSeverity);
  }
}

void csFontServerMultiplexer::NotifyDelete (csFontPlexer* font, 
					    const char* fontid)
{
  bool result = loadedFonts.Delete (fontid, font);
  (void)result;
  CS_ASSERT_MSG ("NotifyDelete() for font not in 'loaded' list",
    result);
}

csPtr<iFont> csFontServerMultiplexer::LoadFont (const char *filename, 
						float size)
{
  csString fontid;
  fontid.Format ("%g:%s", size, filename);
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
    ParseFontLoaderOrder (*order, orderStr, false);
  }
  else
  {
    order->PushSmart (csFontLoadOrderEntry (fontservers, filename, 1.0f, false));
  }

  order->AppendSmart (fallbackOrder);

  csRef<csFontPlexer> newFont;
  newFont.AttachNew (new csFontPlexer (this, fontid, size, order));
  loadedFonts.Put (fontid, newFont);

  // The first font that could be loaded is the "primary" font.
  iFont* primary = 0;
  size_t i;
  bool wasFallback = false;
  for (i = 0; i < order->Length (); i++)
  {
    csFontLoadOrderEntry& orderEntry = (*order)[i];
    if ((i > 0) && !wasFallback && orderEntry.fallback)
    {
      // This means none of the non-fallback fonts loaded.
      // Worth a message to us...
      ReportFontNotFound (false, filename);
    }
    wasFallback = orderEntry.fallback;
    primary = orderEntry.font = orderEntry.GetFont (newFont);
    orderEntry.loaded = true;
    if (primary != 0) break;
  }
  if (primary == 0)
  {
    // Not a single font in the substitution list could be loaded?...
    // Note: order is deleted when newFont gets released.
    return 0;
  }
  else
  {
    newFont->primaryFont = primary;
    return csPtr<iFont> (newFont);
  }
}

void csFontServerMultiplexer::ParseFontLoaderOrder (
  csFontLoaderOrder& order, const char* str, bool fallback)
{
  while ((str != 0) && (*str != 0))
  {
    const char* comma = strchr (str, ',');
    csString fontName;
    size_t partLen = (comma ? (comma - str) : strlen (str));
    fontName.Append (str, partLen);

    csString newserver;
    csString fontScale;

    size_t pos;

    if ((pos = fontName.FindFirst (':')) != (size_t)-1)
    {
      fontName.SubString (newserver, 0, pos);
      fontName.DeleteAt (0, pos);
    }
    if ((pos = fontName.FindFirst ('@')) != (size_t)-1)
    {
      fontName.SubString (fontScale, pos + 1, fontName.Length() - pos - 1);
      fontName.DeleteAt (pos, fontName.Length() - pos);
    }

    float scale;
    if ((fontScale.IsEmpty()) || (sscanf (fontScale, "%f", &scale) <= 0))
    {
      scale = 1.0f;
    }

    if (!newserver.IsEmpty())
    {
      csRef<iFontServer> fs = ResolveFontServer (newserver);
      if (fs)
      {
        csRefArray<iFontServer> a (1, 1);
        a.Push (fs);
	order.PushSmart (csFontLoadOrderEntry (a, fontName, scale, fallback));
      }
    }
    order.PushSmart (csFontLoadOrderEntry (fontservers, fontName, scale, fallback));

    str = comma ? comma + 1 : 0;
  }
}

csPtr<iFontServer> csFontServerMultiplexer::ResolveFontServer (const char* name)
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
    csHash<FontServerMapEntry, csStrKey>::Iterator it = 
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

    fs = csLoadPluginCheck<iFontServer> (plugin_mgr, plugName);
  }
  return csPtr<iFontServer> (fs);
}

//---------------------------------------------------------------------------

csFontPlexer::csFontPlexer (csFontServerMultiplexer* parent,
			    const char* fontid, float size, 
                            csFontLoaderOrder* order) :
  scfImplementationType (this)
{
  csFontPlexer::order = order;
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
}

float csFontPlexer::GetSize ()
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

} 
CS_PLUGIN_NAMESPACE_END(FontPlex)
