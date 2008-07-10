/*
 * appwaterdemo2.cpp
 *
 * Definition of AppWaterdemo2, the main application object.
 */

#include "appwaterdemo2.h"
#include <cstool/initapp.h>
#include <csutil/csstring.h>
#include <csutil/event.h>
#include <csutil/sysfunc.h>
#include <csutil/syspath.h>
#include <iutil/event.h>
#include <iutil/eventq.h>
#include <iutil/vfs.h>
#ifdef USE_CEL
#include <celtool/initapp.h>
#endif

#include <imesh/watermesh.h>

AppWaterdemo2::AppWaterdemo2() : csApplicationFramework()
{
  SetApplicationName("waterdemo2");
}

AppWaterdemo2::~AppWaterdemo2()
{
}

void AppWaterdemo2::ProcessFrame()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  iCamera* c = view->GetCamera();
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the arrow keys will cause
    // the camera to strafe up, down, left or right from it's
    // current position.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * 4 * speed);
  }
  else
  {
    // left and right cause the camera to rotate on the global Y
    // axis; page up and page down cause the camera to rotate on the
    // _camera's_ X axis (more on this in a second) and up and down
    // arrows cause the camera to go forwards and backwards.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      rotY += speed;
    if (kbd->GetKeyState (CSKEY_LEFT))
      rotY -= speed;
    if (kbd->GetKeyState (CSKEY_PGUP))
      rotX += speed;
    if (kbd->GetKeyState (CSKEY_PGDN))
      rotX -= speed;
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_FORWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * 4 * speed);
  }

  // We now assign a new rotation transformation to the camera.  You
  // can think of the rotation this way: starting from the zero
  // position, you first rotate "rotY" radians on your Y axis to get
  // the first rotation.  From there you rotate "rotX" radians on the
  // your X axis to get the final rotation.  We multiply the
  // individual rotations on each axis together to get a single
  // rotation matrix.  The rotations are applied in right to left
  // order .
  csMatrix3 rot = csXRotMatrix3 (rotX) * csYRotMatrix3 (rotY);
  csOrthoTransform ot (rot, c->GetTransform().GetOrigin ());
  c->SetTransform (ot);
  
  if (g3d->BeginDraw(engine->GetBeginDrawFlags() | CSDRAW_3DGRAPHICS))
  {
	view->Draw ();
  }
}

void AppWaterdemo2::FinishFrame()
{
  g3d->FinishDraw();
  g3d->Print(0);
}

bool AppWaterdemo2::OnKeyboard(iEvent& ev)
{
  // We got a keyboard event.
  if (csKeyEventHelper::GetEventType(&ev) == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      // The user pressed escape, so terminate the application.  The proper way
      // to terminate a Crystal Space application is by broadcasting a
      // csevQuit event.  That will cause the main run loop to stop.  To do
      // so we retrieve the event queue from the object registry and then post
      // the event.
      csRef<iEventQueue> q =
        csQueryRegistry<iEventQueue> (GetObjectRegistry());
      if (q.IsValid())
//        q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
		exit(0); // HACK TO AVOID SEGFAULT ON EXIT
    }
  }
  return false;
}

bool AppWaterdemo2::OnInitialize(int argc, char* argv[])
{
  iObjectRegistry* r = GetObjectRegistry();

  // Load application-specific configuration file.
//  if (!csInitializer::SetupConfigManager(r,
//      "/my/vfs/path/AppWaterdemo2.cfg", GetApplicationName()))
//    return ReportError("Failed to initialize configuration manager!");

#ifdef USE_CEL
  celInitializer::SetupCelPluginDirs(r);
#endif

  // RequestPlugins() will load all plugins we specify.  In addition it will
  // also check if there are plugins that need to be loaded from the
  // configuration system (both the application configuration and CS or global
  // configurations).  It also supports specifying plugins on the command line
  // via the --plugin= option.
  if (!csInitializer::RequestPlugins(r,
	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_CONSOLEOUT,
	CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  // "Warm up" the event handler so it can interact with the world
  csBaseEventHandler::Initialize(GetObjectRegistry());
 
  // Set up an event handler for the application.  Crystal Space is fully
  // event-driven.  Everything (except for this initialization) happens in
  // response to an event.
  if (!RegisterQueue (r, csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void AppWaterdemo2::OnExit()
{
}

bool AppWaterdemo2::Application()
{
  iObjectRegistry* r = GetObjectRegistry();

  // Open the main system. This will open all the previously loaded plugins
  // (i.e. all windows will be opened).
  if (!OpenApplication(r))
    return ReportError("Error opening system!");

  // Now get the pointer to various modules we need.  We fetch them from the
  // object registry.  The RequestPlugins() call we did earlier registered all
  // loaded plugins with the object registry.  It is also possible to load
  // plugins manually on-demand.
  g3d = csQueryRegistry<iGraphics3D> (r);
  if (!g3d)
    return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (r);
  if (!engine)
    return ReportError("Failed to locate 3D engine!");
    
  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry());
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry());
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry());
  if (!loader) return ReportError("Failed to locate Loader!");


  engine->SetLightingCacheMode(0);
  engine->SetClearScreen(true);
  
  rotY = 0;
  rotX = 0;

  fact = engine->CreateMeshFactory (
    "crystalspace.mesh.object.genmesh", "cubeFact");
  csRef<iGeneralFactoryState> factstate = scfQueryInterface<iGeneralFactoryState> (
    fact->GetMeshObjectFactory ());

  csEllipsoid ellips (csVector3 (0, 0, 0), csVector3 (1, 1, 1));
  factstate->GenerateSphere (ellips, 8);
  factstate->CalculateNormals ();
    
  waterfact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.watermesh", "meshFact");
  		
  if(!waterfact) return ReportError("Failed to load water mesh factory!");

	csRef<iWaterFactoryState> waterFactState = scfQueryInterface<iWaterFactoryState> (
		waterfact->GetMeshObjectFactory());
	waterFactState->SetLength(11);
	waterFactState->SetWidth(11);
	waterFactState->SetGranularity(2);
	waterFactState->SetMurkiness(0.2);

	// Load in lighting shaders
	csRef<iVFS> vfs (csQueryRegistry<iVFS> (object_reg));
	csRef<iFile> shaderFile = vfs->Open ("/shader/water/water.xml", VFS_FILE_READ);
	csRef<iPluginManager> plugin_mgr (csQueryRegistry<iPluginManager> (object_reg));

	csRef<iDocumentSystem> docsys = csLoadPlugin<iDocumentSystem > 
		(plugin_mgr, "crystalspace.documentsystem.xmlread");
	if (docsys.IsValid())
	    object_reg->Register (docsys, "iDocumentSystem ");

	csRef<iDocument> shaderDoc = docsys->CreateDocument ();
	shaderDoc->Parse (shaderFile, true);

	csRef<iShaderManager> shmgr (csQueryRegistry<iShaderManager> (object_reg));
	csRef<iShaderCompiler> shcom (shmgr->GetCompiler ("XMLShader"));
	csRef<iLoaderContext> ldr_context = engine->CreateLoaderContext();
	csRef<iShader> shader = shcom->CompileShader 
		(ldr_context, shaderDoc->GetRoot()->GetNode("shader"));
	csRef<iMaterial> mat = engine->CreateBaseMaterial (0);
	csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> 
	 	(object_reg, "crystalspace.shared.stringset");
	mat->SetShader (strings->Request ("standard"), shader);

	csRef<iMaterialWrapper> waterMatW = 
		engine->GetMaterialList ()->NewMaterial (mat, "waterMaterial");

  //CreateRoom();

// Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError("Error loading 'stone4' texture!");

  if (!loader->LoadTexture ("wood", "/lib/std/andrew_wood.gif"))
    ReportError("Error loading 'andrew_wood' texture!");

  //iTextureWrapper* normalMap = loader->LoadTexture ("normal map", "/water/723-normal.jpg");
  iTextureWrapper* normalMap = loader->LoadTexture ("normal map", "/water/w_normalmap.png");  
  //iTextureWrapper* normalMap = loader->LoadTexture ("normal map", "/water/940-bump.jpg");
  if(!normalMap)
	ReportError("Error loading normal map!");

  iMaterialWrapper* stone =
    engine->GetMaterialList ()->FindByName ("stone");

  iMaterialWrapper* wood =
    engine->GetMaterialList ()->FindByName ("wood");

  room = engine->CreateSector ("room");


  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (0, -5, 0), csVector3 (10, 5, 10));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
      engine, room, "walls", "walls_factory", &box);

  csRef<iGeneralMeshState> mesh_state = scfQueryInterface<
    iGeneralMeshState> (walls->GetMeshObject ());
  mesh_state->SetShadowReceiving (true);
  walls->GetMeshObject ()->SetMaterialWrapper (wood);

  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (1, 3, 1), 10,
        csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (1, 3,  9), 10,
        csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (19, 3, 1), 10,
        csColor (0, 1, 0));
  ll->Add (light);
  
  light = engine->CreateLight (0, csVector3 (19, 3, 9), 10,
        csColor (1, 1, 1));
  ll->Add (light);
  
    // Make a ball using the genmesh plug-in.

  csRef<iMeshWrapper> ball =
    engine->CreateMeshWrapper (
    	fact, 
    	"ball", 
    	room, 
    	csVector3 (5, 5, 5));
    
  csRef<iMeshObject> ballstate = scfQueryInterface<iMeshObject> (
    ball->GetMeshObject ());
  ballstate->SetMaterialWrapper (stone);

  csRef<iMeshWrapper> watermesh =
    engine->CreateMeshWrapper (
    	waterfact, 
    	"watermesh", 
    	room, 
    	csVector3 (0, 0, 0));

  //watermesh->SetFlagsRecursive(CS_ENTITY_CAMERA);

  csRef<iWaterMeshState> watermeshstate = scfQueryInterface<iWaterMeshState> (
    watermesh->GetMeshObject ());
  csRef<iMeshObject> watermeshobj = scfQueryInterface<iMeshObject> (
    watermesh->GetMeshObject ());

  watermeshobj->SetMaterialWrapper (waterMatW);
  watermeshstate->SetNormalMap(normalMap);

  engine->Prepare ();

  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 3, 0));
  
  //****************** END OF INITIALIZATION STUFFS ***********************//
  // Start the default run/event loop.  This will return only when some code,
  // such as OnKeyboard(), has asked the run loop to terminate.
  Run();

  return true;
}

void AppWaterdemo2::CreateRoom ()
{
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError("Error loading 'stone4' texture!");
    
  if (!loader->LoadTexture ("wood", "/lib/std/andrew_wood.gif"))
    ReportError("Error loading 'andrew_wood' texture!");

  iMaterialWrapper* stone =
    engine->GetMaterialList ()->FindByName ("stone");
    
  iMaterialWrapper* wood =
    engine->GetMaterialList ()->FindByName ("wood");

  room = engine->CreateSector ("room");


  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
      engine, room, "walls", "walls_factory", &box);

  csRef<iGeneralMeshState> mesh_state = scfQueryInterface<
    iGeneralMeshState> (walls->GetMeshObject ());
  mesh_state->SetShadowReceiving (true);
  walls->GetMeshObject ()->SetMaterialWrapper (wood);

  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 5, 0), 10,
        csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 5,  0), 10,
        csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 5, -3), 10,
        csColor (0, 1, 0));
  ll->Add (light);
  
  light = engine->CreateLight (0, csVector3 (0, 15, 0), 10,
        csColor (1, 1, 1));
  ll->Add (light);
  
    // Make a ball using the genmesh plug-in.

  csRef<iMeshWrapper> ball =
    engine->CreateMeshWrapper (
    	fact, 
    	"ball", 
    	room, 
    	csVector3 (-3, 5, -3));
    
  csRef<iMeshObject> ballstate = scfQueryInterface<iMeshObject> (
    ball->GetMeshObject ());
  ballstate->SetMaterialWrapper (stone);
  
  csRef<iMeshWrapper> watermesh =
    engine->CreateMeshWrapper (
    	waterfact, 
    	"watermesh", 
    	room, 
    	csVector3 (-5, 1, -5));
    
  csRef<iMeshObject> watermeshstate = scfQueryInterface<iMeshObject> (
    watermesh->GetMeshObject ());
  //watermeshstate->SetMaterialWrapper (stone);

  engine->Prepare ();
}
