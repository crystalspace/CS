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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_SOFTBODYANIM_H__
#define __CS_SOFTBODYANIM_H__

#include "csutil/scf_implementation.h"
#include "csutil/dirtyaccessarray.h"
#include "iutil/comp.h"
#include "csutil/weakref.h"
#include "ivaria/softanim.h"
#include "iengine/movable.h"

CS_PLUGIN_NAMESPACE_BEGIN(SoftAnim)
{

class SoftBodyControlType : public scfImplementation2<SoftBodyControlType,
    iSoftBodyAnimationControlType, iComponent>
{
  public:
    CS_LEAKGUARD_DECLARE(SoftBodyControlType);

    SoftBodyControlType (iBase* parent);

    //-- iGenMeshAnimationControlType
    virtual csPtr<iGenMeshAnimationControlFactory> CreateAnimationControlFactory ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
};

class SoftBodyControlFactory : public scfImplementation2<SoftBodyControlFactory, 
    scfFakeInterface<iGenMeshAnimationControlFactory>,
    iSoftBodyAnimationControlFactory>
{
  public:
    CS_LEAKGUARD_DECLARE(iSoftBodyAnimationControlFactory);

    SoftBodyControlFactory ();

    //-- iGenMeshAnimationControlFactory
    virtual csPtr<iGenMeshAnimationControl> CreateAnimationControl (iMeshObject* mesh);
    virtual const char* Load (iDocumentNode* node);
    virtual const char* Save (iDocumentNode* parent);
};

class SoftBodyControl : public scfImplementation2<SoftBodyControl, 
    scfFakeInterface<iGenMeshAnimationControl>, iSoftBodyAnimationControl>
{
  public:
    CS_LEAKGUARD_DECLARE(iSoftBodyAnimationControl);

    SoftBodyControl (iMeshObject* mesh);

    //-- iSoftBodyAnimationControl
    virtual void SetSoftBody (CS::Physics::Bullet::iSoftBody* body, bool doubleSided = false);
    virtual CS::Physics::Bullet::iSoftBody* GetSoftBody ();

    //-- iGenMeshAnimationControl
    virtual bool AnimatesColors () const;
    virtual bool AnimatesNormals () const;
    virtual bool AnimatesTexels () const;
    virtual bool AnimatesVertices () const;
    virtual void Update (csTicks current, int num_verts, uint32 version_id);
    virtual const csColor4* UpdateColors (csTicks current, const csColor4* colors,
					  int num_colors, uint32 version_id);
    virtual const csVector3* UpdateNormals (csTicks current, const csVector3* normals,
					    int num_normals, uint32 version_id);
    virtual const csVector2* UpdateTexels (csTicks current, const csVector2* texels,
					   int num_texels, uint32 version_id);
    virtual const csVector3* UpdateVertices (csTicks current, const csVector3* verts,
					     int num_verts, uint32 version_id);

  private:
    csWeakRef<iMeshObject> mesh;
    csRef<CS::Physics::Bullet::iSoftBody> softBody;
    bool doubleSided;
    csDirtyAccessArray<csVector3> vertices;
    csDirtyAccessArray<csVector3> normals;
    csTicks lastTicks;
    csVector3 meshPosition;
};

}
CS_PLUGIN_NAMESPACE_END(SoftAnim)

#endif //__CS_SOFTBODYANIM_H__
