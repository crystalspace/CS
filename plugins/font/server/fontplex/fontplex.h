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

#include "ivideo/fontserv.h"
#include "isys/plugin.h"
#include "csutil/csvector.h"

/**
 * Font server multiplexor plug-in.
 * This plug-in takes all the other font servers and hides them behind
 * itself. Then when the application requests some font, all servers are
 * queried in turn; the first one that is able to load the specified font
 * wins. To use the plug-in you should assign the "FontServer" functionality
 * identifier to this server, and "FontServer.1", "FontServer.2" and so on
 * to auxiliary font servers. Example extract from config file:
 * <code>
 * [PlugIns]
 * ...
 * FontServer = crystalspace.font.server.multiplexor
 * FontServer.1 = crystalspace.font.server.default
 * FontServer.2 = crystalspace.font.server.freetype
 * ...
 * </code>
 */
class csFontServerMultiplexor : public iFontServer
{
  class csFontServerVector : public csVector
  {
  public:
    virtual ~csFontServerVector ()
    { DeleteAll (); }
    virtual bool FreeItem (csSome Item)
    { ((iFontServer *)Item)->DecRef (); return true; }
    iFontServer *Get (int idx)
    { return (iFontServer *)csVector::Get (idx); }
  } fontservers;

public:
  SCF_DECLARE_IBASE;

  /// Create the plugin object
  csFontServerMultiplexor (iBase *pParent);
  /// Destructor: nothing to do
  virtual ~csFontServerMultiplexor ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /**
   * Load a font by name.
   * Returns a new iFont object or NULL on failure.
   */
  virtual iFont *LoadFont (const char *filename);

  /**
   * Get number of loaded fonts.
   */
  virtual int GetFontCount ();

  /**
   * Get Nth loaded font or NULL.
   * You can query all loaded fonts with this method, by looping
   * through all indices starting from 0 until you get NULL.
   */
  virtual iFont *GetFont (int iIndex);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFontServerMultiplexor);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

#endif // __FONTPLEX_H__
