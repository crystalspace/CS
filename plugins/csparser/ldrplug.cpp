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
#include "iutil/document.h"
#include "iutil/objreg.h"

struct csLoaderPluginRec
{
  char* ShortName;
  char* ClassID;
  csRef<iBase> Component;
  csRef<iLoaderPlugin> Plugin;
  csRef<iBinaryLoaderPlugin> BinPlugin;
  csRef<iDocumentNode> defaults;

  csLoaderPluginRec (const char* shortName,
	const char *classID,
	iBase* component,
	iLoaderPlugin *plugin,
	iBinaryLoaderPlugin* binPlugin)
  {
    if (shortName) ShortName = csStrNew (shortName);
    else ShortName = 0;
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

  void SetDefaults (iDocumentNode* defaults)
  {
    csLoaderPluginRec::defaults = defaults;
  }
};

csLoader::csLoadedPluginVector::csLoadedPluginVector ()
{
  plugin_mgr = 0;
  mutex = csMutex::Create (true);
}

csLoader::csLoadedPluginVector::~csLoadedPluginVector ()
{
  DeleteAll ();
}

void csLoader::csLoadedPluginVector::DeleteAll ()
{
  csScopedMutexLock lock (mutex);
  size_t i;
  for (i = 0 ; i < vector.Length () ; i++)
  {
    csLoaderPluginRec* rec = vector[i];
    if (rec->Component && plugin_mgr)
    {
      csRef<iComponent> comp = SCF_QUERY_INTERFACE (rec->Component, iComponent);
      if (comp)
        plugin_mgr->UnloadPlugin (comp);
    }
    delete rec;
  }
  vector.DeleteAll ();
}

csLoaderPluginRec* csLoader::csLoadedPluginVector::FindPluginRec (
	const char* name)
{
  csScopedMutexLock lock (mutex);
  size_t i;
  for (i=0 ; i<vector.Length () ; i++)
  {
    csLoaderPluginRec* pl = vector.Get (i);
    if (pl->ShortName && !strcmp (name, pl->ShortName))
      return pl;
    if (!strcmp (name, pl->ClassID))
      return pl;
  }
  return 0;
}

bool csLoader::csLoadedPluginVector::GetPluginFromRec (
	csLoaderPluginRec *rec, iLoaderPlugin*& plug,
	iBinaryLoaderPlugin*& binplug)
{
  if (!rec->Component)
  {
    rec->Component = CS_QUERY_REGISTRY_TAG (object_reg, rec->ClassID);
    if (!rec->Component)
    {
      csRef<iComponent> comp = CS_LOAD_PLUGIN (plugin_mgr,
    	  rec->ClassID, iComponent);
      rec->Component = comp;
    }
    if (rec->Component)
    {
      rec->Plugin = SCF_QUERY_INTERFACE (rec->Component, iLoaderPlugin);
      rec->BinPlugin = SCF_QUERY_INTERFACE (rec->Component,
      	iBinaryLoaderPlugin);
    }
  }
  plug = rec->Plugin;
  binplug = rec->BinPlugin;
  return rec->Component != 0;
}

bool csLoader::csLoadedPluginVector::FindPlugin (
	const char* Name, iLoaderPlugin*& plug,
	iBinaryLoaderPlugin*& binplug, iDocumentNode*& defaults)
{
  csScopedMutexLock lock (mutex);
  // look if there is already a loading record for this plugin
  csLoaderPluginRec* pl = FindPluginRec (Name);
  if (pl)
  {
    defaults = pl->defaults;
    return GetPluginFromRec (pl, plug, binplug);
  }

  // create a new loading record
  vector.Push (new csLoaderPluginRec (0, Name, 0, 0, 0));
  defaults = 0;
  return GetPluginFromRec (vector.Get(vector.Length()-1),
  	plug, binplug);
}

void csLoader::csLoadedPluginVector::NewPlugin
	(const char *ShortName, iDocumentNode* child)
{
  csScopedMutexLock lock (mutex);
  csRef<iDocumentNode> id = child->GetNode ("id");
  if (id)
  {
    const char* ClassID = id->GetContentsValue ();
    csLoaderPluginRec* pr = new csLoaderPluginRec (ShortName, ClassID, 0, 0, 0);
    csRef<iDocumentNode> defaults = child->GetNode ("defaults");
    pr->SetDefaults (defaults);
    vector.Push (pr);
  }
  else
  {
    const char* ClassID = child->GetContentsValue ();
    vector.Push (new csLoaderPluginRec (ShortName, ClassID, 0, 0, 0));
  }
}

