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

#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlistwalker.h"
#include "imesh/skeleton2.h"
#include "imesh/skeleton2anim.h"

#include "animesh.h"


CS_PLUGIN_NAMESPACE_BEGIN(Animesh)
{
  typedef csVertexListWalker<float, csVector3> MorphTargetOffsetsWalker;
  
#include "csutil/custom_new_disable.h"

  void AnimeshObject::SkinVertices ()
  {
    if (!skeleton)
      return;

    // @@Better checks/handling...
    CS_ASSERT (skinnedVertices->GetElementCount () >= factory->vertexCount);
    
    // Setup some local data
    csVertexListWalker<float, csVector3> srcVerts (factory->vertexBuffer);
    csRenderBufferLock<csVector3> dstVerts (skinnedVertices);
    csSkeletalState2* skeletonState = lastSkeletonState;

    bool hasMorphing = false;
    for (size_t i = 0; i < morphTargetWeights.GetSize(); i++)
    {
      if (morphTargetWeights[i] != 0)
      {
	hasMorphing = true;
	break;
      }
    }
    size_t numMorphTargets = hasMorphing ? morphTargetWeights.GetSize() : 0;
    CS_ALLOC_STACK_ARRAY(uint8, morphWalkersRaw,
      numMorphTargets * sizeof (MorphTargetOffsetsWalker));
    MorphTargetOffsetsWalker* morphWalkers =
      (MorphTargetOffsetsWalker*)(void*)morphWalkersRaw;
    for (size_t m = 0; m < numMorphTargets; m++)
    {
      new (morphWalkers + m) MorphTargetOffsetsWalker (
	factory->morphTargets[m]->offsets);
    }

    csAnimatedMeshBoneInfluence* influence = factory->boneInfluences.GetArray ();

    for (size_t i = 0; i < factory->vertexCount; ++i)
    {
      // Accumulate data for the vertex
      int numInfluences = 0;

      csDualQuaternion dq (csQuaternion (0,0,0,0), csQuaternion (0,0,0,0)); 
      csQuaternion pivot;   

      for (size_t j = 0; j < 4; ++j, ++influence) // @@SOLVE 4
      {
        if (influence->influenceWeight > 0)
        {
          numInfluences++;

          csDualQuaternion inflQuat (
            skeletonState->GetQuaternion (influence->bone),
            skeletonState->GetVector (influence->bone));

          if (numInfluences == 1)
          {
            pivot = inflQuat.real;
          }
          else if (inflQuat.real.Dot (pivot) < 0.0f)
          {
            inflQuat *= -1.0f;
          }

          dq += inflQuat * influence->influenceWeight;
        }
      }

      csVector3 srcVert = *srcVerts;
      for (size_t m = 0; m < numMorphTargets; m++)
      {
	MorphTargetOffsetsWalker& walk = morphWalkers[m];
	srcVert += (*walk) * morphTargetWeights[m];
	++walk;
      }
      
      if (numInfluences == 0)
      {
        dstVerts[i] = srcVert;
      }
      else
      {
        dq = dq.Unit ();

        dstVerts[i] = dq.TransformPoint (srcVert);
      }  

      ++srcVerts;      
    }

    for (size_t m = 0; m < numMorphTargets; m++)
    {
      morphWalkers[m].~MorphTargetOffsetsWalker();
    }
  }

#include "csutil/custom_new_enable.h"


  void AnimeshObject::SkinNormals ()
  {
    if (!skeleton)
      return;

    // @@Better checks/handling...
    CS_ASSERT (skinnedNormals->GetElementCount () >= factory->vertexCount);

    // Setup some local data
    csVertexListWalker<float, csVector3> srcNormals (factory->normalBuffer);
    csRenderBufferLock<csVector3> dstNormals (skinnedNormals);
    csSkeletalState2* skeletonState = lastSkeletonState;    

    csAnimatedMeshBoneInfluence* influence = factory->boneInfluences.GetArray ();

    for (size_t i = 0; i < factory->vertexCount; ++i)
    {
      // Accumulate data for the vertex
      int numInfluences = 0;

      csDualQuaternion dq (csQuaternion (0,0,0,0), csQuaternion (0,0,0,0)); 
      csQuaternion pivot;   

      for (size_t j = 0; j < 4; ++j, ++influence) // @@SOLVE 4
      {
        if (influence->influenceWeight > 0)
        {
          numInfluences++;

          csDualQuaternion inflQuat (
            skeletonState->GetQuaternion (influence->bone),
            skeletonState->GetVector (influence->bone));

          if (numInfluences == 1)
          {
            pivot = inflQuat.real;
          }
          else if (inflQuat.real.Dot (pivot) < 0.0f)
          {
            inflQuat *= -1.0f;
          }

          dq += inflQuat * influence->influenceWeight;
        }
      }

      if (numInfluences == 0)
      {
        dstNormals[i] = *srcNormals;
      }
      else
      {
        dq = dq.Unit ();

        dstNormals[i] = dq.Transform (*srcNormals);
      }  

      ++srcNormals;      
    }
  }


  void AnimeshObject::SkinVerticesAndNormals ()
  {
    if (!skeleton)
      return;

    // @@Better checks/handling...
    CS_ASSERT (skinnedVertices->GetElementCount () >= factory->vertexCount);
    CS_ASSERT (skinnedNormals->GetElementCount () >= factory->vertexCount);


    // Setup some local data
    csVertexListWalker<float, csVector3> srcVerts (factory->vertexBuffer);
    csRenderBufferLock<csVector3> dstVerts (skinnedVertices);
    csVertexListWalker<float, csVector3> srcNormals (factory->normalBuffer);
    csRenderBufferLock<csVector3> dstNormals (skinnedNormals);

    csSkeletalState2* skeletonState = lastSkeletonState;    

    csAnimatedMeshBoneInfluence* influence = factory->boneInfluences.GetArray ();

    for (size_t i = 0; i < factory->vertexCount; ++i)
    {
      // Accumulate data for the vertex
      int numInfluences = 0;

      csDualQuaternion dq (csQuaternion (0,0,0,0), csQuaternion (0,0,0,0)); 
      csQuaternion pivot;   

      for (size_t j = 0; j < 4; ++j, ++influence) // @@SOLVE 4
      {
        if (influence->influenceWeight > 0)
        {
          numInfluences++;

          csDualQuaternion inflQuat (
            skeletonState->GetQuaternion (influence->bone),
            skeletonState->GetVector (influence->bone));

          if (numInfluences == 1)
          {
            pivot = inflQuat.real;
          }
          else if (inflQuat.real.Dot (pivot) < 0.0f)
          {
            inflQuat *= -1.0f;
          }

          dq += inflQuat * influence->influenceWeight;
        }
      }

      if (numInfluences == 0)
      {
        dstVerts[i] = *srcVerts;
        dstNormals[i] = *srcNormals;
      }
      else
      {
        dq = dq.Unit ();

        dstVerts[i] = dq.TransformPoint (*srcVerts);        
        dstNormals[i] = dq.Transform (*srcNormals);
      }  

      ++srcVerts;
      ++srcNormals;
    }
  }

  void AnimeshObject::SkinTangentAndBinormal ()
  {
    if (!skeleton)
      return;

    // @@Better checks/handling...
    CS_ASSERT (skinnedTangents->GetElementCount () >= factory->vertexCount);
    CS_ASSERT (skinnedBinormals->GetElementCount () >= factory->vertexCount);


    // Setup some local data
    csVertexListWalker<float, csVector3> srcTangents (factory->tangentBuffer);
    csRenderBufferLock<csVector3> dstTangents (skinnedTangents);
    csVertexListWalker<float, csVector3> srcBinormals (factory->binormalBuffer);
    csRenderBufferLock<csVector3> dstBinormals (skinnedBinormals);

    csSkeletalState2* skeletonState = lastSkeletonState;    

    csAnimatedMeshBoneInfluence* influence = factory->boneInfluences.GetArray ();

    for (size_t i = 0; i < factory->vertexCount; ++i)
    {
      // Accumulate data for the vertex
      int numInfluences = 0;

      csDualQuaternion dq (csQuaternion (0,0,0,0), csQuaternion (0,0,0,0)); 
      csQuaternion pivot;   

      for (size_t j = 0; j < 4; ++j, ++influence) // @@SOLVE 4
      {
        if (influence->influenceWeight > 0)
        {
          numInfluences++;

          csDualQuaternion inflQuat (
            skeletonState->GetQuaternion (influence->bone),
            skeletonState->GetVector (influence->bone));

          if (numInfluences == 1)
          {
            pivot = inflQuat.real;
          }
          else if (inflQuat.real.Dot (pivot) < 0.0f)
          {
            inflQuat *= -1.0f;
          }

          dq += inflQuat * influence->influenceWeight;
        }
      }

      if (numInfluences == 0)
      {
        dstTangents[i] = *srcTangents;
        dstBinormals[i] = *srcBinormals;
      }
      else
      {
        dq = dq.Unit ();

        dstTangents[i] = dq.Transform (*srcTangents);        
        dstBinormals[i] = dq.Transform (*srcBinormals);
      }  

      ++srcTangents;
      ++srcBinormals;
    }
  }

  void AnimeshObject::SkinAll ()
  {
    if (!skeleton)
      return;

    // @@Better checks/handling...
    CS_ASSERT (skinnedVertices->GetElementCount () >= factory->vertexCount);
    CS_ASSERT (skinnedNormals->GetElementCount () >= factory->vertexCount);
    CS_ASSERT (skinnedTangents->GetElementCount () >= factory->vertexCount);
    CS_ASSERT (skinnedBinormals->GetElementCount () >= factory->vertexCount);

    // Setup some local data
    csVertexListWalker<float, csVector3> srcVerts (factory->vertexBuffer);
    csRenderBufferLock<csVector3> dstVerts (skinnedVertices);
    csVertexListWalker<float, csVector3> srcNormals (factory->normalBuffer);
    csRenderBufferLock<csVector3> dstNormals (skinnedNormals);

    csVertexListWalker<float, csVector3> srcTangents (factory->tangentBuffer);
    csRenderBufferLock<csVector3> dstTangents (skinnedTangents);
    csVertexListWalker<float, csVector3> srcBinormals (factory->binormalBuffer);
    csRenderBufferLock<csVector3> dstBinormals (skinnedBinormals);

    csSkeletalState2* skeletonState = lastSkeletonState;    

    csAnimatedMeshBoneInfluence* influence = factory->boneInfluences.GetArray ();

    for (size_t i = 0; i < factory->vertexCount; ++i)
    {
      // Accumulate data for the vertex
      int numInfluences = 0;

      csDualQuaternion dq (csQuaternion (0,0,0,0), csQuaternion (0,0,0,0)); 
      csQuaternion pivot;   

      for (size_t j = 0; j < 4; ++j, ++influence) // @@SOLVE 4
      {
        if (influence->influenceWeight > 0)
        {
          numInfluences++;

          csDualQuaternion inflQuat (
            skeletonState->GetQuaternion (influence->bone),
            skeletonState->GetVector (influence->bone));

          if (numInfluences == 1)
          {
            pivot = inflQuat.real;
          }
          else if (inflQuat.real.Dot (pivot) < 0.0f)
          {
            inflQuat *= -1.0f;
          }

          dq += inflQuat * influence->influenceWeight;
        }
      }   

      if (numInfluences == 0)
      {
        dstVerts[i] = *srcVerts;
        dstNormals[i] = *srcNormals;
        dstTangents[i] = *srcTangents;
        dstBinormals[i] = *srcBinormals;
      }
      else
      {
        dq = dq.Unit ();

        dstVerts[i] = dq.TransformPoint (*srcVerts);
        dstNormals[i] = dq.Transform (*srcNormals);
        dstTangents[i] = dq.Transform (*srcTangents);        
        dstBinormals[i] = dq.Transform (*srcBinormals);
      }

      ++srcVerts;
      ++srcNormals;
      ++srcTangents;
      ++srcBinormals;
    }
  }

}
CS_PLUGIN_NAMESPACE_END(Animesh)

