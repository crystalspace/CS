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

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError ("Error loading %s texture!",
		 CS::Quote::Single ("stone4"));
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  csRef<iMaterialWrapper> simpleMaterial =
    CS::Material::MaterialBuilder::CreateColorMaterial
    (object_reg,"boxmaterial",csColor(0,1,0));
 
  // We create a new sector called "room".
  room = engine->CreateSector ("room");  

//   // Make a simple box which cast shadows
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> walls = GeneralMeshBuilder::CreateFactoryAndMesh (
    engine, room, "walls", "walls_factory", &box);
  walls-> GetMeshObject ()-> SetMaterialWrapper(tm);
  csRef<iGeneralMeshState> wallsMeshstate = scfQueryInterface<iGeneralMeshState> (
    walls->GetMeshObject ());
  wallsMeshstate->SetLighting (true);
//   wallsMeshstate->SetShadowCasting(true);
//   wallsMeshstate->SetShadowReceiving(true)

  Box simpleBox (3 * csVector3 (-.1, -.1, -.1), 3 * csVector3 (.1, .1, .1));
  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> mesh = GeneralMeshBuilder::CreateFactoryAndMesh (
    engine, room, "cube", "cubeFact", &simpleBox);
  mesh->GetMovable()->SetPosition(csVector3(0, 2, 0));
  mesh->GetMeshObject ()->SetMaterialWrapper (simpleMaterial);

  mesh->SetZBufMode(CS_ZBUF_TEST);
  mesh->GetMeshObject()->SetMixMode(CS_FX_SETALPHA(0.1));

  csRef<iGeneralMeshState> meshstate = scfQueryInterface<iGeneralMeshState> (
    mesh->GetMeshObject ());
  meshstate->SetLighting (true);
//   meshstate->SetShadowCasting(true);
//   meshstate->SetShadowReceiving(true);

  Box simpleSmallBox (csVector3 (-.1, -.1, -.1), csVector3 (.1, .1, .1));
  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> meshSmall = GeneralMeshBuilder::CreateFactoryAndMesh (
    engine, room, "cubeSmall", "cubeFactSmall", &simpleSmallBox);
  meshSmall->GetMovable()->SetPosition(csVector3(0, 1, 0));
  meshSmall->GetMeshObject ()->SetMaterialWrapper (simpleMaterial);

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  // first light - vertical light
  light = engine->CreateLight (0, csVector3 (0, 20, 0), 100, csColor (1, 1, 1));
  light->SetType(CS_LIGHT_DIRECTIONAL);

  csMatrix3 matrixY (cos(PI/4), 0, -sin(PI/4), 0, 1, 0, sin(PI/4), 0, cos(PI/4));
  csMatrix3 matrixX (1, 0, 0, 0, cos(PI/2), -sin(PI/2), 0, sin(PI/2), cos(PI/2));
  light->GetMovable()->Transform(matrixY * matrixX); 

  ll->Add (light);

  // second light - horizontal light
  light = engine->CreateLight (0, csVector3 (0, 20, 0), 100, csColor (1, 0, 1));
  light->SetType(CS_LIGHT_DIRECTIONAL);

  light->GetMovable()->Transform(matrixY); 

  ll->Add (light);

  // Setup the sector and the camera
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 3, -7));
  cameraManager->SetCamera(view->GetCamera());
  cameraManager->SetCameraMode (CS::Utility::CAMERA_ROTATE);
  cameraManager->SetMotionSpeed (10.0f);

//   printf ("Precaching data...\n");
//   engine->PrecacheDraw ();

  printf ("Ready!\n");

  return true;
}
