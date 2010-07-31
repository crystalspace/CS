/*
    Copyright (C) 2007 by Jorrit Tyberghein

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

#ifndef __CS_IVARIA_BULLET_H__
#define __CS_IVARIA_BULLET_H__

/**\file
 * Bullet-specific interfaces
 */

#include "csutil/scf_interface.h"
#include "iutil/objreg.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "imesh/genmesh.h"
#include "csgeom/tri.h"
#include "cstool/primitives.h"

struct iView;
struct iRigidBody;
struct iBulletKinematicCallback;
struct iBulletSoftBody;
struct iBulletPivotJoint;
struct iBulletTerrainCollider;
struct csLockedHeightData;
struct iTerrainCell;
struct iTerrainSystem;

/**
 * The type of the body for a Bullet's collider.
 */
enum csBulletBodyType
{
  CS_BULLET_UNDEFINED_BODY = 0,     /*!< Undefined body type. */
  CS_BULLET_RIGID_BODY,             /*!< The body is a rigid body. */
  CS_BULLET_SOFT_BODY,              /*!< The body is a soft body. */
  CS_BULLET_TERRAIN                 /*!< The body is a terrain collider. */
};

/**
 * Return structure for the iBulletDynamicSystem::HitBeam() routine. It returns
 * whether a rigid body, a soft body or a physical terrain collider has been hit.
 * \sa csHitBeamResult csSectorHitBeamResult
 */
struct csBulletHitBeamResult
{
  csBulletHitBeamResult ()
  : hasHit (false), bodyType (CS_BULLET_UNDEFINED_BODY), rigidBody (0), softBody (0),
    terrain (0), isect (0.0f), normal (0.0f), vertexIndex (0)
  {}

  /**
   * Whether the beam has hit a body or not.
   */
  bool hasHit;

  /**
   * The type of the body that was hit.
   */
  csBulletBodyType bodyType;

  /**
   * The resulting rigid body that was hit, or 0 if no rigid body was hit.
   */
  iRigidBody* rigidBody;

  /**
   * The resulting soft body that was hit, or 0 if no soft body was hit.
   */
  iBulletSoftBody* softBody;

  /**
   * The resulting terrain collider that was hit, or 0 if no terrain collider was hit.
   */
  iBulletTerrainCollider* terrain;

  /**
   * Intersection point in world space.
   */
  csVector3 isect;

  /**
   * Normal to the surface of the body at the intersection point.
   */
  csVector3 normal;

  /**
   * The index of the closest vertex of the soft body to be hit. This is only valid
   * if it is a soft body which is hit (ie softBody is different than 0 and bodyType
   * is equal to CS_BULLET_SOFT_BODY).
   */
  size_t vertexIndex;
};

/**
 * The debug modes to be used with iBulletDynamicSystem::DebugDraw().
 */
enum csBulletDebugMode
{
  CS_BULLET_DEBUG_NOTHING = 0,     /*!< Nothing will be displayed. */
  CS_BULLET_DEBUG_COLLIDERS = 1,   /*!< Display the colliders of the bodies. */
  CS_BULLET_DEBUG_AABB = 2,        /*!< Display the axis aligned bounding boxes of the bodies. */
  CS_BULLET_DEBUG_JOINTS = 4       /*!< Display the joint positions and limits. */
};

/**
 * The Bullet implementation of iDynamicSystem also implements this
 * interface.
 * \sa iDynamicSystem iODEDynamicSystemState
 */
struct iBulletDynamicSystem : public virtual iBase
{
  SCF_INTERFACE(iBulletDynamicSystem, 3, 0, 0);

  /**
   * Draw the debug informations of the dynamic system. This has to be called
   * at each frame, and will add 2D lines on top of the rendered scene. The
   * objects to be displayed are defined by SetDebugMode().
   */
  virtual void DebugDraw (iView* rview) = 0;

  /**
   * Follow a beam from start to end and return the first body that is hit.
   * \return True if a body was hit, false otherwise.
   * \sa csBulletHitBeamResult iMeshWrapper::HitBeam() iSector::HitBeam()
   * iSector::HitBeamPortals()
   */
  virtual csBulletHitBeamResult HitBeam (const csVector3 &start,
					 const csVector3 &end) = 0;

  /**
   * Set the internal scale to be applied to the whole dynamic world. Use this
   * to put back the range of dimensions you use for your objects to the one
   * Bullet was designed for.
   *
   * Bullet does not work well if the dimensions of your objects are smaller
   * than 0.1 to 1.0 units or bigger than 10 to 100 units. Use this method to
   * fix the problem.
   *
   * \warning You have to call this method before adding any objects in the
   * dynamic world, otherwise the objects won't have the same scale.
   */
  virtual void SetInternalScale (float scale) = 0;

  /**
   * Set the parameters of the constraint solver. Use this if you want to find a
   * compromise between accuracy of the simulation and performance cost.
   * \param timeStep The internal, constant, time step of the simulation, in seconds.
   * A smaller value gives better accuracy. Default value is 1/60 s (ie 0.0166 s).
   * \param maxSteps Maximum number of steps that Bullet is allowed to take each
   * time you call iDynamicSystem::Step(). If you pass a very small time step as
   * the first parameter, then you must increase the number of maxSteps to
   * compensate for this, otherwise your simulation is 'losing' time. Default value
   * is 1. If you pass maxSteps=0 to the function, then it will assume a variable
   * tick rate. Don't do it.
   * \param iterations Number of iterations of the constraint solver. A reasonable
   * range of iterations is from 4 (low quality, good performance) to 20 (good
   * quality, less but still reasonable performance). Default value is 10. 
   */
  virtual void SetStepParameters (float timeStep, size_t maxSteps,
				  size_t iterations) = 0;

  /**
   * Set the mode to be used when displaying debug informations. The default value
   * is 'CS_BULLET_DEBUG_COLLIDERS | CS_BULLET_DEBUG_JOINTS'.
   * \remark Don't forget to call DebugDraw() at each frame to effectively display
   * the debug informations.
   */
  virtual void SetDebugMode (csBulletDebugMode mode) = 0;

  /**
   * Return the current mode used when displaying debug informations.
   */
  virtual csBulletDebugMode GetDebugMode () = 0;

  /**
   * Set whether this dynamic world can handle soft bodies or not.
   * \warning You have to call this method before adding any objects in the
   * dynamic world.
   */
  virtual void SetSoftBodyWorld (bool isSoftBodyWorld) = 0;

  /**
   * Return whether this dynamic world can handle soft bodies or not.
   */
  virtual bool GetSoftBodyWorld () = 0;

  /**
   * Return the number of soft bodies in this dynamic world.
   */
  virtual size_t GetSoftBodyCount () = 0;

  /**
   * Return the soft body at the given index.
   */
  virtual iBulletSoftBody* GetSoftBody (size_t index) = 0;

  /**
   * Create a soft body rope.
   * \param start Start position of the rope.
   * \param end End position of the rope.
   * \param segmentCount Number of segments in the rope.
   * \remark You must call SetSoftBodyWorld() prior to this.
   */
  virtual iBulletSoftBody* CreateRope (csVector3 start, csVector3 end,
				       uint segmentCount) = 0;

  /**
   * Create a soft body cloth.
   * \param corner1 The position of the top left corner.
   * \param corner2 The position of the top right corner.
   * \param corner3 The position of the bottom left corner.
   * \param corner4 The position of the bottom right corner.
   * \param segmentCount1 Number of horizontal segments in the cloth.
   * \param segmentCount2 Number of vertical segments in the cloth.
   * \param withDiagonals Whether there must be diagonal segments in the cloth
   * or not. Diagonal segments will make the cloth more rigid.
   * \remark You must call SetSoftBodyWorld() prior to this.
   */
  virtual iBulletSoftBody* CreateCloth (csVector3 corner1, csVector3 corner2,
					csVector3 corner3, csVector3 corner4,
					uint segmentCount1, uint segmentCount2,
					bool withDiagonals = false) = 0;

  /**
   * Create a volumetric soft body from a genmesh.
   * \param genmeshFactory The genmesh factory to use.
   * \param bodyTransform The initial transform of the soft body.
   * \remark You must call SetSoftBodyWorld() prior to this.
   */
  virtual iBulletSoftBody* CreateSoftBody (iGeneralFactoryState* genmeshFactory,
					   const csOrthoTransform& bodyTransform) = 0;

  /**
   * Create a custom volumetric soft body.
   * \param vertices The vertices of the soft body. The position is absolute.
   * \param vertexCount The count of vertices of the soft body.
   * \param triangles The faces of the soft body.
   * \param triangleCount The count of faces of the soft body.
   * \remark You must call SetSoftBodyWorld() prior to this.
   */
  virtual iBulletSoftBody* CreateSoftBody (csVector3* vertices, size_t vertexCount,
					   csTriangle* triangles, size_t triangleCount) = 0;

  /**
   * Remove the given soft body from this dynamic world and delete it.
   */
  virtual void RemoveSoftBody (iBulletSoftBody* body) = 0;

  /**
   * Create a pivot joint and add it to the simulation.
   */
  virtual csPtr<iBulletPivotJoint> CreatePivotJoint () = 0;

  /**
   * Remove the given pivot joint from the simulation.
   */
  virtual void RemovePivotJoint (iBulletPivotJoint* joint) = 0;

  /**
   * Save the current state of the dynamic world in a .bullet serialization file.
   * \return True if the operation succeeds, false otherwise.
   */
  virtual bool SaveBulletWorld (const char* filename) = 0;

  /**
   * Create a new terrain collider and add it to the simulation. All the heights of
   * the terrain must be kept between minimumHeight and maximumHeight, even when the
   * terrain is deformed. If these values are not provided then they will be computed
   * from the current state of the cell.
   * \param heightData The height map of the terrain
   * \param gridWidth The width of the height map data
   * \param gridWidth The height of the height map data
   * \param gridSize The size of the terrain, in world units
   * \param transform The position of the terrain
   * \param minimumHeight The minimum height that will ever be contained in the height map
   * \param maximumHeight The maximum height that will ever be contained in the height map
   * \param 
   */
  virtual iBulletTerrainCollider* AttachColliderTerrain (csLockedHeightData& heightData,
							 int gridWidth, int gridHeight,
							 csVector3 gridSize,
							 csOrthoTransform& transform,
							 float minimumHeight = 0,
							 float maximumHeight = 0) = 0;

  /**
   * Create a new terrain collider and add it to the simulation. All the heights of
   * the terrain must be kept between minimumHeight and maximumHeight, even when the
   * terrain is deformed. If these values are not provided then they will be computed
   * from the current state of the cell.
   * \param cell The terrain cell from which the collider will be created
   * \param minimumHeight The minimum height that will ever be contained in the height map
   * \param maximumHeight The maximum height that will ever be contained in the height map
   */
  virtual iBulletTerrainCollider* AttachColliderTerrain (iTerrainCell* cell,
							 float minimumHeight = 0,
							 float maximumHeight = 0) = 0;

  /**
   * Create a new terrain collider and add it to the simulation. This will create a
   * collider for each cell of the terrain.
   *
   * All the heights of
   * the terrain must be kept between minimumHeight and maximumHeight, even when the
   * terrain is deformed. If these values are not provided then they will be computed
   * from the current state of the cell.
   * \param cell The terrain from which the colliders will be created
   * \param minimumHeight The minimum height that will ever be contained in the height map
   * \param maximumHeight The maximum height that will ever be contained in the height map
   */
  virtual iBulletTerrainCollider* AttachColliderTerrain (iTerrainSystem* terrain,
							 float minimumHeight = 0,
							 float maximumHeight = 0) = 0;
  /**
   * Remove the given terrain collider from the simulation.
   */
  virtual void DestroyCollider (iBulletTerrainCollider* collider) = 0;
};

/**
 * A soft body is a physical body that can be deformed by the physical
 * simulation. It can be used to simulate eg ropes, clothes or any soft
 * volumetric object.
 *
 * A soft body does not have a positional transform by itself, but the
 * position of every vertex of the body can be queried through GetVertexPosition().
 *
 * A soft body can neither be static or kinematic, it is always dynamic.
 * \sa iRigidBody iBulletRigidBody iSoftBodyAnimationControl csBulletSoftBodyHelper
 */
struct iBulletSoftBody : public virtual iBase
{
  SCF_INTERFACE(iBulletSoftBody, 2, 0, 2);

  /**
   * Draw the debug informations of this soft body. This has to be called
   * at each frame, and will add 2D lines on top of the rendered scene.
   */
  virtual void DebugDraw (iView* rview) = 0;

  /**
   * Set the total mass of this body.
   */
  virtual void SetMass (float mass) = 0;

  /**
   * Return the total mass of this body.
   */
  virtual float GetMass () const = 0;

  /**
   * Return the count of vertices of this soft body.
   */
  virtual size_t GetVertexCount () const = 0;

  /**
   * Return the position in world coordinates of the given vertex.
   */
  virtual csVector3 GetVertexPosition (size_t index) const = 0;

  /**
   * Anchor the given vertex to its current position. This vertex will no more move.
   */
  virtual void AnchorVertex (size_t vertexIndex) = 0;

  /**
   * Anchor the given vertex to the given rigid body. The relative position of the
   * vertex and the body will remain constant.
   */
  virtual void AnchorVertex (size_t vertexIndex, iRigidBody* body) = 0;

  /**
   * Set the rigidity of this body. The value should be in the 0 to 1 range, with
   * 0 meaning soft and 1 meaning rigid.
   */
  virtual void SetRigidity (float rigidity) = 0;

  /**
   * Get the rigidity of this body.
   */
  virtual float GetRigidity () const = 0;

  /**
   * Set the linear velocity of the whole body.
   */
  virtual void SetLinearVelocity (csVector3 velocity) = 0;

  /**
   * Set the linear velocity of the given vertex of the body.
   */
  virtual void SetLinearVelocity (csVector3 velocity, size_t vertexIndex) = 0;

  /**
   * Get the linear velocity of the given vertex of the body.
   */
  virtual csVector3 GetLinearVelocity (size_t vertexIndex) const = 0;

  /**
   * Add a force to the whole body.
   */
  virtual void AddForce (csVector3 force) = 0;

  /**
   * Add a force at the given vertex of the body.
   */
  virtual void AddForce (csVector3 force, size_t vertexIndex) = 0;

  /**
   * Return the count of triangles of this soft body.
   */
  virtual size_t GetTriangleCount () const = 0;

  /**
   * Return the triangle with the given index.
   */
  virtual csTriangle GetTriangle (size_t index) const = 0;

  /**
   * Return the normal vector in world coordinates for the given vertex.
   */
  virtual csVector3 GetVertexNormal (size_t index) const = 0;
};

/**
 * General helper class for iBulletSoftBody.
 */
struct csBulletSoftBodyHelper
{
  /**
   * Create a genmesh from the given cloth soft body.
   * The genmesh will be double-sided, in order to have correct normals on both
   * sides of the cloth (ie the vertices of the soft body will be duplicated for the
   * genmesh).
   * \warning Don't forget to use doubleSided = true in
   * iSoftBodyAnimationControl::SetSoftBody()
   */
  static csPtr<iMeshFactoryWrapper> CreateClothGenMeshFactory
  (iObjectRegistry* object_reg, const char* factoryName, iBulletSoftBody* cloth)
  {
    csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);

    // Create the cloth mesh factory.
    csRef<iMeshFactoryWrapper> clothFact = engine->CreateMeshFactory
      ("crystalspace.mesh.object.genmesh", factoryName);
    if (!clothFact)
      return 0;

    csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
      (clothFact->GetMeshObjectFactory ());

    // Create the vertices of the genmesh
    size_t vertexCount = cloth->GetVertexCount ();
    gmstate->SetVertexCount (vertexCount * 2);
    csVector3* vertices = gmstate->GetVertices ();
    for (size_t i = 0; i < vertexCount; i++)
    {
      vertices[i] = cloth->GetVertexPosition (i);
      vertices[i + vertexCount] = cloth->GetVertexPosition (i);
    }

    // Create the triangles of the genmesh
    gmstate->SetTriangleCount (cloth->GetTriangleCount () * 2);
    csTriangle* triangles = gmstate->GetTriangles ();
    for (size_t i = 0; i < cloth->GetTriangleCount (); i++)
    {
      csTriangle triangle = cloth->GetTriangle (i);
      triangles[i * 2] = triangle;
      triangles[i * 2 + 1] = csTriangle (triangle[2] + vertexCount,
					 triangle[1] + vertexCount,
					 triangle[0] + vertexCount);
    }

    gmstate->CalculateNormals ();

    // Set up the texels of the genmesh
    csVector2* texels = gmstate->GetTexels ();
    csVector3* normals = gmstate->GetNormals ();
    CS::Geometry::TextureMapper* mapper = new CS::Geometry::DensityTextureMapper (1.0f);
    for (size_t i = 0; i < vertexCount * 2; i++)
      texels[i] = mapper->Map (vertices[i], normals[i], i);

    gmstate->Invalidate ();

    return csPtr<iMeshFactoryWrapper> (clothFact);
  }
};

/**
 * The physical state of a rigid body.
 */
enum csBulletState
{
  CS_BULLET_STATE_STATIC = 0,     /*!< The body is static, ie this body won't move
				 anymore but dynamic objects will still collide with it. */
  CS_BULLET_STATE_DYNAMIC,        /*!< The body is dynamic, ie the motion of 
				  the body is controlled by the dynamic simulation. */
  CS_BULLET_STATE_KINEMATIC       /*!< The body is kinematic, ie the motion 
				  of the body is controlled by the animation system,
				  but it interacts with the dynamic simulation. */
};

/**
 * The Bullet implementation of iRigidBody also implements this
 * interface.
 * \sa iRigidBody iBulletSoftBody
 */
struct iBulletRigidBody : public virtual iBase
{
  SCF_INTERFACE(iBulletRigidBody, 1, 0, 1);

  /**
   * Set a body in the kinematic state, ie the motion of the body is
   * controlled by you, but it interacts with the dynamic simulation.
   * 
   * You may need to set a callback with SetKinematicCallback() to let
   * the dynamic system know how to update the transform of the body.
   * \sa SetDynamicState() iRigidBody::MakeStatic() iRigidBody::MakeDynamic()
   */
  virtual void MakeKinematic () = 0;

  /**
   * Return the current state of the body.
   */
  virtual csBulletState GetDynamicState () const = 0;

  /**
   * Set the current state of the body.
   * \sa iRigidBody::MakeStatic() iRigidBody::MakeDynamic() MakeKinematic()
   */
  virtual void SetDynamicState (csBulletState state) = 0;

  /**
   * Set the callback to be used to update the transform of the kinematic body.
   * If no callback are provided then the dynamic system will use a default one.
   */
  virtual void SetKinematicCallback (iBulletKinematicCallback* callback) = 0;

  /**
   * Get the callback used to update the transform of the kinematic body.
   */
  virtual iBulletKinematicCallback* GetKinematicCallback () = 0;

  /**
   * Set the linear dampener for this rigid body. The dampening correspond to
   * how much the movements of the objects will be reduced. It is a value
   * between 0 and 1, giving the ratio of speed that will be reduced
   * in one second. 0 means that the movement will not be reduced, while
   * 1 means that the object will not move.
   * The default value is 0.
   * \sa iDynamicSystem::SetLinearDampener()
   */
  virtual void SetLinearDampener (float d) = 0;

  /**
   * Get the linear dampener for this rigid body.
   */
  virtual float GetLinearDampener () const = 0;

  /**
   * Set the angular dampener for this rigid body. The dampening correspond to
   * how much the movements of the objects will be reduced. It is a value
   * between 0 and 1, giving the ratio of speed that will be reduced
   * in one second. 0 means that the movement will not be reduced, while
   * 1 means that the object will not move.
   * The default value is 0.
   * \sa iDynamicSystem::SetRollingDampener()
   */
  virtual void SetRollingDampener (float d) = 0;

  /**
   * Get the angular dampener for this rigid body.
   */
  virtual float GetRollingDampener () const = 0;
};

/**
 * A callback to be implemented when you are using kinematic bodies. If no
 * callback are provided then the dynamic system will use a default one which
 * will update the transform of the body from the position of the attached
 * mesh, body or camera (see iRigidBody::AttachMesh(),
 * iRigidBody::AttachLight(), iRigidBody::AttachCamera()).
 * \sa iBulletRigidBody::SetKinematicCallback()
 */
struct iBulletKinematicCallback : public virtual iBase
{
  SCF_INTERFACE (iBulletKinematicCallback, 1, 0, 0);

  /**
   * Update the new transform of the rigid body.
   */
  virtual void GetBodyTransform (iRigidBody* body,
				 csOrthoTransform& transform) const = 0;
};

/**
 * A joint to attach to a rigid body in order to manipulate it. It is contrained
 * in translation and has free rotation. You can move freely the position of the
 * joint, the body will keep attached to the joint.
 */
struct iBulletPivotJoint : public virtual iBase
{
  SCF_INTERFACE (iBulletPivotJoint, 1, 0, 0);

  /**
   * Attach a rigid body to the joint.
   * \param body The rigid body to attach to the joint.
   * \param position The initial position of the joint, in world coordinates.
   */
  virtual void Attach (iRigidBody* body, const csVector3& position) = 0;

  /**
   * Return the body attached to this joint, or 0 if there are none.
   */
  virtual iRigidBody* GetAttachedBody () const = 0;

  /**
   * Set the new position of the joint, in world coordinates.
   */
  virtual void SetPosition (const csVector3& position) = 0;

  /**
   * Get the current position of the joint, in world coordinates.
   */
  virtual csVector3 GetPosition () const = 0;
};

/**
 * A terrain collider for the dynamic simulation.
 */
struct iBulletTerrainCollider : public virtual iBase
{
  SCF_INTERFACE (iBulletTerrainCollider, 1, 0, 0);

};

#endif // __CS_IVARIA_BULLET_H__

