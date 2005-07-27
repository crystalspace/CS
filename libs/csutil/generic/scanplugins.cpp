/*
    Copyright (C) 2003 by Eric Sunshine
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
#include "csutil/csshlib.h"
#include "csutil/sysfunc.h"
#include "csutil/csstring.h"
#include "csutil/scfstringarray.h"
#include "csutil/physfile.h"
#include "csutil/scfstr.h"
#include "csutil/syspath.h"
#include "csutil/util.h"
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

csRef<iString> csGetPluginMetadata (const char* fullPath, 
				    csRef<iDocument>& metadata)
{
  csRef<iString> result;
  metadata = 0;

  csRef<iDocumentSystem> docsys =
    csPtr<iDocumentSystem>(new csTinyDocumentSystem ());
  csRef<iDocument> doc = docsys->CreateDocument ();

  csPhysicalFile file (fullPath, "rb");
  char const* errmsg = doc->Parse (&file, true);

  if (errmsg == 0)	// Parse successful.
    metadata = doc;
  else			// Parse failed.
  {
    csString errstr;
    errstr.Format ("Error parsing metadata from %s: %s",
      fullPath, errmsg);
    result.AttachNew (new scfString (errstr));
  }

  return result;
}
  
// Scan a directory for .csplugin files
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
        if (n >= 9 && strcasecmp(de->d_name + n - 9, ".csplugin") == 0)
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

  InternalScanPluginDir (messages, dir, plugins, 
    recursive);
	 
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
	(*dirs)[i].path.GetDataSafe ());

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
