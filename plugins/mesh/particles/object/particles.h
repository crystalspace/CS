/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __CS_MESH_PARTICLES_H__
#define __CS_MESH_PARTICLES_H__

#include "cstool/objmodel.h"
#include "cstool/rendermeshholder.h"
#include "csutil/scf_implementation.h"
#include "csutil/flags.h"
#include "csutil/radixsort.h"

#include "imesh/object.h"
#include "imesh/particles.h"
#include "iutil/comp.h"
#include "ivideo/rndbuf.h"

CS_PLUGIN_NAMESPACE_BEGIN(Particles)
{
  struct iVertexSetup;

  /**
  * Particle object type
  */
  class ParticlesMeshObjectType : public scfImplementation2<ParticlesMeshObjectType,
                                                            iMeshObjectType,
                                                            iComponent>
  {
  public:
    ParticlesMeshObjectType (iBase* parent);
    virtual ~ParticlesMeshObjectType ();

    /// Initialize
    virtual bool Initialize (iObjectRegistry* object_reg);

    /// Create a new factory
    virtual csPtr<iMeshObjectFactory> NewFactory ();
  };


  /**
  * Particle object factory
  */
  class ParticlesMeshFactory : public scfImplementation3<ParticlesMeshFactory,
                                                         iMeshObjectFactory,
                                                         iParticleSystemFactory,
                                                         scfFakeInterface<iParticleSystemBase> >
  {
  public:
    /// Constructor
    ParticlesMeshFactory (ParticlesMeshObjectType* objectType);
    virtual ~ParticlesMeshFactory();

    //-- Local methods

    /// Get the object type
    inline ParticlesMeshObjectType* GetObjectType () const
    {
      return objectType;
    }


    //-- iMeshObjectFactory
    virtual csFlags& GetFlags ()
    {
      return flags;
    }

    /// Create an instance of iMeshObject.
    virtual csPtr<iMeshObject> NewInstance ();

    virtual csPtr<iMeshObjectFactory> Clone ();

    virtual void HardTransform (const csReversibleTransform& t) // Not supported
    {}

    virtual bool SupportsHardTransform () const
    {
      return false;
    }

    virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* logparent)
    {
      factoryWrapper = logparent;
    }

    virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const
    {
      return factoryWrapper;
    }

    virtual iMeshObjectType* GetMeshObjectType () const
    {
      return objectType;
    }

    virtual iObjectModel* GetObjectModel ()
    {
      return 0;
    }

    virtual bool SetMaterialWrapper (iMaterialWrapper* material) 
    {
      materialWrapper = material;
      return true;
    }

    virtual iMaterialWrapper* GetMaterialWrapper () const 
    {
      return materialWrapper;
    }

    virtual void SetMixMode (uint mode)
    {
      mixMode = mode;
    }
   
    virtual uint GetMixMode () const
    {
      return mixMode;
    }

    //-- iParticleSystemFactory
    virtual void SetDeepCreation (bool deep)
    {
      deepCreation = deep;
    }

    virtual bool GetDeepCreation () const
    {
      return deepCreation;
    }

    //-- iParticleSystemBase
    virtual void SetParticleRenderOrientation (csParticleRenderOrientation o)
    {
      particleOrientation = o;
    }

    virtual csParticleRenderOrientation GetParticleRenderOrientation () const
    {
      return particleOrientation;
    }

    virtual void SetRotationMode (csParticleRotationMode mode)
    {
      rotationMode = mode;
    }

    virtual csParticleRotationMode GetRotationMode () const
    {
      return rotationMode;
    }

    virtual void SetSortMode (csParticleSortMode mode)
    {
      sortMode = mode;
    }

    virtual csParticleSortMode GetSortMode () const
    {
      return sortMode;
    }

    virtual void SetIntegrationMode (csParticleIntegrationMode mode)
    {
      integrationMode = mode;
    }

    virtual csParticleIntegrationMode GetIntegrationMode () 
    {
      return integrationMode;
    }

    virtual void SetCommonDirection (const csVector3& direction)
    {
      commonDirection = direction;
    }

    virtual const csVector3& GetCommonDirection () const
    {
      return commonDirection;
    }

    virtual void SetLocalMode (bool local)
    {
      localMode = local;
    }

    virtual bool GetLocalMode () const
    {
      return localMode;
    }

    virtual void SetUseIndividualSize (bool individual)
    {
      individualSize = individual;
    }

    virtual bool GetUseIndividualSize () const
    {
      return individualSize;
    }

    virtual void SetParticleSize (const csVector2& size)
    {
      particleSize = size;
    }

    virtual const csVector2& GetParticleSize () const
    {
      return particleSize;
    }

    virtual void SetMinBoundingBox (const csBox3& box)
    {
      minBB = box;
    }

    virtual const csBox3& GetMinBoundingBox () const
    {
      return minBB;
    }

    virtual void AddEmitter (iParticleEmitter* emitter)
    {
      emitters.PushSmart (emitter);
    }

    virtual iParticleEmitter* GetEmitter (size_t index) const
    {
      return emitters[index];
    }

    virtual void RemoveEmitter (size_t index)
    {
      emitters.DeleteIndex (index);
    }

    virtual size_t GetEmitterCount () const
    {
      return emitters.GetSize ();
    }

    virtual void AddEffector (iParticleEffector* effector)
    {
      effectors.PushSmart (effector);
    }

    virtual iParticleEffector* GetEffector (size_t index) const
    {
      return effectors[index];
    }

    virtual void RemoveEffector (size_t index)
    {
      effectors.DeleteIndex (index);
    }

    virtual size_t GetEffectorCount () const
    {
      return effectors.GetSize ();
    }

  private:
    ParticlesMeshObjectType* objectType;

    //-- Needed only for iMeshObjectFactory
    csFlags flags;
    iMeshFactoryWrapper* factoryWrapper;
    iMaterialWrapper *materialWrapper;
    uint mixMode;

    //-- iParticleSystemFactory properties
    bool deepCreation;

    //-- iParticleSystemBase
    csParticleRenderOrientation particleOrientation;
    csParticleRotationMode rotationMode;
    csParticleSortMode sortMode;
    csParticleIntegrationMode integrationMode;
    csVector3 commonDirection;
    bool localMode;
    bool individualSize;
    csVector2 particleSize;
    csBox3 minBB;

    csRefArray<iParticleEmitter> emitters;
    csRefArray<iParticleEffector> effectors;
  };

  /**
  * Particle mesh object
  */
  class ParticlesMeshObject : public scfImplementationExt4<ParticlesMeshObject,
                                                           csObjectModel,
                                                           iMeshObject,
                                                           iParticleSystem,
                                                           scfFakeInterface<iParticleSystemBase>,
                                                           iRenderBufferAccessor>
  {
  public:
    /// Constructor
    ParticlesMeshObject (ParticlesMeshFactory* factory);
    virtual ~ParticlesMeshObject (); 


    //-- Local
    /// Reserve some size in the particle buffer
    void ReserveNewParticles (size_t numNew);

    /// Setup index buffer
    void SetupIndexBuffer (csRenderBufferHolder* bufferHolder,
      const csReversibleTransform& o2c);

    /// Setup vertex buffer
    void SetupVertexBuffer (csRenderBufferHolder* bufferHolder,
      const csReversibleTransform& o2c);

    /// Update the TC buffer
    void UpdateTexCoordBuffer ();
    /// Update the color buffer
    void UpdateColorBuffer ();

    /// Invalidate vertex setup
    void InvalidateVertexSetup ();


    //-- iMeshObject
    virtual iMeshObjectFactory* GetFactory () const
    {
      return factory;
    }

    virtual csFlags& GetFlags ()
    {
      return flags;
    }

    virtual csPtr<iMeshObject> Clone ();

    virtual csRenderMesh** GetRenderMeshes (int& num, iRenderView* rview, 
      iMovable* movable, uint32 frustum_mask);

    virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
    {
      visCallback = cb;
    }

    virtual iMeshObjectDrawCallback* GetVisibleCallback () const
    {
      return visCallback;
    }

    virtual void NextFrame (csTicks current_time,const csVector3& pos, 
      uint currentFrame);

    virtual void HardTransform (const csReversibleTransform& t) //Not supported
    {}

    virtual bool SupportsHardTransform () const
    {
      return false;
    }

    virtual bool HitBeamOutline (const csVector3& start,
      const csVector3& end, csVector3& isect, float* pr)
    {
      return false;
    }

    virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
      csVector3& isect, float* pr, int* polygon_idx = 0,
      iMaterialWrapper** material = 0) 
    {
      return false;
    }

    virtual void SetMeshWrapper (iMeshWrapper* logparent)
    {
      meshWrapper = logparent;
    }

    virtual iMeshWrapper* GetMeshWrapper () const 
    {
      return meshWrapper;
    }

    virtual iObjectModel* GetObjectModel () 
    {
      return this;
    }

    virtual bool SetColor (const csColor& color)
    {
      return false;
    }

    virtual bool GetColor (csColor& color) const
    {
      return false;
    }

    virtual bool SetMaterialWrapper (iMaterialWrapper* material)
    {
      materialWrapper = material;
      return true;
    }

    virtual iMaterialWrapper* GetMaterialWrapper () const
    {
      return materialWrapper;
    }

    virtual void SetMixMode (uint mode)
    {
      mixMode = mode;
    }

    virtual uint GetMixMode () const
    {
      return mixMode;
    }

    virtual void InvalidateMaterialHandles ()
    {}

    virtual void PositionChild (iMeshObject* child,csTicks current_time)
    {}


    //-- iObjectModel
    virtual const csBox3& GetObjectBoundingBox ();

    virtual void GetObjectBoundingBox (csBox3& bbox)
    {
      bbox = GetObjectBoundingBox ();
    }

    virtual void SetObjectBoundingBox (const csBox3& bbox);

    virtual void GetRadius (float& radius, csVector3& center);


    //-- iParticleSystem
    virtual size_t GetParticleCount () const
    {
      return particleBuffer.particleCount;
    }

    virtual csParticle* GetParticle (size_t index)
    {
      return particleBuffer.particleData+index;
    }

    virtual csParticleAux* GetParticleAux (size_t index)
    {
      return particleBuffer.particleAuxData+index;
    }

    virtual csParticleBuffer* LockForExternalControl (size_t maxParticles);

    //-- iParticleSystemBase
    virtual void SetParticleRenderOrientation (csParticleRenderOrientation o)
    {
      particleOrientation = o;
    }

    virtual csParticleRenderOrientation GetParticleRenderOrientation () const
    {
      return particleOrientation;
    }

    virtual void SetRotationMode (csParticleRotationMode mode)
    {
      rotationMode = mode;
    }

    virtual csParticleRotationMode GetRotationMode () const
    {
      return rotationMode;
    }

    virtual void SetSortMode (csParticleSortMode mode)
    {
      sortMode = mode;
    }

    virtual csParticleSortMode GetSortMode () const
    {
      return sortMode;
    }

    virtual void SetIntegrationMode (csParticleIntegrationMode mode)
    {
      integrationMode = mode;
    }

    virtual csParticleIntegrationMode GetIntegrationMode () 
    {
      return integrationMode;
    }

    virtual void SetCommonDirection (const csVector3& direction)
    {
      commonDirection = direction;
    }

    virtual const csVector3& GetCommonDirection () const
    {
      return commonDirection;
    }


    virtual void SetLocalMode (bool local)
    {
      localMode = local;
    }

    virtual bool GetLocalMode () const
    {
      return localMode;
    }

    virtual void SetUseIndividualSize (bool individual)
    {
      individualSize = individual;
      InvalidateVertexSetup ();
    }

    virtual bool GetUseIndividualSize () const
    {
      return individualSize;
    }

    virtual void SetParticleSize (const csVector2& size)
    {
      particleSize = size;
    }

    virtual const csVector2& GetParticleSize () const
    {
      return particleSize;
    }

    /// Set the smallest bounding box particle system should use
    virtual void SetMinBoundingBox (const csBox3& box) 
    {
      minBB = box;
    }

    /// Get the smallest bounding box particle system should use
    virtual const csBox3& GetMinBoundingBox () const
    {
      return minBB;
    }

    virtual void AddEmitter (iParticleEmitter* emitter)
    {
      emitters.PushSmart (emitter);
    }

    virtual iParticleEmitter* GetEmitter (size_t index) const
    {
      return emitters[index];
    }

    virtual void RemoveEmitter (size_t index)
    {
      emitters.DeleteIndex (index);
    }

    virtual size_t GetEmitterCount () const
    {
      return emitters.GetSize ();
    }

    virtual void AddEffector (iParticleEffector* effector)
    {
      effectors.PushSmart (effector);
    }

    virtual iParticleEffector* GetEffector (size_t index) const
    {
      return effectors[index];
    }

    virtual void RemoveEffector (size_t index)
    {
      effectors.DeleteIndex (index);
    }

    virtual size_t GetEffectorCount () const
    {
      return effectors.GetSize ();
    }

    //-- iRenderBufferAccessor
    virtual void PreGetBuffer (csRenderBufferHolder* holder, 
      csRenderBufferName buffer);

  private:
    friend class ParticlesMeshFactory;
    ParticlesMeshFactory* factory;

    iVertexSetup* vertexSetup; //Helper object

    //-- iMeshObject
    iMeshWrapper* meshWrapper;
    csFlags flags;
    csRef<iMeshObjectDrawCallback> visCallback;
    csRef<iMaterialWrapper> materialWrapper;
    uint mixMode;

    csTicks lastUpdateTime;
    csTicks currentDt;
    uint lastFrameNumber;
    float totalParticleTime;
    
    csRenderMeshHolder rmHolder;
    csRef<iRenderBuffer> unsortedIndexBuffer;

    //-- iObjectModel
    float radius, minRadius;
    csBox3 objectBB;

    //-- iParticleSystem
    csParticleBuffer particleBuffer;
    uint8* rawBuffer;
    size_t particleAllocatedSize;
    bool externalControl;

    //-- iParticleSystemBase
    csParticleRenderOrientation particleOrientation;
    csParticleRotationMode rotationMode;
    csParticleIntegrationMode integrationMode;
    csParticleSortMode sortMode;
    csVector3 commonDirection;
    bool localMode;
    bool individualSize;
    csVector2 particleSize;
    csBox3 minBB;

    csRefArray<iParticleEmitter> emitters;
    csRefArray<iParticleEffector> effectors;

    csRadixSorter indexSorter;

    //-- iRenderBufferAccessor
    csRef<iRenderBuffer> tcBuffer;
    csRef<iRenderBuffer> colorBuffer;
  };
}
CS_PLUGIN_NAMESPACE_END(Particles)

#endif
