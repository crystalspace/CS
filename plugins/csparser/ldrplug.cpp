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
#include "isys/system.h"
#include "imap/reader.h"

struct csLoaderPluginRec
{
  char* ShortName;
  char* ClassID;
  iLoaderPlugIn* Plugin;

  csLoaderPluginRec (const char* iShortName,
	const char *iClassID, iLoaderPlugIn *iPlugin)
  {
    if (iShortName) ShortName = csStrNew (iShortName);
    else ShortName = NULL;
    ClassID = csStrNew (iClassID);
    Plugin = iPlugin;
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
  System = NULL;
}

csLoader::csLoadedPluginVector::~csLoadedPluginVector ()
{
  DeleteAll ();
}

bool csLoader::csLoadedPluginVector::FreeItem (csSome Item)
{
  csLoaderPluginRec *rec = (csLoaderPluginRec*)Item;
  if (rec->Plugin)
  {
    if (System)
    {
      iPlugin* p = SCF_QUERY_INTERFACE(rec->Plugin, iPlugin);
      if (p)
      {
        System->UnloadPlugIn(p);
	p->DecRef();
      }
    }
    rec->Plugin->DecRef ();
  }
  delete rec;
  return true;
}

csLoaderPluginRec* csLoader::csLoadedPluginVector::FindPlugInRec (
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

iLoaderPlugIn* csLoader::csLoadedPluginVector::GetPluginFromRec (
	csLoaderPluginRec *rec, const char *FuncID)
{
  if (!rec->Plugin)
    rec->Plugin = CS_LOAD_PLUGIN (System, rec->ClassID, FuncID, iLoaderPlugIn);
  return rec->Plugin;
}

iLoaderPlugIn* csLoader::csLoadedPluginVector::FindPlugIn (
	const char* Name, const char* FuncID)
{
  // look if there is already a loading record for this plugin
  csLoaderPluginRec* pl = FindPlugInRec (Name);
  if (pl)
    return GetPluginFromRec(pl, FuncID);

  // create a new loading record
  NewPlugIn (NULL, Name);
  return GetPluginFromRec((csLoaderPluginRec*)Get(Length()-1), FuncID);
}

void csLoader::csLoadedPluginVector::NewPlugIn
	(const char *ShortName, const char *ClassID)
{
  Push (new csLoaderPluginRec (ShortName, ClassID, NULL));
}
