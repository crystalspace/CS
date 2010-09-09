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

#include "softbodies.h"
#include "rigidbodies.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

csBulletSoftBody::csBulletSoftBody (csBulletDynamicsSystem* dynSys,
				    btSoftBody* body)
  : scfImplementationType (this), bodyType (CS::Physics::Bullet::SOFT_BODY),
  dynSys (dynSys), body (body)
{
  body->setUserPointer (static_cast<iBody*> (this));
}

csBulletSoftBody::~csBulletSoftBody ()
{
  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (dynSys->bulletWorld);
  softWorld->removeSoftBody (body);
  delete body;

  dynSys->anchoredSoftBodies.Delete (this);
}

void csBulletSoftBody::DebugDraw (iView* rview)
{
  if (!dynSys->debugDraw)
  {
    dynSys->debugDraw = new csBulletDebugDraw (dynSys->inverseInternalScale);
    dynSys->bulletWorld->setDebugDrawer (dynSys->debugDraw);
  }

  btSoftBodyHelpers::Draw (body, dynSys->debugDraw);
  dynSys->debugDraw->DebugDraw (rview);
}

void csBulletSoftBody::SetMass (float mass)
{
  CS_ASSERT(mass > 0);

  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (dynSys->bulletWorld);
  softWorld->removeSoftBody (body);

  body->setTotalMass (mass);

  softWorld->addSoftBody (body);
}

float csBulletSoftBody::GetMass () const
{
  return body->getTotalMass ();
}

size_t csBulletSoftBody::GetVertexCount () const
{
  return body->m_nodes.size ();
}

csVector3 csBulletSoftBody::GetVertexPosition (size_t index) const
{
  CS_ASSERT(index < (size_t) body->m_nodes.size ());
  return BulletToCS (body->m_nodes[index].m_x, dynSys->inverseInternalScale);
}

csVector3 csBulletSoftBody::GetVertexNormal (size_t index) const
{
  CS_ASSERT(index < (size_t) body->m_nodes.size ());
  csVector3 normal (body->m_nodes[index].m_n.getX (),
		    body->m_nodes[index].m_n.getY (),
		    body->m_nodes[index].m_n.getZ ());
  normal.Normalize ();
  return normal;
}

void csBulletSoftBody::AnchorVertex (size_t vertexIndex)
{
  CS_ASSERT(vertexIndex < (size_t) body->m_nodes.size ());
  body->setMass (vertexIndex, 0.0f);
}

void csBulletSoftBody::AnchorVertex (size_t vertexIndex, ::iRigidBody* body)
{
  csBulletRigidBody* rigidBody = static_cast<csBulletRigidBody*> (body);
  CS_ASSERT(rigidBody
	    && vertexIndex < (size_t) this->body->m_nodes.size ()
	    && rigidBody->body);
  this->body->appendAnchor (vertexIndex, rigidBody->body);
}

void csBulletSoftBody::AnchorVertex (size_t vertexIndex,
				     iAnchorAnimationControl* controller)
{
  AnimatedAnchor anchor (vertexIndex, controller);
  animatedAnchors.Push (anchor);
  dynSys->anchoredSoftBodies.Push (this);
}

void csBulletSoftBody::UpdateAnchor (size_t vertexIndex, csVector3& position)
{
  CS_ASSERT(vertexIndex < (size_t) body->m_nodes.size ());

  // Update the local position of the anchor
  for (int i = 0; i < this->body->m_anchors.size (); i++)
    if (this->body->m_anchors[i].m_node == &this->body->m_nodes[vertexIndex])
    {
      this->body->m_anchors[i].m_local =
	this->body->m_anchors[i].m_body->getInterpolationWorldTransform ().inverse ()
	* CSToBullet (position, dynSys->internalScale);
      return;
    }
}

void csBulletSoftBody::RemoveAnchor (size_t vertexIndex)
{
  CS_ASSERT(vertexIndex < (size_t) body->m_nodes.size ());

  // Check if it is a fixed anchor
  if (body->getMass (vertexIndex) < SMALL_EPSILON)
  {
    body->setMass (vertexIndex, body->getTotalMass () / body->m_nodes.size ());
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
      dynSys->anchoredSoftBodies.Delete (this);
      return;
    }
  }

  // Check if it is a simple 'rigid body' anchor
  for (int i = 0; i < this->body->m_anchors.size (); i++)
    if (this->body->m_anchors[i].m_node == &this->body->m_nodes[vertexIndex])
    {
      // TODO: this is not possible within Bullet
      //btSoftBody::Anchor* anchor = this->body->m_anchors[i];
      //this->body->m_anchors.remove (i);
      return;
    }
}

void csBulletSoftBody::SetRigidity (float rigidity)
{
  CS_ASSERT(rigidity >= 0.0f && rigidity <= 1.0f);
  body->m_materials[0]->m_kLST = rigidity;
}

float csBulletSoftBody::GetRigidity () const
{
  return body->m_materials[0]->m_kLST;
}

void csBulletSoftBody::SetLinearVelocity (csVector3 velocity)
{
  body->setVelocity (CSToBullet (velocity, dynSys->internalScale));
}

void csBulletSoftBody::SetLinearVelocity (csVector3 velocity, size_t vertexIndex)
{
  CS_ASSERT (vertexIndex < (size_t) body->m_nodes.size ());
  body->addVelocity (CSToBullet (velocity, dynSys->internalScale)
		     - body->m_nodes[vertexIndex].m_v, vertexIndex);
}

csVector3 csBulletSoftBody::GetLinearVelocity (size_t vertexIndex) const
{
  CS_ASSERT (vertexIndex < (size_t) body->m_nodes.size ());
  return BulletToCS (body->m_nodes[vertexIndex].m_v, dynSys->inverseInternalScale);
}

void csBulletSoftBody::AddForce (csVector3 force)
{
  body->addForce (CSToBullet (force, dynSys->internalScale));
}

void csBulletSoftBody::AddForce (csVector3 force, size_t vertexIndex)
{
  CS_ASSERT (vertexIndex < (size_t) body->m_nodes.size ());
  // TODO: why a correction factor of 100?
  body->addForce (CSToBullet (force * 100.0f, dynSys->internalScale), vertexIndex);
}

size_t csBulletSoftBody::GetTriangleCount () const
{
  return body->m_faces.size ();
}

csTriangle csBulletSoftBody::GetTriangle (size_t index) const
{
  CS_ASSERT(index < (size_t) body->m_faces.size ());
  btSoftBody::Face& face = body->m_faces[index];
  return csTriangle (face.m_n[0] - &body->m_nodes[0],
		     face.m_n[1] - &body->m_nodes[0],
		     face.m_n[2] - &body->m_nodes[0]);
}

void csBulletSoftBody::UpdateAnchorPositions ()
{
  for (csArray<AnimatedAnchor>::Iterator it = animatedAnchors.GetIterator (); it.HasNext (); )
  {
    AnimatedAnchor& anchor = it.Next ();
    anchor.position = CSToBullet (anchor.controller->GetAnchorPosition (), dynSys->internalScale);
  }
}

void csBulletSoftBody::UpdateAnchorInternalTick (btScalar timeStep)
{
  for (csArray<AnimatedAnchor>::Iterator it = animatedAnchors.GetIterator (); it.HasNext (); )
  {
    AnimatedAnchor& anchor = it.Next ();

    btVector3 delta = anchor.position - body->m_nodes[anchor.vertexIndex].m_x;
    static const btScalar maxdrag = 10;
    if (delta.length2 () > maxdrag * maxdrag)
      delta = delta.normalized() * maxdrag;
    body->m_nodes[anchor.vertexIndex].m_v += delta / timeStep;
  }  
}

}
CS_PLUGIN_NAMESPACE_END(Bullet)
