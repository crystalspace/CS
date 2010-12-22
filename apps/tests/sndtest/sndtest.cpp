/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include "sndtest.h"

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

SndTest::SndTest ()
{
  SetApplicationName ("CrystalSpace.SndTest");
}

SndTest::~SndTest ()
{
}

static csVector3 GetSoundPos (float angle)
{
  float x = cos (angle) * 3.0f;
  float y = sin (angle) * 3.0f;
  return csVector3 (x, 5.0, y);
}

bool SndTest::OnInitialize(int /*argc*/, char* /*argv*/ [])
{
  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
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
    CS_REQUEST_PLUGIN("crystalspace.sndsys.renderer.software", iSndSysRenderer),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  // "Warm up" the base event handler
  csBaseEventHandler::Initialize (GetObjectRegistry ());

  // Now we need to setup an event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.

  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents (GetObjectRegistry ())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void SndTest::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);
  // Now rotate the sound source around the camera/listener
  cur_angle += speed;
  csVector3 sndpos = GetSoundPos (cur_angle);
  if (sndsource3d)
    sndsource3d->SetPosition (sndpos);
  sprite->GetMovable ()->GetTransform ().SetOrigin (sndpos);
  sprite->GetMovable ()->UpdateMove ();

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
}

bool SndTest::OnKeyboard(iEvent& ev)
{
  // We got a keyboard event.
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      // The user pressed escape to exit the application.
      // The proper way to quit a Crystal Space application
      // is by broadcasting a csevQuit event. That will cause the
      // main runloop to stop. To do that we get the event queue from
      // the object registry and then post the event.
      csRef<iEventQueue> q = 
        csQueryRegistry<iEventQueue> (GetObjectRegistry ());
      if (q.IsValid ()) 
	q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
    }
  }
  return false;
}

bool SndTest::CreateSprites ()
{
  // Load a texture for our sprite.
  iTextureWrapper* txt = loader->LoadTexture ("spark",
    "/lib/std/spark.png");
  if (txt == 0)
    return ReportError("Error loading texture!");

  // Load a sprite template from disk.
  csRef<iMeshFactoryWrapper> imeshfact (
    loader->LoadMeshObjectFactory ("/lib/std/sprite1"));
  if (imeshfact == 0)
    return ReportError("Error loading mesh object factory!");

  // Create the sprite and add it to the engine.
  sprite = engine->CreateMeshWrapper (imeshfact, "MySprite", room,
    GetSoundPos (0));
  csMatrix3 m; m.Identity ();
  sprite->GetMovable ()->SetTransform (m);
  sprite->GetMovable ()->UpdateMove ();
  csRef<iSprite3DState> spstate (
    scfQueryInterface<iSprite3DState> (sprite->GetMeshObject ()));
  spstate->SetAction ("default");

  return true;
}

bool SndTest::LoadSound ()
{
  csRef<iCommandLineParser> cmdline (
        csQueryRegistry<iCommandLineParser> (GetObjectRegistry ()));

  csString fname = cmdline->GetOption ("sndfile");
  if (fname.IsEmpty ())
  {
    fname = "/lib/std/loopbzzt.wav";
    csPrintf ("You can override sound file using -sndfile option (VFS path)\n");
  }
  csPrintf ("Sound file  : %s\n", fname.GetData ());

  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry ());

  csRef<iDataBuffer> soundbuf = vfs->ReadFile (fname.GetData ());
  if (!soundbuf)
    return ReportError ("Can't load file %s!", CS::Quote::Single (fname.GetData ()));

  csRef<iSndSysData> snddata = sndloader->LoadSound (soundbuf);
  if (!snddata)
    return ReportError ("Can't load sound %s!", CS::Quote::Single (fname.GetData ()));

  const csSndSysSoundFormat* format = snddata->GetFormat ();
  csPrintf ("=== iSndSysData format informations ===\n");
  csPrintf ("Format      : %d bits, %d channel(s), %d Hz\n",
        format->Bits, format->Channels, format->Freq);
  csPrintf ("Sample Size : %zu bytes, %zu frames\n", snddata->GetDataSize (),
        snddata->GetFrameCount ());
  csPrintf ("Description : %s\n", snddata->GetDescription ());

  csRef<iSndSysStream> sndstream = sndrenderer->CreateStream (snddata,
        cmdline->GetBoolOption("no3d") ? CS_SND3D_DISABLE : CS_SND3D_ABSOLUTE);
  if (!sndstream)
    return ReportError ("Can't create stream for %s!", CS::Quote::Single (fname.GetData ()));

  const csSndSysSoundFormat* rformat = sndstream->GetRenderedFormat ();
  csPrintf ("=== iSndSysStream \"rendered\" format informations ===\n");
  csPrintf ("Format      : %d bits, %d channel(s), %d Hz\n",
        rformat->Bits, rformat->Channels, rformat->Freq);
  csPrintf ("Stream Size : %zu frames\n", sndstream->GetFrameCount ());
  csPrintf ("Description : %s\n", sndstream->GetDescription ());

  sndsource = sndrenderer->CreateSource (sndstream);
  if (!sndsource)
    return ReportError ("Can't create source for %s!", CS::Quote::Single (fname.GetData ()));
  sndsource3d = scfQueryInterface<iSndSysSource3D> (sndsource);
  if (sndsource3d)
    sndsource3d->SetPosition (GetSoundPos (0));
  sndsource->SetVolume (1.0f);

  sndstream->SetLoopState (CS_SNDSYS_STREAM_LOOP);
  sndstream->Unpause ();

  return true;
}

void SndTest::OnExit()
{
  printer.Invalidate ();
}

bool SndTest::Application()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
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
  if (!sndrenderer) return ReportError("Failed to locate sound renderer!");

  sndloader = csQueryRegistry<iSndSysLoader> (GetObjectRegistry());
  if (!sndloader) return ReportError("Failed to locate sound loader!");

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Here we create our world.
  if (!CreateRoom())
    return false;

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  using namespace CS::Lighting;
  SimpleStaticLighter::ShineLights (room, engine, 4);

  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, 0));

  printer.AttachNew (new FramePrinter (object_reg));

  // Create the model.
  if (!CreateSprites ())
    return false;

  // Load a sound.
  if (!LoadSound ())
    return false;

  cur_angle = 0;
  sndrenderer->GetListener ()->SetPosition (csVector3 (0, 0, 0));

  // This calls the default runloop. This will basically just keep
  // broadcasting process events to keep the game going.
  Run();

  return true;
}

bool SndTest::CreateRoom ()
{
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    return ReportError("Error loading %s texture!",
		       CS::Quote::Single ("stone4"));

  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  // We create a new sector called "room".
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
  walls->GetMeshObject ()->SetMaterialWrapper (tm);

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight(0, csVector3(-3, 5, 0), 10, csColor(1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight(0, csVector3(3, 5,  0), 10, csColor(0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight(0, csVector3(0, 5, -3), 10, csColor(0, 1, 0));
  ll->Add (light);
  return true;
}


/*-------------------------------------------------------------------------*
 * Main function
 *-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  /* Runs the application. 
   *
   * csApplicationRunner<> is a small wrapper to support "restartable" 
   * applications (ie where CS needs to be completely shut down and loaded 
   * again). SndTest1 does not use that functionality itself, however, it
   * allows you to later use "SndTest.Restart();" and it'll just work.
   */
  return csApplicationRunner<SndTest>::Run (argc, argv);
}
