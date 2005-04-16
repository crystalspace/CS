/*
    Copyright (C) 2004 by Jorrit Tyberghein
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
#include "csutil/xmltiny.h"
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
#include "iengine/renderloop.h"
#include "iengine/rendersteps/irsfact.h"
#include "iengine/rendersteps/igeneric.h"
#include "iengine/rendersteps/irenderstep.h"
#include "iengine/rendersteps/icontainer.h"
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

iMeshFactoryWrapper* CsBench::CreateGenmeshLattice (int dim, float size,
	const char* name)
{
  // Create our object.
  csRef<iMeshFactoryWrapper> fact = engine->CreateMeshFactory (
    "crystalspace.mesh.object.genmesh", name);
  csRef<iGeneralFactoryState> factstate = SCF_QUERY_INTERFACE (
    fact->GetMeshObjectFactory (), iGeneralFactoryState);
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
  return fact;
}

bool CsBench::SetupMaterials ()
{
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
  if (!loader->LoadTexture ("stone_normal", "/lib/stdtex/stone2DOT3.png", 
  	CS_TEXTURE_3D, 0, false, false))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.simple1",
        "Error loading 'stone2DOT3' texture!");
    return false;
  }
  material = engine->GetMaterialList ()->FindByName ("stone");
  csShaderVariable* normalSV = 
    material->GetMaterial()->GetVariableAdd (strings->Request ("tex normal"));
  iTextureWrapper* stoneDot3 = 
    engine->GetTextureList()->FindByName ("stone_normal");
  stoneDot3->SetTextureClass ("normalmap");
  normalSV->SetValue (stoneDot3);
  return true;
}

iSector* CsBench::CreateRoom (const char* name, const char* meshname,
	const csVector3& p1, const csVector3& p2)
{
  iSector* room2 = engine->CreateSector (name);
  csRef<iMeshWrapper> walls = engine->CreateSectorWallsMesh (room2, meshname);
  csRef<iThingState> ws =
    SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  walls_state->AddInsideBox (p1, p2);
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, material);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);
  return room2;
}

bool CsBench::CreateTestCaseSingleBigObject ()
{
  iSector* room2 = CreateRoom ("room2_single", "room_single",
  	csVector3 (-5, -5, 5), csVector3 (5, 5, 15));
  // Create our factory.
  iMeshFactoryWrapper* fact = CreateGenmeshLattice (BIGOBJECT_DIM,
  	5.0f, "complexmesh");
  if (!fact) return false;

  // Now create an instance:
  csRef<iMeshWrapper> mesh =
    engine->CreateMeshWrapper (fact, "complex", room2, csVector3 (0, 0, 10.0));
  csRef<iGeneralMeshState> genmesh = 
    SCF_QUERY_INTERFACE (mesh->GetMeshObject (), iGeneralMeshState);
  genmesh->SetMaterialWrapper (material);
  
  gmSingle = genmesh;

  csRef<iLight> l;
  iLightList* ll = room2->GetLights ();
  l = engine->CreateLight (0, csVector3 (-3, 3, 10), 10, csColor (1, 0, 0));
  ll->Add (l);
  l = engine->CreateLight (0, csVector3 (3, 3,  10), 10, csColor (0, 0, 1));
  ll->Add (l);
  l = engine->CreateLight (0, csVector3 (0, 3, 7), 10, csColor (0, 1, 0));
  ll->Add (l);

  room_single = engine->CreateSector ("room_single");
  csVector3 portal_vts[4];
  portal_vts[0].Set (-1.5, 1.5, 0);
  portal_vts[1].Set (1.5, 1.5, 0);
  portal_vts[2].Set (1.5, -1.5, 0);
  portal_vts[3].Set (-1.5, -1.5, 0);
  iPortal* portal;
  csRef<iMeshWrapper> portal_mesh = engine->CreatePortal (
  	"portal_room2_single", room_single, csVector3 (0, 0, 5),
  	room2, portal_vts, 4, portal);
  return true;
}

bool CsBench::CreateTestCaseMultipleObjects ()
{
  iSector* room2 = CreateRoom ("room2_multi", "room_multi",
  	csVector3 (-5, -5, 5), csVector3 (5, 5, 15));
  // Create our factory.
  iMeshFactoryWrapper* fact = CreateGenmeshLattice (SMALLOBJECT_DIM,
  	4.0f, "smallmesh");
  if (!fact) return false;

  // Now create the instances:
  int i;
  csVector3 p (-9, -9, 9);
  float step = 18.0f / float (SMALLOBJECT_NUM);
  float zstep = 2.0f / float (SMALLOBJECT_NUM);
  for (i = 0 ; i < SMALLOBJECT_NUM ; i++)
  {
    csRef<iMeshWrapper> mesh =
      engine->CreateMeshWrapper (fact, "small", room2, p);
    csRef<iGeneralMeshState> genmesh = 
      SCF_QUERY_INTERFACE (mesh->GetMeshObject (), iGeneralMeshState);
    genmesh->SetMaterialWrapper (material);
    p.x += step;
    p.y += step;
    p.z += zstep;
  }

  csRef<iLight> l;
  iLightList* ll = room2->GetLights ();
  l = engine->CreateLight (0, csVector3 (-3, 3, 10), 10, csColor (1, 0, 0));
  ll->Add (l);
  l = engine->CreateLight (0, csVector3 (3, 3,  10), 10, csColor (0, 0, 1));
  ll->Add (l);
  l = engine->CreateLight (0, csVector3 (0, 3, 7), 10, csColor (0, 1, 0));
  ll->Add (l);

  room_multi = engine->CreateSector ("room_multi");
  csVector3 portal_vts[4];
  portal_vts[0].Set (-1.5, 1.5, 0);
  portal_vts[1].Set (1.5, 1.5, 0);
  portal_vts[2].Set (1.5, -1.5, 0);
  portal_vts[3].Set (-1.5, -1.5, 0);
  iPortal* portal;
  csRef<iMeshWrapper> portal_mesh = engine->CreatePortal (
  	"portal_room2_multi", room_multi, csVector3 (0, 0, 5),
  	room2, portal_vts, 4, portal);
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
  cmdline->AddOption ("verbose", "-scf");
  cmdline->AddOption ("console", 0);

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
  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.shared.stringset", iStringSet);
  if (!strings) return ReportError ("No string set!");

  iGraphics2D* g2d = g3d->GetDriver2D ();
  iNativeWindow* nw = g2d->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Graphics Benchmark App");

  unlink ("csbench_report.zip");
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

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  if (!SetupMaterials ()) return false;
  if (!CreateTestCaseSingleBigObject ()) return false;
  if (!CreateTestCaseMultipleObjects ()) return false;

  engine->Prepare ();

  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room_single);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  return true;
}

float CsBench::BenchMark (const char* name, const char* description, 
			 uint drawFlags)
{
  Report ("================================================================");
  Report ("Benchmark %s (%s)...", name, description);

  vc->Advance ();
  csTicks current_time = vc->GetCurrentTicks ();
  int cnt = 0;
  while (vc->GetCurrentTicks () < current_time+BENCHTIME)
  {
    cnt++;

    g3d->BeginDraw (CSDRAW_CLEARSCREEN | CSDRAW_3DGRAPHICS | drawFlags);
    view->Draw ();
    g3d->FinishDraw ();
    g3d->Print (0);

    vc->Advance ();
  }
  iGraphics2D* g2d = g3d->GetDriver2D ();
  csRef<iImage> shot = csPtr<iImage> (g2d->ScreenShot ());
  if (shot)
  {
    csRef<iDataBuffer> shotbuf = imageio->Save (shot, "image/jpg", 
      "compress=10");
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
  float fps = float (cnt * 1000) / float (BENCHTIME);
  Report ("PERF:%s:%d:%d:%g: (%d frames in %d seconds: %g fps)",
  	name, cnt, BENCHTIME / 1000, fps,
  	cnt, BENCHTIME / 1000, fps);

  // Due to some limitation in VFS we cannot let the stdrep plugin write
  // directly to a file in a zip archive. So at this point we copy the
  // information in /tmp/csbench_report.txt to the zip.
  csRef<iDataBuffer> buf = vfs->ReadFile ("/tmp/csbench_report.txt");
  vfs->WriteFile ("/lib/report/csbench_report.txt",
  	(const char*)buf->GetData (), buf->GetSize ());

  vfs->Sync ();
  fflush (stdout);
  return fps;
}

iShaderManager* CsBench::GetShaderManager ()
{
  if (!shader_mgr)
  {
    shader_mgr = CS_QUERY_REGISTRY (object_reg, iShaderManager);
  }
  return shader_mgr;
}

iDocumentSystem* CsBench::GetDocumentSystem ()
{
  if (!docsys)
  {
    docsys = CS_QUERY_REGISTRY(object_reg, iDocumentSystem);
    if (docsys == 0)
    {
      docsys.AttachNew (new csTinyDocumentSystem ());
    }
  }
  return docsys;
}

void CsBench::PerformShaderTest (const char* shaderPath, const char* shtype, 
				 const char* shaderPath2, const char* shtype2,
				 iGeneralMeshState* genmesh)
{
  csRef<iShaderCompiler> shcom = GetShaderManager ()->GetCompiler ("XMLShader");
  csRef<iDocument> shaderDoc = GetDocumentSystem ()->CreateDocument ();
  csRef<iFile> shaderFile = vfs->Open (shaderPath, VFS_FILE_READ);
  shaderDoc->Parse (shaderFile, true);
  csRef<iDocumentNode> shadernode = shaderDoc->GetRoot ()->GetNode ("shader");
  csStringID shadertype = strings->Request (shtype);

  csStringID shadertype2 = 0;
  csRef<iShader> shader2;
  if (shaderPath2)
  {
    csRef<iDocument> shaderDoc2 = GetDocumentSystem ()->CreateDocument ();
    csRef<iFile> shaderFile2 = vfs->Open (shaderPath2, VFS_FILE_READ);
    shaderDoc2->Parse (shaderFile2, true);
    csRef<iDocumentNode> shadernode2 = shaderDoc2->GetRoot ()
    	->GetNode ("shader");
    shadertype2 = strings->Request (shtype2);
    shader2 = shcom->CompileShader (shadernode2);
  }

  csRef<iShaderPriorityList> prilist = shcom->GetPriorities (shadernode);
  size_t i;
  iMaterialWrapper* oldmat = material;
  //csRef<iMeshWrapper> walls (engine->FindMeshObject ("walls"));
  //csRef<iMeshWrapper> walls (
    //view->GetCamera()->GetSector()->GetMeshes()->FindByName ("walls"));
  csRef<iMeshWrapper> walls (engine->FindMeshObject (
    view->GetCamera()->GetSector()->QueryObject()->GetName()));
  csRef<iThingState> ws =
    SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  for (i = 0 ; i < prilist->GetCount () ; i++)
  {
    int pri = prilist->GetPriority (i);
    csRef<iShader> shader = shcom->CompileShader (shadernode, pri);
    if (shader)
    {
      csRef<iMaterial> matinput = engine->CreateBaseMaterial (
	engine->GetTextureList ()->FindByName ("stone"));
      csShaderVariable* normalSV = 
	matinput->GetVariableAdd (strings->Request ("tex normal"));
      normalSV->SetValue (engine->GetTextureList()->FindByName (
      	"stone_normal"));
      matinput->SetShader (shadertype, shader);
      if (shader2)
	matinput->SetShader (shadertype2, shader2);
      iMaterialWrapper* mat = engine->GetMaterialList ()->NewMaterial (
      	matinput, 0);
      genmesh->SetMaterialWrapper (mat);
      ws->ReplaceMaterial (oldmat, mat);

      csString name;
      name.Format ("%s_%d", shader->QueryObject()->GetName(), pri);
      csString description;
      description.Format ("Shader %s with priority %d", 
	shader->QueryObject()->GetName(), pri);
      BenchMark (name, description, CSDRAW_CLEARZBUFFER);
      ws->ClearReplacedMaterials();
    }
  }
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
#ifdef CS_NO_QSQRT
  Report ("csQsqrt() disabled!");
#endif
  Report ("Big object in test has %d triangles.",
  	2*(BIGOBJECT_DIM-1)*(BIGOBJECT_DIM-1));
  Report ("Small object in test has %d triangles. We use %d of them.",
  	2*(SMALLOBJECT_DIM-1)*(SMALLOBJECT_DIM-1), SMALLOBJECT_NUM);

  g3d->SetOption ("StencilThreshold", "0");
  view->GetCamera ()->SetSector (room_single);
  float stencil0 = BenchMark ("stencilclip_single", 
    "Stencil clipping, single object");
  view->GetCamera ()->SetSector (room_multi);
  BenchMark ("stencilclip_multi", "Stencil clipping, multiple objects");

  g3d->SetOption ("StencilThreshold", "100000000");
  view->GetCamera ()->SetSector (room_single);
  float stencil1 = BenchMark ("planeclip_single", "glClipPlane, single object");
  view->GetCamera ()->SetSector (room_multi);
  BenchMark ("planeclip_multi", "glClipPlane, multiple objects");
  // Set back to stencil0 if that method was faster.
  if (stencil0 > stencil1)
  {
    Report ("Switching back to stencil thresshold 0.");
    g3d->SetOption ("StencilThreshold", "0");
  }

  view->GetCamera ()->SetSector (room_single);
  PerformShaderTest ("/shader/std_lighting.xml", "standard", 0, 0, gmSingle);

  iRenderLoopManager* rlmgr = engine->GetRenderLoopManager ();
  csRef<iRenderLoop> loop = rlmgr->Load ("/shader/std_rloop_diffuse.xml");
  engine->SetCurrentDefaultRenderloop (loop);

  PerformShaderTest ("/shader/light_bumpmap.xml", "diffuse", 
    "/shader/ambient.xml", "ambient", gmSingle);
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
