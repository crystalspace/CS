/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __CS_PHYSLDR_H__
#define __CS_PHYSLDR_H__

#include "imap/reader.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"
#include "csutil/scf_implementation.h"

struct iObjectRegistry;
struct iReporter;
struct iSyntaxService;
struct iEngine;
struct iDynamics;
struct iDynamicSystem;
struct iRigidBody;
struct iJoint;

class csPhysicsLoader :
  public scfImplementation2<csPhysicsLoader, iLoaderPlugin, iComponent>
{
public:
  csPhysicsLoader (iBase*);
  virtual ~csPhysicsLoader ();

  bool Initialize (iObjectRegistry*);

  /// Parse the physics node and setup the environment
  virtual csPtr<iBase> Parse (iDocumentNode *node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }

  /// Parse the system specific sub section
  virtual bool ParseSystem (iDocumentNode *node, iDynamicSystem* system, iLoaderContext* ldr_context);
  /// Parse the body specific sub section
  virtual bool ParseBody (iDocumentNode *node, iRigidBody* body, iLoaderContext* ldr_context);
  /// Parse the collider specific sub section
  virtual bool ParseCollider (iDocumentNode *node, iRigidBody* body, iLoaderContext* ldr_context);
  /// Parse an anonymous mesh collider in the system
  virtual bool ParseSystemColliderMesh (iDocumentNode *node,
  	iDynamicSystem* system, bool convex, iLoaderContext* ldr_context);
  /// Parse an anonymous sphere collider in the system
  virtual bool ParseSystemColliderSphere (iDocumentNode *node,
  	iDynamicSystem* system);
  /// Parse an anonymous cylinder collider in the system
  virtual bool ParseSystemColliderCylinder (iDocumentNode *node,
  	iDynamicSystem* system);
  /// Parse an anonymous capsule collider in the system
  virtual bool ParseSystemColliderCapsule (iDocumentNode *node,
  	iDynamicSystem* system);
  /// Parse an anonymous box collider in the system
  virtual bool ParseSystemColliderBox (iDocumentNode *node,
  	iDynamicSystem* system);
  /// Parse an anonymous plane collider in the system
  virtual bool ParseSystemColliderPlane (iDocumentNode *node,
  	iDynamicSystem* system);
  /// Parse the joint specific sub section
  virtual bool ParseJoint (iDocumentNode *node, iJoint* joint,
  	iDynamicSystem* system);
  /// Parse a transform
  virtual bool ParseTransform (iDocumentNode *node, csOrthoTransform &t);
  /// Parse a constraint definition
  virtual bool ParseConstraint (iDocumentNode *node,
  	bool &, bool &, bool &, csVector3 &, csVector3 &);

private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csRef<iEngine> engine;
  csStringHash xmltokens;
};

#endif // __CS_PHYSLDR_H__
