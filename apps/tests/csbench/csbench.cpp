/*
    Copyright (C) 2000 by Jorrit Tyberghein
    With additions by Samuel Humphreys

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
#include "csutil/sysfunc.h"
#include "cstool/initapp.h"
#include "csgeom/vector3.h"
#include "apps/tests/csbench/csbench.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivideo/material.h"
#include "ivaria/conout.h"
#include "igraphic/imageio.h"
#include "igraphic/image.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "csutil/cmdhelp.h"
#include "ivaria/reporter.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "csutil/event.h"
#include "imesh/thing.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/sector.h"
#include "iengine/camera.h"
#include "iengine/material.h"
#include "ivaria/view.h"
#include "cstool/csview.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global system driver
CsBench *System;

CsBench::CsBench ()
{
}

CsBench::~CsBench ()
{
}

void CsBench::Report (const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (System->object_reg, iReporter));
  if (rep)
    rep->ReportV (CS_REPORTER_SEVERITY_NOTIFY, "csbench", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
    fflush (stdout);
  }
  va_end (arg);
}

bool CsBench::ReportError (const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (System->object_reg, iReporter));
  if (rep)
    rep->ReportV (CS_REPORTER_SEVERITY_ERROR, "csbench", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
    fflush (stdout);
  }
  va_end (arg);
  return false;
}

static void Cleanup ()
{
  iObjectRegistry* object_reg = System->object_reg;
  delete System; System = 0;
  csInitializer::DestroyApplication (object_reg);
}

bool CsBench::CreateGeometry ()
{
  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.simple1",
        "Error loading 'stone4' texture!");
    return false;
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room2 = engine->CreateSector ("room2");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room2, "walls"));
  csRef<iThingState> ws =
    SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  walls_state->AddInsideBox (csVector3 (-5, -5, 5), csVector3 (5, 5, 15));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  // Create our object.
  csRef<iMeshFactoryWrapper> fact = engine->CreateMeshFactory (
    "crystalspace.mesh.object.genmesh", "complexmesh");
  csRef<iGeneralFactoryState> factstate = SCF_QUERY_INTERFACE (
    fact->GetMeshObjectFactory (), iGeneralFactoryState);
  //int dim = 128;
  int dim = 64;
  float size = 5.0f;
  factstate->SetVertexCount (dim * dim);
  factstate->SetTriangleCount (2 * (dim-1) * (dim-1));
  int x, y;
  for (y = 0 ; y < dim ; y++)
    for (x = 0 ; x < dim ; x++)
    {
      int vtidx = y * dim + x;
      factstate->GetVertices ()[vtidx].Set (
      	float (x) * size / float (dim),
      	float (y) * size / float (dim),
	0.0f);
      factstate->GetTexels ()[vtidx].Set (
      	float (x) / float (dim),
      	float (y) / float (dim));
    }
  for (y = 0 ; y < dim-1 ; y++)
    for (x = 0 ; x < dim-1 ; x++)
    {
      int vtidx = y * dim + x;
      int tridx = 2 * (y * (dim-1) + x);
      factstate->GetTriangles ()[tridx].Set (vtidx+1, vtidx, vtidx+dim);
      factstate->GetTriangles ()[tridx+1].Set (vtidx+dim+1, vtidx+1, vtidx+dim);
    }

  factstate->CalculateNormals ();

  // Now create an instance:
  csRef<iMeshWrapper> mesh =
    engine->CreateMeshWrapper (fact, "complex", room2, csVector3 (0, 0, 10.0));
  csRef<iGeneralMeshState> meshstate = SCF_QUERY_INTERFACE (
    mesh->GetMeshObject (), iGeneralMeshState);
  meshstate->SetMaterialWrapper (tm);

  csRef<iLight> light;
  iLightList* ll = room2->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 3, 10), 10,
    csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 3,  10), 10,
    csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 3, 7), 10,
    csColor (0, 1, 0));
  ll->Add (light);

  room1 = engine->CreateSector ("room1");
  csVector3 portal_vts[4];
  portal_vts[0].Set (-1.5, 1.5, 0);
  portal_vts[1].Set (1.5, 1.5, 0);
  portal_vts[2].Set (1.5, -1.5, 0);
  portal_vts[3].Set (-1.5, -1.5, 0);
  iPortal* portal;
  csRef<iMeshWrapper> portal_mesh = engine->CreatePortal (
  	"portal_room2", room1, csVector3 (0, 0, 5),
  	room2, portal_vts, 4, portal);

  engine->Prepare ();

  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room1);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());
  return true;
}

bool CsBench::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  // Make sure the commandline has -verbose and -console for consistent
  // results.
  cmdline = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  cmdline->AddOption ("verbose", "true");

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
    return ReportError ("Couldn't initialize app!");

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
        CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
        CS_REQUEST_LEVELLOADER,
	CS_REQUEST_CONSOLEOUT,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
    return ReportError ("Couldn't init app!");

  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!g3d) return ReportError ("No g3d plugin!");
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine) return ReportError ("No engine plugin!");
  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!loader) return ReportError ("No loader plugin!");
  imageio = CS_QUERY_REGISTRY (object_reg, iImageIO);
  if (!imageio) return ReportError ("No image loader plugin!");
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!vfs) return ReportError ("No iVFS plugin!");

  iGraphics2D* g2d = g3d->GetDriver2D ();
  iNativeWindow* nw = g2d->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Graphics Benchmark App");

  vfs->Mount ("/lib/report", "csbench_report.zip");

  csRef<iStandardReporterListener> stdrep = CS_QUERY_REGISTRY (object_reg,
  	iStandardReporterListener);
  if (!stdrep) return ReportError ("No stdrep plugin!");
  stdrep->SetDebugFile ("/tmp/csbench_report.txt");
  stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_BUG,
  	true, false, false, false, true, false);
  stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_ERROR,
  	true, false, false, false, true, false);
  stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_WARNING,
  	true, false, false, false, true, false);
  stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_NOTIFY,
  	true, false, false, false, true, false);
  stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_DEBUG,
  	true, false, false, false, true, false);

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
    return ReportError ("Error opening system!");

  if (!CreateGeometry ())
    return false;

  return true;
}

void CsBench::BenchMark (const char* name, const char* description)
{
  Report ("================================================================");
  Report ("Benchmark %s (%s)...", name, description);

  vc->Advance ();
  csTicks current_time = vc->GetCurrentTicks ();
  int cnt = 0;
  while (vc->GetCurrentTicks () < current_time+5000)
  {
    cnt++;

    g3d->BeginDraw (CSDRAW_CLEARSCREEN | CSDRAW_3DGRAPHICS);
    view->Draw ();
    g3d->FinishDraw ();
    g3d->Print (0);

    vc->Advance ();
  }
  iGraphics2D* g2d = g3d->GetDriver2D ();
  csRef<iImage> shot = csPtr<iImage> (g2d->ScreenShot ());
  if (shot)
  {
    csRef<iDataBuffer> shotbuf = imageio->Save (shot, "image/jpg", "");
    if (shotbuf)
    {
      csString filename;
      filename = "/lib/report/bench_";
      filename += name;
      filename += ".jpg";
      vfs->WriteFile ((const char*)filename, (const char*)shotbuf->GetData (),
      	shotbuf->GetSize ());
    }
  }
  Report ("Performance is %d frames in 5 seconds: %g fps",
  	cnt, float (cnt) / 5.0f);

  // Due to some limitation in VFS we cannot let the stdrep plugin write
  // directly to a file in a zip archive. So at this point we copy the
  // information in /tmp/csbench_report.txt to the zip.
  csRef<iDataBuffer> buf = vfs->ReadFile ("/tmp/csbench_report.txt");
  vfs->WriteFile ("/lib/report/csbench_report.txt",
  	(const char*)buf->GetData (), buf->GetSize ());

  vfs->Sync ();
}

void CsBench::PerformTests ()
{
  Report ("================================================================");
# ifdef CS_DEBUG
  Report ("Crystal Space compiled in debug mode.");
# else
  Report ("Crystal Space compiled in release mode.");
# endif
  Report ("Compiler '%s', platform '%s', processor '%s'",
  	CS_COMPILER_NAME, CS_PLATFORM_NAME, CS_PROCESSOR_NAME);
#ifdef CS_QINT_WORKAROUND
  Report ("QInt() workaround enabled!");
#endif
#ifdef CS_NO_QSQRT
  Report ("qsqrt() disabled!");
#endif

  g3d->SetOption ("StencilThreshold", "0");
  BenchMark ("stencilclip", "Stencil clipping is used");
  g3d->SetOption ("StencilThreshold", "100000000");
  BenchMark ("planeclip", "glClipPlane clipping is used");
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (0));

  // Create our main class.
  System = new CsBench ();

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, 0))
  {
    Cleanup ();
    exit (1);
  }

  // Main loop.
  System->PerformTests ();

  // Cleanup.
  Cleanup ();

  return 0;
}
