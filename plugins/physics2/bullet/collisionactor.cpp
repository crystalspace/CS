#include "cssysdef.h"
#include "collisionactor2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{

csBulletCollisionActor::csBulletCollisionActor (csBulletSystem* sys)
: scfImplementationType (this, sys), upAxis (1), camera (NULL), wasOnGround (true),
wasJumping (false), verticalVelocity (0), fallSpeed (0), jumpSpeed (0), maxJumpHeight (0),
touchingContact (false), speed (0)
{
  type = CS::Collision2::COLLISION_OBJECT_ACTOR;
}

csBulletCollisionActor::~csBulletCollisionActor ()
{
  RemoveBulletObject ();
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
  PreStep ();
  PlayerStep (delta);
  speed = 0;
}

void csBulletCollisionActor::SetUpAxis (int axis)
{
  if (axis < 0)
    axis = 0;
  if (axis > 2)
    axis = 2;
  upAxis = axis;
}

void csBulletCollisionActor::PreStep ()
{
  int numPenetrationLoops = 0;
  touchingContact = false;
  while (RecoverFromPenetration ())
  {
    numPenetrationLoops++;
    touchingContact = true;
    if (numPenetrationLoops > 4)
      break;
  }
  currentPosition = GetTransform ().GetOrigin ();
  targetPosition = currentPosition;
}

void csBulletCollisionActor::PlayerStep (float delta)
{

  wasOnGround = IsOnGround();

  // Update fall velocity.
  csVector3 grav = sector->GetGravity ();
  verticalVelocity += grav[upAxis];
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

  //	printf("walkDirection(%f,%f,%f)\n",walkDirection[0],walkDirection[1],walkDirection[2]);
  //	printf("walkSpeed=%f\n",walkSpeed);

  StepUp ();
  
  StepForwardAndStrafe (delta);
  
  StepDown (delta);

  xform.SetOrigin (currentPosition);
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
  float maxPen = 0.0f;

  csOrthoTransform trans = GetTransform ();
  currentPosition = trans.GetOrigin ();

  csArray<CS::Collision2::CollisionData> collisions;
  sector->CollisionTest (QueryCollisionObject (), collisions);

  for (size_t i = 0; i < collisions.GetSize (); i++)
  {
    float dist = collisions[i].penetration;
    btScalar directionSign = collisions[i].objectA == QueryCollisionObject () ? -1.0f : 1.0f;
    if (dist < 0.0)
    {
      if (dist < maxPen)
      {
        maxPen = dist;
      }
      currentPosition += collisions[i].normalWorldOnB * dist * directionSign * 0.2f;
      penetration = true;
    }
  }
  trans.SetOrigin(currentPosition);
  SetTransform(trans);

  return penetration;
}

void csBulletCollisionActor::StepUp ()
{
//TODO
}

void csBulletCollisionActor::StepForwardAndStrafe (float dt)
{
  //TODO
  csOrthoTransform trans = GetTransform ();
  currentPosition += trans.GetT2O () * csVector3 (0, 0, speed * dt);
}

void csBulletCollisionActor::StepDown (float dt)
{
//TODO
}
}
CS_PLUGIN_NAMESPACE_END (Bullet2)