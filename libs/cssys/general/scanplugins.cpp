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

// Scan a directory for .csplugin files
csRef<iStrVector> csScanPluginsDir (const char* dir, 
				    csRef<iStrVector>& plugins,
				    csRefArray<iDocument>& metadata)
{
  iStrVector* messages = 0;
  
  plugins.AttachNew (new scfStrVector ());
  metadata.DeleteAll ();
    
  csRef<iDocumentSystem> docsys (
    csPtr<iDocumentSystem> (new csTinyDocumentSystem ()));
  
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
	  //csPhysicalFile file(scffilepath, "rb");

          FILE* file = fopen(scffilepath , "rb");          

          // fopen HACK:
	  /*
	     [res] 
	      One user (Charlls) got weird linking errors. This hack is here
	      to have the stuff working at least, until the problem is 
	      properly sorted out.
		g++ -L/usr/local/lib -lpthread  -Lout/UNIX/X86/optimize  -o 
		  csdemo out/UNIX/X86/optimize/apps/demo/demo.o 
		  out/UNIX/X86/optimize/apps/demo/demoldr.o 
		  out/UNIX/X86/optimize/apps/demo/demoop.o 
		  out/UNIX/X86/optimize/apps/demo/demoseq.o -lcsgfx -lcsutil 
		  -lcstool -lcssys -lcsgeom -lcsutil -lcssys -ldl -lm -lnsl
		out/UNIX/X86/optimize/libcssys.a(scanplugins.o)(.text+0x231): 
		  In function `csScanPluginsDir(char const*, 
		  csRef<iStrVector>&, csRefArray<iDocument>&)':
		: undefined reference to 
		  `csPhysicalFile::csPhysicalFile[in-charge](char const*, 
		  char const*)'
		out/UNIX/X86/optimize/libcssys.a(scanplugins.o)(.text+0x310): 
		  In function `csScanPluginsDir(char const*, 
		  csRef<iStrVector>&, csRefArray<iDocument>&)':
		  : undefined reference to `csPhysicalFile::~csPhysicalFile 
		  [in-charge]()'
		collect2: ld returned 1 exit status
		make[1]: *** [csdemo] Error 1
	   */

          fseek( file , 0 , SEEK_END );
          int i = ftell( file );
          fseek( file , 0 , SEEK_SET );
          //null-terminated
          char*  Buffer = new char [ i+1 ];
          Buffer[ i ]='\0';
          fread( Buffer , 1 , i , file );                
	  
	  fclose (file);
	  
          // End of fopen HACK          

	  csRef<iDocument> doc = docsys->CreateDocument();
	  char const* errmsg = doc->Parse( Buffer );
	  
	  delete[] Buffer;
	  
	  if (errmsg == 0)
	  {
	    metadata.Push (doc);
	    plugins->Push (csStrNew (scffilepath));
	  }  
	  else
	  {
	    csString errstr;
	    errstr.Format ("csInitializer::InitializeSCF: "
	      "Error parsing %s: %s\n", scffilepath.GetData(), errmsg);
	    
	    AppendStrVecString (messages, errstr);
	  }
        }
      }
    }
    closedir(dh);
  }
	 
  return csPtr<iStrVector> (messages);
}
