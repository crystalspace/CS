/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "csutil/util.h"
#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "iutil/document.h"
#include "iutil/databuff.h"
#include "iutil/plugin.h"
#include "iutil/objreg.h"
#include "iutil/strvec.h"
#include "iutil/vfs.h"

#include "dsplex.h"

#define DSCLASSPREFIX	"crystalspace.documentsystem."
#define MY_CLASSNAME	DSCLASSPREFIX "multiplex"

SCF_IMPLEMENT_IBASE (csPlexDocument)
  SCF_IMPLEMENTS_INTERFACE (iDocument)
SCF_IMPLEMENT_IBASE_END

csPlexDocument::csPlexDocument (csRef<csMplexDocumentSystem> aPlexer)
{
  SCF_CONSTRUCT_IBASE (NULL);

  plexer = aPlexer;
}

csPlexDocument::~csPlexDocument ()
{
}

void csPlexDocument::Clear ()
{
  if (wrappedDoc)
  {
    wrappedDoc->Clear();
  }
}

csRef<iDocumentNode> csPlexDocument::CreateRoot ()
{
  csRef<iDocumentSystem> wrappedDS;
  if (!wrappedDS)
  {
    wrappedDS = plexer->defaultDocSys;
  }
  if (!wrappedDS && (plexer->orderedlist.Length() > 0))
  {
    wrappedDS = plexer->orderedlist[0];
  }
  if (!wrappedDS)
  {
    if (plexer->autolist.Length() > 0)
    {
      wrappedDS = plexer->autolist[0];
    }
    else
    {
      int pi = 0;
      wrappedDS = plexer->LoadNextPlugin (pi);
    }
  }
  if (!wrappedDS)
  {
    return NULL;
  }
  wrappedDoc = wrappedDS->CreateDocument();
  return wrappedDoc->CreateRoot();
}

csRef<iDocumentNode> csPlexDocument::GetRoot ()
{
  if (wrappedDoc)
  {
    return wrappedDoc->GetRoot();
  }
  else
  {
    return NULL;
  }
}

const char* csPlexDocument::Parse (iFile* file)
{
  size_t oldpos = file->GetPos ();

  int pluginnum = 0;
  csRef<iDocumentSystem> DS;
  wrappedDoc = NULL;
  lasterr.Clear();
  while (DS = plexer->LoadNextPlugin (pluginnum++))
  {
    csRef<iDocument> doc = DS->CreateDocument();
    file->SetPos (oldpos);
    const char* err = doc->Parse (file);
    if (!err)
    {
      wrappedDoc = doc;
      plexer->RewardPlugin (pluginnum);
      return NULL;
    }
    else
    {
      lasterr.Append (err);
      lasterr.Append ("\n");
    }
  }
  return lasterr;
}

const char* csPlexDocument::Parse (iDataBuffer* buf)
{
  int pluginnum = 0;
  csRef<iDocumentSystem> DS;
  wrappedDoc = NULL;
  lasterr.Clear();
  while (DS = plexer->LoadNextPlugin (pluginnum++))
  {
    csRef<iDocument> doc = DS->CreateDocument();
    const char* err = doc->Parse (buf);
    if (!err)
    {
      wrappedDoc = doc;
      plexer->RewardPlugin (pluginnum);
      return NULL;
    }
    else
    {
      lasterr.Append (err);
      lasterr.Append ("\n");
    }
  }
  return lasterr;
}

const char* csPlexDocument::Parse (iString* str)
{
  int pluginnum = 0;
  csRef<iDocumentSystem> DS;
  wrappedDoc = NULL;
  lasterr.Clear();
  while (DS = plexer->LoadNextPlugin (pluginnum++))
  {
    csRef<iDocument> doc = DS->CreateDocument();
    const char* err = doc->Parse (str);
    if (!err)
    {
      wrappedDoc = doc;
      plexer->RewardPlugin (pluginnum);
      return NULL;
    }
    else
    {
      lasterr.Append (err);
      lasterr.Append ("\n");
    }
  }
  return lasterr;
}

const char* csPlexDocument::Parse (const char* buf)
{
  int pluginnum = 0;
  csRef<iDocumentSystem> DS;
  wrappedDoc = NULL;
  lasterr.Clear();
  while (DS = plexer->LoadNextPlugin (pluginnum++))
  {
    csRef<iDocument> doc = DS->CreateDocument();
    const char* err = doc->Parse (buf);
    if (!err)
    {
      wrappedDoc = doc;
      plexer->RewardPlugin (pluginnum - 1);
      return NULL;
    }
    else
    {
      lasterr.Append (err);
      lasterr.Append ("\n");
    }
  }
  return lasterr;
}

const char* csPlexDocument::Write (iFile* file)
{
  if (wrappedDoc)
  {
    return wrappedDoc->Write (file);
  }
  else
  {
    return NULL;
  }
}

const char* csPlexDocument::Write (iString* str)
{
  if (wrappedDoc)
  {
    return wrappedDoc->Write (str);
  }
  else
  {
    return NULL;
  }
}

const char* csPlexDocument::Write (iVFS* vfs, const char* filename)
{
  if (wrappedDoc)
  {
    return wrappedDoc->Write (vfs, filename);
  }
  else
  {
    return NULL;
  }
}

int csPlexDocument::Changeable ()
{
  if (wrappedDoc)
  {
    return wrappedDoc->Changeable();
  }
  else
  {
    // @@@ guess.
    return CS_CHANGEABLE_NEWROOT;
  }
}

SCF_IMPLEMENT_IBASE(csMplexDocumentSystem)
  SCF_IMPLEMENTS_INTERFACE(iDocumentSystem)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

csMplexDocumentSystem::csMplexDocumentSystem(iBase* parent)
{
  SCF_CONSTRUCT_IBASE(parent);
}

csMplexDocumentSystem::~csMplexDocumentSystem ()
{
}

csRef<iDocument> csMplexDocumentSystem::CreateDocument ()
{
  return csPtr<iDocument> (new csPlexDocument (this));
}

bool csMplexDocumentSystem::Initialize (iObjectRegistry* object_reg)
{
  if (object_reg)
  {
    plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

    int idx;
    int errorcount = 0;
    for (idx = 1; ; idx++)
    {
      csRef<iBase> b (CS_QUERY_REGISTRY_TAG (object_reg, 
	csString().Format ("iDocumentSystem.%d", idx)));
      if (!b) 
      {
	// in cases where just one entry in the list doesn't work
	// but later ones would those are loaded, too
	errorcount++;
	if (errorcount == 2) break;
      }
      else
      {
	errorcount = 0;	
	csRef<iDocumentSystem> ds (SCF_QUERY_INTERFACE (b, iDocumentSystem));
	orderedlist.Push (ds);
      }
    }

    csRef<iBase> b (CS_QUERY_REGISTRY_TAG (object_reg, 
      "iDocumentSystem.Default"));
    if (b)
    {
      defaultDocSys = SCF_QUERY_INTERFACE (b, iDocumentSystem);
    }

    classlist = csPtr<iStrVector> (
      iSCF::SCF->QueryClassList (DSCLASSPREFIX));
    return true;
  }
  return false;
}

csRef<iDocumentSystem> csMplexDocumentSystem::LoadNextPlugin (int num)
{
  csRef<iDocumentSystem> res;
  if (num < orderedlist.Length())
  {
    res = orderedlist[num];
  }
  else
  {
    int anum = num - orderedlist.Length();
    if (anum < autolist.Length())
    {
      res = autolist[anum];
    }
    else
    {
      if (classlist)
      {
	while (classlist && !res)
	{
	  char const* classname = NULL;
	  do
	  {
	    if (classname) classlist->Delete (0);
	    if (classlist->Length() == 0)
	    {
	      classlist = NULL;
	      plugin_mgr = NULL;
	      return NULL;
	    }
	    classname = classlist->Get(0);
	  } while (!strcasecmp (classname, MY_CLASSNAME));
          
	  res = CS_LOAD_PLUGIN (plugin_mgr, classname, iDocumentSystem);
	  if (res)
	  {
	    // remember the plugin
	    autolist.Push (res);
	  }
	  classlist->Delete (0);
	}
      }
    }
  }
  return res;
}

void csMplexDocumentSystem::RewardPlugin (int num)
{
  int anum = num - orderedlist.Length();
  if ((anum >= 0) && 
    (autolist.Length() - anum > 4))
  {
    csRef<iDocumentSystem> plugin = autolist[anum];
    autolist.Push (plugin);
    autolist.Delete (anum);
  }
}

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csMplexDocumentSystem)

SCF_EXPORT_CLASS_TABLE(dsplex)
  SCF_EXPORT_CLASS(csMplexDocumentSystem, MY_CLASSNAME,
      "Document system multiplexer")
SCF_EXPORT_CLASS_TABLE_END

