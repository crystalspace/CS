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
#include "isys/plugin.h"
#include "iutil/objreg.h"
#include "csutil/csvector.h"
#include "fontplex.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csFontServerMultiplexor)
  SCF_IMPLEMENTS_INTERFACE (iFontServer)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFontServerMultiplexor::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csFontServerMultiplexor)

SCF_EXPORT_CLASS_TABLE (fontplex)
  SCF_EXPORT_CLASS_DEP (csFontServerMultiplexor,
    "crystalspace.font.server.multiplexor", 
    "Crystal Space font server multiplexor",
    "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

csFontServerMultiplexor::csFontServerMultiplexor (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csFontServerMultiplexor::~csFontServerMultiplexor ()
{
}

bool csFontServerMultiplexor::Initialize (iSystem *System)
{
  iObjectRegistry* object_reg = System->GetObjectRegistry ();
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  // Query the auxiliary font servers in turn
  char funcid [20];
  for (int idx = 1; ; idx++)
  {
    sprintf (funcid, "FontServer.%d", idx);
    iFontServer *fs = CS_QUERY_PLUGIN_ID (plugin_mgr, funcid, iFontServer);
    if (!fs) break;
    fontservers.Push (fs);
  }
  if (!fontservers.Length ())
    System->Printf (CS_MSG_WARNING, 
      "Font server multiplexor: WARNING, no slave font servers found!\n");
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

int csFontServerMultiplexor::GetFontCount ()
{
  int count = 0;
  for (int i = 0; i < fontservers.Length (); i++)
    count += fontservers.Get (i)->GetFontCount ();
  return count;
}

iFont *csFontServerMultiplexor::GetFont (int iIndex)
{
  for (int i = 0; i < fontservers.Length (); i++)
  {
    int count = fontservers.Get (i)->GetFontCount ();
    if (iIndex < count)
      return fontservers.Get (iIndex)->GetFont (iIndex);
    iIndex -= count;
  }
  return NULL;
}
