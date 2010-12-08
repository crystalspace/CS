/*
    Copyright (C) 2002 Anders Stenberg
    Copyright (C) 2003 Leandro Motta Barros

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


#ifndef __CS_ODEDYNAMICS_H__
#define __CS_ODEDYNAMICS_H__

#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/weakrefarr.h"
#include "csutil/nobjvec.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"
#include "iengine/mesh.h"
#include "iengine/light.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "ivaria/dynamics.h"
#include "ivaria/ode.h"

// Avoid conflicts as ODE defines the same types.
#define int8 ode_int8
#define uint8 ode_uint8
#define int32 ode_int32
#define uint32 ode_uint32
#include <ode/ode.h>
#undef uint32
#undef int32
#undef uint8
#undef int8

struct iMeshWrapper;
struct iODEFrameUpdateCallback;
struct iObjectRegistry;
struct iVirtualClock;

CS_PLUGIN_NAMESPACE_BEGIN(odedynam)
{

////////////////////////////////////////////
// NOTE
// There is lots of unused stuff in here involving iCollideSystem
// It was put there when I built the base for making mesh colliders
// The reason I left it in is because it does quite a lot of useful
// stuff conserning custom colliders in ODE
///////////////////////////////////////////


struct colliderdata
{
public:
  csRef<iCollideSystem> collsys;
  csRef<iCollider> collider;
  float friction;
  dReal aabb[6];

  colliderdata (iCollideSystem* cs, iCollider* c, float f )
  {
    collsys=cs; collider=c; friction=f;
    aabb[0]=-dInfinity; aabb[1]=dInfinity;
    aabb[2]=-dInfinity; aabb[3]=dInfinity;
    aabb[4]=-dInfinity; aabb[5]=dInfinity;
  }
  ~colliderdata()
  {
  }
};

typedef csDirtyAccessArray<dGeomID> csGeomList;

struct ContactEntry
{
  dGeomID o1, o2;
  csVector3 point;
  csVector3 dir;
};

struct MeshInfo
{
  iMeshWrapper* mesh;
};

typedef csDirtyAccessArray<ContactEntry> csContactList;

class csODECollider;

// Helper base for classes that can contain multiple colliders
struct ColliderContainer
{
  virtual ~ColliderContainer() {}
  virtual bool DoFullInertiaRecalculation () const = 0;
  virtual void RecalculateFullInertia (csODECollider* thisCol) = 0;
};

/**
 * This is the implementation for the actual plugin.
 * It is responsible for creating iDynamicSystem.
 */
class csODEDynamics : 
  public scfImplementation3<csODEDynamics,
                            iDynamics,
                            iComponent,
                            iODEDynamicState>


{
private:
  iObjectRegistry* object_reg;
  csRef<iVirtualClock> clock;

  bool process_events;

  static int geomclassnum;
  static dJointGroupID contactjoints;

  csRefArrayObject<iDynamicSystem> systems;
  float erp, cfm;

  bool rateenabled;
  float steptime, limittime;
  float total_elapsed;
  csRefArrayObject<iODEFrameUpdateCallback> updates;

  csRefArray<iDynamicsStepCallback> step_callbacks;

  bool stepfast;
  int sfiter;
  bool quickstep;
  int qsiter;
  bool fastobjects;

  csEventID Frame;
public:
  csODEDynamics (iBase* parent);
  virtual ~csODEDynamics ();
  bool Initialize (iObjectRegistry* object_reg);

  static int GetGeomClassNum() { return geomclassnum; }

  virtual csPtr<iDynamicSystem> CreateSystem ();
  virtual void RemoveSystem (iDynamicSystem* system);
  virtual iDynamicSystem* FindSystem (const char *name);
  virtual void RemoveSystems ();

  virtual void Step (float stepsize);

  void AddStepCallback (iDynamicsStepCallback *callback)
  {step_callbacks.Push (callback);}
  void RemoveStepCallback (iDynamicsStepCallback *callback)
  {step_callbacks.Delete (callback);}

  static void NearCallback (void *data, dGeomID o1, dGeomID o2);
  static int CollideMeshMesh (dGeomID mesh1, dGeomID mesh2, int flags,
        dContactGeom *contact, int skip);
  static int CollideMeshBox (dGeomID mesh, dGeomID box, int flags,
        dContactGeom *contact, int skip);
  static int CollideMeshCylinder (dGeomID mesh, dGeomID cyl, int flags,
        dContactGeom *contact, int skip);
  static int CollideMeshSphere (dGeomID mesh, dGeomID sphere, int flags,
        dContactGeom *contact, int skip);
  static int CollideMeshPlane (dGeomID mesh, dGeomID plane, int flags,
        dContactGeom *contact, int skip);
//  static dColliderFn* CollideSelector (int num);
#if 0
  static void GetAABB (dGeomID g, dReal aabb[6]);
#endif

  void SetGlobalERP (float erp);
  float GlobalERP () { return erp; }
  void SetGlobalCFM (float cfm);
  float GlobalCFM () { return cfm; }

  void EnableStepFast (bool enable);
  bool StepFastEnabled () { return stepfast; }
  void SetStepFastIterations (int iter);
  int StepFastIterations () { return sfiter; }
  void EnableQuickStep (bool enable);
  bool QuickStepEnabled () { return quickstep; };
  void SetQuickStepIterations (int iter);
  int QuickStepIterations () { return qsiter; }

  void EnableFrameRate (bool enable) { rateenabled = enable; }
  bool FrameRateEnabled () { return rateenabled; }
  void SetFrameRate (float hz) { steptime = 1.0 / hz; }
  float FrameRate () { return 1.0 / steptime; }
  void SetFrameLimit (float hz) { limittime = 1.0 / hz; }
  float FrameLimit () { return 1.0 / limittime; }
  void AddFrameUpdateCallback (iODEFrameUpdateCallback *cb)
  {
    updates.Push (cb);
  }
  void RemoveFrameUpdateCallback (iODEFrameUpdateCallback *cb)
  {
    updates.Delete (cb);
  }
  void EnableEventProcessing (bool enable);
  bool EventProcessingEnabled () { return process_events; }
  void EnableFastObjects (bool enable) { fastobjects = enable; }
  bool FastObjectsEnabled () { return false; }

  bool HandleEvent (iEvent& Event);
  struct EventHandler : public scfImplementation1<EventHandler, iEventHandler>
  {
  private:
    csWeakRef<csODEDynamics> parent;
  public:
    EventHandler (csODEDynamics* parent) : scfImplementationType (this),
      parent (parent) { }
    virtual ~EventHandler () { }
    virtual bool HandleEvent (iEvent& ev)
    { return parent ? parent->HandleEvent (ev) : false; }
    CS_EVENTHANDLER_PHASE_LOGIC("crystalspace.dynamics.ode")
  };
  csRef<EventHandler> scfiEventHandler;
};

class csODEBodyGroup;
class csODEJoint;
class csODECollider;
struct csStrictODEJoint;

/**
 * This is the implementation for the dynamics core.
 * It handles all bookkeeping for rigid bodies and joints.
 * It also handles collision response.
 * Collision detection is done in another plugin.
 */
class csODEDynamicSystem : 
  public scfImplementationExt2<csODEDynamicSystem,
                               csObject,
                               iDynamicSystem,
                               iODEDynamicSystemState>,
  public ColliderContainer
{
private:
  dWorldID worldID;
  dSpaceID spaceID;
  float roll_damp, lin_damp;

  csRef<iCollideSystem> collidesys;
  csRef<iDynamicsMoveCallback> move_cb;

  csRefArrayObject<iRigidBody> bodies;
  csRefArray<csODEBodyGroup> groups;
  csRefArray<csODEJoint> joints;
  csRefArray<csStrictODEJoint> strict_joints;
  //here are all colliders
  csRefArray<csODECollider> colliders;

  // For getting collision mesh data.
  csStringID base_id;
  csStringID colldet_id;

  bool rateenabled;
  float steptime, limittime;
  float total_elapsed;
  csRefArrayObject<iODEFrameUpdateCallback> updates;

  bool stepfast;
  int sfiter;
  bool quickstep;
  int qsiter;
  bool fastobjects;
  bool autodisable;
  bool correctInertiaWorkAround;

public:
  csStringID GetBaseID () const { return base_id; }
  csStringID GetColldetID () const { return colldet_id; }

  void SetERP (float erp) { dWorldSetERP (worldID, erp); }
  float ERP () { return dWorldGetERP (worldID); }
  void SetCFM (float cfm) { dWorldSetCFM (worldID, cfm); }
  float CFM () { return dWorldGetCFM (worldID); }
  void EnableStepFast (bool enable) { stepfast = enable; quickstep = false; };
  bool StepFastEnabled () { return stepfast; }
  void SetStepFastIterations (int iter) { sfiter = iter; }
  int StepFastIterations () { return sfiter; }
  void EnableQuickStep (bool enable) { quickstep = enable; stepfast = false; };
  bool QuickStepEnabled () { return quickstep; };
  void SetQuickStepIterations (int iter) { qsiter = iter; };
  int QuickStepIterations () { return qsiter; }
  void EnableFrameRate (bool enable) { rateenabled = enable; }
  bool FrameRateEnabled () { return rateenabled; }
  void SetFrameRate (float hz) { steptime = 1.0 / hz; }
  float FrameRate () { return 1.0 / steptime; }
  void SetFrameLimit (float hz) { limittime = 1.0 / hz; }
  float FrameLimit () { return 1.0 / limittime; }
  void AddFrameUpdateCallback (iODEFrameUpdateCallback *cb)
  {
    updates.Push (cb);
  }
  void RemoveFrameUpdateCallback (iODEFrameUpdateCallback *cb)
  {
    updates.Delete (cb);
  }
  void EnableFastObjects (bool enable) { fastobjects = enable; }
  bool FastObjectsEnabled () { return false; }
  void EnableAutoDisable (bool enable);
  bool AutoDisableEnabled () { return autodisable; }
  void SetAutoDisableParams (float linear, float angular, int steps,
  	float time);
  void SetContactMaxCorrectingVel (float v);
  float GetContactMaxCorrectingVel ();
  void SetContactSurfaceLayer (float depth);
  float GetContactSurfaceLayer ();
  void EnableOldInertia (bool enable)
  { correctInertiaWorkAround = enable; }
  bool IsOldInertiaEnabled () const
  { return correctInertiaWorkAround; }

  csODEDynamicSystem (iObjectRegistry* object_reg, float erp, float cfm);
  virtual ~csODEDynamicSystem ();

  dWorldID GetWorldID() { return worldID; }
  dSpaceID GetSpaceID() { return spaceID; }
  iCollideSystem* GetCollideSystem() { return collidesys; }

  /**\name iDynamicSystem implementation
   * @{ */
  iObject* QueryObject () { return this; }

  virtual void SetGravity (const csVector3& v);
  virtual const csVector3 GetGravity () const;
  virtual void SetRollingDampener (float d) { roll_damp = (d > 1) ? 1.0 : d; }
  virtual float GetRollingDampener () const { return roll_damp; }
  virtual void SetLinearDampener (float d) { lin_damp = (d > 1) ? 1.0 : d; }
  virtual float GetLinearDampener () const { return lin_damp; }

  virtual void Step (float stepsize);

  virtual csPtr<iRigidBody> CreateBody ();
  virtual void AddBody (iRigidBody* body) { /* TODO */ }
  virtual void RemoveBody (iRigidBody* body);
  virtual iRigidBody *FindBody (const char *name);
  iRigidBody *GetBody (unsigned int index);
  int GetBodysCount () {return (int)bodies.GetSize();};

  virtual csPtr<iBodyGroup> CreateGroup ();
  virtual void RemoveGroup (iBodyGroup *group);

  virtual csPtr<iJoint> CreateJoint ();
  virtual void AddJoint (iJoint* joint) { /* TODO */ }
  virtual csPtr<iODEBallJoint> CreateBallJoint ();
  virtual csPtr<iODEHingeJoint> CreateHingeJoint ();
  virtual csPtr<iODEHinge2Joint> CreateHinge2Joint ();
  virtual csPtr<iODEAMotorJoint> CreateAMotorJoint ();
  virtual csPtr<iODEUniversalJoint> CreateUniversalJoint ();
  virtual csPtr<iODESliderJoint> CreateSliderJoint ();

  virtual void RemoveJoint (iJoint* joint);
  virtual void RemoveJoint (iODEBallJoint* joint);
  virtual void RemoveJoint (iODEAMotorJoint* joint);
  virtual void RemoveJoint (iODEHingeJoint* joint);
  virtual void RemoveJoint (iODEHinge2Joint* joint);
  virtual void RemoveJoint (iODEUniversalJoint* joint);
  virtual void RemoveJoint (iODESliderJoint* joint);

  virtual iDynamicsMoveCallback* GetDefaultMoveCallback () { return move_cb; }

  virtual bool AttachColliderConvexMesh (iMeshWrapper* mesh,
        const csOrthoTransform& trans, float friction, float elasticity,
	float softness)
  {
    return AttachColliderMesh (mesh, trans, friction, elasticity, softness);
  }
  virtual bool AttachColliderMesh (iMeshWrapper* mesh,
        const csOrthoTransform& trans, float friction, float elasticity,
	float softness);
  virtual bool AttachColliderCylinder (float length, float radius,
        const csOrthoTransform& trans, float friction, float elasticity,
	float softness);
  virtual bool AttachColliderCapsule (float length, float radius,
        const csOrthoTransform& trans, float friction, float elasticity,
	float softness);
  virtual bool AttachColliderBox (const csVector3 &size,
        const csOrthoTransform& trans, float friction, float elasticity,
	float softness);
  virtual bool AttachColliderSphere (float radius, const csVector3 &offset,
        float friction, float elasticity, float softness);
  virtual bool AttachColliderPlane (const csPlane3 &plane, float friction,
    float elasticity, float softness);
  csRef<iDynamicsSystemCollider> GetCollider (unsigned int index);
  int GetColliderCount () {return (int)colliders.GetSize ();};
  csRef<iDynamicsSystemCollider> CreateCollider ();
  void DestroyColliders () {colliders.DeleteAll ();};
  void DestroyCollider (iDynamicsSystemCollider* collider)
  {
    colliders.Delete ((csODECollider*)collider);
  }
  void AttachCollider (iDynamicsSystemCollider* collider);
  /** @} */

  virtual bool DoFullInertiaRecalculation () const
  { return !correctInertiaWorkAround; }
  virtual void RecalculateFullInertia (csODECollider* thisCol) {/*Only static stuff, nothing to recalc*/}

};

class csODERigidBody;

/**
 * odedynam implementation of iBodyGroup.  This will set a
 * variable inside the body which will be compared against
 * inside NearCallback
 */
class csODEBodyGroup : 
  public scfImplementation1<csODEBodyGroup,
                            iBodyGroup>
{
  csWeakRefArray<iRigidBody> bodies;

  csODEDynamicSystem* system;

public:
  csODEBodyGroup (csODEDynamicSystem *sys);
  virtual ~csODEBodyGroup ();

  void AddBody (iRigidBody *body);
  void RemoveBody (iRigidBody *body);
  bool BodyInGroup (iRigidBody *body);
};

#include "csutil/deprecated_warn_off.h"

class csODECollider : public scfImplementation1<csODECollider,
                                                iDynamicsSystemCollider>
{
  csODEDynamicSystem* dynsys;
  csColliderGeometryType geom_type;
  dGeomID geomID;
  dGeomID transformID;
  dSpaceID spaceID; 
  ColliderContainer* container;
  float density;
  csRef<iDynamicsColliderCollisionCallback> coll_cb;
  float surfacedata[3];
  bool is_static;

public:
  csODECollider (csODEDynamicSystem* dynsys, ColliderContainer* container);
  virtual ~csODECollider (); 
  
  bool CreateSphereGeometry (const csSphere& sphere);
  bool CreatePlaneGeometry (const csPlane3& plane);
  inline bool CreateConvexMeshGeometry (iMeshWrapper *mesh)
  { return CreateMeshGeometry (mesh); }
  bool CreateMeshGeometry (iMeshWrapper *mesh);
  bool CreateBoxGeometry (const csVector3& box_size);
  bool CreateCapsuleGeometry (float length, float radius); 
  bool CreateCylinderGeometry (float length, float radius); 
  bool GetBoxGeometry (csVector3& size); 
  bool GetSphereGeometry (csSphere& sphere);
  bool GetPlaneGeometry (csPlane3& box); 
  bool GetCylinderGeometry (float& length, float& radius); 
  bool GetCapsuleGeometry (float& length, float& radius);
  bool GetMeshGeometry (csVector3*& vertices, size_t& vertexCount,
			int*& indices, size_t& triangleCount);
  bool GetConvexMeshGeometry (csVector3*& vertices, size_t& vertexCount,
			      int*& indices, size_t& triangleCount);

  void SetCollisionCallback (iDynamicsColliderCollisionCallback* cb);
  void Collision (csODECollider* other, const csVector3& pos,
      const csVector3& normal, float depth);
  void Collision (iRigidBody* other, const csVector3& pos,
      const csVector3& normal, float depth);
  void SetFriction (float friction) {surfacedata[0] = friction;};
  void SetSoftness (float softness) {surfacedata[2] = softness;};
  void SetElasticity (float elasticity) {surfacedata[1] = elasticity;};
  void SetDensity (float density);
  
  float GetFriction () {return surfacedata[0];};
  float GetSoftness () {return surfacedata[1];};
  float GetElasticity () {return surfacedata[2];};
  float GetDensity () {return density;};

  void FillWithColliderGeometry (csRef<iGeneralFactoryState> genmesh_fact);

  csOrthoTransform GetTransform ();
  csOrthoTransform GetLocalTransform ();
  void SetTransform (const csOrthoTransform& trans);

  csColliderGeometryType GetGeometryType () {return geom_type;};

  void AttachBody (dBodyID bodyID);
  void AddTransformToSpace (dSpaceID spaceID);
  void AddToSpace (dSpaceID spaceID);

  void MakeStatic ();
  void MakeDynamic ();
  bool IsStatic ();

  void AddMassToBody (bool doSet);

private:

  void MassUpdate ();
  void ClearContents ();
  void KillGeoms ();

};

#include "csutil/deprecated_warn_on.h"

struct GeomData 
{
  float *surfacedata;
  csODECollider *collider;
  ~GeomData () 
  {if (surfacedata) delete [] surfacedata; if (collider) collider->DecRef ();}
};

/**
 * This is the implementation for a rigid body.
 * It keeps all properties for the body.
 * It can also be attached to a movable,
 * to automatically update it.
 */
class csODERigidBody : 
  public scfImplementationExt1<csODERigidBody,
                               csObject,
                               iRigidBody>, 
  public ColliderContainer
{
private:
  dBodyID bodyID;
  dSpaceID groupID;
  dJointID statjoint;
  csRefArray<csODECollider> colliders;

  /* these must be ptrs to avoid circular referencing */
  iBodyGroup* collision_group;
  csODEDynamicSystem* dynsys;

  csRef<iMeshWrapper> mesh;
  csRef<iLight> light;
  csRef<iCamera> camera;
  csRef<iDynamicsMoveCallback> move_cb;
  csRef<iDynamicsCollisionCallback> coll_cb;

public:
  csODERigidBody (csODEDynamicSystem* sys);
  virtual ~csODERigidBody ();

  using iObject::GetID;
  inline dBodyID GetID() { return bodyID; }

  iObject* QueryObject () { return this; }
  bool MakeStatic (void);
  bool IsStatic (void) { return statjoint != 0; }
  bool MakeDynamic (void);

  bool Disable (void);
  bool Enable (void);
  bool IsEnabled (void);

  void SetGroup (iBodyGroup *group);
  void UnsetGroup () { collision_group = 0; }
  csRef<iBodyGroup> GetGroup (void) { return collision_group; }

  bool AttachColliderConvexMesh (iMeshWrapper* mesh,
        const csOrthoTransform& trans, float friction, float density,
        float elasticity, float softness)
  {
    return AttachColliderMesh (mesh, trans, friction, density,
      elasticity, softness);
  }
  bool AttachColliderMesh (iMeshWrapper* mesh,
        const csOrthoTransform& trans, float friction, float density,
        float elasticity, float softness);
  bool AttachColliderCylinder (float length, float radius,
        const csOrthoTransform& trans, float friction, float density,
        float elasticity, float softness);
  bool AttachColliderCapsule (float length, float radius,
        const csOrthoTransform& trans, float friction, float density,
        float elasticity, float softness);
  bool AttachColliderBox (const csVector3 &size,
        const csOrthoTransform& trans, float friction, float density,
        float elasticity, float softness);
  bool AttachColliderSphere (float radius, const csVector3 &offset,
        float friction, float density, float elasticity, float softness);
  /// ODE planes are globally transformed, immobile, infinitely dense
  bool AttachColliderPlane (const csPlane3 &plane, float friction,
    float density, float elasticity, float softness);
  void AttachCollider (iDynamicsSystemCollider* collider);
  void DestroyColliders () {colliders.DeleteAll ();};
  void DestroyCollider (iDynamicsSystemCollider* collider)
  {
    colliders.Delete ((csODECollider*)collider);
  }

  void SetPosition (const csVector3& trans);
  const csVector3 GetPosition () const;
  void SetOrientation (const csMatrix3& trans);
  const csMatrix3 GetOrientation () const;
  void SetTransform (const csOrthoTransform& trans);
  const csOrthoTransform GetTransform () const;
  void SetLinearVelocity (const csVector3& vel);
  const csVector3 GetLinearVelocity () const;
  void SetAngularVelocity (const csVector3& vel);
  const csVector3 GetAngularVelocity () const;

  void SetProperties (float mass, const csVector3& center,
        const csMatrix3& inertia);
  void GetProperties (float* mass, csVector3* center,
    csMatrix3* inertia);
  float GetMass ();
  csVector3 GetCenter ();
  csMatrix3 GetInertia ();
  void AdjustTotalMass (float targetmass);

  void AddForce (const csVector3& force);
  void AddTorque (const csVector3& force);
  void AddRelForce (const csVector3& force);
  void AddRelTorque (const csVector3& force) ;
  void AddForceAtPos (const csVector3& force, const csVector3& pos);
  void AddForceAtRelPos (const csVector3& force, const csVector3& pos);
  void AddRelForceAtPos (const csVector3& force, const csVector3& pos);
  void AddRelForceAtRelPos (const csVector3& force, const csVector3& pos);

  const csVector3 GetForce () const;
  const csVector3 GetTorque () const;

  /*const csVector3 GetRelForce () const;
  const csVector3 GetRelTorque () const;*/

  //int GetJointCount () const = 0;

  void AttachMesh (iMeshWrapper* mesh);
  iMeshWrapper* GetAttachedMesh () { return mesh; }
  void AttachLight (iLight* light);
  iLight* GetAttachedLight () { return light; }
  void AttachCamera (iCamera* camera);
  iCamera* GetAttachedCamera () { return camera; }
  void SetMoveCallback (iDynamicsMoveCallback* cb);
  void SetCollisionCallback (iDynamicsCollisionCallback* cb);

  void Collision (iRigidBody *other, const csVector3& pos,
      const csVector3& normal, float depth);
  void Update ();

  csRef<iDynamicsSystemCollider> GetCollider (unsigned int index);
  int GetColliderCount () {return (int)colliders.GetSize ();};

  virtual bool DoFullInertiaRecalculation () const
  { return !dynsys->IsOldInertiaEnabled (); }
  virtual void RecalculateFullInertia (csODECollider* thisCol);
};


struct csStrictODEJoint : public scfImplementation0<csStrictODEJoint>
{
  dJointID jointID;
  csRef<iRigidBody> body[2];
  dBodyID bodyID[2];
  dJointFeedback *feedback;

  csStrictODEJoint () : scfImplementationType (this), feedback (0) {}
  virtual ~csStrictODEJoint () {}

  void Attach (iRigidBody *body1, iRigidBody *body2);
  csRef<iRigidBody> GetAttachedBody (int body);
  void SetParam (int joint_type, int param, int axis, float value);
  float GetParam (int joint_type, int param, int axis);
  csVector3 GetFeedbackForce1 ();
  csVector3 GetFeedbackTorque1 ();
  csVector3 GetFeedbackForce2 ();
  csVector3 GetFeedbackTorque2 ();

private:

  void CreateFeedback ();

};

/**
* This implements the slider joint.  It does this by strict copying
* ODEs interface.
*/
struct ODESliderJoint : 
  public scfImplementationExt2<ODESliderJoint,
                               csStrictODEJoint, 
                               iODESliderJoint,
                               scfFakeInterface<iODEGeneralJointState> >
{
  ODESliderJoint (dWorldID w_id);
  virtual ~ODESliderJoint ();

  void SetLoStop (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamLoStop, axis,
    	value);
  }
  void SetHiStop (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamHiStop, axis,
    	value);
  }
  void SetVel (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamVel, axis,
      value);
  }
  void SetFMax (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamFMax, axis,
      value);
  }
  void SetFudgeFactor (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamFudgeFactor,
      axis, value);
  }
  void SetBounce (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamBounce, axis,
      value);
  }
  void SetCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamCFM, axis,
      value);
  }
  void SetStopERP (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamStopERP, axis,
      value);
  }
  void SetStopCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamStopCFM, axis,
      value);
  }
  void SetSuspensionERP (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamSuspensionERP,
      axis, value);
  }
  void SetSuspensionCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamSuspensionCFM,
      axis, value);
  }

  float GetLoStop (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamLoStop,
      axis);
  }
  float GetHiStop (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamHiStop,
      axis);
  }
  float GetVel (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamVel,
      axis);
  }
  float GetFMax (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamFMax,
      axis);
  }
  float GetFudgeFactor (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER,
      dParamFudgeFactor, axis);
  }
  float GetBounce (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamBounce,
      axis);
  }
  float GetCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER, dParamCFM,
      axis);
  }
  float GetStopERP (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER,
      dParamStopERP, axis);
  }
  float GetStopCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER,
      dParamStopCFM, axis);
  }
  float GetSuspensionERP (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER,
      dParamSuspensionERP, axis);
  }
  virtual float GetSuspensionCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_SLIDER,
      dParamSuspensionCFM, axis);
  }

  void SetSliderAxis (float x, float y, float z)
  {
    dJointSetSliderAxis (jointID, x, y, z);
  }
  csVector3 GetSliderAxis ();
  float GetSliderPosition () {return dJointGetSliderPosition (jointID);};
  float GetSliderPositionRate ()
  {
    return dJointGetSliderPositionRate (jointID);
  }
  csVector3 GetFeedbackForce1 ()
  {
    return csStrictODEJoint::GetFeedbackForce1 ();
  }
  csVector3 GetFeedbackTorque1 ()
  {
    return csStrictODEJoint::GetFeedbackForce2 ();
  }
  csVector3 GetFeedbackForce2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque1 ();
  }
  csVector3 GetFeedbackTorque2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque2 ();
  }

  void Attach (iRigidBody *body1, iRigidBody *body2)
  {
    csStrictODEJoint::Attach (body1, body2);
  }
  csRef<iRigidBody> GetAttachedBody (int body)
  {
    return csStrictODEJoint::GetAttachedBody (body);
  }
};

struct ODEUniversalJoint : 
  public scfImplementationExt2<ODEUniversalJoint,
                               csStrictODEJoint, 
                               iODEUniversalJoint,
                               scfFakeInterface<iODEGeneralJointState> >
{
  ODEUniversalJoint (dWorldID w_id);
  virtual ~ODEUniversalJoint ();

  void SetLoStop (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL, dParamLoStop,
      axis, value);
  }
  void SetHiStop (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL, dParamHiStop,
      axis, value);
  }
  void SetVel (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL, dParamVel, axis,
      value);
  }
  void SetFMax (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL, dParamFMax, axis,
      value);
  }
  void SetFudgeFactor (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamFudgeFactor, axis, value);
  }
  void SetBounce (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL, dParamBounce,
      axis, value);
  }
  void SetCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL, dParamCFM, axis,
      value);
  }
  void SetStopERP (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL, dParamStopERP,
      axis, value);
  }
  void SetStopCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL, dParamStopCFM,
      axis, value);
  }
  void SetSuspensionERP (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamSuspensionERP, axis, value);
  }
  void SetSuspensionCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamSuspensionCFM, axis, value);
  }

  float GetLoStop (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
    	dParamLoStop, axis);
  }
  float GetHiStop (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamHiStop, axis);
  }
  float GetVel (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamVel, axis);
  }
  float GetFMax (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamFMax, axis);
  }
  float GetFudgeFactor (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamFudgeFactor, axis);
  }
  float GetBounce (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamBounce, axis);
  }
  float GetCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamCFM, axis);
  }
  float GetStopERP (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamStopERP, axis);
  }
  float GetStopCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamStopCFM, axis);
  }
  float GetSuspensionERP (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamSuspensionERP, axis);
  }
  virtual float GetSuspensionCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_UNIVERSAL,
      dParamSuspensionCFM, axis);
  }

  void SetUniversalAnchor (float x, float y, float z)
  {
    dJointSetUniversalAnchor (jointID, x, y, z);
  }
  void SetUniversalAxis1 (float x, float y, float z)
  {
    dJointSetUniversalAxis1 (jointID, x, y, z);
  }
  void SetUniversalAxis2 (float x, float y, float z)
  {
    dJointSetUniversalAxis2 (jointID, x, y, z);
  }
  csVector3 GetUniversalAnchor1 ();
  csVector3 GetUniversalAnchor2 ();
  csVector3 GetUniversalAxis1 ();
  csVector3 GetUniversalAxis2 ();
  csVector3 GetFeedbackForce1 ()
  {
    return csStrictODEJoint::GetFeedbackForce1 ();
  }
  csVector3 GetFeedbackTorque1 ()
  {
    return csStrictODEJoint::GetFeedbackForce2 ();
  }
  csVector3 GetFeedbackForce2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque1 ();
  }
  csVector3 GetFeedbackTorque2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque2 ();
  }

  void Attach (iRigidBody *body1, iRigidBody *body2)
  {
    csStrictODEJoint::Attach (body1, body2);
  }
  csRef<iRigidBody> GetAttachedBody (int body)
  {
    return csStrictODEJoint::GetAttachedBody (body);
  }
};

struct ODEBallJoint : 
  public scfImplementationExt1<ODEBallJoint,
                               csStrictODEJoint, 
                               iODEBallJoint>
{
  ODEBallJoint (dWorldID w_id);

  void SetBallAnchor (const csVector3 &pos)
  {
    dJointSetBallAnchor (jointID, pos.x, pos.y, pos.z);
  }
  csVector3 GetBallAnchor1 ();
  csVector3 GetBallAnchor2 ();
  csVector3 GetAnchorError ();
  csVector3 GetFeedbackForce1 ()
  {
    return csStrictODEJoint::GetFeedbackForce1 ();
  }
  csVector3 GetFeedbackTorque1 ()
  {
    return csStrictODEJoint::GetFeedbackForce2 ();
  }
  csVector3 GetFeedbackForce2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque1 ();
  }
  csVector3 GetFeedbackTorque2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque2 ();
  }

  virtual ~ODEBallJoint ();

  void Attach (iRigidBody *body1, iRigidBody *body2)
  {
    csStrictODEJoint::Attach (body1, body2);
  }
  csRef<iRigidBody> GetAttachedBody (int body)
  {
    return csStrictODEJoint::GetAttachedBody (body);
  }
};

struct ODEAMotorJoint : 
  public scfImplementationExt2<ODEAMotorJoint,
                               csStrictODEJoint, 
                               iODEAMotorJoint,
                               scfFakeInterface<iODEGeneralJointState> >
{
  ODEAMotorJoint (dWorldID w_id);
  virtual ~ODEAMotorJoint ();

  void SetLoStop (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamLoStop, axis,
      value);
  }
  void SetHiStop (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamHiStop, axis,
      value);
  }
  void SetVel (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamVel, axis,
      value);
  }
  void SetFMax (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamFMax, axis,
      value);
  }
  void SetFudgeFactor (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamFudgeFactor,
      axis, value);
  }
  void SetBounce (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamBounce, axis,
      value);
  }
  void SetCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamCFM, axis,
      value);
  }
  void SetStopERP (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamStopERP,
      axis, value);
  }
  void SetStopCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamStopCFM,
      axis, value);
  }
  void SetSuspensionERP (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamSuspensionERP,
      axis, value);
  }
  void SetSuspensionCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamSuspensionCFM,
      axis, value);
  }

  float GetLoStop (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamLoStop,
      axis);
  }
  float GetHiStop (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamHiStop,
      axis);
  }
  float GetVel (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamVel,
      axis);
  }
  float GetFMax (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamFMax,
      axis);
  }
  float GetFudgeFactor (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamFudgeFactor,
      axis);
  }
  float GetBounce (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamBounce,
      axis);
  }
  float GetCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamCFM,
      axis);
  }
  float GetStopERP (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamStopERP,
      axis);
  }
  float GetStopCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR, dParamStopCFM,
      axis);
  }
  float GetSuspensionERP (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR,
    	dParamSuspensionERP, axis);
  }
  virtual float GetSuspensionCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_AMOTOR,
    	dParamSuspensionCFM, axis);
  }

  void SetAMotorMode (ODEAMotorMode mode);
  ODEAMotorMode GetAMotorMode ();
  void SetAMotorNumAxes (int axis_num)
  {
    dJointSetAMotorNumAxes (jointID, axis_num);
  }
  int GetAMotorNumAxes () {return dJointGetAMotorNumAxes (jointID);}
  void SetAMotorAxis (int axis_num, int rel_orient, float x, float y, float z)
  {
    dJointSetAMotorAxis (jointID, axis_num, rel_orient, x, y, z);
  }
  void SetAMotorAxis (int axis_num, int rel_orient, const csVector3 &axis)
  {
    dJointSetAMotorAxis (jointID, axis_num, rel_orient, axis.x, axis.y, axis.z);
  }
  csVector3 GetAMotorAxis (int axis_num);
  int GetAMotorAxisRelOrientation (int axis_num)
  {
    return dJointGetAMotorAxisRel (jointID, axis_num);
  }
  void SetAMotorAngle (int axis_num, float angle)
  {
    dJointSetAMotorAngle (jointID, axis_num, angle);
  }
  float GetAMotorAngle (int axis_num)
  {
    return dJointGetAMotorAngle (jointID, axis_num);
  }
  float GetAMotorAngleRate (int axis_num)
  {
    return dJointGetAMotorAngle (jointID, axis_num);
  }
  csVector3 GetFeedbackForce1 ()
  {
    return csStrictODEJoint::GetFeedbackForce1 ();
  }
  csVector3 GetFeedbackTorque1 ()
  {
    return csStrictODEJoint::GetFeedbackForce2 ();
  }
  csVector3 GetFeedbackForce2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque1 ();
  }
  csVector3 GetFeedbackTorque2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque2 ();
  }

  void Attach (iRigidBody *body1, iRigidBody *body2)
  {
    csStrictODEJoint::Attach (body1, body2);
  }
  csRef<iRigidBody> GetAttachedBody (int body)
  {
    return csStrictODEJoint::GetAttachedBody (body);
  }
};

struct ODEHinge2Joint : 
  public scfImplementationExt2<ODEHinge2Joint,
                               csStrictODEJoint, 
                               iODEHinge2Joint,
                               scfFakeInterface<iODEGeneralJointState> >
{
  ODEHinge2Joint (dWorldID w_id);
  virtual ~ODEHinge2Joint ();

  void SetLoStop (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamLoStop, axis,
      value);
  }
  void SetHiStop (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamHiStop, axis,
      value);
  }
  void SetVel (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamVel, axis,
      value);
  }
  void SetFMax (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamFMax, axis,
      value);
  }
  void SetFudgeFactor (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamFudgeFactor,
      axis, value);
  }
  void SetBounce (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamBounce, axis,
      value);
  }
  void SetCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamCFM, axis,
      value);
  }
  void SetStopERP (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamStopERP, axis,
      value);
  }
  void SetStopCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamStopCFM, axis,
      value);
  }
  void SetSuspensionERP (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamSuspensionERP,
      axis, value);
  }
  void SetSuspensionCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamSuspensionCFM,
      axis, value);
  }

  float GetLoStop (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamLoStop,
      axis);
  }
  float GetHiStop (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamHiStop,
      axis);
  }
  float GetVel (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamVel,
      axis);
  }
  float GetFMax (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamFMax,
      axis);
  }
  float GetFudgeFactor (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamFudgeFactor,
      axis);
  }
  float GetBounce (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamBounce,
      axis);
  }
  float GetCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamCFM,
      axis);
  }
  float GetStopERP (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamStopERP,
      axis);
  }
  float GetStopCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2, dParamStopCFM,
      axis);
  }
  float GetSuspensionERP (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2,
    	dParamSuspensionERP, axis);
  }
  virtual float GetSuspensionCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE2,
    	dParamSuspensionCFM, axis);
  }

  void SetHingeAnchor (const csVector3 &pos)
  {
    dJointSetHinge2Anchor (jointID, pos.x, pos.y, pos.z);
  }
  void SetHingeAxis1 (const csVector3 &axis)
  {
    dJointSetHinge2Axis1 (jointID, axis.x, axis.y, axis.z);
  }
  void SetHingeAxis2 (const csVector3 &axis)
  {
    dJointSetHinge2Axis2 (jointID, axis.x, axis.y, axis.z);
  }
  csVector3 GetHingeAnchor1 ();
  csVector3 GetHingeAnchor2 ();
  csVector3 GetHingeAxis1 ();
  csVector3 GetHingeAxis2 ();
  float GetHingeAngle () {return dJointGetHingeAngle (jointID);}
  float GetHingeAngleRate1 () {return dJointGetHinge2Angle1Rate (jointID);}
  float GetHingeAngleRate2 () {return dJointGetHinge2Angle2Rate (jointID);};
  csVector3 GetAnchorError ();
  csVector3 GetFeedbackForce1 ()
  {
    return csStrictODEJoint::GetFeedbackForce1 ();
  }
  csVector3 GetFeedbackTorque1 ()
  {
    return csStrictODEJoint::GetFeedbackForce2 ();
  }
  csVector3 GetFeedbackForce2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque1 ();
  }
  csVector3 GetFeedbackTorque2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque2 ();
  }

  void Attach (iRigidBody *body1, iRigidBody *body2)
  {
    csStrictODEJoint::Attach (body1, body2);
  }
  csRef<iRigidBody> GetAttachedBody (int body)
  {
    return csStrictODEJoint::GetAttachedBody (body);
  }
};

struct ODEHingeJoint : 
  public scfImplementationExt2<ODEHingeJoint,
                               csStrictODEJoint, 
                               iODEHingeJoint,
                               scfFakeInterface<iODEGeneralJointState> >
{
  ODEHingeJoint (dWorldID w_id);
  virtual ~ODEHingeJoint ();

  void SetLoStop (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamLoStop, axis,
      value);
  }
  void SetHiStop (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamHiStop, axis,
      value);
  }
  void SetVel (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamVel, axis,
      value);
  }
  void SetFMax (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamFMax, axis,
      value);
  }
  void SetFudgeFactor (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamFudgeFactor,
      axis, value);
  }
  void SetBounce (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamBounce, axis,
      value);
  }
  void SetCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamCFM, axis,
      value);
  }
  void SetStopERP (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamStopERP, axis,
      value);
  }
  void SetStopCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamStopCFM, axis,
      value);
  }
  void SetSuspensionERP (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamSuspensionERP,
      axis, value);
  }
  void SetSuspensionCFM (float value, int axis)
  {
    csStrictODEJoint::SetParam (CS_ODE_JOINT_TYPE_HINGE, dParamSuspensionCFM,
      axis, value);
  }

  float GetLoStop (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE, dParamLoStop,
      axis);
  }
  float GetHiStop (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE, dParamHiStop,
      axis);
  }
  float GetVel (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE, dParamVel,
      axis);
  }
  float GetFMax (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE, dParamFMax,
      axis);
  }
  float GetFudgeFactor (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE, dParamFudgeFactor,
      axis);
  }
  float GetBounce (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE, dParamBounce,
      axis);
  }
  float GetCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE, dParamCFM,
      axis);
  }
  float GetStopERP (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE, dParamStopERP,
      axis);
  }
  float GetStopCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE, dParamStopCFM,
      axis);
  }
  float GetSuspensionERP (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE,
    	dParamSuspensionERP, axis);
  }
  virtual float GetSuspensionCFM (int axis)
  {
    return csStrictODEJoint::GetParam (CS_ODE_JOINT_TYPE_HINGE,
    	dParamSuspensionCFM, axis);
  }

  void SetHingeAnchor (const csVector3 &pos)
  {
    dJointSetHingeAnchor (jointID, pos.x, pos.y, pos.z);
  }
  void SetHingeAxis (const csVector3 &axis)
  {
    dJointSetHingeAxis (jointID, axis.x, axis.y, axis.z);
  }
  csVector3 GetHingeAnchor1 ();
  csVector3 GetHingeAnchor2 ();
  csVector3 GetHingeAxis ();
  float GetHingeAngle () {return dJointGetHingeAngle (jointID);};
  float GetHingeAngleRate () {return dJointGetHingeAngleRate (jointID);};
  csVector3 GetAnchorError ();
  csVector3 GetFeedbackForce1 ()
  {
    return csStrictODEJoint::GetFeedbackForce1 ();
  }
  csVector3 GetFeedbackTorque1 ()
  {
    return csStrictODEJoint::GetFeedbackForce2 ();
  }
  csVector3 GetFeedbackForce2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque1 ();
  }
  csVector3 GetFeedbackTorque2 ()
  {
    return csStrictODEJoint::GetFeedbackTorque2 ();
  }

  void Attach (iRigidBody *body1, iRigidBody *body2)
  {
    csStrictODEJoint::Attach (body1, body2);
  }
  csRef<iRigidBody> GetAttachedBody (int body)
  {
    return csStrictODEJoint::GetAttachedBody (body);
  }
};


/**
 * This implements the joint.  It does this by determining
 * which type of ODE joint best represents that described
 */
class csODEJoint : public scfImplementation2<
  csODEJoint, iJoint, iODEJointState> 
{
  dJointID jointID;
  dJointID motor_jointID;
  csRef<iRigidBody> body[2];
  dBodyID bodyID[2];

  bool is_dirty;

  int transConstraint[3], rotConstraint[3];
  csVector3 min_angle, max_angle, min_dist, max_dist, vel, fmax, fudge_factor, bounce,
    cfm, stop_erp, stop_cfm, suspension_cfm, suspension_erp;

  csOrthoTransform transform;

  csODEDynamicSystem* dynsys;

public:

  csODEJoint (csODEDynamicSystem *sys);
  virtual ~csODEJoint ();

  inline dJointID GetID () { return jointID; }

  void Attach (iRigidBody *body1, iRigidBody *body2, bool force_update);
  csRef<iRigidBody> GetAttachedBody (int body);

  void SetTransform (const csOrthoTransform &trans, bool force_update);
  csOrthoTransform GetTransform ();

  void SetMotorsParams (dJointID joint);

  void SetTransConstraints (bool X, bool Y, bool Z, bool force_update);
  inline bool IsXTransConstrained () { return (transConstraint[0] != 0); }
  inline bool IsYTransConstrained () { return (transConstraint[1] != 0); }
  inline bool IsZTransConstrained () { return (transConstraint[2] != 0); }
  void SetMinimumDistance (const csVector3 &min, bool force_update);
  csVector3 GetMinimumDistance ();
  void SetMaximumDistance (const csVector3 &max, bool force_update);
  csVector3 GetMaximumDistance ();

  void SetRotConstraints (bool X, bool Y, bool Z, bool force_update);
  inline bool IsXRotConstrained () { return (rotConstraint[0] != 0); }
  inline bool IsYRotConstrained () { return (rotConstraint[1] != 0); }
  inline bool IsZRotConstrained () { return (rotConstraint[2] != 0); }
  void SetMinimumAngle (const csVector3 &min, bool force_update);
  csVector3 GetMinimumAngle ();
  void SetMaximumAngle (const csVector3 &max, bool force_update);
  csVector3 GetMaximumAngle ();

  bool RebuildJoint ();

  void SetBounce (const csVector3 & bounce, bool force_update)
  { 
    csODEJoint::bounce = bounce;
    if (force_update)
      RebuildJoint ();
  }
  void SetBounce (const csVector3 & bounce)
  { 
    SetBounce (bounce, true);
  }
  csVector3 GetBounce (){return bounce;}
  void SetDesiredVelocity (const csVector3 & velocity, bool force_update)
  { 
    vel = velocity;
    if (force_update)
      RebuildJoint ();
  }
  csVector3 GetDesiredVelocity (){return vel;}
  void SetMaxForce (const csVector3 & maxForce, bool force_update)
  { 
    fmax = maxForce;
    if (force_update)
      RebuildJoint ();
  }
  csVector3 GetMaxForce (){return fmax;}
  void SetAngularConstraintAxis (const csVector3 &axis, int body, bool force_update); 
  csVector3 GetAngularConstraintAxis (int body);

  //----------------iODEJointState----------------
  ODEJointType GetType();
  void SetLoStop (const csVector3 &value) 
  { 
    min_angle = min_dist = value;
    RebuildJoint ();
  }
  void SetHiStop (const csVector3 &value) 
  { 
    max_angle = max_dist = value;
    RebuildJoint ();
  }
  void SetVel (const csVector3 &value) 
  {
    vel = value;
    RebuildJoint ();
  }
  void SetFMax (const csVector3 &value) 
  {
    fmax = value;
    RebuildJoint ();
  }
  void SetFudgeFactor (const csVector3 &value) 
  {
    fudge_factor = value;
    RebuildJoint ();
  }
  void SetCFM (const csVector3 &value) 
  {
    cfm = value;
    RebuildJoint ();
  }
  void SetStopERP (const csVector3 &value) 
  {
    stop_erp = value;
    RebuildJoint ();
  }
  void SetStopCFM (const csVector3 &value) 
  { 
    stop_cfm = value;
    RebuildJoint ();
  }
  void SetSuspensionERP (const csVector3 &value)
  {
    suspension_erp = value;
    RebuildJoint ();
  }
  void SetSuspensionCFM (const csVector3 &value)
  {
    suspension_cfm = value;
    RebuildJoint ();
  }

  csVector3 GetLoStop () { return min_angle; }
  csVector3 GetHiStop () { return max_angle; }
  csVector3 GetVel () { return vel; }
  csVector3 GetFMax () { return fmax; }
  csVector3 GetFudgeFactor () { return fudge_factor; }
  csVector3 GetCFM () { return cfm; }
  csVector3 GetStopERP () { return stop_erp; }
  csVector3 GetStopCFM () { return stop_cfm; }
  csVector3 GetSuspensionERP () { return suspension_erp; }
  csVector3 GetSuspensionCFM () { return suspension_cfm; }

private:

  void ApplyJointProperty (dJointID joint, int parameter, const csVector3 &values);
  csVector3 GetParam (int parameter);

  void BuildHinge ();
  void BuildHinge2 ();
  void BuildBall ();
  void BuildAMotor ();
  void BuildSlider ();
  
  void Clear ();

};

/**
 * This is the implementation for a default dynamics move callback.
 * It can update mesh.
 */
class csODEDefaultMoveCallback : 
  public scfImplementation1<csODEDefaultMoveCallback,
                            iDynamicsMoveCallback>
{
private:
  void Execute (iMovable* movable, csOrthoTransform& t);

public:
  csODEDefaultMoveCallback ();
  virtual ~csODEDefaultMoveCallback ();

  virtual void Execute (iMeshWrapper* mesh, csOrthoTransform& t);
  virtual void Execute (iLight* light, csOrthoTransform& t);
  virtual void Execute (iCamera* camera, csOrthoTransform& t);
  virtual void Execute (csOrthoTransform& t);
};

}
CS_PLUGIN_NAMESPACE_END(odedynam)

#endif // __CS_ODEDYNAMICS_H__
