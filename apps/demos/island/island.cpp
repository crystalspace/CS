/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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

#include "island.h"

#define WATER_LEVEL 50.0

IslandDemo::IslandDemo ()
  : DemoApplication ("CrystalSpace.IslandDemo", "csavatarstudio", "csavatarstudio",
		     "Crystal Space island demo."),
    inWater (false)
{
  // Configure the options for DemoApplication

  // Set the camera mode
  cameraManager.SetCameraMode (CS::Demo::CAMERA_MOVE_FREE);
  cameraManager.SetStartPosition (csVector3 (500.0f, 200.0f, 500.0f));
  cameraManager.SetMotionSpeed (10.0f);
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
  csRef<iVFS> VFS (csQueryRegistry<iVFS> (GetObjectRegistry ()));
  VFS->ChDir ("/lev/island");
  if (!loader->LoadMapFile ("world"))
    ReportError("Error couldn't load level!");

  // Setup the the sector and the camera
  room = engine->FindSector ("TerrainSector");
  view->GetCamera ()->SetSector (room);

  printf ("Level loaded...\n");

  return true;
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

int main (int argc, char* argv[])
{
  return IslandDemo ().Main (argc, argv);
}
