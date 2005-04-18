/*
    Copyright (C) 2002 by Marten Svanfeldt
                          Anders Stenberg

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
#include <limits.h>
#include "csgeom/vector3.h"
#include "csgeom/vector4.h"
#include "cstypes.h"
#include "csutil/hashhandlers.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"
#include "igeom/clip2d.h"
#include "imap/services.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventq.h"
#include "iutil/object.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "iutil/verbositymanager.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "ivideo/texture.h"
#include "shadermgr.h"

// Pluginstuff
CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csShaderManager)  
  SCF_IMPLEMENTS_INTERFACE (iShaderManager)
  SCF_IMPLEMENTS_INTERFACE (iShaderVariableContext)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csShaderManager::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csShaderManager::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csShaderManager)


//=================== csShaderManager ================//

// General stuff
csShaderManager::csShaderManager(iBase* parent)
{
  SCF_CONSTRUCT_IBASE(parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  scfiEventHandler = 0;
  seqnumber = 0;
}

csShaderManager::~csShaderManager()
{
  //clear all shaders
  shaders.DeleteAll ();
  if (scfiEventHandler) scfiEventHandler->DecRef();
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

void csShaderManager::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (objectreg, severity, "crystalspace.graphics3d.shadermgr", 
    msg, args);
  va_end (args);
}

bool csShaderManager::Initialize(iObjectRegistry *objreg)
{
  objectreg = objreg;
  vc = CS_QUERY_REGISTRY (objectreg, iVirtualClock);
  txtmgr = CS_QUERY_REGISTRY (objectreg, iTextureManager);

  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (objectreg, iVerbosityManager));
  if (verbosemgr) 
    do_verbose = verbosemgr->Enabled ("renderer.shader");
  else
    do_verbose = false;

  csRef<iEventQueue> q = CS_QUERY_REGISTRY (objectreg, iEventQueue);
  if (q)
    q->RegisterListener (scfiEventHandler,
	CSMASK_Broadcast | CSMASK_Nothing);

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY  (objectreg,
	iPluginManager);

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    objectreg, "crystalspace.shared.stringset", iStringSet);

  csRef<iStringArray> classlist =
    iSCF::SCF->QueryClassList("crystalspace.graphics3d.shadercompiler.");
  size_t const nmatches = classlist.IsValid() ? classlist->Length() : 0;
  if (nmatches != 0)
  {
    size_t i;
    for (i = 0; i < nmatches; ++i)
    {
      const char* classname = classlist->Get(i);
      csRef<iShaderCompiler> plugin = 
	CS_LOAD_PLUGIN (plugin_mgr, classname, iShaderCompiler);
      if (plugin)
      {
	if (do_verbose)
	  Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "Loaded compiler plugin %s, compiler: %s", 
	    classname,plugin->GetName());
        RegisterCompiler(plugin);
      }
    }
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_WARNING, "No shader plugins found!");
  }
  
  config.AddConfig (objectreg, "/config/shadermgr.cfg");

  csString cfgKey;
  const csString keyPrefix ("Video.ShaderManager.Tags.");
  csSet<csStrKey, csConstCharHashKeyHandler> knownKeys;
  csRef<iConfigIterator> it (config->Enumerate (keyPrefix));
  while (it->Next ())
  {
    const char* key = it->GetKey (true);
    const char* dot = strrchr (key, '.');
    cfgKey.Clear ();
    cfgKey.Append (key, dot - key);

    if (knownKeys.In ((const char*)cfgKey)) continue;
    knownKeys.Add ((const char*)cfgKey);

    const char* tagName = config->GetStr (
      keyPrefix + cfgKey + ".TagName", cfgKey);
    const char* tagPresence = config->GetStr (
      keyPrefix + cfgKey + ".Presence", "Neutral");
    int tagPriority = config->GetInt (
      keyPrefix + cfgKey + ".Priority", 0);

    csShaderTagPresence presence;
    if (strcasecmp (tagPresence, "neutral") == 0)
    {
      presence = TagNeutral;
    }
    else if (strcasecmp (tagPresence, "forbidden") == 0)
    {
      presence = TagForbidden;
    }
    else if (strcasecmp (tagPresence, "required") == 0)
    {
      presence = TagRequired;
    }
    else
    {
      Report (CS_REPORTER_SEVERITY_WARNING, 
	"Unknown shader tag presence '%s' for tag config '%s'",
	tagPresence, (const char*)cfgKey);
      continue;
    }
    csStringID tagID = strings->Request (tagName);
    SetTagOptions (tagID, presence, tagPriority);
  }

  sv_time.AttachNew (new csShaderVariable (strings->Request ("standard time")));
  sv_time->SetValue (0.0f);
  svcontext.AddVariable (sv_time);

  return true;
}

void csShaderManager::Open ()
{
}

void csShaderManager::Close ()
{
}

bool csShaderManager::HandleEvent(iEvent& event)
{
  if (event.Type == csevBroadcast)
  {
    switch(event.Command.Code)
    {
      case cscmdPreProcess:
        UpdateStandardVariables();
	return false;
      case cscmdSystemOpen:
	{
	  Open ();
	  return true;
	}
      case cscmdSystemClose:
	{
	  Close ();
	  return true;
	}
    }
  }
  return false;
}

void csShaderManager::UpdateStandardVariables()
{
  sv_time->SetValue ((float)vc->GetCurrentTicks() / 1000.0f);
}

static int ShaderCompare (iShader* const& s1,
			  iShader* const& s2)
{
  return strcasecmp (s1->QueryObject ()->GetName(), 
    s2->QueryObject ()->GetName());
}

void csShaderManager::RegisterShader (iShader* shader)
{
  if (shader != 0)
    shaders.InsertSorted (shader, &ShaderCompare);
}

void csShaderManager::UnregisterShader (iShader* shader)
{
  if (shader != 0)
  {
    size_t index = shaders.FindSortedKey (
      csArrayCmp<iShader*, iShader*> (shader, &ShaderCompare));
    if (index != csArrayItemNotFound) shaders.DeleteIndex (index);
  }
}

static int ShaderCompareName (iShader* const& s1,
			      const char* const& name)
{
  return strcasecmp (s1->QueryObject ()->GetName(), 
    name);
}

iShader* csShaderManager::GetShader(const char* name)
{
  size_t index = shaders.FindSortedKey (
    csArrayCmp<iShader*, const char*> (name, &ShaderCompareName));
  if (index != csArrayItemNotFound) return shaders[index];

  return 0;
}

static int CompilerCompare (iShaderCompiler* const& c1,
			    iShaderCompiler* const& c2)
{
  return strcasecmp (c1->GetName(), c2->GetName());
}

void csShaderManager::RegisterCompiler(iShaderCompiler* compiler)
{
  compilers.InsertSorted (compiler, &CompilerCompare);
}

static int CompilerCompareName (iShaderCompiler* const& c1,
				const char* const& name)
{
  return strcasecmp (c1->GetName(), name);
}

iShaderCompiler* csShaderManager::GetCompiler(const char* name)
{
  size_t index = compilers.FindSortedKey (
    csArrayCmp<iShaderCompiler*, const char*> (name, &CompilerCompareName));
  if (index != csArrayItemNotFound) return compilers[index];

  return 0;
}

csSet<csStringID>& csShaderManager::GetTagSet (csShaderTagPresence presence)
{
  switch (presence)
  {
    case TagNeutral:
      return neutralTags;
    case TagForbidden:
      return forbiddenTags;
    case TagRequired:
      return requiredTags;

    default:
      return neutralTags;
  }
}

void csShaderManager::SetTagOptions (csStringID tag, csShaderTagPresence presence, 
  int priority)
{
  TagInfo* info = tagInfo.GetElementPointer (tag);
  if (info != 0)
  {
    if (info->presence != presence)
    {
      GetTagSet (info->presence).Delete (tag);
      if ((presence == TagNeutral) && (priority == 0))
      {
	tagInfo.DeleteAll (tag);
      }
      else
      {
	GetTagSet (presence).Add (tag);
	info->priority = priority;
      }
    }
  }
  else
  {
    if ((presence != TagNeutral) || (priority != 0))
    {
      TagInfo newInfo;
      newInfo.presence = presence;
      newInfo.priority = priority;
      GetTagSet (presence).Add (tag);
      tagInfo.Put (tag, newInfo);
    }
  }
}

void csShaderManager::GetTagOptions (csStringID tag, csShaderTagPresence& presence, 
  int& priority)
{
  TagInfo* info = tagInfo.GetElementPointer (tag);
  if (info == 0) 
  {
    presence = TagNeutral;
    priority = 0;
    return;
  }

  presence = info->presence;
  priority = info->priority;
}

const csSet<csStringID>& csShaderManager::GetTags (csShaderTagPresence presence,
    int& count)
{
  csSet<csStringID>& set = GetTagSet (presence);
  count = (int)set.GetSize ();
  return set;
}


void csShaderManager::SetActiveLights (const csArray<iLight*>& lights)
{
  //copy over the list
  activeLights.Empty ();
  activeLights = lights;
}
