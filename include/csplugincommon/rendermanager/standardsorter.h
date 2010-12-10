/*
    Copyright (C) 2007-2008 by Marten Svanfeldt

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

/**\file
 * Standard rendermesh sorter
 */

#include "csplugincommon/rendermanager/operations.h"

#include "ivideo/rendermesh.h"
#include "csgeom/vector3.h"

namespace CS
{
namespace RenderManager
{

  /**
   * Standard rendermesh sorter.
   * Sorts mesh nodes depending on their render priority, either 
   * back2front/front2back or based on material and factories.
   *
   * Usage: with mesh node iteration.
   * SetupCameraLocation () must be called with the current camera location.
   * Sorting is typically done after visibility culling and portal
   * setup. Example:
   * \code
   * // Sort the mesh lists  
   * {
   *   StandardMeshSorter<RenderTree> mySorter (renderView->GetEngine ());
   *   mySorter.SetupCameraLocation (rview->GetCamera ()->GetTransform ().GetOrigin ());
   *   ForEachMeshNode (context, mySorter);
   * }
   * \endcode
   */
  template<typename Tree>
  class StandardMeshSorter    
  {
  public:
    StandardMeshSorter (iEngine* engine)
      : engine (engine), cameraOrigin (0,0,0)
    {
      // Build table of sorting options
      renderPriorityCount = engine->GetRenderPriorityCount ();
      sortingTypeTable = new int[renderPriorityCount];
      
      for (uint i = 0; i < renderPriorityCount; ++i)
      {       
        sortingTypeTable[i] = -1;
      }      
    }

    ~StandardMeshSorter ()
    {
      delete[] sortingTypeTable;
    }

    /// Set the camera location to be used for b2f/f2b sorting
    void SetupCameraLocation (const csVector3& vec)
    {
      cameraOrigin = vec;
    }

    /**
     * Sort given mesh node
     */
    void operator()(typename Tree::MeshNode* meshNode)
    {
      // Get the render priority for node
      CS::Graphics::RenderPriority renderPrio (meshNode->priority);
      
      int sorting = GetSorting (renderPrio);
      meshNode->sorting = sorting;
      switch (sorting)
      {
      case CS_RENDPRI_SORT_NONE:
        {
          NormalSorter sorter;
          meshNode->meshes.SortStable (sorter); // Use a stable sort to keep some b2f
        }
        break;
      case CS_RENDPRI_SORT_BACK2FRONT:
        {
	  // B2F: we want to render meshes with the larger distance first.
          DistanceSorter sorter (cameraOrigin, -1);
          meshNode->meshes.Sort (sorter);
        }
        break;
      case CS_RENDPRI_SORT_FRONT2BACK:
        {
	  // F2B: we want to render meshes with the smaller distance first.
          DistanceSorter sorter (cameraOrigin, 1);
          meshNode->meshes.Sort (sorter);
        }
        break;
      default:
        CS_ASSERT_MSG("Invalid sorting mode!", 0);
      }
    }
    

  private:

    /**
     * Sorter functor doing normal sorting
     */
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

    /**
     * Sorter function sorting based on distance between render mesh and
     * camera origin.
     * Meshes with the smaller (scaled) distance to the camera are sorted first.
     * Scale factor is used to be able to invert sorting.
     */
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
  
    csRenderPrioritySorting GetSorting (CS::Graphics::RenderPriority renderPrio)
    {
      CS_ASSERT(renderPrio.IsValid() && renderPrio < renderPriorityCount);

      if (sortingTypeTable[renderPrio] < 0)
      {
        sortingTypeTable[renderPrio] = engine->GetRenderPrioritySorting (renderPrio);
      }

      return (csRenderPrioritySorting)sortingTypeTable[renderPrio];
    }

    uint renderPriorityCount;
    int* sortingTypeTable;
    iEngine* engine;
    csVector3 cameraOrigin;
  };
  
  /// The sorter is currently non-parallel safe due to how the caching is implemented.
  template<typename RenderTree>  
  struct OperationTraits<StandardMeshSorter<RenderTree> >
  {
    typedef OperationUnordered Ordering;
  };

}
}

#endif
