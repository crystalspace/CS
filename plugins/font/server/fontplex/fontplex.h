/*
    Copyright (C) 2000 by Jerry A. Segler, Jr.
    Based on csFont
    Copyright (C) 2000 by Norman Kramer
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

#ifndef __CS_FONTPLEX_H__
#define __CS_FONTPLEX_H__

#include "ivideo/fontserv.h"
#include "csutil/weakref.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/refarr.h"
#include "csutil/cfgacc.h"
#include "csutil/hash.h"
#include "csutil/hashhandlers.h"

class csFontServerMultiplexor;
class csFontPlexer;

struct csFontLoadOrderEntry
{
  char* fontName;
  csRef<iFontServer> server;

  bool loaded;
  csRef<iFont> font;
  float scale;

  csFontLoadOrderEntry (iFontServer* server, const char* fontName,
    float scale);
  csFontLoadOrderEntry (const csFontLoadOrderEntry& other);
  ~csFontLoadOrderEntry ();

  bool operator== (const csFontLoadOrderEntry& e2);

  iFont* GetFont (csFontPlexer* parent);
};

class csFontLoaderOrder : public csArray<csFontLoadOrderEntry>
{
public:
  void AppendSmart (const csFontLoaderOrder& other);
};

class csFontPlexer : public iFont
{
private:
  friend struct csFontLoadOrderEntry;

  csRef<csFontServerMultiplexor> parent;
  char* fontid;
  int size;
  iFont* primaryFont;

  csFontLoaderOrder* order;
  csRefArray<iFontDeleteNotify> DeleteCallbacks;
public:
  SCF_DECLARE_IBASE;

  csFontPlexer (csFontServerMultiplexor* parent, 
    char* fontid, iFont* primary, 
    float size, csFontLoaderOrder* order);
  virtual ~csFontPlexer ();

  virtual float GetSize ();
  virtual void GetMaxSize (int &oW, int &oH);
  virtual bool GetGlyphMetrics (utf32_char c, csGlyphMetrics& metrics);

  virtual csPtr<iDataBuffer> GetGlyphBitmap (utf32_char c, 
    csBitmapMetrics& metrics);
  virtual csPtr<iDataBuffer> GetGlyphAlphaBitmap (utf32_char c,
    csBitmapMetrics& metrics);

  virtual void GetDimensions (const char *text, int &oW, int &oH, int &desc);
  virtual void GetDimensions (const char *text, int &oW, int &oH);
  virtual int GetLength (const char *text, int maxwidth);

  virtual void AddDeleteCallback (iFontDeleteNotify* func);
  virtual bool RemoveDeleteCallback (iFontDeleteNotify* func);

  virtual int GetDescent (); 
  virtual int GetAscent (); 
  virtual bool HasGlyph (utf32_char c); 

  virtual int GetTextHeight ();
  virtual int GetUnderlinePosition ();
  virtual int GetUnderlineThickness ();

};

/**
 * Font server multiplexer plug-in.
 * This plug-in takes all the other font servers and hides them behind
 * itself. Then when the application requests some font, all servers are
 * queried in turn; the first one that is able to load the specified font
 * wins. To use the plug-in you should assign the "FontServer" functionality
 * identifier to this server, and "FontServer.1", "FontServer.2" and so on
 * to auxiliary font servers. Example extract from config file:
 * <code>
 * ...
 * System.Plugins.iFontServer = crystalspace.font.server.multiplexer
 * System.Plugins.iFontServer.1 = crystalspace.font.server.freetype2
 * System.Plugins.iFontServer.2 = crystalspace.font.server.default
 * ...
 * </code>
 */
class csFontServerMultiplexor : public iFontServer
{
private:
  iObjectRegistry* object_reg;
  csRefArray<iFontServer> fontservers;

  csConfigAccess config;
  const char *fontset;

  struct FontServerMapEntry
  {
    csRef<iFontServer> server;
    char* name;

    FontServerMapEntry (const char* name, iFontServer* server);
    FontServerMapEntry (const FontServerMapEntry& source);
    ~FontServerMapEntry ();
  };
  csHash<FontServerMapEntry, csStrKey, 
    csConstCharHashKeyHandler> fontServerMap;
  csHash<iFont*, const char*, csConstCharHashKeyHandler> loadedFonts;

  csFontLoaderOrder fallbackOrder;

  void ParseFontLoaderOrder (csFontLoaderOrder& order, 
    const char* str);
  csPtr<iFontServer> ResolveFontServer (const char* name);
public:
  void NotifyDelete (csFontPlexer* font, char* fontid);

  SCF_DECLARE_IBASE;

  /// Create the plugin object
  csFontServerMultiplexor (iBase *pParent);
  /// Destructor: nothing to do
  virtual ~csFontServerMultiplexor ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /**
   * Load a font by name.
   * Returns a new iFont object or 0 on failure.
   */
  virtual csPtr<iFont> LoadFont (const char *filename, float size = 10.0f);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFontServerMultiplexor);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __CS_FONTPLEX_H__
