/*
    Copyright (C) 2000 by Jerry A. Segler, Jr. Based on csFont
    Major improvements by Andrew Zabolotny, <bit@eltech.ru>

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

#include <stdlib.h>
#include "cssysdef.h"
#include "isys/system.h"
#include "csutil/csvector.h"
#include "fontplex.h"

IMPLEMENT_IBASE (csFontServerMultiplexor)
  IMPLEMENTS_INTERFACE (iFontServer)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csFontServerMultiplexor)

EXPORT_CLASS_TABLE (fontplex)
  EXPORT_CLASS_DEP (csFontServerMultiplexor, "crystalspace.font.server.multiplexor", 
    "Crystal Space font server multiplexor", "crystalspace.font.server.")
EXPORT_CLASS_TABLE_END

csFontServerMultiplexor::csFontServerMultiplexor (iBase *pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csFontServerMultiplexor::~csFontServerMultiplexor ()
{
}

bool csFontServerMultiplexor::Initialize (iSystem *System)
{
  // Query the auxiliary font servers in turn
  char funcid [20];
  for (int idx = 1; ; idx++)
  {
    sprintf (funcid, "FontServer.%d", idx);
    iFontServer *fs = QUERY_PLUGIN_ID (System, funcid, iFontServer);
    if (!fs) break;
    fontservers.Push (fs);
  }
  if (!fontservers.Length ())
    System->Printf (MSG_WARNING, "Font server multiplexor: WARNING, no slave font servers found!\n");
  return true;
}

iFont *csFontServerMultiplexor::LoadFont (const char *filename)
{
  for (int i = 0; i < fontservers.Length (); i++)
  {
    iFont *font = fontservers.Get (i)->LoadFont (filename);
    if (font) return font;
  }
  return NULL;
}

int csFontServerMultiplexor::GetNumFonts ()
{
  int count = 0;
  for (int i = 0; i < fontservers.Length (); i++)
    count += fontservers.Get (i)->GetNumFonts ();
  return count;
}

iFont *csFontServerMultiplexor::GetFont (int iIndex)
{
  for (int i = 0; i < fontservers.Length (); i++)
  {
    int count = fontservers.Get (i)->GetNumFonts ();
    if (iIndex < count)
      return fontservers.Get (iIndex)->GetFont (iIndex);
    iIndex -= count;
  }
  return NULL;
}
