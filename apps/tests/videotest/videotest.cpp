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

#define WATER_LEVEL 50.0

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
  iCamera* camera = view->GetCamera ();

  //draw the room
  view->Draw();


  // Default behavior from DemoApplication
  DemoApplication::Frame ();

  //in order to be able to draw 2D, it seems you need to do it after DemoApplication::Frame ()
  //not really major, but might help when drawing the video on-screen

  int w, h;
  logoTex->GetRendererDimensions (w, h);

  int screenW = g2d->GetWidth ();

  // Margin to the edge of the screen, as a fraction of screen width
  const float marginFraction = 0.01f;
  const int margin = (int)screenW * marginFraction;

  // Width of the logo, as a fraction of screen width
  const float widthFraction = 0.3f;
  const int width = (int)screenW * widthFraction;
  const int height = width * h / w;

  g3d->BeginDraw (CSDRAW_2DGRAPHICS);
  g3d->DrawPixmap (logoTex, 
                          10, 
                          10,
                          width,
                          height,
                          0,
                          0,
                          w,
                          h,
                          0);

}

bool VideoTest::Application ()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  if (!csInitializer::RequestPlugins (object_reg,
        CS_REQUEST_PLUGIN ("crystalspace.vpl.loader", iVPLLoader),
 	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.vidplaydemo",
	"Can't initialize plugins!");
    return false;
  }

  csRef<iVPLLoader> vlpLoader = csQueryRegistry<iVPLLoader> (object_reg);
  csRef<iVPLData> a= vlpLoader->LoadSound("123pixel_aspect_ratio.ogg");

   // Create the scene
  if (!CreateScene ())
    return false;

   // Run the application
  Run();

  return true;
}

bool VideoTest::CreateScene ()
{
  printf ("Creating level...\n");

  logoTex = loader->LoadTexture ("/lib/std/cslogo2.png", CS_TEXTURE_2D, NULL);
  if (!logoTex.IsValid ())
  {
    return ReportError("Could not load logo %s!",
		       "/lib/std/cslogo2.png");
  }

  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError ("Error loading %s texture!",
		 CS::Quote::Single ("stone4"));
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
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
    engine, room, "walls", "walls_factory", &box);
  walls->GetMeshObject ()->SetMaterialWrapper (tm);

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

  return true;
}
//---------------------------------------------------------------------------