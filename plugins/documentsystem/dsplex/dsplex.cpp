/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#include "csutil/util.h"
#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "iutil/document.h"
#include "iutil/databuff.h"
#include "iutil/plugin.h"
#include "iutil/objreg.h"
#include "iutil/stringarray.h"
#include "iutil/vfs.h"

#include "dsplex.h"

#define DSCLASSPREFIX "crystalspace.documentsystem."
#define DOCPLEX_CLASSNAME DSCLASSPREFIX "multiplexer"

SCF_IMPLEMENT_IBASE (csPlexDocument)
  SCF_IMPLEMENTS_INTERFACE (iDocument)
SCF_IMPLEMENT_IBASE_END

csPlexDocument::csPlexDocument (csRef<csDocumentSystemMultiplexer> aPlexer)
{
  SCF_CONSTRUCT_IBASE (0);
  plexer = aPlexer;
}

csPlexDocument::~csPlexDocument ()
{
  SCF_DESTRUCT_IBASE();
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
    wrappedDS = plexer->defaultDocSys;
  size_t pluginnum = 0;
  if (!wrappedDS)
    wrappedDS = plexer->LoadNextPlugin (pluginnum++);
  if (!wrappedDS)
    return 0;
  wrappedDoc = wrappedDS->CreateDocument ();
  while (wrappedDoc->Changeable() == CS_CHANGEABLE_NEVER)
  {
    wrappedDS = plexer->LoadNextPlugin (pluginnum++);
    if (!wrappedDS)
    {
      wrappedDoc = 0;
      return 0;
    }
    wrappedDoc = wrappedDS->CreateDocument ();
  }
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
    return 0;
  }
}

const char* csPlexDocument::Parse (iFile* file, bool collapse)
{
  size_t oldpos = file->GetPos ();

  size_t pluginnum = 0;
  csRef<iDocumentSystem> DS;
  wrappedDoc = 0;
  lasterr.Clear();
  while (DS = plexer->LoadNextPlugin (pluginnum++))
  {
    csRef<iDocument> doc = DS->CreateDocument();
    file->SetPos (oldpos);
    const char* err = doc->Parse (file, collapse);
    if (!err)
    {
      wrappedDoc = doc;
      plexer->RewardPlugin (pluginnum);
      return 0;
    }
    else
    {
      lasterr.Append (err);
      lasterr.Append ("\n");
    }
  }
  return lasterr;
}

const char* csPlexDocument::Parse (iDataBuffer* buf, bool collapse)
{
  size_t pluginnum = 0;
  csRef<iDocumentSystem> DS;
  wrappedDoc = 0;
  lasterr.Clear();
  while (DS = plexer->LoadNextPlugin (pluginnum++))
  {
    csRef<iDocument> doc = DS->CreateDocument();
    const char* err = doc->Parse (buf, collapse);
    if (!err)
    {
      wrappedDoc = doc;
      plexer->RewardPlugin (pluginnum);
      return 0;
    }
    else
    {
      lasterr.Append (err);
      lasterr.Append ("\n");
    }
  }
  return lasterr;
}

const char* csPlexDocument::Parse (iString* str, bool collapse)
{
  size_t pluginnum = 0;
  csRef<iDocumentSystem> DS;
  wrappedDoc = 0;
  lasterr.Clear();
  while (DS = plexer->LoadNextPlugin (pluginnum++))
  {
    csRef<iDocument> doc = DS->CreateDocument();
    const char* err = doc->Parse (str, collapse);
    if (!err)
    {
      wrappedDoc = doc;
      plexer->RewardPlugin (pluginnum);
      return 0;
    }
    else
    {
      lasterr.Append (err);
      lasterr.Append ("\n");
    }
  }
  return lasterr;
}

const char* csPlexDocument::Parse (const char* buf, bool collapse)
{
  size_t pluginnum = 0;
  csRef<iDocumentSystem> DS;
  wrappedDoc = 0;
  lasterr.Clear();
  while (DS = plexer->LoadNextPlugin (pluginnum++))
  {
    csRef<iDocument> doc = DS->CreateDocument();
    const char* err = doc->Parse (buf, collapse);
    if (!err)
    {
      wrappedDoc = doc;
      plexer->RewardPlugin (pluginnum - 1);
      return 0;
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
    return 0;
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
    return 0;
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
    return 0;
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

SCF_IMPLEMENT_IBASE(csDocumentSystemMultiplexer)
  SCF_IMPLEMENTS_INTERFACE(iDocumentSystem)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

csDocumentSystemMultiplexer::csDocumentSystemMultiplexer(iBase* parent) : autolist (0, 1)
{
  SCF_CONSTRUCT_IBASE(parent);
}

csDocumentSystemMultiplexer::~csDocumentSystemMultiplexer ()
{
  SCF_DESTRUCT_IBASE();
}

csRef<iDocument> csDocumentSystemMultiplexer::CreateDocument ()
{
  return csPtr<iDocument> (new csPlexDocument (this));
}

bool csDocumentSystemMultiplexer::Initialize (iObjectRegistry* object_reg)
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
    orderedlist.ShrinkBestFit();

    csRef<iBase> b (CS_QUERY_REGISTRY_TAG (object_reg, 
      "iDocumentSystem.Default"));
    if (b)
    {
      defaultDocSys = SCF_QUERY_INTERFACE (b, iDocumentSystem);
    }

    classlist = iSCF::SCF->QueryClassList (DSCLASSPREFIX);
    return true;
  }
  return false;
}

csRef<iDocumentSystem> csDocumentSystemMultiplexer::LoadNextPlugin (size_t num)
{
  csRef<iDocumentSystem> res;
  if (num < orderedlist.Length())
  {
    res = orderedlist[num];
  }
  else
  {
    size_t anum = num - orderedlist.Length();
    if (anum < autolist.Length())
    {
      res = autolist[anum];
    }
    else
    {
      if (classlist.IsValid())
      {
	while (classlist.IsValid() && !res.IsValid())
	{
	  char const* classname = 0;
	  do
	  {
	    if (classname) classlist->DeleteIndex (0);
	    if (classlist->Length() == 0)
	    {
	      classlist = 0;
	      plugin_mgr = 0;
	      return 0;
	    }
	    classname = classlist->Get(0);
	  } while (!strcasecmp (classname, DOCPLEX_CLASSNAME));
          
	  res = CS_LOAD_PLUGIN (plugin_mgr, classname, iDocumentSystem);
	  if (res.IsValid())
	  {
	    // remember the plugin
	    autolist.Push (res);
	  }
	  classlist->DeleteIndex (0);
	}
      }
    }
  }
  return res;
}

void csDocumentSystemMultiplexer::RewardPlugin (size_t num)
{
  size_t anum;
  if ((num >= orderedlist.Length()) && 
    (autolist.Length() - (anum = num - orderedlist.Length()) > 4))
  {
    csRef<iDocumentSystem> plugin = autolist[anum];
    autolist.Push (plugin);
    autolist.DeleteIndex (anum);
  }
}

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csDocumentSystemMultiplexer)
