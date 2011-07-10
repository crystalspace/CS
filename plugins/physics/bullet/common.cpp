/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
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

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

struct btSoftBodyWorldInfo;

#include "common.h"
#include "rigidbodies.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

//------------------------ csBulletMotionState ----------------------

  csBulletMotionState::csBulletMotionState (csBulletRigidBody* body,
					    const btTransform& initialTransform,
					    const btTransform& principalAxis)
    : btDefaultMotionState (initialTransform), body (body),
    inversePrincipalAxis (principalAxis.inverse ())
  {
    if (body->body)
      body->body->setInterpolationWorldTransform (initialTransform);

    // update attached object
    if (!body->moveCb)
      return;

    csOrthoTransform tr = BulletToCS (initialTransform * inversePrincipalAxis,
				      body->dynSys->inverseInternalScale);

    if (body->mesh)
      body->moveCb->Execute (body->mesh, tr);
    if (body->light)
      body->moveCb->Execute (body->light, tr);
    if (body->camera)
      body->moveCb->Execute (body->camera, tr);
  }

  void csBulletMotionState::setWorldTransform (const btTransform& trans)
  {
    btDefaultMotionState::setWorldTransform (trans);

    // update attached object
    if (!body->moveCb)
      return;

    csOrthoTransform tr = BulletToCS (trans * inversePrincipalAxis,
				      body->dynSys->inverseInternalScale);

    if (body->mesh)
      body->moveCb->Execute (body->mesh, tr);
    if (body->light)
      body->moveCb->Execute (body->light, tr);
    if (body->camera)
      body->moveCb->Execute (body->camera, tr);
  }

//------------------------ csBulletKinematicMotionState ----------------------

  csBulletKinematicMotionState::csBulletKinematicMotionState
  (csBulletRigidBody* body, const btTransform& initialTransform,
   const btTransform& principalAxis)
    : csBulletMotionState (body, initialTransform, principalAxis),
      principalAxis (BulletToCS (principalAxis, body->dynSys->inverseInternalScale))
  {
  }

  void csBulletKinematicMotionState::getWorldTransform (btTransform& trans) const
  {
    if (!body->kinematicCb)
      return;

    // get the body transform from the callback
    csOrthoTransform transform;
    body->kinematicCb->GetBodyTransform (body, transform);
    trans = CSToBullet (principalAxis * transform, body->dynSys->internalScale);
  }

}
CS_PLUGIN_NAMESPACE_END(Bullet)
