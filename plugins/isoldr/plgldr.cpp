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

// Note - This file is copied from plugins/csparser/ldrplg.cpp
// with only the class names changed - perhaps there's a better
// way to do this ? or indeed to manage the plugins ?
// if there is then let me know - Ta - richard@starport.net

#include "cssysdef.h"
#include "isoload.h"
#include "imap/reader.h"

struct csLoaderPluginRec
{
  char* ShortName;
  char* ClassID;
  csRef<iLoaderPlugin> Plugin;

  csLoaderPluginRec (const char* iShortName,
	const char *iClassID, iLoaderPlugin *iPlugin)
  {
    if (iShortName) ShortName = csStrNew (iShortName);
    else ShortName = 0;
    ClassID = csStrNew (iClassID);
    Plugin = iPlugin;
  }

  ~csLoaderPluginRec ()
  {
    delete [] ShortName;
    delete [] ClassID;
  }
};

csIsoLoader::csLoadedPluginVector::csLoadedPluginVector (
	int iLimit, int iThresh)
	: csPDelArray<csLoaderPluginRec> (iLimit, iThresh)
{
  plugin_mgr = 0;
}

csIsoLoader::csLoadedPluginVector::~csLoadedPluginVector ()
{
  FreeAll ();
}

void csIsoLoader::csLoadedPluginVector::FreeAll ()
{
  int i;
  for (i = 0 ; i < Length () ; i++)
    FreeItem (Get (i));
  DeleteAll ();
}

void csIsoLoader::csLoadedPluginVector::FreeItem (csLoaderPluginRec* rec)
{
  if (rec->Plugin)
  {
    if (plugin_mgr)
    {
      csRef<iComponent> p (SCF_QUERY_INTERFACE(rec->Plugin, iComponent));
      if (p)
        plugin_mgr->UnloadPlugin(p);
    }
  }
}

csLoaderPluginRec* csIsoLoader::csLoadedPluginVector::FindPluginRec (
	const char* name)
{
  int i;
  for (i=0 ; i<Length () ; i++)
  {
    csLoaderPluginRec* pl = Get (i);
    if (pl->ShortName && !strcmp (name, pl->ShortName))
      return pl;
    if (!strcmp (name, pl->ClassID))
      return pl;
  }
  return 0;
}

iLoaderPlugin* csIsoLoader::csLoadedPluginVector::GetPluginFromRec (
	csLoaderPluginRec *rec)
{

  if (!rec->Plugin)
    rec->Plugin = CS_LOAD_PLUGIN (plugin_mgr, rec->ClassID, iLoaderPlugin);

  return rec->Plugin;
}

iLoaderPlugin* csIsoLoader::csLoadedPluginVector::FindPlugin (
	const char* Name)
{
  // look if there is already a loading record for this plugin
  csLoaderPluginRec* pl = FindPluginRec (Name);
  if (pl)
    return GetPluginFromRec(pl);

  // create a new loading record
  NewPlugin (0, Name);
  return GetPluginFromRec(Get(Length()-1));
}

void csIsoLoader::csLoadedPluginVector::NewPlugin
	(const char *ShortName, const char *ClassID)
{
  Push (new csLoaderPluginRec (ShortName, ClassID, 0));
}
