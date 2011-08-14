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

  float moveFactor = 0;
  csMatrix3 rotateMatrix = csMatrix3();
  float rotateFactor = speed;

  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    if (csKeyEventHelper::GetCookedCode (&ev) == 'a')
    {
      rotateMatrix = csMatrix3(1, 0, 0, 0, cos(rotateFactor), -sin(rotateFactor), 
        0, sin(rotateFactor), cos(rotateFactor));
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'q')
    {
      rotateMatrix = csMatrix3(1, 0, 0, 0, cos(rotateFactor), sin(rotateFactor), 
        0, -sin(rotateFactor), cos(rotateFactor));
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'd')
    {
      rotateMatrix = csMatrix3(cos(rotateFactor), -sin(rotateFactor), 0,
        sin(rotateFactor), cos(rotateFactor), 0, 0, 0, 1);
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'e')
    {
      rotateMatrix = csMatrix3(cos(rotateFactor), sin(rotateFactor), 0,
        -sin(rotateFactor), cos(rotateFactor), 0, 0, 0, 1);
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 's')
    {
      moveFactor = -speed;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'w')
    {
      moveFactor = speed;
    }

    light->GetMovable()->Transform(rotateMatrix);

    csVector3 oldDirection = light->GetMovable()->GetPosition();
    float oldLength = oldDirection.Norm();
    oldDirection.Normalize();

    csVector3 newDirection = (rotateMatrix * oldDirection);
    float newLength = oldLength + moveFactor;
    newDirection.Normalize();

    csVector3 position = newDirection * newLength;
    light->GetMovable()->SetPosition(position);
    light->GetMovable()->UpdateMove();

    return true;
  }

  return false;
}

bool SelfShadowDemo::OnInitialize (int argc, char* argv[])
{
  // Default behavior from csDemoApplication
  if (!DemoApplication::OnInitialize (argc, argv))
    return false;

  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN("crystalspace.mesh.object.furmesh", CS::Mesh::iFurMeshType),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  return true;
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
  char *world = "world_tree";

  printf ("Loading level...\n");
  vfs->ChDir ("/lev/selfshadow");
  if (!loader->LoadMapFile (world))
    ReportError("Error couldn't load level!");

//   LoadKrystal();

  engine->Prepare ();

  // Setup the camera
  cameraManager->SetCamera(view->GetCamera());
  cameraManager->SetCameraMode (CS::Utility::CAMERA_ROTATE);
  cameraManager->SetMotionSpeed (10.0f);

  printf ("Ready!\n");

  return true;
}

void SelfShadowDemo::LoadKrystal()
{
  iSector* sector = engine->FindSector("Scene");

  if (!sector)
    ReportError("Could not find default room!");

  printf ("Loading Krystal...\n");

  // Load animesh factory
  csLoadResult rc = loader->Load ("/lib/krystal/krystal.xml");
  if (!rc.success)
    ReportError ("Can't load Krystal library file!");

  csRef<iMeshFactoryWrapper> meshfact =
    engine->FindMeshFactory ("krystal");
  if (!meshfact)
    ReportError ("Can't find Krystal's mesh factory!");

  csRef<CS::Mesh::iAnimatedMeshFactory> animeshFactory = 
    scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());
  if (!animeshFactory)
    ReportError ("Can't find Krystal's animesh factory!");

  // Create the animated mesh
  csRef<iMeshWrapper> avatarMesh =
    engine->CreateMeshWrapper (meshfact, "krystal",
    room, csVector3 (0.0f));

  csRef<CS::Mesh::iAnimatedMesh> animesh = 
    scfQueryInterface<CS::Mesh::iAnimatedMesh> (avatarMesh->GetMeshObject ());
  avatarMesh->GetMovable()->SetSector(sector);
  avatarMesh->GetMovable()->UpdateMove();

  // Load some fur
  rc = loader->Load ("/lib/hairtest/krystal_furmesh.xml");
  if (!rc.success)
    ReportError ("Can't load krystal furmesh library!");

  csRef<iMeshWrapper> krystalFurmeshObject = 
    engine->FindMeshObject ("krystal_furmesh_object");
  if (!krystalFurmeshObject)
    ReportError ("Can't find fur mesh object!");
  krystalFurmeshObject->SetRenderPriority(engine->GetRenderPriority("alpha"));

  krystalFurmeshObject->GetMovable()->SetSector(sector);
  krystalFurmeshObject->GetMovable()->UpdateMove();

  // Load the fur material
  rc = loader->Load ("/lib/hairtest/fur_material_krystal.xml");
  if (!rc.success)
    ReportError ("Can't load Fur library file!");

  // Find the fur mesh plugin
  csRef<CS::Mesh::iFurMeshType> furMeshType = 
    csQueryRegistry<CS::Mesh::iFurMeshType> (GetObjectRegistry ());
  if (!furMeshType)
    ReportError("Failed to locate CS::Mesh::iFurMeshType plugin!");

  // Load the Marschner shader
  csRef<iMaterialWrapper> materialWrapper = 
    engine->FindMaterial ("marschner_material");
  if (!materialWrapper)
    ReportError ("Can't find marschner material!");

  // Create the fur properties for the hairs
  csRef<CS::Mesh::iFurMeshMaterialProperties> hairMeshProperties = 
    furMeshType->CreateHairMeshMarschnerProperties ("krystal_marschner");
  hairMeshProperties->SetMaterial(materialWrapper->GetMaterial ());
  animesh->GetSubMesh (1)->SetMaterial (materialWrapper);

  csRef<CS::Animation::iFurAnimatedMeshControl> animationPhysicsControl = 
    scfQueryInterface<CS::Animation::iFurAnimatedMeshControl>
    (furMeshType->CreateFurAnimatedMeshControl ("krystal_hairs_animation"));

  animationPhysicsControl->SetAnimatedMesh (animesh);

  csRef<iMeshObject> imo = krystalFurmeshObject->GetMeshObject();

  // Get reference to the iFurMesh interface
  csRef<CS::Mesh::iFurMesh> furMesh = scfQueryInterface<CS::Mesh::iFurMesh> (imo);

  csRef<CS::Mesh::iFurMeshState> ifms = 
    scfQueryInterface<CS::Mesh::iFurMeshState> (furMesh);

  animationPhysicsControl->SetDisplacement (ifms->GetDisplacement ());

  furMesh->SetFurMeshProperties (hairMeshProperties);

  // Shader variables
  csRef<iShaderVarStringSet> svStrings = 
    csQueryRegistryTagInterface<iShaderVarStringSet> (
    object_reg, "crystalspace.shader.variablenameset");

  if (!svStrings) 
   ReportError ("No SV names string set!\n");

  // remove diffuse, blonde and transparent
  CS::ShaderVarName objColor (svStrings, "fur color");	

  csVector4 color = csVector4(0.51f, 0.34f, 0.25f, 0.4f); 
  materialWrapper->GetMaterial()->GetVariableAdd(objColor)->SetValue(color);  

  CS::ShaderVarName objTexture (svStrings, "texture map");	
  materialWrapper->GetMaterial()-> RemoveVariable(objTexture);  

  CS::ShaderVarName diffuseType (svStrings, "diffuse type");	
  // use ambient instead
  materialWrapper->GetMaterial()-> GetVariableAdd(diffuseType)->SetValue(100);  

  furMesh->GetFurMeshProperties()->Invalidate();

  furMesh->SetAnimatedMesh (animesh);
  furMesh->SetMeshFactory (animeshFactory);
  furMesh->SetMeshFactorySubMesh (animesh->GetSubMesh (2)->GetFactorySubMesh ());
  furMesh->GenerateGeometry (view, room);

  furMesh->SetAnimationControl (animationPhysicsControl);
  furMesh->StartAnimationControl ();

  furMesh->SetGuideLOD (0);
  furMesh->SetStrandLOD (1);
  furMesh->SetControlPointsLOD (0.0f);

  furMesh->ResetMesh ();
}
