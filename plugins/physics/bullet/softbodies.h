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

#ifndef __CS_BULLET_SOFTBODIES_H__
#define __CS_BULLET_SOFTBODIES_H__

#include "bullet.h"
#include "common.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

class csBulletSoftBody : public scfImplementation1<csBulletSoftBody,
    iBulletSoftBody>
{
  friend class csBulletDynamicsSystem;

 public:
  csBulletSoftBody (csBulletDynamicsSystem* dynSys, btSoftBody* body);
  ~csBulletSoftBody ();

  //-- iBulletBody
  virtual CS::Physics::Bullet::BodyType GetType ()
  { return bodyType; }

  //-- iBulletSoftBody
  virtual void DebugDraw (iView* rview);
  virtual void SetMass (float mass);
  virtual float GetMass () const;
  virtual size_t GetVertexCount () const;
  virtual csVector3 GetVertexPosition (size_t index) const;
  virtual csVector3 GetVertexNormal (size_t index) const;
  virtual void AnchorVertex (size_t vertexIndex);
  virtual void AnchorVertex (size_t vertexIndex, iRigidBody* body);
  virtual void SetRigidity (float rigidity);
  virtual float GetRigidity () const;
  virtual void SetLinearVelocity (csVector3 velocity);
  virtual void SetLinearVelocity (csVector3 velocity, size_t vertexIndex);
  virtual csVector3 GetLinearVelocity (size_t vertexIndex) const;
  virtual void AddForce (csVector3 force);
  virtual void AddForce (csVector3 force, size_t vertexIndex);
  virtual size_t GetTriangleCount () const;
  virtual csTriangle GetTriangle (size_t index) const;

 private:
  CS::Physics::Bullet::BodyType bodyType;
  csBulletDynamicsSystem* dynSys;
  btSoftBody* body;
};

}
CS_PLUGIN_NAMESPACE_END(Bullet)

#endif //__CS_BULLET_SOFTBODIES_H__
