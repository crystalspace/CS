/*
    Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_STANDARDSOTER_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_STANDARDSOTER_H__

#include "iengine/camera.h"
#include "ivideo/rendermesh.h"
#include "csgeom/vector3.h"

namespace CS
{
namespace RenderManager
{

  template<typename Tree>
  class StandardMeshSorter
  {
  public:
    StandardMeshSorter (iEngine* engine, iCamera* camera)
      : engine (engine)
    {
      // Build table of sorting options
      renderPriorityCount = engine->GetRenderPriorityCount ();
      sortingTypeTable = new int[renderPriorityCount];
      
      for (int i = 0; i < renderPriorityCount; ++i)
      {       
        sortingTypeTable[i] = -1;
      }

      cameraOrigin = camera->GetTransform ().GetOrigin ();
    }

    ~StandardMeshSorter ()
    {
      delete[] sortingTypeTable;
    }

    void operator()(const typename Tree::TreeTraitsType::MeshNodeKeyType& key, 
      typename Tree::MeshNode* meshNode, typename Tree::ContextNode& contextNode, Tree& tree)
    {
      // Get the render priority for node
      int renderPrio = meshNode->priority;
      
      
      switch (GetSorting (renderPrio))
      {
      case CS_RENDPRI_SORT_NONE:
        {
          NormalSorter sorter;
          meshNode->meshes.Sort (sorter);
        }
        break;
      case CS_RENDPRI_SORT_BACK2FRONT:
        {
          DistanceSorter sorter (cameraOrigin, 1);
          meshNode->meshes.Sort (sorter);
        }
        break;
      case CS_RENDPRI_SORT_FRONT2BACK:
        {
          DistanceSorter sorter (cameraOrigin, -1);
          meshNode->meshes.Sort (sorter);
        }
        break;
      default:
        CS_ASSERT_MSG("Invalid sorting mode!", 0);
      }
    }

    

  private:

    class NormalSorter
    {
    public:
      bool operator() (typename Tree::MeshNode::SingleMesh const& m1,
        typename Tree::MeshNode::SingleMesh const& m2)
      {
        const csRenderMesh* rm1 = m1.renderMesh;
        const csRenderMesh* rm2 = m2.renderMesh;

        if(rm1->material < rm2->material)
          return true;

        if((rm1->material == rm2->material) &&
          (rm1->geometryInstance < rm2->geometryInstance))
          return true;

        return false;
      }

    };

    class DistanceSorter : NormalSorter
    {
    public:
      DistanceSorter (const csVector3& cameraOrigin, const float scaleFactor)
        : cameraOrigin (cameraOrigin), scaleFactor (scaleFactor)
      {}

      bool operator() (typename Tree::MeshNode::SingleMesh const& m1,
        typename Tree::MeshNode::SingleMesh const& m2)
      {
        const csRenderMesh* rm1 = m1.renderMesh;
        const csRenderMesh* rm2 = m2.renderMesh;

        const float distSqRm1 = scaleFactor*(rm1->worldspace_origin - cameraOrigin).SquaredNorm ();
        const float distSqRm2 = scaleFactor*(rm2->worldspace_origin - cameraOrigin).SquaredNorm ();

        if (distSqRm1 < distSqRm2)
          return true;

        if (distSqRm2 < distSqRm1)
          return false;

        return NormalSorter::operator () (m1, m2);
      }


    private:
      const csVector3& cameraOrigin;
      const float scaleFactor;
    };
  
    csRenderPrioritySorting GetSorting (int renderPrio)
    {
      CS_ASSERT(renderPrio >= 0 && renderPrio < renderPriorityCount);

      if (sortingTypeTable[renderPrio] < 0)
      {
        sortingTypeTable[renderPrio] = engine->GetRenderPrioritySorting (renderPrio);
      }

      return (csRenderPrioritySorting)sortingTypeTable[renderPrio];
    }

    int renderPriorityCount;
    int* sortingTypeTable;
    iEngine* engine;
    csVector3 cameraOrigin;
  };

}
}

#endif
