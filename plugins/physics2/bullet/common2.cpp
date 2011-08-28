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
#include "iengine/scenenode.h"
#include "iengine/mesh.h"

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

#include "bullet2.h"
#include "common2.h"
#include "collisionobject2.h"
#include "rigidbody2.h"

struct btSoftBodyWorldInfo;
CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

//------------------------ csBulletMotionState ----------------------

  csBulletMotionState::csBulletMotionState (csBulletCollisionObject* body,
					    const btTransform& initialTransform,
					    const btTransform& principalAxis)
    : btDefaultMotionState (initialTransform), body (body),
    inversePrincipalAxis (principalAxis.inverse ())
  {
    if (body->btObject)
      body->btObject->setInterpolationWorldTransform (initialTransform);

    csOrthoTransform tr = BulletToCS (initialTransform * inversePrincipalAxis,
				      body->system->getInverseInternalScale ());

    iMovable* movable = body->GetAttachedMovable ();
    if (movable)
    {
      // Update movable
      movable->SetFullTransform (tr);
      movable->UpdateMove ();
    }
    iCamera* camera = body->GetAttachedCamera ();
    if (camera)
    {
      csOrthoTransform& cameraTrans = camera->GetTransform ();
      if (cameraTrans.GetOrigin () == tr.GetOrigin () &&
        cameraTrans.GetT2O () == tr.GetT2O ())
        return;

      // Update camera position
      cameraTrans.SetOrigin (tr.GetOrigin ());
      cameraTrans.SetT2O (tr.GetT2O ());
    }
  }

  void csBulletMotionState::setWorldTransform (const btTransform& trans)
  {
    btDefaultMotionState::setWorldTransform (trans);

    // update attached object
    /*if (!body->moveCb)
      return;*/

    csOrthoTransform tr = BulletToCS (trans * inversePrincipalAxis,
				      body->system->getInverseInternalScale ());

    iMovable* movable = body->GetAttachedMovable ();
    if (movable)
    {
      // Update movable
      movable->SetFullTransform (tr);
      movable->UpdateMove ();
    }
    iCamera* camera = body->GetAttachedCamera ();
    if (camera)
    {
      csOrthoTransform& cameraTrans = camera->GetTransform ();
      if (cameraTrans.GetOrigin () == tr.GetOrigin () &&
        cameraTrans.GetT2O () == tr.GetT2O ())
        return;

      // Update camera position
      cameraTrans.SetOrigin (tr.GetOrigin ());
      cameraTrans.SetT2O (tr.GetT2O ());
    }
  }

//------------------------ csBulletKinematicMotionState ----------------------

  csBulletKinematicMotionState::csBulletKinematicMotionState
  (csBulletCollisionObject* body, const btTransform& initialTransform,
   const btTransform& principalAxis)
    : csBulletMotionState (body, initialTransform, principalAxis),
      principalAxis (BulletToCS (principalAxis, body->system->getInverseInternalScale ()))
  {
  }

  void csBulletKinematicMotionState::getWorldTransform (btTransform& trans) const
  {
    csBulletRigidBody* rb = dynamic_cast<csBulletRigidBody*> (body);
    if (!rb || !rb->kinematicCb)
      return;

    // get the body transform from the callback
    csOrthoTransform transform;
    
    rb->kinematicCb->GetBodyTransform (rb, transform);
    trans = CSToBullet (principalAxis * transform, body->system->getInternalScale ());
    
  }

}
CS_PLUGIN_NAMESPACE_END(Bullet2)
