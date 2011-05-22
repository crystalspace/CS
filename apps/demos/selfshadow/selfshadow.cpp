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

bool SelfShadowDemo::Application ()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  // Manual loading RM
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  cfg->AddDomain ("/config/engine.cfg", vfs, iConfigManager::ConfigPriorityPlugin);

  csRef<iRenderManager> rm = csLoadPlugin<iRenderManager> (GetObjectRegistry(), 
    "crystalspace.rendermanager.shadow_pssm");
  if (!rm)
    return ReportError("Failed to load deferred Render Manager!");

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
  printf ("Loading level...\n");
  vfs->ChDir ("/lev/selfshadow");
  if (!loader->LoadMapFile ("world"))
    ReportError("Error couldn't load level!");

  // We create a new sector called "room".
  room = engine->FindSector ("Scene");  
  if (!room)
    ReportError("Sector not found!");

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  // first light - vertical light
  light = engine->CreateLight (0, csVector3 (0, 10, 0), 100, csColor (1, 1, 1));
  light->SetType(CS_LIGHT_DIRECTIONAL);

  csMatrix3 matrixY (cos(PI/4), 0, -sin(PI/4), 0, 1, 0, sin(PI/4), 0, cos(PI/4));
  csMatrix3 matrixX (1, 0, 0, 0, cos(PI/2), -sin(PI/2), 0, sin(PI/2), cos(PI/2));
  light->GetMovable()->Transform(matrixY * matrixX); 

  ll->Add (light);

  // Setup the sector and the camera
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -10));
  cameraManager->SetCamera(view->GetCamera());
  cameraManager->SetCameraMode (CS::Utility::CAMERA_ROTATE);
  cameraManager->SetMotionSpeed (10.0f);

//   printf ("Precaching data...\n");
//   engine->PrecacheDraw ();

  printf ("Ready!\n");

  return true;
}
