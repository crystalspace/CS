#include "testcull.h"

typedef CS::RenderManager::RenderTree<
CS::RenderManager::RenderTreeStandardTraits> RenderTreeType;

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

Testcull::Testcull ()
{
  SetApplicationName ("CrystalSpace.Testcull");
}

Testcull::~Testcull ()
{
}

csSimpleRenderMesh ConstructBBoxMesh(csBox3 &box,csRenderMeshType rmtype,csZBufMode zbuf)
{
  csSimpleRenderMesh srm;
  csVector3* verts = 0;
  csVector4* cols = 0;

  verts=new csVector3[25];
  cols=new csVector4[25];

  verts[0]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
  verts[1]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
  verts[2]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());
  verts[3]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());

  verts[4]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
  verts[5]=csVector3(box.MinX(),box.MinY(),box.MinZ());
  verts[6]=csVector3(box.MinX(),box.MaxY(),box.MinZ());
  verts[7]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());

  verts[8]=csVector3(box.MinX(),box.MinY(),box.MinZ());
  verts[9]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
  verts[10]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());
  verts[11]=csVector3(box.MinX(),box.MaxY(),box.MinZ());

  verts[12]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
  verts[13]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
  verts[14]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());
  verts[15]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());

  if(rmtype==CS_MESHTYPE_QUADS)
  {
    verts[16]=csVector3(box.MinX(),box.MaxY(),box.MinZ());
    verts[17]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());
    verts[18]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());
    verts[19]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());

    verts[20]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
    verts[21]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
    verts[22]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
    verts[23]=csVector3(box.MinX(),box.MinY(),box.MinZ());
  }
  else
  {
    verts[16]=csVector3(box.MinX(),box.MinY(),box.MinZ());
    verts[17]=csVector3(box.MinX(),box.MaxY(),box.MinZ());
    verts[18]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
    verts[19]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());

    verts[20]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
    verts[21]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());
    verts[22]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
    verts[23]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());
  }
  cols[0]=csVector4(0.0f,1.0f,0.0f);
  for(int i=1;i<24; ++i)
  {
    cols[i]=cols[0];
  }

  srm.vertices=verts;
  srm.colors=cols;
  srm.vertexCount=24;

  srm.meshtype = rmtype;
  csAlphaMode alf;
  alf.alphaType = alf.alphaSmooth;
  alf.autoAlphaMode = false;
  srm.alphaType = alf;
  srm.z_buf_mode=zbuf;
  return srm;
}

void Testcull::SortByDepth()
{
  int i,ord=0;
  do
  {
    ord=0;
    for(i=0;i<numHouses-1;i++)
    {
      if(house[vIndexes[i]]->GetWorldBoundingBox().SquaredPosDist(view->GetCamera()->GetTransform().GetOrigin())
        > house[vIndexes[i+1]]->GetWorldBoundingBox().SquaredPosDist(view->GetCamera()->GetTransform().GetOrigin()))
      {
        vIndexes[i]^=vIndexes[i+1];
        vIndexes[i+1]^=vIndexes[i];
        vIndexes[i]^=vIndexes[i+1];
        ord=1;
      }
    }
  }while(ord);
}

void Testcull::SetupContext(RenderTreeType::ContextNode& context, iShaderManager* shaderManager)
{
  CS::RenderManager::RenderView* rview = context.renderView;

  StandardMeshSorter<RenderTreeType> mySorter (rview->GetEngine ());
  mySorter.SetupCameraLocation (rview->GetCamera ()->GetTransform ().GetOrigin ());
  ForEachMeshNode (context, mySorter);

  SingleMeshContextNumbering<RenderTreeType> numbering;
  ForEachMeshNode (context, numbering);

  csRef<iStringSet> strSet = csQueryRegistryTagInterface<iStringSet> 
    (GetObjectRegistry (), "crystalspace.shared.stringset");

  csStringID shaderType = strSet->Request("depthwrite");
  SingleRenderLayer renderLayer(shaderType, shaderManager->GetShader("occ_queries"), 0, 0);
  renderLayer.SetAmbient(true);
  SetupStandardSVs (context, renderLayer, shaderManager, room);

  SetupStandardShader (context, shaderManager, renderLayer);
  SetupStandardTicket (context, shaderManager, renderLayer);
}

void Testcull::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  iCamera* c = view->GetCamera ();

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
  csOrthoTransform ot (rot, c->GetTransform ().GetOrigin ());
  c->SetTransform (ot);
  rmanager->RenderView(view);

  /*c->SetViewportSize (g3d->GetWidth(), g3d->GetHeight());
  const csReversibleTransform& camt = c->GetTransform ();
  const float leftx = (float)(-c->GetShiftX () * c->GetInvFOV ());
  const float rightx = (float)((g3d->GetWidth () - c->GetShiftX ()) * c->GetInvFOV ());
  const float topy = (float)(-c->GetShiftY () * c->GetInvFOV ());
  const float boty = (float)((g3d->GetHeight () - c->GetShiftY ()) * c->GetInvFOV ());
  CS::RenderManager::RenderView rview(c,view->GetClipper(),g3d,g2d);
  rview.SetEngine(engine);
  rview.SetOriginalCamera(c);
  rview.SetFrustum (leftx, rightx, topy, boty);
  rview.GetClipPlane ().Set (0, 0, -1, 0);
  g3d->SetClipper (view->GetClipper(), CS_CLIPPER_TOPLEVEL);  // We are at top-level.
  g3d->ResetNearPlane ();
  g3d->SetProjectionMatrix (c->GetProjectionMatrix ());

  if (!g3d->BeginDraw(engine->GetBeginDrawFlags() | CSDRAW_3DGRAPHICS | CSDRAW_CLEARZBUFFER | CSDRAW_CLEARSCREEN))
  {
    printf("Cannot prepare renderer for 3D drawing\n");
  }

  g3d->SetWorldToCamera (camt.GetInverse ());
  g3d->SetZMode(CS_ZBUF_USE);

  csSimpleRenderMesh srm;
  csBox3 bb;

  for(int j=0;j<numHouses;j+=2)
  {
    int numRMeshes=0;
    csRenderMesh **rmeshes=house[j]->GetRenderMeshes(numRMeshes,&rview,0);
    for(int q=0;q<numRMeshes;q++)
    {
      csVertexAttrib vA=CS_VATTRIB_POSITION;
      iRenderBuffer *rB=rmeshes[q]->buffers->GetRenderBuffer(CS_BUFFER_POSITION);
      g3d->ActivateBuffers(&vA,&rB,1);
      g3d->DrawMeshBasic(rmeshes[q],*rmeshes[q]);
      g3d->DeactivateBuffers(&vA,1);
    }
  }

  for(int i=1; i<numHouses; i+=2)
  {
    g3d->OQBeginQuery(queries[i]);
    bb=house[i]->GetWorldBoundingBox();
    srm=ConstructBBoxMesh(bb,CS_MESHTYPE_QUADS,CS_ZBUF_TEST);
    g3d->DrawSimpleMesh(srm);
    g3d->OQEndQuery();

    //g3d->SetZMode(CS_ZBUF_USE);
  }

  for(int i=1; i<numHouses; i+=2)
  {
    if(g3d->OQIsVisible(queries[i],0))
    {
      printf("%d visible\n",i);
    }
    else
    {
      printf("%d NOT visible\n",i);
    }
  }

  g3d->FinishDraw();

  printf("\n\n");*/



  /*if (!g3d->BeginDraw(engine->GetBeginDrawFlags() | CSDRAW_3DGRAPHICS | CSDRAW_CLEARZBUFFER | CSDRAW_CLEARSCREEN))
  {
    printf("Cannot prepare renderer for 3D drawing\n");
  }
  g3d->FinishDraw();

  c->SetViewportSize (g3d->GetWidth(), g3d->GetHeight());
  const csReversibleTransform& camt = c->GetTransform ();
  const float leftx = (float)(-c->GetShiftX () * c->GetInvFOV ());
  const float rightx = (float)((g3d->GetWidth () - c->GetShiftX ()) * c->GetInvFOV ());
  const float topy = (float)(-c->GetShiftY () * c->GetInvFOV ());
  const float boty = (float)((g3d->GetHeight () - c->GetShiftY ()) * c->GetInvFOV ());
  CS::RenderManager::RenderView rview(c,view->GetClipper(),g3d,g2d);
  rview.SetEngine(engine);
  rview.SetOriginalCamera(c);
  rview.SetFrustum (leftx, rightx, topy, boty);
  rview.GetClipPlane ().Set (0, 0, -1, 0);
  g3d->SetClipper (view->GetClipper(), CS_CLIPPER_TOPLEVEL);  // We are at top-level.
  g3d->ResetNearPlane ();
  g3d->SetProjectionMatrix (c->GetProjectionMatrix ());

  if (!g3d->BeginDraw(engine->GetBeginDrawFlags() | CSDRAW_3DGRAPHICS | CSDRAW_CLEARZBUFFER | CSDRAW_CLEARSCREEN))
  {
    printf("Cannot prepare renderer for 3D drawing\n");
  }
  g3d->SetWorldToCamera (camt.GetInverse ());
  g3d->SetZMode(CS_ZBUF_USE);

  //engine->Draw(c,view->GetClipper(),house[1]);
  SortByDepth();

  unsigned int *queries;
  int numq=numHouses,oldq=0;
  g3d->OQInitQueries(queries,oldq,numq);
  bool IsVis[100]={0};

  int k=0;
  for(int i=0;i<sqrtl(numHouses);i++)
  {
    for(int j=0;j<sqrtl(numHouses);j++)
    {
      int numRMeshes=0;
      csRenderMesh **rmeshes=house[vIndexes[k]]->GetRenderMeshes(numRMeshes,&rview,0);
      g3d->OQBeginQuery(queries[vIndexes[k]]);
      for(int q=0;q<numRMeshes;q++)
      {
        csVertexAttrib vA=CS_VATTRIB_POSITION;
        iRenderBuffer *rB=rmeshes[q]->buffers->GetRenderBuffer(CS_BUFFER_POSITION);
        g3d->ActivateBuffers(&vA,&rB,1);
        g3d->DrawMeshBasic(rmeshes[q],*rmeshes[q]);
        //g3d->DeactivateBuffers(&vA,1);
      }
      g3d->OQEndQuery();
      k++;
    }
  }
  g3d->SetClipper (0, CS_CLIPPER_NONE);
  g3d->FinishDraw();

  k=0;
  for(int i=0;i<sqrtl(numHouses);i++)
  {
    for(int j=0;j<sqrtl(numHouses);j++)
    {
      while(!g3d->OQueryFinished(queries[vIndexes[k]])) 1;
      if(g3d->OQIsVisible(queries[vIndexes[k]],0))
      {
        IsVis[vIndexes[k]]=true;
        printf("visible\n");
      }
      else
      {
        IsVis[vIndexes[k]]=false;
        printf("not visible\n");
      }
      k++;
    }
  }

  g3d->OQDelQueries(queries,numq);
  rview.GetEngine()->UpdateNewFrame();

  printf("\n");*/
}

bool Testcull::OnKeyboard (iEvent& ev)
{// We got a keyboard event.
  csKeyEventType eventtype = csKeyEventHelper::GetEventType (&ev);
  if (eventtype == csKeyEventTypeDown)
  {// The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode (&ev);
    if (code == CSKEY_ESC)
    {
      // The user pressed escape to exit the application.
      // The proper way to quit a Crystal Space application
      // is by broadcasting a csevQuit event. That will cause the
      // main runloop to stop. To do that we get the event queue from
      // the object registry and then post the event.
      csRef<iEventQueue> q = 
        csQueryRegistry<iEventQueue> (GetObjectRegistry ());
      if (q.IsValid ()) q->GetEventOutlet ()->Broadcast (
        csevQuit (GetObjectRegistry ()));
    }
  }

  return false;
}

bool Testcull::OnInitialize (int /*argc*/, char* /*argv*/ [])
{
  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  csBaseEventHandler::Initialize (GetObjectRegistry ());

  // Now we need to register the event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  // Rather than simply handling all events, we subscribe to the
  // particular events we're interested in.
  csEventID events[] = {
    csevFrame (GetObjectRegistry ()),
    csevKeyboardEvent (GetObjectRegistry ()),
    CS_EVENTLIST_END
  };

  if (!RegisterQueue (GetObjectRegistry (), events))
    return ReportError ("Failed to set up event handler!");

  // Report success
  return true;
}

void Testcull::OnExit ()
{
  // Shut down the event handlers we spawned earlier.
  printer.Invalidate ();
}

bool Testcull::Application ()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication (GetObjectRegistry ()))
    return ReportError ("Error opening system!");

  if (SetupModules ())
  {
    // This calls the default runloop. This will basically just keep
    // broadcasting process events to keep the game going.
    Run ();
  }

  return true;
}

bool Testcull::SetupModules ()
{
  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if(!vfs) return ReportError("Failed to locate Virtual File System!");
  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  g2d = csQueryRegistry<iGraphics2D> (GetObjectRegistry ());
  if (!g2d) return ReportError ("Failed to locate 3D renderer!");

  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry ());
  if (!g3d) return ReportError ("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry ());
  if (!engine) return ReportError ("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry ());
  if (!vc) return ReportError ("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry ());
  if (!kbd) return ReportError ("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry ());
  if (!loader) return ReportError ("Failed to locate Loader!");

  // We need a View to the virtual world.
  view.AttachNew (new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Here we create our world.
  CreateRoom ();

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  // Now calculate static lighting for our geometry.
  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);

  rmanager = engine->GetRenderManager ();

  // These are used store the current orientation of the camera
  rotY = rotX = 0;

  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, -70));

  // We use some other "helper" event handlers to handle 
  // pushing our work into the 3D engine and rendering it
  // to the screen.
  printer.AttachNew (new FramePrinter (object_reg));

  oldNum=0;
  newNum=numHouses;

  return true;
}

void Testcull::CreateRoom ()
{
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError ("Error loading 'stone4' texture!");
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  // We create a new sector called "room".
  room = engine->CreateSector ("room");

  // Creating the walls for our room.

  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);


  // Now we make a factory and a mesh at once.
  /*csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
  engine, room, "walls", "walls_factory", &box);
  walls->GetMeshObject ()->SetMaterialWrapper (tm);*/

  if(!vfs->Mount("/modelpath/","$@data$/culltest$/"))
  {
    ReportError("Failed to mount specified location!");
    return;
  }

  csLoadResult result = loader->Load("/modelpath/house.xml");
  if(!result.success)
  {
    ReportError("Failed to load mesh file!");
    return;
  }
  csRef<iMeshFactoryWrapper> meshFactW=engine->FindMeshFactory("House");

  csMatrix3 m;
  m.Identity ();
  csQuaternion q,qTrans;
  q.SetIdentity();
  q.SetAxisAngle(csVector3(0,1,0),90*3.1415/180);
  m.Set(q);

  if(meshFactW==NULL)
  {
    ReportError("Failed to find mesh factory!");
    return;
  }

  int k=0;
  char aux[100],name[]="MyHouse";

  numVisible=numHouses=NUM_HOUSES;

  for(int i=0;i<numHouses;i++)
  {
    invisible[i]=false;
    vIndexes[i]=i;
  }
  for(int i=0;i<sqrtl(numHouses);i++)
  {
    for(int j=0;j<sqrtl(numHouses);j++)
    {
      sprintf(aux,"%s%d",name,k);
      house[k] = csRef<iMeshWrapper>(engine->CreateMeshWrapper (
        meshFactW, aux, room,
        csVector3 ( ((j%2)?0:7.5) + (-15)*sqrtl(numHouses)/2+15*i, 0,15*j),true));
      k++;
    }
  }

  g3d->OQInitQueries(queries,numHouses);

  /*house[1] = csRef<iMeshWrapper> (engine->CreateMeshWrapper (
                  meshFactW, "MyHouse2", room,
                  csVector3 (0, 0, 0),true));*/

  /*house[2] = csRef<iMeshWrapper> (engine->CreateMeshWrapper (
  meshFactW, "MyHouse3", room,
  csVector3 (20, 0, 0),true));*/

  /*house[0]->GetMovable ()->SetTransform (m);
  house[1]->GetMovable ()->SetTransform (m);
  house[2]->GetMovable ()->SetTransform (m);*/

  bUseBB=true;
  bShowBB=true;

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-43, 5, 0), 120, csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (43, 5,  0), 120, csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 43, -3), 120, csColor (0, 1, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 0, -43), 120, csColor (0, 1, 0));
  ll->Add (light);
}

/*-------------------------------------------------------------------------*
* Main function
*-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<Testcull>::Run (argc, argv);
}
