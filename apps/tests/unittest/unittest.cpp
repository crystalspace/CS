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

#include <stdarg.h>

#include "cssysdef.h"
#include "iengine/engine.h"
#include "iutil/string.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/dbghelp.h"
#include "iutil/vfs.h"
#include "cstool/initapp.h"
#include "csgeom/math3d.h"
#include "csutil/utiltest.h"
#include "iengine/viscull.h"
#include "ivaria/collider.h"
#include "imap/services.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

static void Test (iBase* obj, const char* name)
{
  if (!obj)
  {
    printf ("Object '%s' is missing!\n", name);
    fflush (stdout);
    return;
  }
  csRef<iDebugHelper> dbghelp (SCF_QUERY_INTERFACE (obj, iDebugHelper));
  if (dbghelp && (dbghelp->GetSupportedTests () & CS_DBGHELP_UNITTEST))
  {
    csRef<iString> str (dbghelp->UnitTest ());
    if (str)
    {
      printf ("%s unit testing failed!\n", name);
      printf ("%s\n", str->GetData ());
    }
    else
    {
      printf ("%s unit testing succeeded!\n", name);
    }
  }
  else
    printf ("%s unit test not performed (object doesn't support it).\n", name);
  fflush (stdout);
}

#if 0
static void Benchmark (iBase* obj, const char* name, int num_iterations)
{
  if (!obj)
  {
    printf ("Object '%s' is missing!\n", name);
    fflush (stdout);
    return;
  }
  csRef<iDebugHelper> dbghelp (SCF_QUERY_INTERFACE (obj, iDebugHelper));
  if (dbghelp && (dbghelp->GetSupportedTests () & CS_DBGHELP_BENCHMARK))
  {
    csTicks t = dbghelp->Benchmark (num_iterations);
    printf ("Benchmarking %s: %u ms\n", name, t);
  }
  else
    printf ("%s benchmark not performed (object doesn't support it).\n", name);
  fflush (stdout);
}
#endif

static int DoStuff (iObjectRegistry* object_reg)
{
  csRef<iPluginManager> plugmgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  if (!plugmgr)
  {
    csInitializer::DestroyApplication (object_reg);
    return -1;
  }

  printf ("================================================================\n");

  csRef<iCollideSystem> cdsys (CS_LOAD_PLUGIN (plugmgr,
  	"crystalspace.collisiondetection.opcode", iCollideSystem));
  Test (cdsys, "Opcode");

  printf ("================================================================\n");

  csRef<iEngine> engine (CS_QUERY_REGISTRY (object_reg, iEngine));
  Test (engine, "Engine");

  printf ("================================================================\n");

  csGeomDebugHelper* geomdbghelp = new csGeomDebugHelper ();
  Test (geomdbghelp, "csgeom");
  delete geomdbghelp;

  printf ("================================================================\n");

  csUtilDebugHelper* utildbghelp = new csUtilDebugHelper ();
  Test (utildbghelp, "csutil");
  delete utildbghelp;

  printf ("================================================================\n");

  csRef<iSyntaxService> syntax (CS_LOAD_PLUGIN (plugmgr,
	"crystalspace.syntax.loader.service.text", iSyntaxService));
  Test (syntax, "Syntax Services");

  printf ("================================================================\n");

  csRef<iVisibilityCuller> viscull (CS_LOAD_PLUGIN (plugmgr,
  	"crystalspace.culling.dynavis", iVisibilityCuller));
  Test (viscull, "DynaVis");
  //Benchmark (viscull, "DynaVis", 100);

  printf ("================================================================\n");
  return 0;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  // Initialize the random number generator
  srand (time (0));

  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg)
    return -1;
  if (!csInitializer::RequestPlugins (object_reg,
	CS_REQUEST_VFS,
	CS_REQUEST_ENGINE,
	CS_REQUEST_END))
  {
    csInitializer::DestroyApplication (object_reg);
    return -1;
  }

  int rc = DoStuff (object_reg);

  csInitializer::DestroyApplication (object_reg);
  return rc;
}

