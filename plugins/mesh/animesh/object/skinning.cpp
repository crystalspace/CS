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


  
  void AnimeshObject::SkinVertices (iRenderBuffer* vResultBuffer)
  {
    if (!skeleton)
      return;

    // @@Better checks/handling...
    CS_ASSERT (vResultBuffer->GetElementCount () >= factory->vertexCount);
    
    // Get the buffer to skin from 
    iRenderBuffer* srcBuffer = factory->vertexBuffer;

    // Setup some local data
    csVertexListWalker<float, csVector3> srcVerts (srcBuffer);
    csRenderBufferLock<csVector3> dstVerts (vResultBuffer);
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
      }
      else
      {
        dq = dq.Unit ();

        dstVerts[i] = dq.TransformPoint (*srcVerts);        
      }  

      ++srcVerts;      
    }
  }

}
CS_PLUGIN_NAMESPACE_END(Animesh)

