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

#ifndef __FONTPLEX_H__
#define __FONTPLEX_H__

#include "ifontsrv.h"

struct csManagedFontDef
{
  iFontServer * pFontServer;
  int FontID;
};

class csVectorFonts : public csVector
{
public:
  csVectorFonts () : csVector (16, 16) {}
  virtual ~csVectorFonts () { DeleteAll (); }
  virtual bool FreeItem (csSome Item)
  { delete (csManagedFontDef *)Item; return true; }
  virtual int CompareKey (csSome Item, csConstSome Key, int) const
  { 
    if ( ((csManagedFontDef *)Item)->pFontServer < ((csManagedFontDef *)Key)->pFontServer ) return -1;
    if ( ((csManagedFontDef *)Item)->pFontServer > ((csManagedFontDef *)Key)->pFontServer ) return 1;
    if ( ((csManagedFontDef *)Item)->FontID < ((csManagedFontDef *)Key)->FontID ) return -1;
    if ( ((csManagedFontDef *)Item)->FontID > ((csManagedFontDef *)Key)->FontID ) return 1;
    return 0;
  }
  virtual int Compare (csSome Item, csConstSome Key, int) const
  { 
    if ( ((csManagedFontDef *)Item)->pFontServer < ((csManagedFontDef *)Key)->pFontServer ) return -1;
    if ( ((csManagedFontDef *)Item)->pFontServer > ((csManagedFontDef *)Key)->pFontServer ) return 1;
    if ( ((csManagedFontDef *)Item)->FontID < ((csManagedFontDef *)Key)->FontID ) return -1;
    if ( ((csManagedFontDef *)Item)->FontID > ((csManagedFontDef *)Key)->FontID ) return 1;
    return 0;
  }
};

/**
 * Default font server.
 */
class csDefaultFontManager : public iFontServer
{
  // The list of registered fonts
  csVectorFonts * FontList;

  iFontServer ** FontServers;
  // Number of font servers
  int FontServerCount;

public:
  DECLARE_IBASE;

  /// Create the plugin object
  csDefaultFontManager (iBase *pParent);

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /**
   * Load a font by name.
   * Returns a font id by which the font can be referenced further.
   * RETURN:
   * -1 ... loading failed
   * >=0 ... the font id
   */
  virtual int LoadFont (const char* name, const char* filename);

  /**
   * Set font property.
   * Return true if chage was successfull.  If not, the server returns false,
   * decides whats the next best possible thing to this propertym, returns
   * this new value in invalue and if wanted also applies this next best
   * thing.
   */
  virtual bool SetFontProperty (int fontId, CS_FONTPROPERTY propertyId,
    long& property, bool autoApply );

  /// Get a font property. Returns true if the property could be retrieved.
  virtual bool GetFontProperty (int fontId, CS_FONTPROPERTY propertyId,
    long& property);

  /**
   * Return a pointer to a bitmap containing a rendered character (by current
   * font).  NULL if error occured.
   */
  virtual unsigned char* GetGlyphBitmap (int fontId, unsigned char c, int &oW, int &oH);

  /// Return character size in pixels. Returns false if values could not be determined.
  virtual bool GetGlyphSize (int fontId, unsigned char c, int &oW, int &oH);

  /// Get maximal font height
  virtual int GetMaximumHeight (int fontId);

  /// Return minimal boundings of text.
  virtual void GetTextDimensions (int fontId, const char* text,
    int& width, int& height);

  /// Return font count.
  virtual int GetFontCount ()
  { return FontList->Length(); }

  virtual ~csDefaultFontManager();
};

#endif // __FONTPLEX_H__
