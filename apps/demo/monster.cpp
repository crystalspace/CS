/*
  Copyright (C) 2010 Jelle Hellemans

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "monster.h"

#include "player.h"

#include <string>


static float GetAngle (float x, float y)
{
  if (x > 1.0 )  x = 1.0;
  if (x < -1.0 ) x = -1.0;

  float angle = acos (x);
  if (y < 0)
    angle = 2*PI - angle;

  return angle;
}

static float Matrix2YRot (const csMatrix3& mat)
{
  csVector3 vec (0,0,1);
  vec = mat * vec;

  return GetAngle (vec.z, vec.x);
}

Monster::Monster(iObjectRegistry* obj_reg, iMeshWrapper* spawn) : Entity(obj_reg)
{
  csRef<iView> view (csQueryRegistry<iView> (object_reg));
  csRef<iConfigManager> cfg (csQueryRegistry<iConfigManager> (object_reg));

  //Replace spawn mesh with real mesh.
  std::string name = spawn->QueryObject()->GetName();

  std::string tmp, filename;
  filename = name;
  filename = filename.substr(7, filename.length());
  size_t dotpos = filename.find_first_of(".");
  filename = filename.substr(0, dotpos);

  std::string path = "/data/bias/models/";
  path += filename;
  path += "/";
  path += filename;

  std::string factname = "gen";
  factname += filename; 

  mesh = LoadMesh(factname.c_str(), path.c_str());

  csVector3 pos = spawn->GetMovable()->GetPosition();
  iSector* sector = spawn->GetMovable()->GetSectors()->Get(0);
  mesh->GetMovable()->SetPosition(sector, pos);
  mesh->GetMovable()->UpdateMove();

  mesh->QueryObject()->ObjAdd(this);

  weapon->mesh = mesh;


  float cfg_body_height = cfg->GetFloat ("Walktest.CollDet.BodyHeight", 0.5f);
  float cfg_body_width = cfg->GetFloat ("Walktest.CollDet.BodyWidth", 0.5f);
  float cfg_body_depth = cfg->GetFloat ("Walktest.CollDet.BodyDepth", 0.5f);
  float cfg_eye_offset = cfg->GetFloat ("Walktest.CollDet.EyeOffset", 1.2f);
  float cfg_legs_width = cfg->GetFloat ("Walktest.CollDet.LegsWidth", 0.4f);
  float cfg_legs_depth = cfg->GetFloat ("Walktest.CollDet.LegsDepth", 0.4f);
  float cfg_legs_offset = cfg->GetFloat ("Walktest.CollDet.LegsOffset", 0.0f);

  csVector3 legs (cfg_legs_width, cfg_eye_offset-cfg_legs_offset, cfg_legs_depth);
  csVector3 body (cfg_body_width, cfg_body_height, cfg_body_depth);
  csVector3 shift (0, cfg_legs_offset, 0);

  collider_actor.InitializeColliders (mesh, legs, body, shift);


  awareRadius = curAwareRadius = 10.0f;
}

Monster::~Monster()
{
}

void Monster::Behaviour()
{
  if (died | frozen) return;


  static float desiredAngle = 0.0f;
  static bool angleToReachFlag = false;

  csRef<iBase> base = csQueryRegistryTag(object_reg, "Player");
  Player* player = dynamic_cast<Player*>(&*base);

  if (!player) return;
  
  if (weapon->IsReady())
  {
    //printf("I: player found!\n");
    csVector3 v1, v2;
    v2 = this->GetPosition();
    v1 = player->GetPosition();

    if ((v2 - v1).Norm() < 2) // We're at the player
    {
      // Stop all movement.
      Stop();
      if (weapon->Fire())
      {
        printf("Monster HIT\n");
        weapon->ApplyDamage(player);
      }
    }
    else if (sqrt (csSquaredDist::PointPoint (v1, v2)) < curAwareRadius) // chase the player
    {
      float len = sqrt (csSquaredDist::PointPoint (v1, v2));
      float angle = acos ((v2.x-v1.x) / len);
      if ((v2.z-v1.z) > 0) angle = 2*PI - angle;
      angle += PI / 2.0f;
      if (angle > 2*PI) angle -= 2*PI;

      desiredAngle = angle;
      angleToReachFlag = true;

      Step(-1);
    }
    else
    {
      Step(0);
      angleToReachFlag = false;
    }
  }
  
  if (angleToReachFlag)
  {
    float currentAngle = Matrix2YRot(mesh->GetMovable()->GetTransform().GetT2O());
    currentAngle = atan2f (sin (currentAngle), cos (currentAngle) );

    if (fabs(desiredAngle - currentAngle) > 0.01f) 
    {
      csTicks elapsed_time = vc->GetElapsedTicks ();
      float delta = float (elapsed_time) / 1000.0f;
      delta *= 1700.0f;
      float angle = 100.0f * cfg_rotate_maxspeed * cfg_walk_maxspeed_multreal * delta;

      float yrot_delta = fabs (atan2f (sin (desiredAngle - currentAngle), cos (desiredAngle - currentAngle)));

      //printf("%f\n", yrot_delta);
      if (fabs(angle) > yrot_delta)
      {
        angle = (angle / fabs (angle)) * yrot_delta;
        //angle_velocity = 0;
        //desired_angle_velocity = 0;
        angleToReachFlag = false;
        desiredAngle = currentAngle;
        printf("angle reached!\n");
      }

      csYRotMatrix3 rotMat (angle);
      mesh->GetMovable()->SetTransform (mesh->GetMovable()->GetTransform().GetT2O() * rotMat);
    }
  }
}

csVector3 Monster::GetPosition()
{
  return mesh->GetMovable()->GetPosition();
}

void Monster::PlayAnimation (const char* script, bool lock)
{
  csRef<iGeneralMeshState> spstate (scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject ()));
  csRef<iGenMeshSkeletonControlState> animcontrol (scfQueryInterface<iGenMeshSkeletonControlState> (spstate->GetAnimationControl ()));
  iSkeleton* skeleton = animcontrol->GetSkeleton ();

  skeleton->StopAll();
  skeleton->ClearPendingAnimations();

  if (script)
  {
    if(lock)
      skeleton->Append(script);
    else
    {
      skeleton->Execute(script);
      skeleton->Append("idle");
    }
  }
}

void Monster::Explode()
{
  // Stop all movement.
  Stop();

  csVector3 pos = mesh->GetMovable()->GetPosition();
  iSector* sector = mesh->GetMovable()->GetSectors()->Get(0);

  //Remove old mesh.
  csRef<iEngine> engine (csQueryRegistry<iEngine> (object_reg));
  engine->WantToDie(mesh);

  // Change the mesh.
  mesh = LoadMesh("gengibs", "/data/bias/models/iceblocks/gibs");
  mesh->GetMovable()->SetPosition(sector, pos);
  mesh->GetMovable()->UpdateMove();

  collider_actor.InitializeColliders (mesh, csVector3(0), csVector3(0), csVector3(0));

  // Add more gibs
  iMeshFactoryWrapper* fact = engine->FindMeshFactory("gengibs");
  csRef<iMeshWrapper> meshexplo;

  //1
  meshexplo = fact->CreateMeshWrapper();
  meshexplo->GetMovable()->Transform(csYRotMatrix3(0));
  meshexplo->QuerySceneNode()->SetParent(mesh->QuerySceneNode());
  meshexplo->GetMovable()->UpdateMove();

  //2
  meshexplo = fact->CreateMeshWrapper();
  meshexplo->GetMovable()->Transform(csYRotMatrix3(-1.97051f));
  meshexplo->QuerySceneNode()->SetParent(mesh->QuerySceneNode());
  meshexplo->GetMovable()->UpdateMove();

  //3
  meshexplo = fact->CreateMeshWrapper();
  meshexplo->GetMovable()->Transform(csYRotMatrix3(2.9665f));
  meshexplo->QuerySceneNode()->SetParent(mesh->QuerySceneNode());
  meshexplo->GetMovable()->UpdateMove();
}

void Monster::ChangeMaterial()
{
  // Get the new material.
  csRef<iEngine> engine (csQueryRegistry<iEngine> (object_reg));
  iMaterialWrapper* newmatw = engine->FindMaterial("kwartz-ice");
  if (!newmatw) return;

  // Get MeshObject.
  iMeshObject* meshobj = mesh->GetMeshObject();
  if (!meshobj) return;

  // Change the material.
  meshobj->SetMaterialWrapper(newmatw);
}
