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

#include "cssysdef.h"

#include "csutil/scf.h"

#include "animesh.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Animesh)
{

  SCF_IMPLEMENT_FACTORY(AnimeshObjectType);

  AnimeshObjectType::AnimeshObjectType (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  csPtr<iMeshObjectFactory> AnimeshObjectType::NewFactory ()
  {
    return 0;
  }

  bool AnimeshObjectType::Initialize (iObjectRegistry*)
  {
    return true;
  }




  AnimeshObjectFactory::AnimeshObjectFactory ()
    : scfImplementationType (this)
  {
  }

  iAnimatedMeshFactorySubMesh* AnimeshObjectFactory::CreateSubMesh (iRenderBuffer* indices)
  {
    return 0;
  }

  iAnimatedMeshFactorySubMesh* AnimeshObjectFactory::GetSubMesh (size_t index) const
  {
    return 0;
  }

  size_t AnimeshObjectFactory::GetSubMeshCount () const
  {
    return 0;
  }

  void AnimeshObjectFactory::DeleteSubMesh (iAnimatedMeshFactorySubMesh* mesh)
  {
  }

  void AnimeshObjectFactory::SetVertexCount (uint count)
  {
  }

  uint AnimeshObjectFactory::GetVertexCount () const
  {
    return 0;
  }

  iRenderBuffer* AnimeshObjectFactory::GetVertices ()
  {
    return 0;
  }

  iRenderBuffer* AnimeshObjectFactory::GetTexCoords ()
  {
    return 0;
  }

  iRenderBuffer* AnimeshObjectFactory::GetNormals ()
  {
    return 0;
  }

  iRenderBuffer* AnimeshObjectFactory::GetTangents ()
  {
    return 0;
  }

  iRenderBuffer* AnimeshObjectFactory::GetBinormals ()
  {
    return 0;
  }

  iRenderBuffer* AnimeshObjectFactory::GetColors ()
  {
    return 0;
  }

  void AnimeshObjectFactory::Invalidate ()
  {   
  }

  void AnimeshObjectFactory::SetBoneInfluencesPerVertex (uint num)
  {    
  }

  uint AnimeshObjectFactory::GetBoneInfluencesPerVertex () const
  {
    return 0;
  }

  csAnimatedMeshBoneInfluence* AnimeshObjectFactory::GetBoneInfluences ()
  {
    return 0;
  }

  iAnimatedMeshMorphTarget* AnimeshObjectFactory::CreateMorphTarget ()
  {
    return 0;
  }

  iAnimatedMeshMorphTarget* AnimeshObjectFactory::GetMorphTarget (uint target)
  {
    return 0;
  }

  uint AnimeshObjectFactory::GetMorphTargetCount () const
  {
    return 0;
  }

  void AnimeshObjectFactory::ClearMorphTargets ()
  {    
  }

  csFlags& AnimeshObjectFactory::GetFlags ()
  {
    return factoryFlags;
  }

  csPtr<iMeshObject> AnimeshObjectFactory::NewInstance ()
  {
    return 0;
  }

  csPtr<iMeshObjectFactory> AnimeshObjectFactory::Clone ()
  {
    return 0;
  }

  void AnimeshObjectFactory::HardTransform (const csReversibleTransform& t)
  {    
  }

  bool AnimeshObjectFactory::SupportsHardTransform () const
  {
    return 0;
  }

  void AnimeshObjectFactory::SetMeshFactoryWrapper (iMeshFactoryWrapper* logparent)
  {    
  }

  iMeshFactoryWrapper* AnimeshObjectFactory::GetMeshFactoryWrapper () const
  {
    return 0;
  }

  iMeshObjectType* AnimeshObjectFactory::GetMeshObjectType () const
  {
    return 0;
  }

  iObjectModel* AnimeshObjectFactory::GetObjectModel ()
  {
    return 0;
  }

  bool AnimeshObjectFactory::SetMaterialWrapper (iMaterialWrapper* material)
  {
    return false;
  }

  iMaterialWrapper* AnimeshObjectFactory::GetMaterialWrapper () const
  {
    return 0;
  }

  void AnimeshObjectFactory::SetMixMode (uint mode)
  {
  }
  
  uint AnimeshObjectFactory::GetMixMode () const
  {
    return 0;
  }




  AnimeshObject::AnimeshObject ()
    : scfImplementationType (this)
  {
  }

  void AnimeshObject::SetSkeleton (iSkeleton2* skeleton)
  {
  }

  iAnimatedMeshSubMesh* AnimeshObject::GetSubMesh (size_t index) const
  {
    return 0;
  }

  size_t AnimeshObject::GetSubMeshCount () const
  {
    return 0;
  }

  void AnimeshObject::SetMorphTargetWeight (uint target, float weight)
  {
  }

  float AnimeshObject::GetMorphTargetWeight (uint target) const
  {
    return 0;
  }

  iMeshObjectFactory* AnimeshObject::GetFactory () const
  {
    return 0;
  }

  csFlags& AnimeshObject::GetFlags ()
  {
    return meshObjectFlags;
  }

  csPtr<iMeshObject> AnimeshObject::Clone ()
  {
    return 0;
  }

  CS::Graphics::RenderMesh** AnimeshObject::GetRenderMeshes (int& num, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask)
  {
    return 0;
  }

  void AnimeshObject::SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
  }

  iMeshObjectDrawCallback* AnimeshObject::GetVisibleCallback () const
  {
    return 0;
  }

  void AnimeshObject::NextFrame (csTicks current_time,const csVector3& pos,
    uint currentFrame)
  {    
  }

  void AnimeshObject::HardTransform (const csReversibleTransform& t)
  {
  }

  bool AnimeshObject::SupportsHardTransform () const
  {
    return false;
  }

  bool AnimeshObject::HitBeamOutline (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr)
  {
    return false;
  }

  bool AnimeshObject::HitBeamObject (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr, int* polygon_idx,
    iMaterialWrapper** material)
  {
    return false;
  }

  void AnimeshObject::SetMeshWrapper (iMeshWrapper* logparent)
  {
  }

  iMeshWrapper* AnimeshObject::GetMeshWrapper () const
  {
    return 0;
  }

  iObjectModel* AnimeshObject::GetObjectModel ()
  {
    return 0;
  }

  bool AnimeshObject::SetColor (const csColor& color)
  {
    return false;
  }

  bool AnimeshObject::GetColor (csColor& color) const
  {
    return false;
  }

  bool AnimeshObject::SetMaterialWrapper (iMaterialWrapper* material)
  {
    return false;
  }

  iMaterialWrapper* AnimeshObject::GetMaterialWrapper () const
  {
    return 0;
  }

  void AnimeshObject::SetMixMode (uint mode)
  {
  }

  uint AnimeshObject::GetMixMode () const
  {
    return 0;
  }

  void AnimeshObject::InvalidateMaterialHandles ()
  {
  }

  void AnimeshObject::PositionChild (iMeshObject* child,csTicks current_time)
  {
  }

  void AnimeshObject::BuildDecal(const csVector3* pos, float decalRadius,
    iDecalBuilder* decalBuilder)
  {
  }


}
CS_PLUGIN_NAMESPACE_END(Animesh)

