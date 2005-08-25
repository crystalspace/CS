/*
    Load plugin meta data from object file headers.
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Eric Sunshine
	      (C) 2003 by Frank Richter
	      (C) 2003 by Mat Sutcliffe

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
#include "csutil/physfile.h"
#include "csutil/scfstr.h"
#include "csutil/scfstringarray.h"
#include "csutil/syspath.h"
#include "csutil/xmltiny.h"
#include "iutil/document.h"

static void AppendStrVecString (iStringArray*& strings, const char* str)
{
  if (!strings)
  {
    strings = new scfStringArray ();
  }
  strings->Push (str);
}

extern char* csExtractMetadata (const char* fullPath, const char*& errMsg);

csRef<iString> csGetPluginMetadata (const char* fullPath, 
				    csRef<iDocument>& metadata)
{
  csRef<iString> result;
  metadata = 0;
  bool conflict = false;

  int len = strlen (fullPath);
  csString cspluginPath (fullPath);
  cspluginPath.Truncate (len - 3);
  cspluginPath << ".csplugin";

  csRef<iDocumentSystem> docsys =
    csPtr<iDocumentSystem>(new csTinyDocumentSystem ());
  csRef<iDocument> doc = docsys->CreateDocument ();

  char const* errmsg = 0;
  bool usebfd = false;
  char* buf = csExtractMetadata (fullPath, errmsg);
  if (buf)
  {
    usebfd = true;
    errmsg = doc->Parse (buf, true);
    if (errmsg == 0)            // Parse successful.
      metadata = doc;
    delete[] buf;
  }

  csPhysicalFile file (cspluginPath, "rb");
  if (file.GetStatus () == VFS_STATUS_OK)
  {
    if (usebfd)
      conflict = true;
    else
    {
      errmsg = doc->Parse (&file, true);
      if (errmsg == 0)			// Parse successful.
	metadata = doc;
    }
  }

  csString errstr;
  if (conflict)
    errstr << csString().Format ("Warning: %s has embedded data and"
      " .csplugin file, using embedded.%s", fullPath, errmsg ? "\n" : "");
  if (errmsg)
    errstr << csString().Format ("Error parsing metadata in %s: %s",
      usebfd ? fullPath : cspluginPath.GetData (), errmsg);
  if (errstr.Length ())
    result.AttachNew(new scfString (errstr));

  return result;
}
  
void InternalScanPluginDir (iStringArray*& messages,
			    const char* dir, 
			    csRef<iStringArray>& plugins,
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
        if (n >= 3 && strcasecmp(de->d_name + n - 3, ".so") == 0)
        {
	    csString scffilepath;
	    scffilepath << dir << CS_PATH_SEPARATOR << de->d_name;

	    plugins->Push (scffilepath);
        }
      }
      else
      {
	if (recursive && (strcmp (de->d_name, ".") != 0)
	  && (strcmp (de->d_name, "..") != 0))
	{
	  iStringArray* subdirMessages = 0;
	    
	  csString scffilepath;
	  scffilepath << dir << CS_PATH_SEPARATOR << de->d_name;
	  
	  InternalScanPluginDir (subdirMessages, scffilepath,
	    plugins, recursive);
  
	  if (subdirMessages != 0)
	  {
	    for (size_t i = 0; i < subdirMessages->Length(); i++)
	    {
	      AppendStrVecString (messages, subdirMessages->Get (i));
	    }
	    subdirMessages->DecRef();
	  }
        }
      }
    }
    closedir(dh);
  }
}

csRef<iStringArray> csScanPluginDir (const char* dir, 
				   csRef<iStringArray>& plugins,
				   bool recursive)
{
  iStringArray* messages = 0;

  if (!plugins)
    plugins.AttachNew (new scfStringArray ());

  InternalScanPluginDir (messages, dir, plugins, recursive);
 
  return csPtr<iStringArray> (messages);
}

csRef<iStringArray> csScanPluginDirs (csPathsList* dirs, 
				      csRef<iStringArray>& plugins)
{
  iStringArray* messages = 0;

  if (!plugins)
    plugins.AttachNew (new scfStringArray ());

  for (size_t i = 0; i < dirs->Length (); i++)
  {
    iStringArray* dirMessages = 0;
    InternalScanPluginDir (dirMessages, (*dirs)[i].path, plugins, 
      (*dirs)[i].scanRecursive);
    
    if (dirMessages != 0)
    {
      csString tmp;
      tmp.Format ("The following error(s) occured while scanning '%s':",
	(*dirs)[i].path.GetData());

      AppendStrVecString (messages, tmp);

      for (size_t i = 0; i < dirMessages->Length(); i++)
      {
	tmp.Format (" %s", dirMessages->Get (i));
	AppendStrVecString (messages, tmp);
      }
      dirMessages->DecRef();
    }
  }
 
  return csPtr<iStringArray> (messages);
}
