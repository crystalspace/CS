/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "cstool/initapp.h"
#include "csutil/scf.h"
#include "csutil/plugmgr.h"
#include "iutil/vfs.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "imesh/object.h"
#include "imesh/terrbig.h"
#include "ivaria/reporter.h"
#include "igraphic/imageio.h"
#include "tbconv.h"

CS_IMPLEMENT_APPLICATION

TerrBigTool::TerrBigTool (int argc, char **argv) 
{
  if ((object_reg = csInitializer::CreateEnvironment (argc, argv)) == NULL) {
    ReportError ("Cannot create environment\n");
    abort ();
  }
  if (!csInitializer::RequestPlugins (object_reg,
        CS_REQUEST_VFS,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_END)) {
    ReportError ("Unable to request plugins\n");
    abort ();
  }
  
  if ((cmdline = CS_QUERY_REGISTRY (object_reg, iCommandLineParser)) == NULL) {
    ReportError ("Cannot query command line parser\n");
    abort ();
  }
  cmdline->Initialize (argc, argv);
  cmdline->AddOption ("scale", "10.0");
  if ((vfs = CS_QUERY_REGISTRY (object_reg, iVFS)) == NULL) {
    ReportError ("Unable to get vfs plugin\n");
    abort ();	
  }
  if ((imageio = CS_QUERY_REGISTRY (object_reg, iImageIO)) == NULL) {
    ReportError ("Unable to get imageio plugin\n");
    abort ();	
  }
}

TerrBigTool::~TerrBigTool ()
{
  if (imageio) { imageio->DecRef(); }
  if (vfs) { vfs->DecRef(); }	
  if (cmdline) { cmdline->DecRef(); }
  csInitializer::DestroyApplication (object_reg);
}

bool TerrBigTool::Init ()
{
  float scale;
  const char *scalestr;
  if ((scalestr = cmdline->GetOption ("scale")) == NULL) {
    scale = 10.0;
  } else {
  	sscanf (scalestr, "%f", &scale);
  }
  if (!cmdline->GetName(0) || !cmdline->GetName(1)) {
    ReportError ("Format tbtool inputfile outputfile\n");
    return false;
  }
  input = vfs->Open (cmdline->GetName(0), VFS_FILE_MODE | VFS_FILE_READ);
  if (!input) {
    ReportError ("Unable to open %s on vfs\n", cmdline->GetName(0));
    return false;
  }
  return true;
}

bool TerrBigTool::Convert ()
{
  iPluginManager *plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iMeshObjectType *terrtype = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.terrbig", iMeshObjectType);
  if (!terrtype)
    terrtype = CS_LOAD_PLUGIN (plugin_mgr,
	"crystalspace.mesh.object.terrbig", iMeshObjectType); 
  if (!terrtype) {
    ReportError ("Cannot find big terrain plugin\n");
    abort ();
  }

  csRef<iMeshObjectFactory> terrfact (terrtype->NewFactory ());
  terrfact->IncRef ();	// @@@ MEMORY LEAK!!! Investigate.
  csRef<iMeshObject> terrobj (terrfact->NewInstance ());
  terrobj->IncRef ();	// @@@ MEMORY LEAK!!!
  csRef<iTerrBigState> terrstate (SCF_QUERY_INTERFACE (terrobj, iTerrBigState));

  terrstate->ConvertImageToMapFile (input, imageio, cmdline->GetName(1));

  terrtype->DecRef();
  plugin_mgr->DecRef();

  return true;
}

void TerrBigTool::ReportError (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR,
  	"crystalspace.apps.tbtool", description, arg);
  va_end (arg);
}

int main (int argc, char **argv) {
  TerrBigTool tbt (argc, argv);
  if (tbt.Init() && tbt.Convert()) {
    return 0;
  } else {
    return -1;
  }
}
