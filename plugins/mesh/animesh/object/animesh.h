/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#ifndef __CS_ANIMESH_H__
#define __CS_ANIMESH_H__

#include "csutil/flags.h"
#include "csutil/scf_implementation.h"
#include "imesh/animesh.h"
#include "imesh/object.h"
#include "iutil/comp.h"


CS_PLUGIN_NAMESPACE_BEGIN(Animesh)
{

  class AnimeshObjectType : 
    public scfImplementation2<AnimeshObjectType, 
                              iMeshObjectType, 
                              iComponent>
  {
  public:
    AnimeshObjectType (iBase* parent);

    //-- iMeshObjectType
    virtual csPtr<iMeshObjectFactory> NewFactory ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);
  };


  class AnimeshObjectFactory :
    public scfImplementation2<AnimeshObjectFactory,
                              iAnimatedMeshFactory,
                              iMeshObjectFactory>
  {
  public:
    AnimeshObjectFactory ();

    //-- iAnimatedMeshFactory
    virtual iAnimatedMeshFactorySubMesh* CreateSubMesh (iRenderBuffer* indices);
    virtual iAnimatedMeshFactorySubMesh* GetSubMesh (size_t index) const;
    virtual size_t GetSubMeshCount () const;
    virtual void DeleteSubMesh (iAnimatedMeshFactorySubMesh* mesh);

    virtual void SetVertexCount (uint count);
    virtual uint GetVertexCount () const;

    virtual iRenderBuffer* GetVertices ();
    virtual iRenderBuffer* GetTexCoords ();
    virtual iRenderBuffer* GetNormals ();
    virtual iRenderBuffer* GetTangents ();
    virtual iRenderBuffer* GetBinormals ();
    virtual iRenderBuffer* GetColors ();
    virtual void Invalidate ();

    virtual void SetBoneInfluencesPerVertex (uint num);
    virtual uint GetBoneInfluencesPerVertex () const;
    virtual csAnimatedMeshBoneInfluence* GetBoneInfluences ();

    virtual iAnimatedMeshMorphTarget* CreateMorphTarget ();
    virtual iAnimatedMeshMorphTarget* GetMorphTarget (uint target);
    virtual uint GetMorphTargetCount () const;
    virtual void ClearMorphTargets ();

    //-- iMeshObjectFactory
    virtual csFlags& GetFlags ();

    virtual csPtr<iMeshObject> NewInstance ();

    virtual csPtr<iMeshObjectFactory> Clone ();

    virtual void HardTransform (const csReversibleTransform& t);
    virtual bool SupportsHardTransform () const;

    virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* logparent);
    virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const;

    virtual iMeshObjectType* GetMeshObjectType () const;

    virtual iObjectModel* GetObjectModel ();

    virtual bool SetMaterialWrapper (iMaterialWrapper* material);
    virtual iMaterialWrapper* GetMaterialWrapper () const;

    virtual void SetMixMode (uint mode);
    virtual uint GetMixMode () const;

  private:
    csFlags factoryFlags;
  };


  class AnimeshObject :
    public scfImplementation2<AnimeshObject,
                              iAnimatedMesh,
                              iMeshObject>
  {
  public:
    AnimeshObject ();

    //-- iAnimatedMesh
    virtual void SetSkeleton (iSkeleton2* skeleton);

    virtual iAnimatedMeshSubMesh* GetSubMesh (size_t index) const;
    virtual size_t GetSubMeshCount () const;

    virtual void SetMorphTargetWeight (uint target, float weight);
    virtual float GetMorphTargetWeight (uint target) const;

    //-- iMeshObject
    virtual iMeshObjectFactory* GetFactory () const;

    virtual csFlags& GetFlags ();

    virtual csPtr<iMeshObject> Clone ();

    virtual CS::Graphics::RenderMesh** GetRenderMeshes (int& num, iRenderView* rview, 
      iMovable* movable, uint32 frustum_mask);

    virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb);

    virtual iMeshObjectDrawCallback* GetVisibleCallback () const;

    virtual void NextFrame (csTicks current_time,const csVector3& pos,
      uint currentFrame);

    virtual void HardTransform (const csReversibleTransform& t);
    virtual bool SupportsHardTransform () const;

    virtual bool HitBeamOutline (const csVector3& start,
      const csVector3& end, csVector3& isect, float* pr);
    virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
      csVector3& isect, float* pr, int* polygon_idx,
      iMaterialWrapper** material);

    virtual void SetMeshWrapper (iMeshWrapper* logparent);
    virtual iMeshWrapper* GetMeshWrapper () const;

    virtual iObjectModel* GetObjectModel ();

    virtual bool SetColor (const csColor& color);
    virtual bool GetColor (csColor& color) const;

    virtual bool SetMaterialWrapper (iMaterialWrapper* material);
    virtual iMaterialWrapper* GetMaterialWrapper () const;

    virtual void SetMixMode (uint mode);
    virtual uint GetMixMode () const;

    virtual void InvalidateMaterialHandles ();

    virtual void PositionChild (iMeshObject* child,csTicks current_time);

    virtual void BuildDecal(const csVector3* pos, float decalRadius,
      iDecalBuilder* decalBuilder);

  private:

    csFlags meshObjectFlags;

  };

}
CS_PLUGIN_NAMESPACE_END(Animesh)


#endif
