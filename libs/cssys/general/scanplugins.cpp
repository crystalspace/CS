/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Eric Sunshine
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

#define CS_SYSDEF_PROVIDE_DIR
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "cssys/sysfunc.h"
#include "csutil/csstring.h"
#include "csutil/csstrvec.h"
#include "csutil/physfile.h"
#include "csutil/scfstr.h"
#include "csutil/scfstrv.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "iutil/document.h"

static void AppendStrVecString (iStrVector*& strings, const char* str)
{
  if (!strings)
  {
    strings = new scfStrVector ();
  }
  strings->Push (csStrNew (str));
}

static csRef<iString> InternalGetPluginMetadata (const char* fullPath, 
						 csRef<iDocument>& metadata,
						 iDocumentSystem* docsys)
{
  iString* result = 0;

  csPhysicalFile file (fullPath, "rb");

  csRef<iDocument> doc = docsys->CreateDocument();
  char const* errmsg = doc->Parse (&file/*Buffer*/);
  if (errmsg == 0)
  {
    metadata = doc;
  }
  else
  {
    metadata = 0;

    csString errstr;
    errstr.Format ("Error parsing metadata from %s: %s",
      fullPath, errmsg);

    result = new scfString (errstr);
  }

  //delete[] Buffer;
	  
  return csPtr<iString> (result);
}
  
csRef<iString> csGetPluginMetadata (const char* fullPath, 
				    csRef<iDocument>& metadata)
{
  csRef<iDocumentSystem> docsys = csPtr<iDocumentSystem>
    (new csTinyDocumentSystem ());
  return InternalGetPluginMetadata (fullPath, metadata, docsys);
}

// Scan a directory for .csplugin files
void InternalScanPluginDir (iStrVector*& messages,
			    iDocumentSystem* docsys,
			    const char* dir, 
			    csRef<iStrVector>& plugins,
			    csRefArray<iDocument>& metadata,
			    bool recursive)
{
  struct dirent* de;
  DIR* dh = opendir(dir);
  if (dh != 0)
  {
    while ((de = readdir(dh)) != 0)
    {
      if (!isdir(dir, de))
      {
        int const n = strlen(de->d_name);
        if (n >= 9 && strcasecmp(de->d_name + n - 9, ".csplugin") == 0)
        {
	  csString scffilepath;
	  scffilepath << dir << PATH_SEPARATOR << de->d_name;
	  
	  csRef<iDocument> doc;
	  csRef<iString> error = InternalGetPluginMetadata (
	    scffilepath, doc, docsys);
	    
	  if (error == 0)
	  {
	    metadata.Push (doc);
	    plugins->Push (csStrNew (scffilepath));
	  }  
	  else
	  {
	    AppendStrVecString (messages, error->GetData ());
	  }
        }
	else
	{
	  if (recursive && (strcmp (de->d_name, ".") != 0)
	    && (strcmp (de->d_name, "..") != 0))
	  {
	    iStrVector* subdirMessages = 0;
	    
	    csString scffilepath;
	    scffilepath << dir << PATH_SEPARATOR << de->d_name;
	  
	    InternalScanPluginDir (subdirMessages, docsys, scffilepath,
	      plugins, metadata, recursive);
  
	    if (subdirMessages != 0)
	    {
	      for (int i = 0; i < subdirMessages->Length(); i++)
	      {
		AppendStrVecString (messages, subdirMessages->Get (i));
	      }
	      subdirMessages->DecRef();
	    }
	  }
	}
      }
    }
    closedir(dh);
  }
}

csRef<iStrVector> csScanPluginDir (const char* dir, 
				   csRef<iStrVector>& plugins,
				   csRefArray<iDocument>& metadata,
				   bool recursive)
{
  iStrVector* messages = 0;

  if (!plugins)
    plugins.AttachNew (new scfStrVector ());

  csRef<iDocumentSystem> docsys = csPtr<iDocumentSystem>
    (new csTinyDocumentSystem ());

  InternalScanPluginDir (messages, docsys, dir, plugins, metadata, 
    recursive);
	 
  return csPtr<iStrVector> (messages);
}

csRef<iStrVector> csScanPluginDirs (csPluginPaths* dirs, 
				    csRef<iStrVector>& plugins,
				    csRefArray<iDocument>& metadata)
{
  iStrVector* messages = 0;

  if (!plugins)
    plugins.AttachNew (new scfStrVector ());

  /*
    TinyXML documents hold references to the document system.
    So we have to create a new csTinyDocumentSystem (). Using just
    'csTinyDocumentSystem docsys' would result in a crash when the
    documents try to DecRef() to already destructed document system.
   */
  csRef<iDocumentSystem> docsys = csPtr<iDocumentSystem>
    (new csTinyDocumentSystem ());

  for (int i = 0; i < dirs->GetCount (); i++)
  {
    iStrVector* dirMessages = 0;
    InternalScanPluginDir (dirMessages, docsys, (*dirs)[i].path, plugins, 
      metadata, (*dirs)[i].scanRecursive);
    
    if (dirMessages != 0)
    {
      csString tmp;
      tmp.Format ("The following error(s) occured while scanning '%s':",
	(*dirs)[i].path);

      AppendStrVecString (messages, tmp);

      for (int i = 0; i < dirMessages->Length(); i++)
      {
	tmp.Format (" %s", dirMessages->Get (i));
	AppendStrVecString (messages, tmp);
      }
      dirMessages->DecRef();
    }
  }
	 
  return csPtr<iStrVector> (messages);
}
