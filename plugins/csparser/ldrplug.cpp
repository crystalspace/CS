/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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
#include "csloader.h"
#include "imap/reader.h"

struct csLoaderPluginRec
{
  char* ShortName;
  char* ClassID;
  csRef<iComponent> Component;
  csRef<iLoaderPlugin> Plugin;
  csRef<iBinaryLoaderPlugin> BinPlugin;

  csLoaderPluginRec (const char* shortName,
	const char *classID,
	iComponent* component,
	iLoaderPlugin *plugin,
	iBinaryLoaderPlugin* binPlugin)
  {
    if (shortName) ShortName = csStrNew (shortName);
    else ShortName = NULL;
    ClassID = csStrNew (classID);
    Component = component;
    Plugin = plugin;
    BinPlugin = binPlugin;
  }

  ~csLoaderPluginRec ()
  {
    delete [] ShortName;
    delete [] ClassID;
  }
};

csLoader::csLoadedPluginVector::csLoadedPluginVector (
	int iLimit, int iThresh) : csVector (iLimit, iThresh)
{
  plugin_mgr = NULL;
}

csLoader::csLoadedPluginVector::~csLoadedPluginVector ()
{
  DeleteAll ();
}

bool csLoader::csLoadedPluginVector::FreeItem (csSome Item)
{
  csLoaderPluginRec *rec = (csLoaderPluginRec*)Item;
  if (rec->Component && plugin_mgr)
  {
    plugin_mgr->UnloadPlugin (rec->Component);
  }
  delete rec;
  return true;
}

csLoaderPluginRec* csLoader::csLoadedPluginVector::FindPluginRec (
	const char* name)
{
  int i;
  for (i=0 ; i<Length () ; i++)
  {
    csLoaderPluginRec* pl = (csLoaderPluginRec*)Get (i);
    if (pl->ShortName && !strcmp (name, pl->ShortName))
      return pl;
    if (!strcmp (name, pl->ClassID))
      return pl;
  }
  return NULL;
}

bool csLoader::csLoadedPluginVector::GetPluginFromRec (
	csLoaderPluginRec *rec, iLoaderPlugin*& plug,
	iBinaryLoaderPlugin*& binplug)
{
  if (!rec->Component)
  {
    rec->Component = CS_LOAD_PLUGIN (plugin_mgr,
    	rec->ClassID, iComponent);
    if (rec->Component)
    {
      rec->Plugin = SCF_QUERY_INTERFACE (rec->Component, iLoaderPlugin);
      rec->BinPlugin = SCF_QUERY_INTERFACE (rec->Component,
      	iBinaryLoaderPlugin);
    }
  }
  plug = rec->Plugin;
  binplug = rec->BinPlugin;
  return rec->Component != NULL;
}

bool csLoader::csLoadedPluginVector::FindPlugin (
	const char* Name, iLoaderPlugin*& plug,
	iBinaryLoaderPlugin*& binplug)
{
  // look if there is already a loading record for this plugin
  csLoaderPluginRec* pl = FindPluginRec (Name);
  if (pl)
  {
    return GetPluginFromRec (pl, plug, binplug);
  }

  // create a new loading record
  NewPlugin (NULL, Name);
  return GetPluginFromRec ((csLoaderPluginRec*)Get(Length()-1),
  	plug, binplug);
}

void csLoader::csLoadedPluginVector::NewPlugin
	(const char *ShortName, const char *ClassID)
{
  Push (new csLoaderPluginRec (ShortName, ClassID, NULL, NULL, NULL));
}

