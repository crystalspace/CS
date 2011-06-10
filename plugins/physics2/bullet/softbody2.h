#ifndef __CS_BULLET_SOFTBODY_H__
#define __CS_BULLET_SOFTBODY_H__

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{

class csBulletSoftBody : scfImplementationExt2<csBulletSoftBody, 
  csBulletCollisionObject, CS::Physics::iSoftBody,
  CS::Physics::Bullet::iSoftBody>
{
private:
  //CS::Physics::Bullet::BodyType bodyType;
  float friction;
  float density;
  bool bending;
  btSoftBody* btBody;   //Don't know if I should add this to rigidbody too.
  struct AnimatedAnchor
  {
    AnimatedAnchor (size_t vertexIndex, iAnchorAnimationControl* controller)
      : vertexIndex (vertexIndex), controller (controller) {}

    size_t vertexIndex;
    csRef<iAnchorAnimationControl> controller;
    btVector3 position;
  };
  csArray<AnimatedAnchor> animatedAnchors;

public:
  csBulletSoftBody (csBulletSystem* phySys, btSoftBody* body);
  virtual ~csBulletSoftBody ();

  //iCollisionObject

  virtual void SetTransform (const csOrthoTransform& trans);
  virtual csOrthoTransform GetTransform ();

  virtual void RebuildObject ();

  //virtual bool Collide (iCollisionObject* otherObject);
  virtual HitBeamResult HitBeam (const csVector3& start, const csVector3& end);

  btSoftBody* GetBulletSoftPointer () {return btBody;}
  virtual void RemoveBulletObject ();
  virtual void AddBulletObject ();

  //iPhysicalBody

  virtual PhysicalBodyType GetBodyType () {return bodyType;}
  virtual iRigidBody* QueryRigidBody () {return NULL;}
  virtual iSoftBody* QuerySoftBody () {return this;}

  virtual bool Disable ();
  virtual bool Enable ();
  virtual bool IsEnabled ();

  virtual float GetMass ();

  virtual float GetDensity () {return density;}
  virtual void SetDensity (float density);

  virtual float GetVolume ();

  virtual void AddForce (const csVector3& force);

  virtual void SetLinearVelocity (const csVector3& vel);
  virtual csVector3 GetLinearVelocity (size_t index = 0);

  virtual void SetFriction (float friction);
  virtual float GetFriction () {return friction;}

  //iSoftBody
  virtual void SetVertexMass (float mass, size_t index);
  virtual float GetVertexMass (size_t index);

  virtual size_t GetVertexCount ();
  virtual csVector3 GetVertexPosition (size_t index) const;

  virtual void AnchorVertex (size_t vertexIndex);
  virtual void AnchorVertex (size_t vertexIndex,
    iRigidBody* body);
  virtual void AnchorVertex (size_t vertexIndex,
    iAnchorAnimationControl* controller);

  virtual void UpdateAnchor (size_t vertexIndex,
    csVector3& position);
  virtual void RemoveAnchor (size_t vertexIndex);

  virtual void SetRigidity (float rigidity);
  virtual float GetRidigity ();

  virtual void SetLinearVelocity (const csVector3& velocity,
    size_t vertexIndex);

  virtual void SetWindVelocity (const csVector3& velocity);
  virtual const csVector3 GetWindVelocity () const;

  virtual void AddForce (const csVector3& force, size_t vertexIndex);

  virtual size_t GetTriangleCount ();

  virtual csTriangle GetTriangle (size_t index) const;

  virtual csVector3 GetVertexNormal (size_t index) const;

  //Bullet::iSoftBody

  virtual void DebugDraw (iView* rView);

  virtual void SetLinearStiff (float stiff);
  virtual void SetAngularStiff (float stiff);
  virtual void SetVolumeStiff (float stiff);

  virtual void ResetCollisionFlag ();

  virtual void SetClusterCollisionRS (bool cluster);
  virtual void SetClusterCollisionSS (bool cluster);

  virtual void SetSRHardness (float hardness);
  virtual void SetSKHardness (float hardness);
  virtual void SetSSHardness (float hardness);

  virtual void SetSRImpulse (float impulse);
  virtual void SetSKImpulse (float impulse);
  virtual void SetSSImpulse (float impulse);

  virtual void SetVeloCorrectionFactor (float factor);

  virtual void SetDamping (float damping);
  virtual void SetDrag (float drag);
  virtual void SetLift (float lift);
  virtual void SetPressure (float pressure);

  virtual void SetVolumeConversationCoefficient (float conversation);
  virtual void SetShapeMatchThreshold (float matching);

  virtual void SetRContactsHardness (float hardness);
  virtual void SetKContactsHardness (float hardness);
  virtual void SetSContactsHardness (float hardness);
  virtual void SetAnchorsHardness (float hardness);

  virtual void SetVeloSolverIterations (int iter);
  virtual void SetPositionIterations (int iter);
  virtual void SetDriftIterations (int iter);
  virtual void SetClusterIterations (int iter);

  virtual void SetShapeMatching (bool match);
  virtual void SetBendingConstraint (bool bending);

  virtual void GenerateCluster (int iter);
};
}
CS_PLUGIN_NAMESPACE_END (Bullet2)
#endif