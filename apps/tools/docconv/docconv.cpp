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

#include <stdarg.h>

#include "cssysdef.h"
#include "docconv.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "csutil/csstring.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/cmdline.h"
#include "iutil/document.h"
#include "cstool/initapp.h"
#include "ivaria/reporter.h"

//-----------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

void DocConv::ReportError (const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR,
  	"crystalspace.apps.docconv", description, arg);
  va_end (arg);
}

void DocConv::Report (int severity, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, severity,
  	"crystalspace.apps.docconv", description, arg);
  va_end (arg);
}

//-----------------------------------------------------------------------------

void DocConv::CloneNode (iDocumentNode* from, iDocumentNode* to)
{
  to->SetValue (from->GetValue ());
  csRef<iDocumentNodeIterator> it = from->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	child->GetType (), 0);
    CloneNode (child, child_clone);
  }
  csRef<iDocumentAttributeIterator> atit = from->GetAttributes ();
  while (atit->HasNext ())
  {
    csRef<iDocumentAttribute> attr = atit->Next ();
    to->SetAttribute (attr->GetName (), attr->GetValue ());
  }
}

//-----------------------------------------------------------------------------

DocConv::DocConv ()
{
  object_reg = 0;
}

DocConv::~DocConv ()
{
}

//----------------------------------------------------------------------------

#define OP_HELP 0
#define OP_TRANSLATE 1

void DocConv::Main ()
{
  cmdline = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  csRef<iPluginManager> plugin_mgr = 
    CS_QUERY_REGISTRY (object_reg, iPluginManager);

  int op = OP_HELP;

  if (cmdline->GetOption ("inds") || cmdline->GetOption ("outds"))
    op = OP_TRANSLATE;

  if (cmdline->GetOption ("help")) op = OP_HELP;

  if (op == OP_HELP)
  {
    csPrintf ("docconv <options> <zipfile|filename>\n");
    csPrintf ("  -inds=<plugin>:\n");       
    csPrintf ("     Document system plugin for reading world.\n");
    csPrintf ("  -outds=<plugin>:\n");       
    csPrintf ("     Document system plugin for writing world.\n");
    return;
  }

  csRef<iDocumentSystem> inputDS;
  csRef<iDocumentSystem> outputDS;
  {
    const char *inds = cmdline->GetOption ("inds");
    if (inds)
    {
/*      inputDS = csPtr<iDocumentSystem> (CS_LOAD_PLUGIN (plugin_mgr,
	inds, iDocumentSystem));
      if (!inputDS)*/
      {
        inputDS = csPtr<iDocumentSystem> (CS_LOAD_PLUGIN (plugin_mgr,
	  csString().Format ("crystalspace.documentsystem.%s", inds),
	  iDocumentSystem));
      }
      if (!inputDS)
      {
	ReportError ("Unable to load input document system '%s'!",
	  inds);
        return;
      }
    }

    const char *outds = cmdline->GetOption ("outds");
    if (outds)
    {
/*      outputDS = csPtr<iDocumentSystem> (CS_LOAD_PLUGIN (plugin_mgr,
	outds, iDocumentSystem));
      if (!outputDS)*/
      {
	outputDS = csPtr<iDocumentSystem> (CS_LOAD_PLUGIN (plugin_mgr,
	  csString().Format ("crystalspace.documentsystem.%s", outds),
	  iDocumentSystem));
      }
      if (!outputDS)
      {
	ReportError ("Unable to load output document system '%s'!",
	  outds);
        return;
      }
    }
  }

  const char* val = cmdline->GetName ();
  if (!val)
  {
    ReportError ("Please give VFS world file name or name of the zip archive! "
      "Use 'docconv -help' to get a list of possible options.");
    return;
  }

  csString filename;
  if (strstr (val, ".zip"))
  {
    vfs->Mount ("/tmp/docconv_data", val);
    filename = "/tmp/docconv_data/world";
  }
  else
  {
    filename = val;
  }

  vfs->ChDir ("/this");
  csRef<iDataBuffer> buf = vfs->ReadFile (filename);
  if (!buf || !buf->GetSize ())
  {
    ReportError ("File '%s' does not exist!", (const char*)filename);
    return;
  }

  // Make backup.
  vfs->WriteFile (filename+".bak", **buf, buf->GetSize ());

  csRef<iDocumentSystem> xml;
  if (inputDS)
  {
    xml = inputDS;
  }
  else
  {
    xml = csPtr<iDocumentSystem> (CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.documentsystem.multiplex", iDocumentSystem));
    if (xml == 0)
      xml = (csPtr<iDocumentSystem> (
	new csTinyDocumentSystem ()));
  }
  csRef<iDocument> doc = xml->CreateDocument ();
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Parsing...");
  csTicks parse_start = csGetTicks();
  const char* error = doc->Parse (buf, true);
  csTicks parse_end = csGetTicks();
  buf = 0;
  if (error != 0)
  {
    ReportError ("Error parsing document: %s!", error);
    return;
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
      (float)(parse_end - parse_start) / (float)1000);
  }

  //---------------------------------------------------------------

  csRef<iDocumentSystem> newsys;
  if (outputDS)
  {
    newsys = outputDS;
  }
  else
  {
    newsys = (csPtr<iDocumentSystem> (
      new csTinyDocumentSystem ()));
  }

  csRef<iDocumentNode> root = doc->GetRoot ();

  switch (op)
  {
    case OP_TRANSLATE:
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Cloning...");
	csTicks cloning_start = csGetTicks();
	csRef<iDocumentNode> root = doc->GetRoot ();
	csRef<iDocument> newdoc = newsys->CreateDocument ();
	csRef<iDocumentNode> newroot = newdoc->CreateRoot ();
	CloneNode (root, newroot);
	csTicks cloning_end = csGetTicks();
	Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
	  (float)(cloning_end - cloning_start) / (float)1000);
	root = 0;
	doc = 0;
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Writing...");
	csTicks writing_start = csGetTicks();
        error = newdoc->Write (vfs, filename);
	csTicks writing_end = csGetTicks();
	if (error != 0)
	{
	  ReportError ("Error writing '%s': %s!", (const char*)filename, error);
	  return;
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
	    (float)(writing_end - writing_start) / (float)1000);
	}
	newroot = 0;
	newdoc = 0;
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Updating VFS...");
	vfs->Sync();
      }
      break;
  }

  //---------------------------------------------------------------
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  // Initialize the random number generator
  srand (time (0));

  DocConv* dc = new DocConv ();

  iObjectRegistry* object_reg;
  dc->object_reg = csInitializer::CreateEnvironment (argc, argv);
  object_reg = dc->object_reg;
  if (!dc->object_reg)
    return -1;
  if (!csInitializer::RequestPlugins (dc->object_reg,
	CS_REQUEST_VFS,
	CS_REQUEST_END))
  {
    delete dc;
    return -1;
  }

  dc->Main ();
  delete dc;

  csInitializer::DestroyApplication (object_reg);

  return 0;
}

