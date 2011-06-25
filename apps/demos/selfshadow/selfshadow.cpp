/*
  Copyright (C) 2011 Alexandru - Teodor Voicu
      Imperial College London
      http://www3.imperial.ac.uk/

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

#include "iengine/campos.h"
#include "selfshadow.h"

SelfShadowDemo::SelfShadowDemo ()
: DemoApplication ("CrystalSpace.SelfShadow")
{
}

void SelfShadowDemo::PrintHelp ()
{
  csCommandLineHelper commandLineHelper;

  // Printing help
  commandLineHelper.PrintApplicationHelp
    (GetObjectRegistry (), "selfshadowdemo", "selfshadowdemo", 
      "Crystal Space's self shadow RM demo.");
}

void SelfShadowDemo::Frame ()
{
  // Default behavior from DemoApplication
  DemoApplication::Frame ();
}

bool SelfShadowDemo::OnKeyboard (iEvent &ev)
{
  // Default behavior from csDemoApplication
  DemoApplication::OnKeyboard (ev);
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  csRef<iLight> light = engine->GetSectors()->Get(0)->GetLights()->Get(0);

  float moveFactor = 10 * speed;
  float rotateFactor = asin(moveFactor / light->GetMovable()->GetPosition().Norm());

  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    if (csKeyEventHelper::GetCookedCode (&ev) == 'a')
    {
      csMatrix3 matrixX (1, 0, 0, 0, cos(rotateFactor), -sin(rotateFactor), 
        0, sin(rotateFactor), cos(rotateFactor));
      light->GetMovable()->Transform(matrixX);

      light->GetMovable()->MovePosition(csVector3(0, 0, moveFactor));
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'q')
    {
      csMatrix3 matrixX (1, 0, 0, 0, cos(rotateFactor), sin(rotateFactor), 
        0, -sin(rotateFactor), cos(rotateFactor));
      light->GetMovable()->Transform(matrixX);

      light->GetMovable()->MovePosition(csVector3(0, 0, -moveFactor));
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 's')
    {
      light->GetMovable()->MovePosition(csVector3(0, moveFactor, 0));
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'w')
    {
      light->GetMovable()->MovePosition(csVector3(0, -moveFactor, 0));
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'd')
    {
      csMatrix3 matrixY (cos(rotateFactor), -sin(rotateFactor), 0,
        sin(rotateFactor), cos(rotateFactor), 0, 0, 0, 1);
      light->GetMovable()->Transform(matrixY);

      light->GetMovable()->MovePosition(csVector3(-moveFactor, 0, 0));
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'e')
    {
      csMatrix3 matrixY (cos(rotateFactor), sin(rotateFactor), 0,
        -sin(rotateFactor), cos(rotateFactor), 0, 0, 0, 1);
      light->GetMovable()->Transform(matrixY);

      light->GetMovable()->MovePosition(csVector3(moveFactor, 0, 0));
    }

    light->GetMovable()->UpdateMove();

    return true;
  }

  return false;
}

bool SelfShadowDemo::Application ()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  /* NOTE: Config settings for render managers are stored in 'engine.cfg' 
   * and are needed when loading a render manager. Normally these settings 
   * are added by the engine when it loads a render manager. However, since
   * we are loading the shadow_pssm render manager manually we must also manually
   * add the proper config file. */
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  cfg->AddDomain ("/config/engine.cfg", vfs, iConfigManager::ConfigPriorityPlugin);

  csRef<iRenderManager> rm = csLoadPlugin<iRenderManager> (GetObjectRegistry(), 
    "crystalspace.rendermanager.osm");
  if (!rm)
    return ReportError("Failed to load OSM Render Manager!");

  cfg->RemoveDomain ("/config/engine.cfg");

  engine->SetRenderManager(rm);

  // Create the scene
  if (!CreateScene ())
    return false;

  // Run the application
  Run();

  return true;
}

bool SelfShadowDemo::CreateScene ()
{
  char *world = "world_grass";

  printf ("Loading level...\n");
  vfs->ChDir ("/lev/selfshadow");
  if (!loader->LoadMapFile (world))
    ReportError("Error couldn't load level!");

  engine->Prepare ();

  // Setup the camera
  cameraManager->SetCamera(view->GetCamera());
  cameraManager->SetCameraMode (CS::Utility::CAMERA_ROTATE);
  cameraManager->SetMotionSpeed (10.0f);

  printf ("Ready!\n");

  return true;
}
