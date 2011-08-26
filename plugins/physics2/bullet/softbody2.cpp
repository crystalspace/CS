/*
  Copyright (C) 2011 by Liu Lu

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
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csgeom/quaternion.h"
#include "csgeom/vector3.h"
#include "iutil/strset.h"

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"

#include "softbody2.h"
#include "rigidbody2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
csBulletSoftBody::csBulletSoftBody (csBulletSystem* phySys, btSoftBody* body)
  :scfImplementationType (this, phySys), btBody (body), anchorCount (0),
  friction (5.0f), density (0.1f)
{
  btObject = body;
  btBody->setUserPointer (dynamic_cast<iPhysicalBody*> (this));
  this->type = CS::Collision2::COLLISION_OBJECT_PHYSICAL;
}

csBulletSoftBody::~csBulletSoftBody ()
{
  RemoveBulletObject ();
  (sector->anchoredSoftBodies).Delete (this);
}

void csBulletSoftBody::SetTransform (const csOrthoTransform& trans)
{
  btTransform btTrans = CSToBullet (trans, system->getInternalScale ());
  btTransform t = transform.inverse () * btTrans;
  transform = btTrans;
  btBody->transform (t);
}

csOrthoTransform csBulletSoftBody::GetTransform ()
{
  return BulletToCS (transform, system->getInverseInternalScale ());
}

void csBulletSoftBody::RebuildObject ()
{
  btBody->setCollisionFlags(0);
}

CS::Collision2::HitBeamResult csBulletSoftBody::HitBeam (const csVector3& start, const csVector3& end)
{
  CS::Collision2::HitBeamResult result;
  btVector3 rayFrom = CSToBullet (start, system->getInternalScale ());
  btVector3 rayTo = CSToBullet (end, system->getInternalScale ());
  btSoftBody::sRayCast ray;

  btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);
  if (btBody->rayTest (rayFrom, rayTo, ray))
  {
    result.hasHit = true;
    result.object = QueryCollisionObject ();
    result.isect = BulletToCS (rayCallback.m_hitPointWorld,
      system->getInverseInternalScale ());
    result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
      system->getInverseInternalScale ());	
    
    btVector3 impact = rayFrom + (rayTo - rayFrom) * ray.fraction;
    switch (ray.feature)
    {
    case btSoftBody::eFeature::Face:
      {
        btSoftBody::Face& face = btBody->m_faces[ray.index];
        btSoftBody::Node* node = face.m_n[0];
        float distance = (node->m_x - impact).length2 ();

        for (int i = 1; i < 3; i++)
        {
          float nodeDistance = (face.m_n[i]->m_x - impact).length2 ();
          if (nodeDistance < distance)
          {
            node = face.m_n[i];
            distance = nodeDistance;
          }
        }
        result.vertexIndex = (size_t) (node - &btBody->m_nodes[0]);
        break;
      }
    default:
      break;
    }
  }
  return result;
}

bool csBulletSoftBody::RemoveBulletObject ()
{
  if (insideWorld)
  {
    for (size_t i = 0; i < joints.GetSize (); i++)
      sector->RemoveJoint (joints[i]);
    btSoftRigidDynamicsWorld* softWorld =
      static_cast<btSoftRigidDynamicsWorld*> (sector->bulletWorld);
    softWorld->removeSoftBody (btBody);
    insideWorld = false;
    anchorCount = 0;
    animatedAnchors.DeleteAll ();
    sector->anchoredSoftBodies.Delete (this);
    for (int i = 0; i < btBody->m_anchors.size (); i++)
    {
      btRigidBody* btRB = btBody->m_anchors[i].m_body;
      CS::Collision2::iCollisionObject* collObject = static_cast<CS::Collision2::iCollisionObject*> (
        btRB->getUserPointer ());
      csBulletRigidBody* rb = dynamic_cast<csBulletRigidBody*> (collObject);
      rb->anchorCount -- ;
    }
    return true;
  }
  return false;
}

bool csBulletSoftBody::AddBulletObject ()
{
  if (!insideWorld)
  {
    btBody->m_worldInfo = sector->softWorldInfo;
    btSoftRigidDynamicsWorld* softWorld =
      static_cast<btSoftRigidDynamicsWorld*> (sector->bulletWorld);

    softWorld->addSoftBody (btBody, collGroup.value, collGroup.mask);
    btBody->setUserPointer (static_cast<CS::Collision2::iCollisionObject*> (
      dynamic_cast<iPhysicalBody*>(this)));
    insideWorld = true;
  }
  return true;
}

bool csBulletSoftBody::Disable ()
{
  SetLinearVelocity (csVector3 (0.0f));
  CS_ASSERT (btBody);
  btBody->setActivationState (ISLAND_SLEEPING);
  return true;
}

bool csBulletSoftBody::Enable ()
{
  CS_ASSERT (btBody);
  btBody->setActivationState (ACTIVE_TAG);
  return true;
}

bool csBulletSoftBody::IsEnabled ()
{
 CS_ASSERT (btBody);
 return btBody->isActive ();
}

void csBulletSoftBody::SetMass (float mass)
{
  CS_ASSERT (btBody);
  this->totalMass = mass;
  btBody->setTotalMass (mass);
}

float csBulletSoftBody::GetMass ()
{
  CS_ASSERT (btBody);
  return btBody->getTotalMass ();
}

void csBulletSoftBody::SetDensity (float density)
{
  CS_ASSERT (btBody);
  this->density = density;
  btBody->setTotalDensity (density);
}

float csBulletSoftBody::GetVolume ()
{
  CS_ASSERT (btBody);
  return btBody->getVolume ();
}

void csBulletSoftBody::AddForce (const csVector3& force)
{
  CS_ASSERT (btBody);
  btBody->addForce (CSToBullet (force, system->getInternalScale ()));
}

void csBulletSoftBody::SetLinearVelocity (const csVector3& vel)
{
  CS_ASSERT (btBody);
  btBody->setVelocity (CSToBullet (vel, system->getInternalScale ()));
}

csVector3 csBulletSoftBody::GetLinearVelocity (size_t index /* = 0 */) const
{
  CS_ASSERT ( btBody && index < (size_t) btBody->m_nodes.size ());
  return BulletToCS (btBody->m_nodes[index].m_v, system->getInverseInternalScale ());
}

void csBulletSoftBody::SetFriction (float friction)
{
  CS_ASSERT (btBody);
  this->friction = friction;
  if (friction >= 0.0f && friction <= 1.0f)
    btBody->m_cfg.kDF = friction;
}

void csBulletSoftBody::SetVertexMass (float mass, size_t index)
{
  CS_ASSERT (btBody);
  btBody->setMass (index, mass);
}

float csBulletSoftBody::GetVertexMass (size_t index)
{
  CS_ASSERT (btBody);
  return btBody->getMass (index);
}

size_t csBulletSoftBody::GetVertexCount ()
{
  CS_ASSERT (btBody);
  return btBody->m_nodes.size ();
}

csVector3 csBulletSoftBody::GetVertexPosition (size_t index) const
{
  CS_ASSERT(btBody && index < (size_t) btBody->m_nodes.size ());
  return BulletToCS (btBody->m_nodes[index].m_x, system->getInverseInternalScale ());
}

void csBulletSoftBody::AnchorVertex (size_t vertexIndex)
{
  CS_ASSERT(vertexIndex < (size_t) btBody->m_nodes.size ());
  btBody->setMass (vertexIndex, 0.0f);
  anchorCount ++;
}

void csBulletSoftBody::AnchorVertex (size_t vertexIndex, iRigidBody* body)
{
  csBulletRigidBody* rigidBody = static_cast<csBulletRigidBody*> (body);
  CS_ASSERT(rigidBody
    && vertexIndex < (size_t) this->btBody->m_nodes.size ()
    && rigidBody->btBody);
  this->btBody->appendAnchor (vertexIndex, rigidBody->btBody);
  anchorCount ++;
  rigidBody->anchorCount ++;
}

void csBulletSoftBody::AnchorVertex (size_t vertexIndex,
                                     CS::Physics2::iAnchorAnimationControl* controller)
{
  AnimatedAnchor anchor (vertexIndex, controller);
  animatedAnchors.Push (anchor);
  (sector->anchoredSoftBodies).Push (this);
  anchorCount ++;
}

void csBulletSoftBody::UpdateAnchor (size_t vertexIndex, csVector3& position)
{
  CS_ASSERT(vertexIndex < (size_t) btBody->m_nodes.size ());

  // Update the local position of the anchor
  for (int i = 0; i < this->btBody->m_anchors.size (); i++)
    if (this->btBody->m_anchors[i].m_node == &this->btBody->m_nodes[vertexIndex])
    {
      this->btBody->m_anchors[i].m_local =
        this->btBody->m_anchors[i].m_body->getInterpolationWorldTransform ().inverse ()
        * CSToBullet (position, system->getInternalScale ());
      return;
    }
}

void csBulletSoftBody::RemoveAnchor (size_t vertexIndex)
{
  CS_ASSERT(vertexIndex < (size_t) btBody->m_nodes.size ());

  // Check if it is a fixed anchor
  if (btBody->getMass (vertexIndex) < SMALL_EPSILON)
  {
    btBody->setMass (vertexIndex, btBody->getTotalMass () / btBody->m_nodes.size ());
    anchorCount --;
    return;
  }

  // Check if it is an animated anchor
  size_t index = 0;
  for (csArray<AnimatedAnchor>::Iterator it = animatedAnchors.GetIterator (); it.HasNext (); index++)
  {
    AnimatedAnchor& anchor = it.Next ();
    if (anchor.vertexIndex == vertexIndex)
    {
      animatedAnchors.DeleteIndex (index);
      (sector->anchoredSoftBodies).Delete (this);
      anchorCount --;
      return;
    }
  }

  // Check if it is a simple 'rigid body' anchor
  for (int i = 0; i < this->btBody->m_anchors.size (); i++)
    if (this->btBody->m_anchors[i].m_node == &this->btBody->m_nodes[vertexIndex])
    {
      // TODO: this is not possible within Bullet
      //btSoftBody::Anchor* anchor = this->body->m_anchors[i];
      //this->body->m_anchors.remove (i);
      return;
    }
}

float csBulletSoftBody::GetRigidity ()
{
  CS_ASSERT (btBody);
  return this->btBody->m_materials[0]->m_kLST;
}

void csBulletSoftBody::SetRigidity (float rigidity)
{
  CS_ASSERT(rigidity >= 0.0f && rigidity <= 1.0f);

  btBody->m_materials[0]->m_kLST = rigidity;
}

void csBulletSoftBody::SetLinearVelocity (const csVector3& velocity, size_t vertexIndex)
{
  CS_ASSERT (vertexIndex < (size_t) btBody->m_nodes.size ());
  btBody->addVelocity (CSToBullet (velocity, system->getInternalScale ())
    - btBody->m_nodes[vertexIndex].m_v, vertexIndex);
}

void csBulletSoftBody::SetWindVelocity (const csVector3& velocity)
{
  CS_ASSERT (btBody);
  btVector3 velo = CSToBullet (velocity, system->getInternalScale ());
  btBody->setWindVelocity (velo);
}

const csVector3 csBulletSoftBody::GetWindVelocity () const
{
  CS_ASSERT (btBody);
  csVector3 velo = BulletToCS (btBody->getWindVelocity (), system->getInternalScale ());
  return velo;
}

void csBulletSoftBody::AddForce (const csVector3& force, size_t vertexIndex)
{
  CS_ASSERT (vertexIndex < (size_t) btBody->m_nodes.size());
  //TODO: in softbodies.cpp the force was multiplied by 100, why?
  btBody->addForce (CSToBullet (force, system->getInternalScale () * system->getInternalScale ()), vertexIndex);
}

size_t csBulletSoftBody::GetTriangleCount ()
{
  CS_ASSERT (btBody);
  return btBody->m_faces.size ();
}

csTriangle csBulletSoftBody::GetTriangle (size_t index) const
{
  CS_ASSERT(index < (size_t) btBody->m_faces.size ());
  btSoftBody::Face& face = btBody->m_faces[index];
  return csTriangle (face.m_n[0] - &btBody->m_nodes[0],
    face.m_n[1] - &btBody->m_nodes[0],
    face.m_n[2] - &btBody->m_nodes[0]);
}

csVector3 csBulletSoftBody::GetVertexNormal (size_t index) const
{
  CS_ASSERT(index < (size_t) btBody->m_nodes.size ());
  csVector3 normal (btBody->m_nodes[index].m_n.getX (),
    btBody->m_nodes[index].m_n.getY (),
    btBody->m_nodes[index].m_n.getZ ());
  normal.Normalize ();
  return normal;
}

void csBulletSoftBody::DebugDraw (iView* rView)
{
  if (!sector->debugDraw)
  {
    sector->debugDraw = new csBulletDebugDraw (system->getInverseInternalScale ());
    sector->bulletWorld->setDebugDrawer (sector->debugDraw);
  }

  btSoftBodyHelpers::Draw (btBody, sector->debugDraw);
  sector->debugDraw->DebugDraw (rView);
}

void csBulletSoftBody::SetLinearStiff (float stiff)
{
  if (stiff >= 0.0f && stiff <= 1.0f)
  {
    btSoftBody::Material*	pm=btBody->m_materials[0];
    pm->m_kLST = stiff;
  }
}

void csBulletSoftBody::SetAngularStiff (float stiff)
{
  CS_ASSERT (btBody);
  if (stiff >= 0.0f && stiff <= 1.0f)
  {
    btSoftBody::Material*	pm=btBody->m_materials[0];
    pm->m_kAST = stiff;
  }
}

void csBulletSoftBody::SetVolumeStiff (float stiff)
{
  CS_ASSERT (btBody);
  if (stiff >= 0.0f && stiff <= 1.0f)
  {
    btSoftBody::Material*	pm=btBody->m_materials[0];
    pm->m_kVST = stiff;
  }
}

void csBulletSoftBody::ResetCollisionFlag ()
{
  CS_ASSERT (btBody);
  btBody->m_cfg.collisions = 0;
}

void csBulletSoftBody::SetClusterCollisionRS (bool cluster)
{
  CS_ASSERT (btBody);
  if (cluster)
    btBody->m_cfg.collisions	+=	btSoftBody::fCollision::CL_RS;
  else
    btBody->m_cfg.collisions	+=	btSoftBody::fCollision::SDF_RS;
}

bool csBulletSoftBody::GetClusterCollisionRS ()
{
  CS_ASSERT (btBody);
  if (btBody->m_cfg.collisions & btSoftBody::fCollision::CL_RS)
    return true;
  return false;
}

void csBulletSoftBody::SetClusterCollisionSS (bool cluster)
{
  CS_ASSERT (btBody);
  if (cluster)
    btBody->m_cfg.collisions += btSoftBody::fCollision::CL_SS;
  else
    btBody->m_cfg.collisions += btSoftBody::fCollision::VF_SS;
}

bool csBulletSoftBody::GetClusterCollisionSS ()
{
  CS_ASSERT (btBody);
  if (btBody->m_cfg.collisions & btSoftBody::fCollision::CL_SS)
    return true;
  return false;
}

void csBulletSoftBody::SetSRHardness (float hardness)
{
  CS_ASSERT (btBody);
  if (hardness >= 0.0f && hardness <= 1.0f)
    btBody->m_cfg.kSRHR_CL = hardness;
}

void csBulletSoftBody::SetSKHardness (float hardness)
{
  CS_ASSERT (btBody);
  if (hardness >= 0.0f && hardness <= 1.0f)
    btBody->m_cfg.kSKHR_CL = hardness;
}

void csBulletSoftBody::SetSSHardness (float hardness)
{
  CS_ASSERT (btBody);
  if (hardness >= 0.0f && hardness <= 1.0f)
    btBody->m_cfg.kSSHR_CL = hardness;
}

void csBulletSoftBody::SetSRImpulse (float impulse)
{
  CS_ASSERT (btBody);
  if (impulse >= 0.0f && impulse <= 1.0f)
    btBody->m_cfg.kSR_SPLT_CL = impulse;
}

void csBulletSoftBody::SetSKImpulse (float impulse)
{
  CS_ASSERT (btBody);
  if (impulse >= 0.0f && impulse <= 1.0f)
    btBody->m_cfg.kSK_SPLT_CL = impulse;
}

void csBulletSoftBody::SetSSImpulse (float impulse)
{
  CS_ASSERT (btBody);
  if (impulse >= 0.0f && impulse <= 1.0f)
    btBody->m_cfg.kSS_SPLT_CL = impulse;
}

void csBulletSoftBody::SetVeloCorrectionFactor (float factor)
{
  CS_ASSERT (btBody);
  btBody->m_cfg.kVCF = factor;
}

void csBulletSoftBody::SetDamping (float damping)
{
  CS_ASSERT (btBody);
  if (damping >= 0.0f && damping <= 1.0f)
    btBody->m_cfg.kDP = damping;
}

void csBulletSoftBody::SetDrag (float drag)
{
  CS_ASSERT (btBody);
  if (drag >= 0.0f)
    btBody->m_cfg.kDG = drag;
}

void csBulletSoftBody::SetLift (float lift)
{
  CS_ASSERT (btBody);
  if (lift >= 0.0f)
    btBody->m_cfg.kLF = lift;
}

void csBulletSoftBody::SetPressure (float pressure)
{
  CS_ASSERT (btBody);
  if (pressure >= 0.0f && pressure <= 1.0f)
    btBody->m_cfg.kPR = pressure;
}

void csBulletSoftBody::SetVolumeConversationCoefficient (float conversation)
{
  CS_ASSERT (btBody);
  btBody->m_cfg.kVC = conversation;
}

void csBulletSoftBody::SetShapeMatchThreshold (float matching)
{
  CS_ASSERT (btBody);
  if (matching >= 0.0f && matching <= 1.0f)
    btBody->m_cfg.kMT = matching;
}

void csBulletSoftBody::SetRContactsHardness (float hardness)
{
  CS_ASSERT (btBody);
  if (hardness >= 0.0f && hardness <= 1.0f)
    btBody->m_cfg.kCHR = hardness;
}

void csBulletSoftBody::SetKContactsHardness (float hardness)
{
  CS_ASSERT (btBody);
  if (hardness >= 0.0f && hardness <= 1.0f)
    btBody->m_cfg.kKHR = hardness;
}

void csBulletSoftBody::SetSContactsHardness (float hardness)
{
  CS_ASSERT (btBody);
  if (hardness >= 0.0f && hardness <= 1.0f)
    btBody->m_cfg.kSHR = hardness;
}

void csBulletSoftBody::SetAnchorsHardness (float hardness)
{
  CS_ASSERT (btBody);
  if (hardness >= 0.0f && hardness <= 1.0f)
    btBody->m_cfg.kAHR = hardness;
}

void csBulletSoftBody::SetVeloSolverIterations (int iter)
{
  CS_ASSERT (btBody);
  btBody->m_cfg.piterations = iter;
}

void csBulletSoftBody::SetPositionIterations (int iter)
{
  CS_ASSERT (btBody);
  btBody->m_cfg.viterations = iter;
}

void csBulletSoftBody::SetDriftIterations (int iter)
{
  CS_ASSERT (btBody);
  btBody->m_cfg.diterations = iter;
}

void csBulletSoftBody::SetClusterIterations (int iter)
{
  CS_ASSERT (btBody);
  btBody->m_cfg.citerations = iter;
}

void csBulletSoftBody::SetShapeMatching (bool match)
{
  CS_ASSERT (btBody);
  if (match)
    btBody->setPose (false,true);
  else
    btBody->setPose (true,false);
}

void csBulletSoftBody::SetBendingConstraint (bool bending)
{
  CS_ASSERT (btBody);
  if (bending)
  {
    btBody->generateBendingConstraints (2);
    btBody->randomizeConstraints();
  }
}

void csBulletSoftBody::GenerateCluster (int iter)
{
  CS_ASSERT (btBody);
  if (btBody->m_cfg.collisions & (btSoftBody::fCollision::CL_RS + btSoftBody::fCollision::CL_SS))
    btBody->generateClusters(iter);
}

void csBulletSoftBody::UpdateAnchorPositions ()
{
  CS_ASSERT (btBody);
  for (csArray<AnimatedAnchor>::Iterator it = animatedAnchors.GetIterator (); it.HasNext (); )
  {
    AnimatedAnchor& anchor = it.Next ();
    anchor.position = CSToBullet (anchor.controller->GetAnchorPosition (), system->getInternalScale ());
  }
}

void csBulletSoftBody::UpdateAnchorInternalTick (btScalar timeStep)
{
  CS_ASSERT (btBody);
  for (csArray<AnimatedAnchor>::Iterator it = animatedAnchors.GetIterator (); it.HasNext (); )
  {
    AnimatedAnchor& anchor = it.Next ();

    btVector3 delta = anchor.position - btBody->m_nodes[anchor.vertexIndex].m_x;
    static const btScalar maxdrag = 10;
    if (delta.length2 () > maxdrag * maxdrag)
      delta = delta.normalized() * maxdrag;
    btBody->m_nodes[anchor.vertexIndex].m_v += delta / timeStep;
  }  
}
}
CS_PLUGIN_NAMESPACE_END (Bullet2)