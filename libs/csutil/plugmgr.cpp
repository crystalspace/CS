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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cssysdef.h"
#include "csutil/plugmgr.h"
#include "csutil/util.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/cmdline.h"
#include "iutil/cfgmgr.h"
#include "ivaria/reporter.h"

//------------------------------------------------------ csPlugin class -----//

csPluginManager::csPlugin::csPlugin (iComponent *iObject, const char *iClassID,
  const char *iFuncID)
{
  Plugin = iObject;
  ClassID = csStrNew (iClassID);
  FuncID = csStrNew (iFuncID);
}

csPluginManager::csPlugin::~csPlugin ()
{
  delete [] ClassID;
  delete [] FuncID;
  Plugin->DecRef ();
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csPluginManager)
  SCF_IMPLEMENTS_INTERFACE (iPluginManager)
SCF_IMPLEMENT_IBASE_END

csPluginManager::csPluginManager (iObjectRegistry* object_reg) :
  Plugins (8, 8), OptionList (16, 16)
{
  SCF_CONSTRUCT_IBASE (NULL);

  csPluginManager::object_reg = object_reg;
}

csPluginManager::~csPluginManager ()
{
  OptionList.DeleteAll ();

  // Free all plugins.
  int i;
  for (i = Plugins.Length()-1 ; i >= 0 ; i--)
    UnloadPlugin ((iComponent *)Plugins.Get(i)->Plugin);
}

void csPluginManager::QueryOptions (iComponent *iObject)
{
  iCommandLineParser* CommandLine = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);

  iConfig *Config = SCF_QUERY_INTERFACE (iObject, iConfig);
  if (Config)
  {
    int on = OptionList.Length ();
	int i;
    for (i = 0 ; ; i++)
    {
      csOptionDescription option;
      if (!Config->GetOptionDescription (i, &option))
        break;
      OptionList.Push (new csPluginOption (option.name, option.type, option.id,
        (option.type == CSVAR_BOOL) || (option.type == CSVAR_CMD), Config));
      if (option.type == CSVAR_BOOL)
      {
        char buf[100];
        strcpy (buf, "no");
        strcpy (buf + 2, option.name);
        OptionList.Push (new csPluginOption (buf, option.type, option.id,
          false, Config));
      }
    } /* endfor */

    // Parse the command line for plugin options and pass them to plugin
    for (; on < OptionList.Length (); on++)
    {
      csPluginOption *pio = (csPluginOption *)OptionList.Get (on);
      const char *val;
      if ((val = CommandLine->GetOption (pio->Name)))
      {
        csVariant optval;
        switch (pio->Type)
        {
          case CSVAR_CMD:
	    optval.SetCommand ();
	    break;
          case CSVAR_BOOL:
            optval.SetBool (pio->Value);
            break;
          case CSVAR_LONG:
            if (!val) continue;
            optval.SetLong (atol (val));
            break;
          case CSVAR_FLOAT:
            if (!val) continue;
            optval.SetFloat (atof (val));
            break;
	  case CSVAR_STRING:
	    if (!val) continue;
	    optval.SetString (val);
	    break;
        }
        pio->Config->SetOption (pio->ID, &optval);
      }
    }
    Config->DecRef ();
  }
}

iBase *csPluginManager::LoadPlugin (const char *iClassID, const char *iFuncID,
  const char *iInterface, int iVersion)
{
  iComponent *p = SCF_CREATE_INSTANCE (iClassID, iComponent);
  if (!p)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
    	"crystalspace.pluginmgr.loadplugin",
    	"WARNING: could not load plugin '%s'", iClassID);
  }
  else
  {
    int index = Plugins.Push (new csPlugin (p, iClassID, iFuncID));
    if (p->Initialize (object_reg))
    {
      iBase *ret;
      if (iInterface)
        ret = (iBase *)p->QueryInterface (
	  iSCF::SCF->GetInterfaceID (iInterface), iVersion);
      else
        (ret = p)->IncRef();
      if (ret)
      {
        QueryOptions (p);
        return ret;
      }
    }
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
    	"crystalspace.pluginmgr.loadplugin",
    	"WARNING: failed to initialize plugin '%s'", iClassID);
    Plugins.Delete (index);
  }
  return NULL;
}

bool csPluginManager::RegisterPlugin (const char *iClassID,
  const char *iFuncID, iComponent *iObject)
{
  int index = Plugins.Push (new csPlugin (iObject, iClassID, iFuncID));
  if (iObject->Initialize (object_reg))
  {
    QueryOptions (iObject);
    iObject->IncRef ();
    return true;
  }
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
    	"crystalspace.pluginmgr.registerplugin",
    	"WARNING: failed to initialize plugin '%s'", iClassID);
    Plugins.Delete (index);
    return false;
  }
}

int csPluginManager::GetPluginCount ()
{
  return Plugins.Length ();
}

iBase* csPluginManager::GetPlugin (int idx)
{
  csPlugin* pl = Plugins.Get (idx);
  return pl->Plugin;
}

iBase *csPluginManager::QueryPlugin (const char *iInterface, int iVersion)
{
  scfInterfaceID ifID = iSCF::SCF->GetInterfaceID (iInterface);
  int i;
  for (i = 0; i < Plugins.Length (); i++)
  {
    iBase *ret =
      (iBase *)Plugins.Get (i)->Plugin->QueryInterface (ifID, iVersion);
    if (ret)
      return ret;
  }
  return NULL;
}

iBase *csPluginManager::QueryPlugin (
  const char *iFuncID, const char *iInterface, int iVersion)
{
  int idx = Plugins.FindKey (iFuncID, 1);
  if (idx < 0)
    return NULL;

  return (iBase *)Plugins.Get (idx)->Plugin->QueryInterface (
    iSCF::SCF->GetInterfaceID (iInterface), iVersion);
}

iBase *csPluginManager::QueryPlugin (const char* iClassID, const char *iFuncID, 
				    const char *iInterface, int iVersion)
{
  scfInterfaceID ifID = iSCF::SCF->GetInterfaceID (iInterface);
  int i;
  for (i = 0 ; i < Plugins.Length () ; i++)
  {
    csPlugin* pl = Plugins.Get (i);
    if (pl->ClassID && pl->FuncID)
      if (pl->ClassID == iClassID || !strcmp (pl->ClassID, iClassID))
      {
	if (pl->FuncID == iFuncID || !strcmp (pl->FuncID, iFuncID))
	{
	  return (iBase*)Plugins.Get(i)->Plugin->QueryInterface(ifID,iVersion);
	}
      }
  }
  return NULL;
}

bool csPluginManager::UnloadPlugin (iComponent *iObject)
{
  int idx = Plugins.FindKey (iObject);
  if (idx < 0)
    return false;

  iConfig *config = SCF_QUERY_INTERFACE (iObject, iConfig);
  if (config)
  {
    for (int i = OptionList.Length () - 1; i >= 0; i--) 
    {
      csPluginOption *pio = (csPluginOption *)OptionList.Get (i);
      if (pio->Config == config)
        OptionList.Delete (i);
    }
    config->DecRef ();
  }

  object_reg->Unregister ((iBase *)iObject, NULL);
  return Plugins.Delete (idx);
}

