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

TerrBigTool::TerrBigTool (iObjectRegistry* object_reg)
{
  TerrBigTool::object_reg = object_reg;
  if (!csInitializer::RequestPlugins (object_reg,
        CS_REQUEST_VFS,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_END)) {
    ReportError ("Unable to request plugins\n");
    abort ();
  }
  
  cmdline = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  if (cmdline == 0)
  {
    ReportError ("Cannot query command line parser\n");
    abort ();
  }
  cmdline->AddOption ("scale_x", "1.0");
  cmdline->AddOption ("scale_y", "10.0");
  cmdline->AddOption ("scale_z", "1.0");
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (vfs == 0)
  {
    ReportError ("Unable to get vfs plugin\n");
    abort ();	
  }
  imageio = CS_QUERY_REGISTRY (object_reg, iImageIO);
  if (imageio == 0)
  {
    ReportError ("Unable to get imageio plugin\n");
    abort ();	
  }
}

TerrBigTool::~TerrBigTool ()
{
}

bool TerrBigTool::Init ()
{
  const char *scalestr;
  if ((scalestr = cmdline->GetOption ("scale_x")) == 0) {
    scale.x = 1.0;
  } else {
    sscanf (scalestr, "%f", &scale.x);
  }
  if ((scalestr = cmdline->GetOption ("scale_y")) == 0) {
    scale.y = 10.0;
  } else {
    sscanf (scalestr, "%f", &scale.y);
  }
  if ((scalestr = cmdline->GetOption ("scale_z")) == 0) {
    scale.z = 1.0;
  } else {
    sscanf (scalestr, "%f", &scale.z);
  }

  if (!cmdline->GetName(0) || !cmdline->GetName(1))
  {
    ReportError ("Format tbtool inputfile outputfile\n");
    return false;
  }
  input = vfs->Open (cmdline->GetName(0), VFS_FILE_MODE | VFS_FILE_READ);
  if (!input)
  {
    ReportError ("Unable to open %s on vfs\n", cmdline->GetName(0));
    return false;
  }
  return true;
}

bool TerrBigTool::Convert ()
{
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  csRef<iMeshObjectType> terrtype (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.terrbig", iMeshObjectType));
  if (!terrtype)
    terrtype = CS_LOAD_PLUGIN (plugin_mgr,
	"crystalspace.mesh.object.terrbig", iMeshObjectType); 
  if (!terrtype)
  {
    ReportError ("Cannot find big terrain plugin\n");
    abort ();
  }

  csRef<iMeshObjectFactory> terrfact (terrtype->NewFactory ());
  terrfact->IncRef ();	// @@@ MEMORY LEAK!!! Investigate.
  csRef<iMeshObject> terrobj (terrfact->NewInstance ());
  terrobj->IncRef ();	// @@@ MEMORY LEAK!!!
  csRef<iTerrBigState> terrstate (SCF_QUERY_INTERFACE (terrobj, iTerrBigState));

  terrstate->SetScaleFactor (scale);
  terrstate->ConvertImageToMapFile (input, imageio, cmdline->GetName(1));

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

int main (int argc, char **argv)
{
  iObjectRegistry* object_reg;
  if ((object_reg = csInitializer::CreateEnvironment (argc, argv)) == 0)
  {
    printf ("Cannot create environment!\n");
    return -1;
  }

  int rc;
  {
    TerrBigTool tbt (object_reg);
    if (tbt.Init() && tbt.Convert())
      rc = 0;
    else
      rc = -1;
  }
  csInitializer::DestroyApplication (object_reg);
  return rc;
}

