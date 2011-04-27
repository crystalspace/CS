/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "csutil/scf.h"
#include "csutil/plugldr.h"
#include "csutil/util.h"
#include "csutil/snprintf.h"
#include "csutil/stringquote.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/cmdline.h"
#include "iutil/cfgmgr.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/comp.h"

csPluginLoader::csPluginLoader (iObjectRegistry* object_reg)
{
  csPluginLoader::object_reg = object_reg;
}

csPluginLoader::~csPluginLoader ()
{
}

bool csPluginLoader::ReplaceRequestedPlugin (const char* pluginName,
                                             const char* tagName)
{
  if (tagName && *tagName)
  {
    for (size_t i = 0; i < requested_plugins.GetSize(); i++)
    {
      csPluginLoadRec*& req_plugin = requested_plugins[i];
      if (req_plugin->Tag.Compare (tagName))
      {
	req_plugin->ClassID = pluginName;
	return true;
      }
    }
  }
  requested_plugins.Push (new csPluginLoadRec (tagName, pluginName));
  return false;
}

void csPluginLoader::AddConfigurationPlugins (iConfigFile* Config,
                                              const char* prefix)
{
  // Now load and initialize all plugins
  csRef<iConfigIterator> plugin_list (Config->Enumerate (prefix));
  if (plugin_list)
  {
    while (plugin_list->HasNext ())
    {
      plugin_list->Next();
      const char *tag = plugin_list->GetKey (true);
      const char *classID = plugin_list->GetStr ();
      if (classID)
        ReplaceRequestedPlugin (classID, tag);
    }
  }

}

void csPluginLoader::AddCommandLinePlugins (iCommandLineParser* CommandLine)
{
  // Now eat all common-for-plugins command-line switches
  const char *val = CommandLine->GetOption ("video");
  if (val)
  {
    // Alternate videodriver
    csStringFast<100> temp;
    temp.Format ("crystalspace.graphics3d.%s", val);
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    	"crystalspace.pluginloader.loadplugins",
    	"Using alternative 3D driver: %s", temp.GetData());
    ReplaceRequestedPlugin (temp, "iGraphics3D");
  }

  val = CommandLine->GetOption ("canvas");
  if (val)
  {
    if (!strchr (val, '.'))
    {
      csStringFast<100> temp;
      temp.Format ("crystalspace.graphics2d.%s", val);
      csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    	  "crystalspace.pluginloader.loadplugins",
    	  "Using alternative 2D canvas: %s", temp.GetData());
      CommandLine->ReplaceOption ("canvas", temp);
    }
  }

  // Eat all --plugin switches specified on the command line
  size_t n = 0;
  while ((val = CommandLine->GetOption ("plugin", n++)))
  {
    csStringFast<100> temp;
    temp = val;
    char *tag = strchr ((char*)temp.GetData(), ':');
    if (tag) *tag++ = 0;
    // If an ID isn't registered try to insert "crystalspace.utilities."
    // at the beginning. That makes it possible to specfiy e.g. 
    // '-plugin=bugplug' on the cmd line.
    if (!iSCF::SCF->ClassRegistered (temp))
    {
      csStringFast<100> temp2;
      temp2.Format ("crystalspace.utilities.%s", temp.GetData());
      ReplaceRequestedPlugin (temp2, tag);
    }
    else
    {
      ReplaceRequestedPlugin (temp, tag);
    }
  }
}

bool csPluginLoader::LoadPlugins ()
{
  csRef<iVFS> VFS (csQueryRegistry<iVFS> (object_reg));

  csRef<iPluginManager> plugin_mgr (
  	csQueryRegistry<iPluginManager> (object_reg));

  // Load all plugins
  for (size_t n = 0; n < requested_plugins.GetSize (); n++)
  {
    csPluginLoadRec* r = requested_plugins.Get(n);
    plugin_mgr->SetTagClassIDMapping (r->Tag, r->ClassID);
  }
  for (size_t n = 0; n < requested_plugins.GetSize (); n++)
  {
    csPluginLoadRec* r = requested_plugins.Get(n);
    if (r->Tag)
    {
      // If we have a tag check if an object is already registered
      r->plugin = csPtr<iBase> (object_reg->Get (r->Tag));
      if (r->plugin.IsValid()) continue; // If yes, don't load the plugin
    }
    
    csRef<iComponent> c (plugin_mgr->LoadPluginInstance (r->ClassID,
      iPluginManager::lpiInitialize | iPluginManager::lpiReportErrors
      | iPluginManager::lpiLoadDependencies));
    r->plugin = c;
    if (r->plugin)
    {
      if (!object_reg->Register (r->plugin, r->Tag))
      {
	if (r->Tag)
	  csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	    "crystalspace.pluginloader.loadplugins",
	    "Duplicate tag %s found for plugin %s!",
	    CS::Quote::Single (r->Tag.GetData()), 
	    CS::Quote::Single (r->ClassID.GetData()));
	else
	  csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	    "crystalspace.pluginloader.loadplugins",
	    "Could not register plugin %s!",
	    CS::Quote::Single (r->ClassID.GetData()));
	return false;
      }
    }
  }

  return true;
}

void csPluginLoader::RequestPlugin (const char *pluginName,
	const char* tagName)
{
  requested_plugins.Push (new csPluginLoadRec (tagName, pluginName));
}

