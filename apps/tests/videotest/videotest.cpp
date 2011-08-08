/*
Copyright (C) 2010 by Alin Baciu

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "videotest.h"

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include <CEGUIWindowManager.h>
#include <CEGUILogger.h>
#include "csutil/custom_new_enable.h"
#include <csutil/array.h>
#include <csutil/nobjvec.h>
#include <iostream>

using namespace std;

VideoTest::VideoTest ()
: DemoApplication ("CrystalSpace.VideoTest"), inWater (false)
{
}

void VideoTest::PrintHelp ()
{
  csCommandLineHelper commandLineHelper;

  // Printing help
  commandLineHelper.PrintApplicationHelp
    (GetObjectRegistry (), "csvid", "csvid", "Crystal Space's video player demo.");
}

void VideoTest::Frame ()
{
//  iCamera* camera = view->GetCamera ();

  //draw the room
  view->Draw();
  mediaPlayer->StartPlayer ();


  if(updateSeeker)
  {
    float videoPos = mediaPlayer->GetPosition ();
    CEGUI::Scrollbar * seeker = 
      static_cast<CEGUI::Scrollbar*>(CEGUI::WindowManager::getSingleton().getWindow("Video/Window1/Seek"));
    seeker->setScrollPosition (videoPos);
  }


  // Default behavior from DemoApplication
  DemoApplication::Frame ();

  //in order to be able to draw 2D, it seems you need to do it after DemoApplication::Frame ()
  //not really major, but might help when drawing the video on-screen

  int w, h;
  logoTex->GetOriginalDimensions (w, h);

  int screenW = g2d->GetWidth ();

  // Margin to the edge of the screen, as a fraction of screen width
//  const float marginFraction = 0.01f;
//  const int margin = (int)screenW * marginFraction;

  // Width of the logo, as a fraction of screen width
  const float widthFraction = 0.5f;
  const int width = (int)screenW * widthFraction;
  const int height = width * h / w;

  g3d->BeginDraw (CSDRAW_2DGRAPHICS);
  g3d->DrawPixmap (logoTex, 
    10, 
    10,
    width * mediaPlayer->GetAspectRatio (),
    height,
    0,
    0,
    w,
    h,
    0);
}

void VideoTest::OnExit ()
{
  mediaPlayer->StopPlayer ();
}

bool VideoTest::Application ()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  if (!csInitializer::RequestPlugins (object_reg,
    CS_REQUEST_VFS,
    CS_REQUEST_PLUGIN ("crystalspace.vpl.loader", iMediaLoader),
    CS_REQUEST_PLUGIN ("crystalspace.vpl.player", iMediaPlayer),
    CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
    CS_REQUEST_PLUGIN("crystalspace.sndsys.renderer.software", iSndSysRenderer),
    CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.vidplaydemo",
      "Can't initialize plugins!");
    return false;
  }

  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs) return ReportError("Failed to locate VFS!");

  cegui = csQueryRegistry<iCEGUI> (GetObjectRegistry());
  if (!cegui) return ReportError("Failed to locate CEGUI plugin");

  sndrenderer = csQueryRegistry<iSndSysRenderer> (GetObjectRegistry());
  if (!sndrenderer) return ReportError("Failed to locate sound renderer!");

  sndrenderer->GetListener ()->SetPosition (csVector3 (0, 0, 0));

  // Set the working directory
  vfs->ChDir ("/videodecode/");

  //Get the path of the video we want to load
  csRef<iDataBuffer> path;
  path = vfs->GetRealPath ("vid420.ogg");

  // Get the loader and load the video
  csRef<iMediaLoader> vlpLoader = csQueryRegistry<iMediaLoader> (object_reg);
  csRef<iMediaContainer> video = vlpLoader->LoadMedia(path->GetData ());

  if (video.IsValid ())
  {
    printf ("%d streams in media container\n",(int)video->GetMediaCount ());
  }

  csRef<iSndSysStream> audioStream;
  mediaPlayer = csQueryRegistry<iMediaPlayer> (object_reg);
  mediaPlayer->InitializePlayer (video,5);

  // Specifying -1 as index triggers auto stream activation
  mediaPlayer->SetActiveStream (-1);
  mediaPlayer->GetTargetTexture (logoTex);
  mediaPlayer->GetTargetAudio (audioStream);

  if (audioStream.IsValid ())
  {
    sndsource = sndrenderer->CreateSource (audioStream);
    if (!sndsource)
      ReportError ("Can't create source");
  }
  else
    ReportError ("Audio stream cannot be played");

  mediaPlayer->Loop (false);
  mediaPlayer->Play ();

  if(sndsource.IsValid () )
  {
    sndsource->SetVolume (1.0f);
    audioStream->SetLoopState (CS_SNDSYS_STREAM_DONTLOOP);
    audioStream->Unpause ();
  }

  InitializeCEGUI ();

  updateSeeker=true;

  // Create the scene
  if (!CreateScene ())
    return false;



  // Run the application
  Run();

  return true;
}

void VideoTest::InitializeCEGUI ()
{
  // Initialize CEGUI wrapper
  cegui->Initialize ();

  /* Let CEGUI plugin install an event handler that takes care of rendering
     every frame */
  cegui->SetAutoRender (true);
  
  // Set the logging level 
  cegui->GetLoggerPtr ()->setLoggingLevel(CEGUI::Informative);

  vfs->ChDir ("/cegui/");

  // Load the ice skin (which uses Falagard skinning system)
  cegui->GetSchemeManagerPtr ()->create("ice.scheme");

  cegui->GetSystemPtr ()->setDefaultMouseCursor("ice", "MouseArrow");

  cegui->GetFontManagerPtr ()->createFreeTypeFont("DejaVuSans", 10, true, "/fonts/ttf/DejaVuSans.ttf");

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  // Load layout and set as root
  vfs->ChDir ("/videodecode/");
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("ice.layout"));

  // Subscribe to the events that we need
  CEGUI::Window* btn = winMgr->getWindow("Video/Window1/Quit");

  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&VideoTest::OnExitButtonClicked, this));

  CEGUI::Window* btn2 = winMgr->getWindow("Video/Window1/Play");

  btn2->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&VideoTest::OnPlayButtonClicked, this));

  CEGUI::Window* btn3 = winMgr->getWindow("Video/Window1/Pause");

  btn3->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&VideoTest::OnPauseButtonClicked, this));

  CEGUI::Window* btn4 = winMgr->getWindow("Video/Window1/Stop");

  btn4->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&VideoTest::OnStopButtonClicked, this));

  CEGUI::Window* btn5 = winMgr->getWindow("Video/Window1/Loop");

  btn5->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged,
    CEGUI::Event::Subscriber(&VideoTest::OnLoopToggle, this));

  CEGUI::Scrollbar* slider = (CEGUI::Scrollbar*)winMgr->getWindow("Video/Window1/Seek");

  slider->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackStarted ,
    CEGUI::Event::Subscriber(&VideoTest::OnSeekingStart, this));
  slider->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded ,
    CEGUI::Event::Subscriber(&VideoTest::OnSeekingEnd, this));
  slider->setDocumentSize (mediaPlayer->GetLength ());
  slider->setStepSize (0.1f);
  slider->setTooltipText ("Seek");
}

bool VideoTest::CreateScene ()
{
  printf ("Creating level...\n");

  // Create a MaterialWrapper from the video texture
  iTextureList* texList = engine->GetTextureList();
  iTextureWrapper* texWrapper = texList->NewTexture (logoTex);
  iMaterialWrapper* vidMaterial = engine->CreateMaterial ("vidMaterial",texWrapper);

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
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
    engine, room, "walls", "walls_factory", &box);

  // Set the material to each wall of the room
  walls->GetMeshObject ()->SetMaterialWrapper (vidMaterial);

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 5, 0), 10, csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 5,  0), 10, csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 5, -3), 10, csColor (0, 1, 0));
  ll->Add (light);

  // Setup the sector and the camera
  view->GetCamera ()->SetSector (room);
  cameraManager->SetStartPosition (csVector3 (0,3,-4.5));
  cameraManager->SetCamera (view->GetCamera ());
  cameraManager->SetCameraMode (CS::Utility::CAMERA_MOVE_FREE);
  cameraManager->SetMotionSpeed (10.0f);

  printf ("Precaching data...\n");
  engine->PrecacheDraw ();

  printf ("Ready!\n");

  cegui->EnableMouseCapture ();

  return true;
}

// CEGUI listeners
bool VideoTest::OnExitButtonClicked (const CEGUI::EventArgs&)
{
  csRef<iEventQueue> q =
    csQueryRegistry<iEventQueue> (GetObjectRegistry());
  if (q.IsValid())
  {
    q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
  }
  return true;
}
bool VideoTest::OnPlayButtonClicked (const CEGUI::EventArgs&)
{
  mediaPlayer->Play ();
  return true;
}
bool VideoTest::OnPauseButtonClicked (const CEGUI::EventArgs&)
{
  mediaPlayer->Pause ();
  return true;
}
bool VideoTest::OnStopButtonClicked (const CEGUI::EventArgs&)
{
  mediaPlayer->Stop ();
  return false;
}
bool VideoTest::OnLoopToggle (const CEGUI::EventArgs& e)
{
  CEGUI::RadioButton * radioButton1 = 
    static_cast<CEGUI::RadioButton*>(CEGUI::WindowManager::getSingleton().getWindow("Video/Window1/Loop"));

  if (radioButton1->isSelected())
  {
    mediaPlayer->Loop (true);
  }
  else
  {
    mediaPlayer->Loop (false);
  }
  return true;
}
bool VideoTest::OnSeekingStart (const CEGUI::EventArgs&)
{
  updateSeeker=false;
  return true;
}
bool VideoTest::OnSeekingEnd (const CEGUI::EventArgs&)
{

  CEGUI::Scrollbar * seeker = 
    static_cast<CEGUI::Scrollbar*>(CEGUI::WindowManager::getSingleton().getWindow("Video/Window1/Seek"));

  mediaPlayer->Seek (seeker->getScrollPosition ());
  updateSeeker=true;
  return true;
}
//---------------------------------------------------------------------------
