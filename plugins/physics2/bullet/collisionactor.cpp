#include "cssysdef.h"
#include "collisionactor2.h"
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

class btKinematicClosestNotMeRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
  btKinematicClosestNotMeRayResultCallback (btCollisionObject* me) : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
  {
    m_me[0] = me;
    count = 1;
  }

  btKinematicClosestNotMeRayResultCallback (btCollisionObject* me[], int count_) : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
  {
    count = count_;

    for(int i = 0; i < count; i++)
      m_me[i] = me[i];
  }

  virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,bool normalInWorldSpace)
  {
    for(int i = 0; i < count; i++)
      if (rayResult.m_collisionObject == m_me[i])
        return 1.0;

    return ClosestRayResultCallback::addSingleResult (rayResult, normalInWorldSpace);
  }
protected:
  btCollisionObject* m_me[10];
  int count;
};

class btKinematicClosestNotMeConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
  btKinematicClosestNotMeConvexResultCallback( btCollisionObject* me, const btVector3& up, btScalar minSlopeDot )
    : btCollisionWorld::ClosestConvexResultCallback( btVector3( 0.0, 0.0, 0.0 ), btVector3( 0.0, 0.0, 0.0 ) ),
    m_me( me ), m_up( up ), m_minSlopeDot( minSlopeDot )
  {
  }

  virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
  {
    if( convexResult.m_hitCollisionObject == m_me )
      return btScalar( 1 );

    btVector3 hitNormalWorld;
    if( normalInWorldSpace )
    {
      hitNormalWorld = convexResult.m_hitNormalLocal;
    }
    else
    {
      ///need to transform normal into worldspace
      hitNormalWorld = m_hitCollisionObject->getWorldTransform().getBasis()*convexResult.m_hitNormalLocal;
    }

    // NOTE : m_hitNormalLocal is not always vertical on the ground with a capsule or a box...

    btScalar dotUp = m_up.dot(hitNormalWorld);
    if( dotUp < m_minSlopeDot )
      return btScalar( 1 );

    return ClosestConvexResultCallback::addSingleResult (convexResult, normalInWorldSpace);
  }

protected:
  btCollisionObject* m_me;
  const btVector3 m_up;
  btScalar m_minSlopeDot;
};

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{

csBulletCollisionActor::csBulletCollisionActor (csBulletSystem* sys)
: scfImplementationType (this, sys), camera (NULL), wasOnGround (true),
wasJumping (false), verticalVelocity (0), maxJumpHeight (0),
touchingContact (false), speed (0), useGhostSweep (true), recoveringFactor (0.2f)
{
  type = CS::Collision2::COLLISION_OBJECT_ACTOR;
  SetMaxSlope (btRadians (45.0f));
  fallSpeed = 55.0f * system->getInternalScale ();
  jumpSpeed = 9.8f * system->getInternalScale ();
  stepHeight = 0.5f * system->getInternalScale ();
}

csBulletCollisionActor::~csBulletCollisionActor ()
{
  RemoveBulletObject ();
}

void csBulletCollisionActor::AddCollider (CS::Collision2::iCollider* collider, const csOrthoTransform& relaTrans)
{
  csBulletCollider* coll = dynamic_cast<csBulletCollider*> (collider);

  if (coll->shape->isConvex ())
  {
    if (colliders.GetSize () == 0)
    {
      colliders.Push(coll);
      relaTransforms.Push(relaTrans);
    }
    else
    {
      colliders[0] = coll;
      relaTransforms[0] = relaTrans;
    }
    shapeChanged = true;
  }
}

void csBulletCollisionActor::RemoveCollider (CS::Collision2::iCollider* collider)
{
  if (collider == colliders[0])
    colliders.Empty ();
}

void csBulletCollisionActor::RemoveCollider (size_t index)
{
  if (index == 0)
    colliders.Empty ();
}

bool csBulletCollisionActor::IsOnGround ()
{
  return verticalVelocity == 0.0f && verticalOffset == 0.0f;
}

void csBulletCollisionActor::SetRotation (const csMatrix3& rot)
{
  csOrthoTransform trans = GetTransform ();
  trans.SetT2O (rot);
  SetTransform (trans);
}

void csBulletCollisionActor::Rotate (const csVector3& v, float angle)
{
  csOrthoTransform trans = GetTransform ();
  trans.RotateThis (v, angle);
  SetTransform (trans);
}

void csBulletCollisionActor::SetCamera (iCamera* camera)
{
  SetTransform (camera->GetTransform ());
  this->camera = camera; 
}

void csBulletCollisionActor::UpdateAction (float delta)
{
  csVector3 upVec = GetTransform ().GetUp ();
  upVector.setValue (upVec.x, upVec.y, upVec.z);
  
  csVector3 frVec = GetTransform ().GetFront ();
  frontVector.setValue (frVec.x, frVec.y, frVec.z);

  PreStep ();
  PlayerStep (delta);
  speed = 0;
}

void csBulletCollisionActor::PreStep ()
{
  for (int i = 0; i < 4 && RecoverFromPenetration (); i++);
}

void csBulletCollisionActor::PlayerStep (float delta)
{

  wasOnGround = IsOnGround();

  // Update fall velocity.
  btVector3 grav = sector->bulletWorld->getGravity ();
  verticalVelocity += grav[1] * delta;
  if(verticalVelocity > 0.0 && verticalVelocity > jumpSpeed)
  {
    verticalVelocity = jumpSpeed;
  }
  if(verticalVelocity < 0.0 && fabs(verticalVelocity) > fabs(fallSpeed))
  {
    verticalVelocity = -fabs(fallSpeed);
  }
  verticalOffset = verticalVelocity * delta;

  csOrthoTransform xform;
  xform = GetTransform ();
  currentPosition = CSToBullet (xform.GetOrigin (), system->getInternalScale ());

  currentPosition = StepUp ();
  
  currentPosition = StepForwardAndStrafe (delta);
  
  currentPosition = StepDown (delta);

  xform.SetOrigin (BulletToCS (currentPosition, system->getInverseInternalScale ()));
  SetTransform (xform);
}

void csBulletCollisionActor::Jump ()
{
  if (!IsOnGround ())
    return;

  verticalVelocity = jumpSpeed;
  wasJumping = true;
}

void csBulletCollisionActor::SetMaxSlope (float slopeRadians)
{
  maxSlopeRadians = slopeRadians;
  maxSlopeCosine = btCos(slopeRadians);
}

bool csBulletCollisionActor::RecoverFromPenetration ()
{
  bool penetration = false;

  btPairCachingGhostObject* ghost = dynamic_cast<btPairCachingGhostObject*> (btObject);
  sector->bulletWorld->getDispatcher()->dispatchAllCollisionPairs( 
    ghost->getOverlappingPairCache(),
    sector->bulletWorld->getDispatchInfo(),
    sector->bulletWorld->getDispatcher() );

  currentPosition = btObject->getWorldTransform().getOrigin();

  for( int i = 0; i < ghost->getOverlappingPairCache()->getNumOverlappingPairs(); i++ )
  {
    btManifoldArray manifoldArray;

    btBroadphasePair* collisionPair = &ghost->getOverlappingPairCache()->getOverlappingPairArray()[i];

    if( collisionPair->m_algorithm )
      collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

    for( int j = 0; j < manifoldArray.size(); j++ )
    {
      btPersistentManifold* manifold = manifoldArray[j];

      btScalar directionSign = manifold->getBody0() == ghost ? btScalar( -1.0 ) : btScalar( 1.0 );

      for( int p = 0; p < manifold->getNumContacts(); p++ )
      {
        const btManifoldPoint &pt = manifold->getContactPoint( p );

        btScalar dist = pt.getDistance();

        if( dist < 0.0 )
        {
          // NOTE : btScalar affects the stairs but the parkinson...
          // 0.0 , the capsule can break the walls...
          currentPosition += pt.m_normalWorldOnB * directionSign * dist * recoveringFactor;          

          penetration = true;
        }
      }
    }
  }

  btTransform trans = ghost->getWorldTransform();
  trans.setOrigin( currentPosition );

  SetTransform (BulletToCS (trans, system->getInverseInternalScale ()));

  return penetration;
}

btVector3 csBulletCollisionActor::StepUp ()
{
//TODO
  targetPosition = currentPosition + upVector * (stepHeight + 
    verticalOffset > EPSILON ? verticalOffset : 0.0f);

  btCollisionShape* collisionShape = colliders[0]->shape;
  // Add this to add collider. Ghost object can only have convex shape.
  btAssert (collisionShape->isConvex ());
  btConvexShape* convexShape = dynamic_cast<btConvexShape*> (collisionShape);

  btTransform start;
  start.setIdentity ();
  start.setOrigin (currentPosition + upVector * colliders[0]->GetMargin () * system->getInternalScale ());

  btTransform end;
  end.setIdentity ();
  end.setOrigin (targetPosition);

  btKinematicClosestNotMeConvexResultCallback callback( btGhostObject::upcast (btObject), -upVector, maxSlopeCosine );
  callback.m_collisionFilterGroup = btObject->getBroadphaseHandle()->m_collisionFilterGroup;
  callback.m_collisionFilterMask = btObject->getBroadphaseHandle()->m_collisionFilterMask;

  if( useGhostSweep )
    btGhostObject::upcast (btObject)->convexSweepTest( convexShape, start, 
    end, callback, sector->bulletWorld->getDispatchInfo().m_allowedCcdPenetration );

  else
    sector->bulletWorld->convexSweepTest( convexShape, start, end, callback );

  if( callback.hasHit() )
  {
    // Only modify the position if the hit was a slope and not a wall or ceiling.
    //
    if( callback.m_hitNormalWorld.dot(upVector) > btScalar( 0.0 ) )
    {

      currentStepOffset = stepHeight * callback.m_closestHitFraction;
      return currentPosition.lerp( targetPosition, callback.m_closestHitFraction );
    }

    verticalVelocity = btScalar( 0.0 );
    verticalOffset = btScalar( 0.0 );

    return currentPosition;
  }
  else
  {
    currentStepOffset = stepHeight;
    return targetPosition;
  }
}

///////////////////

///Reflect the vector d around the vector r
inline btVector3 reflect( const btVector3& d, const btVector3& r )
{
  return d - ( btScalar( 2.0 ) * d.dot( r ) ) * r;
}


///Project a vector u on another vector v
inline btVector3 project( const btVector3& u, const btVector3& v )
{
  return v * u.dot( v );
}


///Helper for computing the character sliding
inline btVector3 slide( const btVector3& direction, const btVector3& planeNormal )
{
  return direction - project( direction, planeNormal );
}

btVector3 slideOnCollision( const btVector3& fromPosition, const btVector3& toPosition, const btVector3& hitNormal )
{
  btVector3 moveDirection = toPosition - fromPosition;
  btScalar moveLength = moveDirection.length();

  if( moveLength <= btScalar( SIMD_EPSILON ) )
    return toPosition;

  moveDirection.normalize();

  btVector3 reflectDir = reflect( moveDirection, hitNormal );
  reflectDir.normalize();

  return fromPosition + slide( reflectDir, hitNormal ) * moveLength;
}

///////////////////

btVector3 csBulletCollisionActor::StepForwardAndStrafe (float dt)
{
  //TODO

  targetPosition = currentPosition + frontVector * speed * dt;

  btCollisionShape* collisionShape = colliders[0]->shape;
  btAssert (collisionShape->isConvex ());
  btConvexShape* convexShape = dynamic_cast<btConvexShape*> (collisionShape);

  btTransform start;
  start.setIdentity ();

  btTransform end;
  end.setIdentity ();

  float fraction = 1.0f;

  for (int i = 0; i < 4 && fraction > 0.01f; i++)
  {
    start.setOrigin( currentPosition );
    end.setOrigin( targetPosition );

    btVector3 sweepDirNegative = currentPosition - targetPosition;

    btKinematicClosestNotMeConvexResultCallback callback( btGhostObject::upcast (btObject), sweepDirNegative, 0.0f);
    callback.m_collisionFilterGroup = btObject->getBroadphaseHandle()->m_collisionFilterGroup;
    callback.m_collisionFilterMask = btObject->getBroadphaseHandle()->m_collisionFilterMask;

    if( useGhostSweep )
      btGhostObject::upcast (btObject)->convexSweepTest( convexShape, start, end, callback, sector->bulletWorld->getDispatchInfo().m_allowedCcdPenetration );

    else
      sector->bulletWorld->convexSweepTest( convexShape, start, end, callback, sector->bulletWorld->getDispatchInfo().m_allowedCcdPenetration );

    if( callback.hasHit() )
    {
      // Try another target position
      //
      targetPosition = slideOnCollision( currentPosition, targetPosition, callback.m_hitNormalWorld );
      fraction = callback.m_closestHitFraction;
    }
    else

      // Move to the valid target position
      //
      return targetPosition;
  }
  // Don't move if you can't find a valid target position...
  // It prevents some flickering.
  return currentPosition;
}

btVector3 csBulletCollisionActor::StepDown (float dt)
{
  btVector3 stepDrop = upVector * currentStepOffset;

  // Be sure we are falling from the last m_currentPosition
  // It prevents some flickering
  targetPosition = currentPosition - stepDrop;

  btCollisionShape* collisionShape = colliders[0]->shape;
  // Add this to add collider. Ghost object can only have convex shape.
  btAssert (collisionShape->isConvex ());
  btConvexShape* convexShape = dynamic_cast<btConvexShape*> (collisionShape);

  btTransform start;
  start.setIdentity();
  start.setOrigin( currentPosition );

  btTransform end;
  end.setIdentity();
  end.setOrigin( targetPosition );

  //Fix me (in terrain mode the phystut2 demo will crush)

  btKinematicClosestNotMeConvexResultCallback callback( btGhostObject::upcast (btObject), upVector, maxSlopeCosine );
  callback.m_collisionFilterGroup = btObject->getBroadphaseHandle()->m_collisionFilterGroup;
  callback.m_collisionFilterMask = btObject->getBroadphaseHandle()->m_collisionFilterMask;

  if( useGhostSweep )
    btGhostObject::upcast (btObject)->convexSweepTest( convexShape, start, 
    end, callback, sector->bulletWorld->getDispatchInfo().m_allowedCcdPenetration );

  else
    sector->bulletWorld->convexSweepTest( convexShape, start, end, callback );
  
  if( callback.hasHit() )
  {
    verticalVelocity = 0.0f;
    verticalOffset = 0.0f;
    wasJumping = false;

    // We dropped a fraction of the height -> hit floor
    //
    return currentPosition.lerp( targetPosition, callback.m_closestHitFraction );
  }
  else

    // We dropped the full height
    //		
    return targetPosition;
}

float csBulletCollisionActor::AddFallOffset (bool wasOnGround, btScalar currentStepOffset, btScalar dt)
{
  float downVelocity = ( verticalVelocity < 0.0 ? -verticalVelocity :  0.0f ) * dt;

  if( downVelocity > 0.0f && downVelocity < stepHeight && ( wasOnGround || !wasJumping ) )
    downVelocity = stepHeight;

  return currentStepOffset + downVelocity;
}
}
CS_PLUGIN_NAMESPACE_END (Bullet2)