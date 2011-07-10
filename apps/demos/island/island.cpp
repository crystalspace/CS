/*
  Copyright (C) 2010-11 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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
#include "cssysdef.h"

#include "iengine/campos.h"
#include "island.h"

#define WATER_LEVEL 50.0

IslandDemo::IslandDemo ()
  : DemoApplication ("CrystalSpace.IslandDemo"), inWater (false)
{
}

void IslandDemo::PrintHelp ()
{
  csCommandLineHelper commandLineHelper;

  // Printing help
  commandLineHelper.PrintApplicationHelp
    (GetObjectRegistry (), "csisland", "csisland", "Crystal Space's island environment demo.");
}

void IslandDemo::Frame ()
{
  iCamera* camera = view->GetCamera ();

  // Update the fog when the camera transitions in/out the water
  if ((inWater && camera->GetTransform ().GetOrigin ().y > WATER_LEVEL)
      || (!inWater && camera->GetTransform ().GetOrigin ().y < WATER_LEVEL))
  {
    inWater = !inWater;

    if (inWater)
      room->SetFog (0.001f, csColor (0.3f, 0.3, 0.9f));

    else
      room->DisableFog ();
  }

  // Default behavior from DemoApplication
  DemoApplication::Frame ();
}

bool IslandDemo::Application ()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

   // Create the scene
  if (!CreateScene ())
     return false;

   // Run the application
   Run();

   return true;
}

bool IslandDemo::CreateScene ()
{
  printf ("Loading level...\n");

  // Load the level file named 'world'.
  vfs->ChDir ("/lev/island");
  if (!loader->LoadMapFile ("world"))
    ReportError("Error couldn't load level!");

  // Setup the sector and the camera
  room = engine->FindSector ("TerrainSector");
  view->GetCamera ()->SetSector (room);
  cameraManager->SetCamera (view->GetCamera ());
  cameraManager->SetCameraMode (CS::Utility::CAMERA_MOVE_FREE);
  cameraManager->SetMotionSpeed (10.0f);

  printf ("Precaching data...\n");
  engine->PrecacheDraw ();

  printf ("Ready!\n");

  return true;
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

int main (int argc, char* argv[])
{
  return IslandDemo ().Main (argc, argv);
}
