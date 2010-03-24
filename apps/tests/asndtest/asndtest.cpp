/*
    Copyright (C) 2001 by Søren Bøg

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

#include "asndtest.h"

CS_IMPLEMENT_APPLICATION

ASndTest::ASndTest ()
{
  SetApplicationName ("CrystalSpace.ASndTest");
}

ASndTest::~ASndTest ()
{
}

void ASndTest::CreateWorld ()
{
  // Load the first texture from the standard library.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError("Error loading 'stone4' texture!");
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  // Load a second texture from the standard library.
  if (!loader->LoadTexture ("spark", "/lib/std/spark.png"))
    ReportError("Error loading 'spark' texture!");

  // Load a sprite from the standard library.
  csRef<iMeshFactoryWrapper> imeshfact (loader->LoadMeshObjectFactory ("/lib/std/sprite1"));
  if (imeshfact == 0)
    ReportError("Error loading 'sprite1' mesh object factory!");
  csRef<iMeshWrapper> sprite;
  csRef<iSprite3DState> spstate;

  // Create and prepare the world
  world = engine->CreateSector ("room");

  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-40, 0, 0), csVector3 (40, 10, 40));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
      engine, world, "walls", "walls_factory", &box);
  walls->GetMeshObject ()->SetMaterialWrapper (tm);

  // Prepare lighting
  csRef<iLight> light;
  iLightList* ll = world->GetLights ();

  csRef<iCommandLineParser> cmdline (
        csQueryRegistry<iCommandLineParser> (GetObjectRegistry ()));

  csString fname = cmdline->GetOption ("sndfile");
  if (fname.IsEmpty ())
  {
    fname = "/lib/std/loopbzzt.wav";
    csPrintf ("You can override sound file using -sndfile option (VFS path)\n");
  }
  csPrintf ("Sound file  : %s\n", fname.GetData ());

  csRef<iDataBuffer> soundbuf = vfs->ReadFile (fname.GetData ());
  if (!soundbuf)
    ReportError ("Can't load file '%s'!", fname.GetData ());

  // Interpret the sound
  csRef<iSndSysData> snddata = sndloader->LoadSound (soundbuf);
  if (!snddata)
    ReportError ("Can't load sound '%s'!", fname.GetData ());

  const csSndSysSoundFormat* format = snddata->GetFormat ();
  csPrintf ("=== iSndSysData format informations ===\n");
  csPrintf ("Format      : %d bits, %d channel(s), %d Hz\n",
        format->Bits, format->Channels, format->Freq);
  csPrintf ("Sample Size : %zu bytes, %zu frames\n", snddata->GetDataSize (),
        snddata->GetFrameCount ());
  csPrintf ("Description : %s\n", snddata->GetDescription ());

  // Create a stream for the sound
  csRef<iSndSysStream> sndstream = sndrenderer->CreateStream (snddata, CS_SND3D_ABSOLUTE);
  if (!sndstream)
    ReportError ("Can't create stream for '%s'!", fname.GetData ());

  const csSndSysSoundFormat* rformat = sndstream->GetRenderedFormat ();
  csPrintf ("=== iSndSysStream format informations ===\n");
  csPrintf ("Format      : %d bits, %d channel(s), %d Hz\n",
        rformat->Bits, rformat->Channels, rformat->Freq);
  csPrintf ("Stream Size : %zu frames\n", sndstream->GetFrameCount ());
  csPrintf ("Description : %s\n", sndstream->GetDescription ());

  // Make the stream loop and play (unpaused)
  sndstream->SetLoopState (CS_SNDSYS_STREAM_LOOP);
  sndstream->Unpause ();

  csRef<iSndSysSource> sndsource;
  csRef<iSndSysSource3D> sndsource3d;

  // Add some quick lighting
  light = engine->CreateLight (0, csVector3 (-30, 5, 10), 20, csColor (0.22f, 0.2f, 0.25f));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 (-30, 5, 30), 20, csColor (0.22f, 0.2f, 0.25f));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 (-10, 5, 10), 20, csColor (0.22f, 0.2f, 0.25f));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 (-10, 5, 30), 20, csColor (0.22f, 0.2f, 0.25f));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 ( 10, 5, 30), 20, csColor (0.22f, 0.2f, 0.25f));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 ( 10, 5, 10), 20, csColor (0.22f, 0.2f, 0.25f));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 ( 30, 5, 30), 20, csColor (0.22f, 0.2f, 0.25f));
  ll->Add (light);
  light = engine->CreateLight (0, csVector3 ( 30, 5, 10), 20, csColor (0.22f, 0.2f, 0.25f));
  ll->Add (light);

  // Create a small sound source
  sndsource = sndrenderer->CreateSource (sndstream);
  sndsource3d = scfQueryInterface<iSndSysSource3D> (sndsource);
  sndsource->SetVolume( 1.0f );
  sndsource3d->SetMinimumDistance( 1.0f );
  sndsource3d->SetPosition( csVector3 (3.0f, 5.0f, 10.0f) );
  light = engine->CreateLight (0, csVector3 (30.0f, 1.0f, 10.0f), 5.0f, csColor (0.15f, 0.0f, 0.0f));
  ll->Add (light);
  sprite = engine->CreateMeshWrapper (imeshfact, "Sound1Sprite", world, csVector3 (30.0f, 5.0f, 10.0f));
  spstate = scfQueryInterface<iSprite3DState> (sprite->GetMeshObject());
  spstate->SetAction ("default");

  // Create a medium sound source
  sndsource = sndrenderer->CreateSource (sndstream);
  sndsource3d = scfQueryInterface<iSndSysSource3D> (sndsource);
  sndsource->SetVolume( 1.0 );
  sndsource3d->SetMinimumDistance( 2.0f );
  sndsource3d->SetPosition( csVector3 (3.0f, 5.0f, 30.0f) );
  light = engine->CreateLight (0, csVector3 (30.0f, 1.0f, 30.0f), 5.0f, csColor (0.3f, 0.0f, 0.0f));
  ll->Add (light);
  sprite = engine->CreateMeshWrapper (imeshfact, "Sound2Sprite", world, csVector3 (30.0f, 5.0f, 30.0f));
  spstate = scfQueryInterface<iSprite3DState> (sprite->GetMeshObject());
  spstate->SetAction ("default");

  // Create a large sound source
  sndsource = sndrenderer->CreateSource (sndstream);
  sndsource3d = scfQueryInterface<iSndSysSource3D> (sndsource);
  sndsource->SetVolume( 1.0f );
  sndsource3d->SetMinimumDistance( 4.0f );
  sndsource3d->SetPosition( csVector3 (10.0f, 5.0f, 30.0f) );
  light = engine->CreateLight (0, csVector3 (10.0f, 1.0f, 30.0f), 5.0f, csColor (0.6f, 0.0f, 0.0f));
  ll->Add (light);
  sprite = engine->CreateMeshWrapper (imeshfact, "Sound3Sprite", world, csVector3 (10.0f, 5.0f, 30.0f));
  spstate = scfQueryInterface<iSprite3DState> (sprite->GetMeshObject());
  spstate->SetAction ("default");

  // Create a simple directional source
  sndsource = sndrenderer->CreateSource (sndstream);
  sndsource3d = scfQueryInterface<iSndSysSource3D> (sndsource);
  sndsource->SetVolume( 1.0 );
  sndsource3d->SetMinimumDistance( 8 );
  sndsource3d->SetPosition( csVector3 (-10, 5, 30) );
  csRef<iSndSysSource3DDirectionalSimple> sndsource3dds = scfQueryInterface<iSndSysSource3DDirectionalSimple> (sndsource);
  sndsource3dds->SetDirection( csVector3 (0, 0, 1) );
  sndsource3dds->SetDirectionalRadiation( PI/4 );
  light = engine->CreateLight (0, csVector3 (-10.0f, 5.0f, 39.0f), 5.0f, csColor (0.6f, 0.0f, 0.0f));
  ll->Add (light);
  sprite = engine->CreateMeshWrapper (imeshfact, "Sound4Sprite", world, csVector3 (-10, 5, 30));
  spstate = scfQueryInterface<iSprite3DState> (sprite->GetMeshObject());
  spstate->SetAction ("default");

  // Create a normal directional source
  sndsource = sndrenderer->CreateSource (sndstream);
  sndsource3d = scfQueryInterface<iSndSysSource3D> (sndsource);
  sndsource->SetVolume( 1.0 );
  sndsource3d->SetMinimumDistance( 8 );
  sndsource3d->SetPosition( csVector3 (-30, 5, 30) );
  csRef<iSndSysSource3DDirectional> sndsource3dd = scfQueryInterface<iSndSysSource3DDirectional> (sndsource);
  sndsource3dd->SetDirection( csVector3 (0, 0, 1) );
  sndsource3dd->SetDirectionalRadiationInnerCone( PI/4 );
  sndsource3dd->SetDirectionalRadiationOuterCone( PI/2 );
  sndsource3dd->SetDirectionalRadiationOuterGain( 0 );
  light = engine->CreateLight (0, csVector3 (-30.0f, 5.0f, 39.0f), 5.0f, csColor (0.6f, 0.0f, 0.0f));
  ll->Add (light);
  sprite = engine->CreateMeshWrapper (imeshfact, "Sound5Sprite", world, csVector3 (-30, 5, 30));
  spstate = scfQueryInterface<iSprite3DState> (sprite->GetMeshObject());
  spstate->SetAction ("default");

  // Create a doppler source
  sndsource = sndrenderer->CreateSource (sndstream);
  movingsound = scfQueryInterface<iSndSysSource3D> (sndsource);
  sndsource->SetVolume( 1.0 );
  movingsound->SetMinimumDistance( 8 );
  movingsound->SetPosition( csVector3 (-10, 5, 10) );
  movingsounddoppler = scfQueryInterface<iSndSysSource3DDoppler> (sndsource);
  movingsoundsprite = engine->CreateMeshWrapper (imeshfact, "Sound6Sprite", world, csVector3 (-10, 5, 10));
  spstate = scfQueryInterface<iSprite3DState> (movingsoundsprite->GetMeshObject());
  spstate->SetAction ("default");
  movingsoundstep = 0;
  listenerdoppler = scfQueryInterface<iSndSysListenerDoppler> (sndrenderer->GetListener ());
  // 10 Units = 1 meter
  listenerdoppler->SetSpeedOfSound( 3433 );
  // Amplify Doppler effect by 100
  listenerdoppler->SetDopplerFactor( 100 );

  engine->Prepare ();

  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (world, engine, 4);
}

void ASndTest::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  iCamera* c = view->GetCamera();
  // left and right cause the camera to rotate on the global Y axis and up and
  // down arrows cause the camera to go forwards and backwards.
  bool moving = false;
  if (kbd->GetKeyState (CSKEY_RIGHT))
    rotYaw += speed;
  if (kbd->GetKeyState (CSKEY_LEFT))
    rotYaw -= speed;
  if (kbd->GetKeyState (CSKEY_UP))
  {
    c->Move (CS_VEC_FORWARD * 4 * speed);
    moving = true;
  }
  if (kbd->GetKeyState (CSKEY_DOWN))
  {
    c->Move (CS_VEC_BACKWARD * 4 * speed);
    moving = true;
  }

  // We now assign a new rotation transformation to the camera.  You
  // can think of the rotation this way: starting from the zero
  // position, you first rotate "rotY" radians on your Y axis to get
  // the first rotation.  From there you rotate "rotX" radians on the
  // your X axis to get the final rotation.  We multiply the
  // individual rotations on each axis together to get a single
  // rotation matrix.  The rotations are applied in right to left
  // order .
  csMatrix3 rot = csYRotMatrix3 (rotYaw);
  csOrthoTransform ot (rot, c->GetTransform().GetOrigin ());
  c->SetTransform (ot);
  sndrenderer->GetListener ()->SetPosition (ot.GetOrigin ());
  sndrenderer->GetListener ()->SetDirection (rot.Row3(), rot.Row2());
  if (moving)
  {
    listenerdoppler->SetVelocity (rot.Row3() * 4);
  }
  else
  {
    listenerdoppler->SetVelocity (csVector3(0,0,0));
  }

  // Move and update the doppler source.
  movingsoundstep += speed;
  movingsoundposition = csVector3 (10*sin(movingsoundstep) - 20, 5, 10);
  csVector3 movingsoundvelocity = csVector3 (10*cos(movingsoundstep), 0, 0);
  movingsound->SetPosition (movingsoundposition);
  movingsounddoppler->SetVelocity (movingsoundvelocity);
  movingsoundsprite->GetMovable ()->GetTransform ().SetOrigin (movingsoundposition);
  movingsoundsprite->GetMovable ()->UpdateMove ();

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw(
    engine->GetBeginDrawFlags() | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
}

bool ASndTest::OnKeyboard(iEvent& ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (GetObjectRegistry());
      if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit (GetObjectRegistry ()));
    }
  }
  return false;
}

bool ASndTest::OnInitialize(int argc, char* argv[])
{
  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN("crystalspace.sndsys.element.loader", iSndSysLoader),
    CS_REQUEST_PLUGIN("crystalspace.sndsys.renderer.openal", iSndSysRenderer),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  csBaseEventHandler::Initialize(GetObjectRegistry());
  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void ASndTest::OnExit()
{
  printer.Invalidate ();
}

bool ASndTest::Application()
{
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs) return ReportError("Failed to locate Virtual filesystem!");

  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry());
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry());
  if (!engine) return ReportError("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry());
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry());
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry());
  if (!loader) return ReportError("Failed to locate Loader!");

  sndrenderer = csQueryRegistry<iSndSysRenderer> (GetObjectRegistry());
  if (!sndrenderer) return ReportError("Failed to locate Sound renderer!");

  sndloader = csQueryRegistry<iSndSysLoader> (GetObjectRegistry());
  if (!sndloader) return ReportError("Failed to locate Sound loader!");

  CreateWorld ();

  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  view->GetCamera ()->SetSector (world);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (10, 5, 10));

  printer.AttachNew (new FramePrinter (object_reg));

  Run();

  return true;
}

/*---------------*
 * Main function
 *---------------*/
int main (int argc, char* argv[])
{
  return csApplicationRunner<ASndTest>::Run (argc, argv);
}
