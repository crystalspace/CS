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

#define CS_SYSDEF_PROVIDE_DIR
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "cssys/sysfunc.h"
#include "csutil/csstring.h"
#include "csutil/scfstringarray.h"
#include "csutil/physfile.h"
#include "csutil/scfstr.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "iutil/document.h"

#include <string.h>
#include <bfd.h>

static void AppendStrVecString (iStringArray*& strings, const char* str)
{
  if (!strings)
  {
    strings = new scfStringArray ();
  }
  strings->Push (csStrNew (str));
}

csRef<iString> csGetPluginMetadata (const char* fullPath, 
				    csRef<iDocument>& metadata)
{
  iString* result = 0;

  if (!metadata)
  {
    // Since TinyXML documents maintain references to the document system, we
    // must allocate the document system via the heap, rather than the stack.
    csRef<iDocumentSystem> docsys = csPtr<iDocumentSystem>
      (new csTinyDocumentSystem ());
    metadata = docsys->CreateDocument ();
  }
  char const* errmsg;

  int len = strlen (fullPath);
  if (len >= 3 && strcasecmp (fullPath + len - 3, ".so") == 0)
  {
    bfd *abfd = bfd_openr (fullPath, 0);
    if (! abfd) errmsg = "libbfd can't open file";
    else
    {
      if (! bfd_check_format (abfd, bfd_object))
        errmsg = "libbfd can't parse file";
      else
      {
        asection *sect = bfd_get_section_by_name (abfd, ".crystal");
        if (! sect) errmsg = "libbfd can't find '.crystal' section";
        else
        {
          int size = bfd_section_size (abfd, sect);
          char *buf = (char *) malloc (size + 1);
          if (!buf)
	    errmsg = "can't allocate memory for metadata";
          else
	  {
	    if (! bfd_get_section_contents (abfd, sect, buf, 0, size))
	      errmsg = "libbfd can't get section contents";
	    else
	    {
	      buf[size] = 0;
	      errmsg = metadata->Parse (buf);
	    }
            free (buf);
          }
        }
      }
      bfd_close (abfd);
    }
  }
  else
  {
    csPhysicalFile file (fullPath, "rb");
    errmsg = metadata->Parse (&file);
  }
  if (errmsg != 0)
  {
    metadata = 0;

    csString errstr;
    errstr.Format ("Error parsing metadata from %s: %s",
      fullPath, errmsg);

    result = new scfString (errstr);
  }

  return csPtr<iString> (result);
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
        if (n >= 9 && strcasecmp(de->d_name + n - 9, ".csplugin") == 0)
        {
          csString csp (de->d_name);
          csp.Truncate (n - 9);
          csp.Append (".so");
          csp.Insert (0, PATH_SEPARATOR);
          csp.Insert (0, dir);

printf("* %s\n", csp.GetData());
          if (plugins->FindContent (csp) == -1)
          {
	    csString scffilepath;
	    scffilepath << dir << PATH_SEPARATOR << de->d_name;
	  
	    plugins->Push (csStrNew (scffilepath));
          }
        }
        else if (n >= 3 && strcasecmp(de->d_name + n - 3, ".so") == 0)
        {
          static bool init = true;
          if (init)
          {
            bfd_init ();
            init = false;
          }

          bfd *abfd = bfd_openr (de->d_name, 0);
          if (abfd)
          {
            if (bfd_check_format (abfd, bfd_object)
             && bfd_get_section_by_name (abfd, ".crystal"))
            {
              csString csp (de->d_name);
              csp.Truncate (n - 3);
              csp.Append (".csplugin");
              csString cspath (csp);
              cspath.Insert (0, PATH_SEPARATOR);
              cspath.Insert (0, dir);

              struct stat tmp;
              if (stat (cspath, & tmp) == 0)
              {
                if (! messages) messages = new scfStringArray;
                csString message;
                message.Format ("Warning: both %s and %s found, using %s.",
                  de->d_name, csp.GetData (), de->d_name);
                messages->Push (csStrNew (message));
                int index = plugins->Find (csp);
                if (index != -1) plugins->DeleteIndex (index);
              }

	      csString scffilepath;
	      scffilepath << dir << PATH_SEPARATOR << de->d_name;
	  
	      plugins->Push (csStrNew (scffilepath));
            }

            bfd_close (abfd);
          }
        }
      }
      else
      {
	if (recursive && (strcmp (de->d_name, ".") != 0)
	  && (strcmp (de->d_name, "..") != 0))
	{
	  iStringArray* subdirMessages = 0;
	    
	  csString scffilepath;
	  scffilepath << dir << PATH_SEPARATOR << de->d_name;
	  
	  InternalScanPluginDir (subdirMessages, scffilepath,
	    plugins, recursive);
  
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
 
for (int i = 0; i < plugins->Length (); i++) printf("%s\n", plugins->Get(i));
  return csPtr<iStringArray> (messages);
}

csRef<iStringArray> csScanPluginDirs (csPluginPaths* dirs, 
				    csRef<iStringArray>& plugins)
{
  iStringArray* messages = 0;

  if (!plugins)
    plugins.AttachNew (new scfStringArray ());

  for (int i = 0; i < dirs->GetCount (); i++)
  {
    iStringArray* dirMessages = 0;
    InternalScanPluginDir (dirMessages, (*dirs)[i].path, plugins, 
      (*dirs)[i].scanRecursive);
    
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
 
for (int i = 0; i < plugins->Length (); i++) printf("%s\n", plugins->Get(i));
  return csPtr<iStringArray> (messages);
}
